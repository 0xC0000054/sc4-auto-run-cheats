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
#include "ICheatListCommand.h"
#include "cISC4ZoneManager.h"

class PlaceZoneCheatListCommand : public ICheatListCommand
{
public:
	PlaceZoneCheatListCommand(
		cISC4ZoneManager::ZoneType zoneType,
		long x1,
		long y1,
		long x2,
		long y2);

private:
	void Execute(
		cISC4City* pCity,
		cIGZCheatCodeManager* pCheatCodeManager,
		cIGZCommandServer* pCommandServer) override;

	cISC4ZoneManager::ZoneType zoneType;
	long x1;
	long y1;
	long x2;
	long y2;
};
