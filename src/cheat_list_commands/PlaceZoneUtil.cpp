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
