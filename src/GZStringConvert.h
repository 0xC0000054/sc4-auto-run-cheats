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

#pragma once
#include "cRZBaseString.h"
#include <filesystem>

// Provides utility functions for cIGZString character set conversion.
// The native cIGZString character set is UTF-8.
namespace GZStringConvert
{
	cRZBaseString FromUtf16(const std::wstring& str);

	cRZBaseString FromFileSystemPath(const std::filesystem::path& path);

	std::wstring ToUtf16(const cIGZString& str);

	std::filesystem::path ToFileSystemPath(const cIGZString& str);
}