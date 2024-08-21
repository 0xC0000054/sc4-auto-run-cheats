////////////////////////////////////////////////////////////////////////
//
// This file is part of sc4-auto-run-cheats, a DLL Plugin for
// SimCity 4 that automatically executes cheat codes when loading the
// game or a city.
//
// Copyright (c) 2024 Nicholas Hayes
//
// This file is licensed under terms of the MIT License.
// See LICENSE.txt for more information.
//
////////////////////////////////////////////////////////////////////////

#include "Logger.h"
#include "version.h"
#include "StringViewUtil.h"
#include "CheatListCommandFactory.h"
#include "cIGZCheatCodeManager.h"
#include "cIGZCommandParameterSet.h"
#include "cIGZCommandServer.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZMessage2Standard.h"
#include "cIGZMessageServer2.h"
#include "cISC4App.h"
#include "cISC4City.h"
#include "cRZAutoRefCount.h"
#include "cRZBaseString.h"
#include "cRZMessage2COMDirector.h"
#include "GZServPtrs.h"

#include <boost/algorithm/string.hpp>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/ini_parser.hpp"

#include <set>
#include <string>
#include <vector>

#include <Windows.h>
#include "wil/resource.h"
#include "wil/result.h"
#include "wil/win32_helpers.h"

static constexpr uint32_t kSC4MessagePostCityInitComplete = 0xEA8AE29A;
static constexpr uint32_t kSC4MessageCityEstablished = 0x26D31EC4;
static constexpr uint32_t kSC4MessagePostRegionInit = 0xCBB5BB45;

static constexpr uint32_t kAutoRunCheatsDllDirector = 0x21E2B214;

static constexpr std::string_view PluginConfigFileName = "SC4AutoRunCheats.ini";
static constexpr std::string_view PluginLogFileName = "SC4AutoRunCheats.log";

using namespace std::literals::string_view_literals; // Required for the sv suffix

namespace
{
	std::filesystem::path GetDllFolderPath()
	{
		wil::unique_cotaskmem_string modulePath = wil::GetModuleFileNameW(wil::GetModuleInstanceHandle());

		std::filesystem::path temp(modulePath.get());

		return temp.parent_path();
	}

	void ExecuteCheatList(
		const std::vector<std::unique_ptr<ICheatListCommand>>& cheats,
		cISC4City* pCity,
		cIGZCheatCodeManager* pCheatCodeManager,
		cIGZCommandServer* pCommandServer)
	{
		Logger& logger = Logger::GetInstance();

		for (const auto& command : cheats)
		{
			if (command)
			{
				command->Execute(pCity, pCheatCodeManager, pCommandServer);
			}
		}
	}

	void ParseCommandFile(const std::string& path, std::vector<std::unique_ptr<ICheatListCommand>>& output)
	{
		std::ifstream externalCommandFile(path);

		// The external command file lists each command on its own line.
		// This is intended for users that want to do things like automate the
		// network or zone placements when they establish a city.

		for (std::string line; std::getline(externalCommandFile, line);)
		{
			// Lines that start with a semicolon are comments.
			if (!line.empty() && line[0] != ';')
			{
				output.push_back(CheatListCommandFactory::Create(line));
			}
		}
	}

	void ParseCommandString(const std::string& input, std::vector<std::unique_ptr<ICheatListCommand>>& output)
	{
		if (!input.empty())
		{
			constexpr std::string_view filePrefix = "File:"sv;

			try
			{
				if (input.starts_with(filePrefix))
				{
					if (input.length() > filePrefix.size())
					{
						std::string path = input.substr(filePrefix.size());

						ParseCommandFile(path, output);
					}
				}
				else
				{
					std::vector<std::string_view> commands;

					StringViewUtil::Split(input, ',', commands);

					for (const auto& command : commands)
					{
						// Trim any leading or trailing white space from the string.
						const std::string_view trimmedCommand = StringViewUtil::TrimWhiteSpace(command);

						output.push_back(CheatListCommandFactory::Create(trimmedCommand));
					}
				}
			}
			catch (const std::exception& e)
			{
				Logger::GetInstance().WriteLine(LogLevel::Error, e.what());
			}
		}
	}
}

class AutoRunCheatsDllDirector final : public cRZMessage2COMDirector
{
public:

	AutoRunCheatsDllDirector()
	{
		std::filesystem::path dllFolderPath = GetDllFolderPath();

		configFilePath = dllFolderPath;
		configFilePath /= PluginConfigFileName;

		std::filesystem::path logFilePath = dllFolderPath;
		logFilePath /= PluginLogFileName;

		Logger& logger = Logger::GetInstance();
		logger.Init(logFilePath, LogLevel::Error);
		logger.WriteLogFileHeader("SC4AutoRunCheats v" PLUGIN_VERSION_STR);
	}

	uint32_t GetDirectorID() const
	{
		return kAutoRunCheatsDllDirector;
	}

	bool OnStart(cIGZCOM* pCOM)
	{
		cIGZFrameWork* const pFramework = pCOM->FrameWork();

		if (pFramework->GetState() < cIGZFrameWork::kStatePreAppInit)
		{
			pFramework->AddHook(this);
		}
		else
		{
			PreAppInit();
		}

		return true;
	}

private:

	void ExecuteEstablishedCityCommands(
		cISC4City* pCity,
		cIGZCheatCodeManager* pCheatCodeManager,
		cIGZCommandServer* pCommandServer)
	{
		ExecuteCheatList(
			establishedTileLoadCommands,
			pCity,
			pCheatCodeManager,
			pCommandServer);

		if (!establishedTileLoadRunOnceCommands.empty())
		{
			ExecuteCheatList(
				establishedTileLoadRunOnceCommands,
				pCity,
				pCheatCodeManager,
				pCommandServer);
			establishedTileLoadRunOnceCommands.clear();
		}
	}

	void CityEstablished(cIGZMessage2Standard* pStandardMsg)
	{
		cISC4City* pCity = static_cast<cISC4City*>(pStandardMsg->GetVoid1());

		if (pCity)
		{
			cISC4AppPtr pSC4App;

			if (pSC4App)
			{
				cIGZCheatCodeManager* pCheatCodeManager = pSC4App->GetCheatCodeManager();
				cIGZCommandServerPtr pCommandServer;

				if (pCheatCodeManager && pCommandServer)
				{
					ExecuteEstablishedCityCommands(pCity, pCheatCodeManager, pCommandServer);

					if (establishedTileLoadCommands.empty()
						&& establishedTileLoadRunOnceCommands.empty())
					{
						RemoveNotification(kSC4MessageCityEstablished);
					}
				}
			}
		}
	}

	void PostCityInitComplete(cIGZMessage2Standard* pStandardMsg)
	{
		cISC4City* pCity = static_cast<cISC4City*>(pStandardMsg->GetVoid1());

		if (pCity)
		{
			cISC4AppPtr pSC4App;

			if (pSC4App)
			{
				cIGZCheatCodeManager* pCheatCodeManager = pSC4App->GetCheatCodeManager();
				cIGZCommandServerPtr pCommandServer;

				if (pCheatCodeManager && pCommandServer)
				{
					ExecuteCheatList(
						tileLoadCommands,
						pCity,
						pCheatCodeManager,
						pCommandServer);

					if (!tileLoadRunOnceCommands.empty())
					{
						ExecuteCheatList(
							tileLoadRunOnceCommands,
							pCity,
							pCheatCodeManager,
							pCommandServer);
						tileLoadRunOnceCommands.clear();
					}

					if (pCity->GetEstablished())
					{
						ExecuteEstablishedCityCommands(pCity, pCheatCodeManager, pCommandServer);
					}
					else
					{
						ExecuteCheatList(
							unestablishedTileLoadCommands,
							pCity,
							pCheatCodeManager,
							pCommandServer);

						if (!unestablishedTileLoadRunOnceCommands.empty())
						{
							ExecuteCheatList(
								unestablishedTileLoadRunOnceCommands,
								pCity,
								pCheatCodeManager,
								pCommandServer);
							unestablishedTileLoadRunOnceCommands.clear();
						}
					}

					if (tileLoadCommands.empty()
						&& tileLoadRunOnceCommands.empty()
						&& establishedTileLoadCommands.empty()
						&& establishedTileLoadRunOnceCommands.empty()
						&& unestablishedTileLoadCommands.empty()
						&& unestablishedTileLoadRunOnceCommands.empty())
					{
						RemoveNotification(kSC4MessagePostCityInitComplete);
					}
				}
			}
		}
	}

	void PostRegionInit()
	{
		cISC4AppPtr pSC4App;

		if (pSC4App)
		{
			cIGZCheatCodeManager* pCheatCodeManager = pSC4App->GetCheatCodeManager();
			cIGZCommandServerPtr pCommandServer;

			if (pCheatCodeManager && pCommandServer)
			{
				ExecuteCheatList(appStartupCommands, nullptr, pCheatCodeManager, pCommandServer);

				// The application startup commands are only run once when
				// the first region is loaded.
				RemoveNotification(kSC4MessagePostRegionInit);
			}
		}
	}

	bool DoMessage(cIGZMessage2* pMsg)
	{
		cIGZMessage2Standard* pStandardMsg = static_cast<cIGZMessage2Standard*>(pMsg);

		switch (pStandardMsg->GetType())
		{
		case kSC4MessageCityEstablished:
			CityEstablished(pStandardMsg);
			break;
		case kSC4MessagePostCityInitComplete:
			PostCityInitComplete(pStandardMsg);
			break;
		case kSC4MessagePostRegionInit:
			PostRegionInit();
			break;
		}

		return true;
	}


	bool PostAppInit()
	{
		Logger& logger = Logger::GetInstance();

		if (LoadCheatCodes())
		{
			cIGZMessageServer2Ptr pMS2;

			if (pMS2)
			{
				std::set<uint32_t> requiredNotifications;

				if (!appStartupCommands.empty())
				{
					requiredNotifications.emplace(kSC4MessagePostRegionInit);
				}

				if (!tileLoadCommands.empty()
					|| !tileLoadRunOnceCommands.empty())
				{
					requiredNotifications.emplace(kSC4MessagePostCityInitComplete);
				}

				if (!establishedTileLoadCommands.empty()
					|| !establishedTileLoadRunOnceCommands.empty())
				{
					requiredNotifications.emplace(kSC4MessageCityEstablished);
					requiredNotifications.emplace(kSC4MessagePostCityInitComplete);
				}

				if (!unestablishedTileLoadCommands.empty()
					|| !unestablishedTileLoadRunOnceCommands.empty())
				{
					requiredNotifications.emplace(kSC4MessagePostCityInitComplete);
				}

				for (uint32_t messageID : requiredNotifications)
				{
					if (!pMS2->AddNotification(this, messageID))
					{
						logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
						return false;
					}
				}
			}
			else
			{
				logger.WriteLine(LogLevel::Error, "Failed to subscribe to the required notifications.");
				return false;
			}
		}

		return true;
	}


	bool LoadCheatCodes()
	{
		bool result = false;

		try
		{
			std::ifstream stream(configFilePath, std::ifstream::in);

			if (!stream)
			{
				throw std::runtime_error("Failed to open the settings file.");
			}

			boost::property_tree::ptree tree;

			boost::property_tree::ini_parser::read_ini(stream, tree);

			ParseCommandString(tree.get<std::string>("Startup.CommandList"), appStartupCommands);
			ParseCommandString(tree.get<std::string>("Tile.CommandList"), tileLoadCommands);
			ParseCommandString(tree.get<std::string>("Tile.RunOnceCommandList"), tileLoadRunOnceCommands);
			ParseCommandString(tree.get<std::string>("EstablishedTile.CommandList"), establishedTileLoadCommands);
			ParseCommandString(tree.get<std::string>("EstablishedTile.RunOnceCommandList"), establishedTileLoadRunOnceCommands);
			ParseCommandString(tree.get<std::string>("UnestablishedTile.CommandList"), unestablishedTileLoadCommands);
			ParseCommandString(tree.get<std::string>("UnestablishedTile.RunOnceCommandList"), unestablishedTileLoadRunOnceCommands);
			result = true;
		}
		catch (const std::exception& e)
		{
			Logger::GetInstance().WriteLineFormatted(
				LogLevel::Error,
				"Failed to read the settings file: %s",
				e.what());
		}

		return result;
	}

	void RemoveNotification(uint32_t messageID)
	{
		cIGZMessageServer2Ptr pMS2;

		if (pMS2)
		{
			pMS2->RemoveNotification(this, messageID);
		}
	}

	std::filesystem::path configFilePath;
	std::vector<std::unique_ptr<ICheatListCommand>> appStartupCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> tileLoadCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> tileLoadRunOnceCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> establishedTileLoadCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> establishedTileLoadRunOnceCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> unestablishedTileLoadCommands;
	std::vector<std::unique_ptr<ICheatListCommand>> unestablishedTileLoadRunOnceCommands;
};

cRZCOMDllDirector* RZGetCOMDllDirector() {
	static AutoRunCheatsDllDirector sDirector;
	return &sDirector;
}