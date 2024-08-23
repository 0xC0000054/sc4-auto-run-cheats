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
#include "cRZBaseString.h"

class StringCheatListCommand final : public ICheatListCommand
{
public:
	StringCheatListCommand(const std::string_view& view);

private:
	std::string GetCommandDescription() const override;

	void Execute(
		cISC4City* pCity,
		cIGZCheatCodeManager* pCheatCodeManager,
		cIGZCommandServer* pCommandServer) override;

	cRZBaseString command;
};

