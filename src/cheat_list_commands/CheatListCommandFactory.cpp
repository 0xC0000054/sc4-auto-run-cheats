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

#include "CheatListCommandFactory.h"
#include "Logger.h"
#include "PlaceZoneCheatListCommand.h"
#include "StringCheatListCommand.h"
#include "ZoneBitmapCheatListCommand.h"
#include "StringViewUtil.h"
#include <cstdarg>
#include <stdexcept>

using namespace std::string_view_literals;

static constexpr std::string_view PlaceZoneStringView = "PlaceZone"sv;
static constexpr std::string_view ZoneBitmapStringView = "ZoneBitmap"sv;

namespace
{
	std::unique_ptr<ICheatListCommand> CreatePlaceZoneCommand(const std::string_view& view)
	{
		Logger& logger = Logger::GetInstance();

		std::vector<std::string_view> arguments;
		arguments.reserve(6);

		StringViewUtil::Split(view, ' ', arguments);

		if (arguments.size() == 6)
		{
			cISC4ZoneManager::ZoneType zoneType = cISC4ZoneManager::ZoneType::None;
			uint32_t zoneTypeInteger = 0;

			if (StringViewUtil::TryParse(arguments[1], zoneTypeInteger))
			{
				if (zoneTypeInteger > static_cast<uint32_t>(cISC4ZoneManager::ZoneType::Plopped))
				{
					throw std::runtime_error("Invalid Zone type for the PlaceZone command.");
				}

				zoneType = static_cast<cISC4ZoneManager::ZoneType>(zoneTypeInteger);
			}
			else
			{
				throw std::runtime_error("Failed to parse the first PlaceZone argument (zone type).");
			}

			long x1 = 0;

			if (!StringViewUtil::TryParse(arguments[2], x1))
			{
				throw std::runtime_error("Failed to parse the second PlaceZone argument (x1).");
			}

			long y1 = 0;

			if (!StringViewUtil::TryParse(arguments[3], y1))
			{
				throw std::runtime_error("Failed to parse the third PlaceZone argument (y1).");
			}

			long x2 = 0;

			if (!StringViewUtil::TryParse(arguments[4], x2))
			{
				throw std::runtime_error("Failed to parse the fourth PlaceZone argument (x2).");
			}

			long y2 = 0;

			if (!StringViewUtil::TryParse(arguments[5], y2))
			{
				throw std::runtime_error("Failed to parse the fifth PlaceZone argument (y2).");
			}

			return std::make_unique<PlaceZoneCheatListCommand>(zoneType, x1, y1, x2, y2);
		}
		else
		{
			throw std::runtime_error("PlaceZone must have 5 arguments.");
		}
	}

	std::unique_ptr<ICheatListCommand> CreateZoneBitmapCommand(const std::string_view& view)
	{
		// The command format is: ZoneBitmap <path>

		std::string_view bitmapPath;

		if (view.size() > (ZoneBitmapStringView.size() + 1))
		{
			bitmapPath = view.substr(ZoneBitmapStringView.size() + 1);
		}

		if (bitmapPath.empty())
		{
			throw std::runtime_error("ZoneBitmap must use the format: ZoneBitmap <path>");
		}

		return std::make_unique<ZoneBitmapCheatListCommand>(bitmapPath);
	}
}

std::unique_ptr<ICheatListCommand> CheatListCommandFactory::Create(const std::string_view& view)
{
	if (StringViewUtil::StartsWithIgnoreCase(view, PlaceZoneStringView))
	{
		return CreatePlaceZoneCommand(view);
	}
	else if (StringViewUtil::StartsWithIgnoreCase(view, ZoneBitmapStringView))
	{
		return CreateZoneBitmapCommand(view);
	}

	return std::make_unique<StringCheatListCommand>(view);
}
