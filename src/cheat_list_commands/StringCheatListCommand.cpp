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

#include "StringCheatListCommand.h"
#include "cIGZCheatCodeManager.h"
#include "cIGZCommandParameterSet.h"
#include "cIGZCommandServer.h"
#include "cRZAutoRefCount.h"
#include "Logger.h"

StringCheatListCommand::StringCheatListCommand(const std::string_view& view)
	: command(view.data(), view.size())
{
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
			Logger::GetInstance().WriteLineFormatted(
				LogLevel::Error,
				"Unknown cheat or command: %s",
				command.ToChar());
		}
	}
}
