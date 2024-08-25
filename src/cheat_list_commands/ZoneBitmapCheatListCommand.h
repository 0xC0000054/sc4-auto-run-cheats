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
#include "cRZBaseString.h"
#include "ZoneBitmapReader.h"
#include <string>

class ZoneBitmapCheatListCommand : public ICheatListCommand
{
public:
	ZoneBitmapCheatListCommand(const std::string_view& path);

	std::string GetCommandDescription() const override;

	void Execute(
		cISC4City* pCity,
		cIGZCheatCodeManager* pCheatCodeManager,
		cIGZCommandServer* pCommandServer) override;

private:
	void WriteZoneManagerColorTextFile() const;

	const cRZBaseString bitmapPath;
	ZoneInfo zoneInfo;
	bool bitmapRead;
};

