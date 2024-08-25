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

#include "ZoneBitmapCheatListCommand.h"
#include "GZStringConvert.h"
#include "PlaceZoneUtil.h"
#include "cGZPersistResourceKey.h"
#include "cIGZPersistResourceManager.h"
#include "cIGZVariant.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cISC4City.h"
#include "cISC4ZoneDeveloper.h"
#include "cISC4ZoneManager.h"
#include "cRZAutoRefCount.h"
#include "GZServPtrs.h"
#include "Logger.h"
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>

class AutoZoneOptions
{
public:
	AutoZoneOptions(cISC4ZoneDeveloper* pZoneDeveloper, ZoneCreationOptions options)
		: pZoneDeveloper(pZoneDeveloper)
	{
		if (pZoneDeveloper)
		{
			bool alternateOrientation = (options & ZoneCreationOptions::AlternateOrientation) != ZoneCreationOptions::None;
			bool placeStreets = (options & ZoneCreationOptions::PlaceStreets) != ZoneCreationOptions::None;
			bool customZoneSize = (options & ZoneCreationOptions::CustomSize) != ZoneCreationOptions::None;

			pZoneDeveloper->SetOptions(alternateOrientation, placeStreets, customZoneSize);
		}
	}

	~AutoZoneOptions()
	{
		if (pZoneDeveloper)
		{
			pZoneDeveloper->SetOptions(false, true, false);
			pZoneDeveloper = nullptr;
		}
	}

private:
	cISC4ZoneDeveloper* pZoneDeveloper;
};

namespace
{
	std::filesystem::path GetZoneColorTextFilePath(const cRZBaseString& bitmapPath)
	{
		std::filesystem::path path = GZStringConvert::ToFileSystemPath(bitmapPath);

		// Remove the file extension and append our text file name.
		// We keep the image file name in case the user has multiple
		// zone images in the same folder.

		path.replace_extension();
		path += L" Zone Drag Colors.txt";

		return path;
	}
}

ZoneBitmapCheatListCommand::ZoneBitmapCheatListCommand(const std::string_view& path)
	: bitmapPath(path),
	  zoneInfo(),
	  bitmapRead(false)
{
}

std::string ZoneBitmapCheatListCommand::GetCommandDescription() const
{
	return std::string("ZoneBitmap ").append(bitmapPath.ToChar(), bitmapPath.Strlen());
}

void ZoneBitmapCheatListCommand::Execute(
	cISC4City* pCity,
	cIGZCheatCodeManager* pCheatCodeManager,
	cIGZCommandServer* pCommandServer)
{
	if (!bitmapRead)
	{
		zoneInfo = ZoneBitmapReader::Read(bitmapPath);
		bitmapRead = true;
		WriteZoneManagerColorTextFile();
	}

	if (pCity)
	{
		cISC4ZoneDeveloper* pZoneDeveloper = pCity->GetZoneDeveloper();
		cISC4ZoneManager* pZoneManager = pCity->GetZoneManager();

		if (pZoneDeveloper && pZoneManager)
		{
			AutoZoneOptions autoZoneOptions(pZoneDeveloper, zoneInfo.creationOptions);

			for (const auto& entry : zoneInfo.zones)
			{
				SC4CellRegion cellRegion(entry.second.region);

				int32_t errorCode = 0;

				bool result = pZoneManager->PlaceZone(
					cellRegion,
					entry.first,
					true,
					false,
					false,
					true,
					false,
					nullptr,
					&errorCode,
					0);

				if (!result)
				{
					throw std::runtime_error(PlaceZoneUtil::GetErrorCodeDescription(errorCode));
				}
			}
		}
	}
}

void ZoneBitmapCheatListCommand::WriteZoneManagerColorTextFile() const
{
	static constexpr std::array<uint32_t, 16> MaxisDefaultZoneColors =
	{
		// All colors are specified in 0xRRGGBBAA
		0xFF0000FF, // Dezone
		0x00C400FF, // Res. Low Density
		0x009A00FF, // Res. Medium Density
		0x007200FF, // Res. High Density
		0x4F77FFFF, // Com. Low Density
		0x2053FFFF, // Com. Medium Density
		0x1420E2FF, // Com. High Density
		0xFFDB32FF, // Ind. Low Density  (Resource/Agriculture/Farm)
		0xFFB233FF, // Ind. Medium Density
		0xCE901FFF, // Ind. High Density
		0x00000000, // Military
		0x00000000, // Airport
		0x00000000, // Seaport
		0x00000000, // Spaceport
		0x00000000, // Landfill
		0x00000000  // Plopped
	};

	Logger& logger = Logger::GetInstance();

	try
	{
		const std::filesystem::path textFilePath = GetZoneColorTextFilePath(bitmapPath);

		std::ofstream file(textFilePath, std::ofstream::out | std::ofstream::trunc);

		if (file)
		{
			file << "-----------------------------------\n";
			file << "Zone Manager Zone Drag Colors\n";
			file << "-----------------------------------\n";
			file << '\n';
			file << "If you want the Zone colors in the game to match those you used in your network layout BMP, download:\n";
			file << "https://community.simtropolis.com/files/file/33590-scoty-zoning-mod/ and use the following two files:\n";
			file << '\n';
			file << "cori_ZM_tex_img2zones.dat\n";
			file << "cori_ZManag_img2zones.dat\n";
			file << '\n';
			file << "Then in the second file, replace the \"Values as text\" in Zone Drag Color with the following:\n";

			constexpr size_t lastItemIndex = MaxisDefaultZoneColors.size() - 1;

			for (size_t i = 0; i < MaxisDefaultZoneColors.size(); i++)
			{
				const auto entry = zoneInfo.zones.find(static_cast<cISC4ZoneManager::ZoneType>(i));

				const uint32_t value = entry != zoneInfo.zones.end() ? entry->second.zoneColorRGBA : MaxisDefaultZoneColors[i];

				file << std::format("0x{0:08X}", value);
				file << (i < lastItemIndex ? ',' : '\n');
			}
		}
		else
		{
			logger.WriteLineFormatted(
				LogLevel::Error,
				"Failed to open the zone drag colors file: %s",
				textFilePath.string().c_str());
		}
	}
	catch (const std::exception& e)
	{
		logger.WriteLineFormatted(
			LogLevel::Error,
			"An error occurred when writing the zone drag colors file: %s",
			e.what());
	}
}
