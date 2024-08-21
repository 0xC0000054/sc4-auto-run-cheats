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
#include <string_view>
#include <vector>

namespace StringViewUtil
{
	bool EqualsIgnoreCase(
		const std::string_view& lhs,
		const std::string_view& rhs);

	bool StartsWithIgnoreCase(
		const std::string_view& lhs,
		const std::string_view& rhs);

	void Split(
		const std::string_view& input,
		std::string_view::value_type delim,
		std::vector<std::string_view>& results);

	std::string_view TrimWhiteSpace(const std::string_view& input);

	bool TryParse(const std::string_view& input, long& outValue);
	bool TryParse(const std::string_view& input, uint32_t& outValue);
}