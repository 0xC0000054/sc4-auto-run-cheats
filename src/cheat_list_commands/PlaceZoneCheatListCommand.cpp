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

#include "PlaceZoneCheatListCommand.h"
#include "cISC4City.h"
#include "cISC4ZoneManager.h"
#include "Logger.h"
#include "SC4CellRegion.h"

namespace
{
	std::string GetErrorCodeString(int32_t errorCode)
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
}

PlaceZoneCheatListCommand::PlaceZoneCheatListCommand(
	cISC4ZoneManager::ZoneType zoneType,
	int32_t x1,
	int32_t y1,
	int32_t x2,
	int32_t y2)
	: zoneType(zoneType),
	  x1(x1),
	  y1(y1),
	  x2(x2),
	  y2(y2)
{
}

void PlaceZoneCheatListCommand::Execute(
	cISC4City* pCity,
	cIGZCheatCodeManager* pCheatCodeManager,
	cIGZCommandServer* pCommandServer)
{
	if (pCity)
	{
		cISC4ZoneManager* pZoneManager = pCity->GetZoneManager();

		if (pZoneManager)
		{
			SC4CellRegion cellRegion(x1, y1, x2, y2, true);

			int32_t errorCode = 0;

			bool result = pZoneManager->PlaceZone(
				cellRegion,
				zoneType,
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
				Logger::GetInstance().WriteLineFormatted(
					LogLevel::Error,
					"PlaceZone %u %d %d %d %d failed: %s",
					static_cast<uint32_t>(zoneType),
					x1,
					y1,
					x2,
					y2,
					GetErrorCodeString(errorCode).c_str());
			}
		}
	}
}
