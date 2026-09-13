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

#include "StringCheatListCommand.h"
#include "cIGZCheatCodeManager.h"
#include "cIGZCommandParameterSet.h"
#include "cIGZCommandServer.h"
#include "cRZAutoRefCount.h"
#include <stdexcept>

StringCheatListCommand::StringCheatListCommand(const std::string_view& view)
	: command(view)
{
}

std::string StringCheatListCommand::GetCommandDescription() const
{
	return std::string(command.ToChar(), command.Strlen());
}

void StringCheatListCommand::Execute(
	cISC4City* pCity,
	cIGZCheatCodeManager* pCheatCodeManager,
	cIGZCommandServer* pCommandServer)
{
	uint32_t id = 0;

	if (pCheatCodeManager->DoesCheatCodeMatch(command, id))
	{
		pCheatCodeManager->SendCheatNotifications(command, id);
	}
	else
	{
		cIGZCommandParameterSet* inputParameters = nullptr;

		if (pCommandServer->ConvertStringToCommand(
			command.ToChar(),
			command.Strlen(),
			id,
			inputParameters))
		{
			cRZAutoRefCount<cIGZCommandParameterSet> outputParameters;

			if (pCommandServer->CreateCommandParameterSet(outputParameters.AsPPObj()))
			{
				pCommandServer->ExecuteCommand(id, inputParameters, outputParameters);
			}

			if (inputParameters)
			{
				inputParameters->Release();
			}
		}
		else
		{
			throw std::runtime_error("Unknown cheat or command.");
		}
	}
}
