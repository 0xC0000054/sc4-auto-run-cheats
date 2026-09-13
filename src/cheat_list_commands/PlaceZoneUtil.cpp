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

#include "PlaceZoneUtil.h"

std::string PlaceZoneUtil::GetErrorCodeDescription(int32_t errorCode)
{
	switch (errorCode)
	{
	case 1:
		return std::string("Insufficient funds");
	case 2:
		return std::string("Can't place zone on water.");
	case 3:
		return std::string("Zone size too small.");
	case 4:
		return std::string("Zone size too large.");
	case 5:
		return std::string("Can't replace existing objects.");
	default:
		return std::string("ErrorCode=").append(std::to_string(errorCode));
	}
}
