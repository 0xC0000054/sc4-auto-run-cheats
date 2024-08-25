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

#pragma once
#include "cISC4ZoneManager.h"
#include "cRZBaseString.h"
#include "SC4CellRegion.h"
#include <map>
#include <string>

enum class ZoneCreationOptions : uint32_t
{
	None = 0,
	// Use a custom zone size.
	// This is equivalent th holding down the Control key with the zoning tool.
	CustomSize = 1,
	// Use an alternate zone layout.
	// This is equivalent th holding down the Alt key with the zoning tool.
	AlternateOrientation = 2,
	// Place streets within the zone.
	// This is equivalent the Shift key being up with the zoning tool.
	// We default to not placing streets because most users will be placing zones as markers for
	// other things such as where the transportation networks should be placed in a city tile.
	PlaceStreets = 4
};

inline constexpr ZoneCreationOptions operator|(ZoneCreationOptions lhs, ZoneCreationOptions rhs)
{
	return static_cast<ZoneCreationOptions>(static_cast<std::underlying_type<ZoneCreationOptions>::type>(lhs)
											| static_cast<std::underlying_type<ZoneCreationOptions>::type>(rhs));
}

inline ZoneCreationOptions operator|=(ZoneCreationOptions& lhs, ZoneCreationOptions rhs)
{
	return reinterpret_cast<ZoneCreationOptions&>(reinterpret_cast<std::underlying_type<ZoneCreationOptions>::type&>(lhs)
												|= static_cast<std::underlying_type<ZoneCreationOptions>::type>(rhs));
}

inline constexpr ZoneCreationOptions operator&(ZoneCreationOptions lhs, ZoneCreationOptions rhs)
{
	return static_cast<ZoneCreationOptions>(static_cast<std::underlying_type<ZoneCreationOptions>::type>(lhs)
											& static_cast<std::underlying_type<ZoneCreationOptions>::type>(rhs));
}

inline ZoneCreationOptions operator&=(ZoneCreationOptions& lhs, ZoneCreationOptions rhs)
{
	return reinterpret_cast<ZoneCreationOptions&>(reinterpret_cast<std::underlying_type<ZoneCreationOptions>::type&>(lhs)
												&= static_cast<std::underlying_type<ZoneCreationOptions>::type>(rhs));
}

struct ZoneEntry
{
	SC4CellRegion<int32_t> region;
	uint32_t zoneColorRGBA;

	ZoneEntry(int32_t width, int32_t height, bool initialValue, uint32_t zoneColorRGBA)
		: region(width, height, initialValue),
		  zoneColorRGBA(zoneColorRGBA)
	{
	}
};

struct ZoneInfo
{
	ZoneCreationOptions creationOptions;
	std::map<cISC4ZoneManager::ZoneType, ZoneEntry> zones;

	ZoneInfo() : ZoneInfo(ZoneCreationOptions::None)
	{
	}

	ZoneInfo(ZoneCreationOptions creationOptions)
		: creationOptions(creationOptions),
		  zones()
	{
	}
};

namespace ZoneBitmapReader
{
	ZoneInfo Read(const cRZBaseString& path);
}