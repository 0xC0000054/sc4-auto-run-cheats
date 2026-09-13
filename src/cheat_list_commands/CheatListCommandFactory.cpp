/*
 * This file is part of sc4-auto-run-cheats, a DLL Plugin for SimCity 4
 * that automatically executes cheat codes when loading the game or a city.
 *
 * Copyright (C) 2024, 2026 Nicholas Hayes
 *
 * sc4-auto-run-cheats is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * sc4-auto-run-cheats is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with sc4-auto-run-cheats.
 * If not, see <http://www.gnu.org/licenses/>.
 */

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

			int32_t x1 = 0;

			if (!StringViewUtil::TryParse(arguments[2], x1))
			{
				throw std::runtime_error("Failed to parse the second PlaceZone argument (x1).");
			}

			int32_t y1 = 0;

			if (!StringViewUtil::TryParse(arguments[3], y1))
			{
				throw std::runtime_error("Failed to parse the third PlaceZone argument (y1).");
			}

			int32_t x2 = 0;

			if (!StringViewUtil::TryParse(arguments[4], x2))
			{
				throw std::runtime_error("Failed to parse the fourth PlaceZone argument (x2).");
			}

			int32_t y2 = 0;

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
		// We strip the command name and the separator space to get the file path.
		// Leading and trailing quotes are removed because the OS can't handle quoted paths.

		std::string_view bitmapPath;

		constexpr size_t pathStartIndex = ZoneBitmapStringView.size() + 1;

		if (view.size() > pathStartIndex)
		{
			const std::string_view pathWithoutCommandName = view.substr(pathStartIndex);

			bitmapPath = StringViewUtil::Trim(pathWithoutCommandName, [](char a) { return a != '"'; });
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
