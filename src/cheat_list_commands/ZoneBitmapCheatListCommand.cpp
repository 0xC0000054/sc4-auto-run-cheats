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

#include "ZoneBitmapCheatListCommand.h"
#include "GZStringConvert.h"
#include "PlaceZoneUtil.h"
#include "cGZPersistResourceKey.h"
#include "cIGZPersistResourceManager.h"
#include "cIGZVariant.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cISC4City.h"
#include "cISC4ZoneDeveloper.h"
#include "cISC4ZoneManager.h"
#include "cRZAutoRefCount.h"
#include "GZServPtrs.h"
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>

class AutoZoneOptions
{
public:
	AutoZoneOptions(cISC4ZoneDeveloper* pZoneDeveloper, ZoneCreationOptions options)
		: pZoneDeveloper(pZoneDeveloper)
	{
		if (pZoneDeveloper)
		{
			bool alternateOrientation = (options & ZoneCreationOptions::AlternateOrientation) != ZoneCreationOptions::None;
			bool placeStreets = (options & ZoneCreationOptions::PlaceStreets) != ZoneCreationOptions::None;
			bool customZoneSize = (options & ZoneCreationOptions::CustomSize) != ZoneCreationOptions::None;

			pZoneDeveloper->SetOptions(alternateOrientation, placeStreets, customZoneSize);
		}
	}

	~AutoZoneOptions()
	{
		if (pZoneDeveloper)
		{
			pZoneDeveloper->SetOptions(false, true, false);
			pZoneDeveloper = nullptr;
		}
	}

private:
	cISC4ZoneDeveloper* pZoneDeveloper;
};

namespace
{
	std::filesystem::path GetZoneColorTextFilePath(const cRZBaseString& bitmapPath)
	{
		std::filesystem::path path = GZStringConvert::ToFileSystemPath(bitmapPath);

		// Remove the file extension and append our text file name.
		// We keep the image file name in case the user has multiple
		// zone images in the same folder.

		path.replace_extension();
		path += L" Zone Drag Colors.txt";

		return path;
	}

	bool GetZoneManagerDefaultZoneColors(std::array<uint32_t, 16>& defaultZoneColors)
	{
		bool result = false;

		cIGZPersistResourceManagerPtr pResourceManager;

		if (pResourceManager)
		{
			// The TGI of the zone manager exemplar.
			cGZPersistResourceKey key(0x6534284A, 0xE7E2C2DB, 0xE9482490);

			cRZAutoRefCount<cISCPropertyHolder> propertyHolder;

			if (pResourceManager->GetResource(
				key,
				GZIID_cISCPropertyHolder,
				propertyHolder.AsPPVoid(),
				0,
				nullptr))
			{
				constexpr uint32_t kZoneDragColor = 0xE94825B8;

				cISCProperty* pProperty = propertyHolder->GetProperty(kZoneDragColor);

				if (pProperty)
				{
					cIGZVariant* pVariant = pProperty->GetPropertyValue();

					if (pVariant
						&& pVariant->GetType() == cIGZVariant::Uint32Array
						&& pVariant->GetCount() == defaultZoneColors.size())
					{
						const uint32_t* pZoneColors = pVariant->RefUint32();

						for (size_t i = 0; i < defaultZoneColors.size(); i++)
						{
							defaultZoneColors[i] = pZoneColors[i];
						}
						result = true;
					}
				}
			}
		}

		return result;
	}
}

ZoneBitmapCheatListCommand::ZoneBitmapCheatListCommand(const std::string_view& path)
	: bitmapPath(path),
	  zoneInfo(),
	  bitmapRead(false)
{
}

std::string ZoneBitmapCheatListCommand::GetCommandDescription() const
{
	return std::string("ZoneBitmap ").append(bitmapPath.ToChar(), bitmapPath.Strlen());
}

void ZoneBitmapCheatListCommand::Execute(
	cISC4City* pCity,
	cIGZCheatCodeManager* pCheatCodeManager,
	cIGZCommandServer* pCommandServer)
{
	if (!bitmapRead)
	{
		zoneInfo = ZoneBitmapReader::Read(bitmapPath);
		bitmapRead = true;
		WriteZoneManagerColorTextFile();
	}

	if (pCity)
	{
		cISC4ZoneDeveloper* pZoneDeveloper = pCity->GetZoneDeveloper();
		cISC4ZoneManager* pZoneManager = pCity->GetZoneManager();

		if (pZoneDeveloper && pZoneManager)
		{
			AutoZoneOptions autoZoneOptions(pZoneDeveloper, zoneInfo.creationOptions);

			for (const auto& entry : zoneInfo.zones)
			{
				SC4CellRegion cellRegion(entry.second.region);

				int32_t errorCode = 0;

				bool result = pZoneManager->PlaceZone(
					cellRegion,
					entry.first,
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
					throw std::runtime_error(PlaceZoneUtil::GetErrorCodeDescription(errorCode));
				}
			}
		}
	}
}

void ZoneBitmapCheatListCommand::WriteZoneManagerColorTextFile() const
{
	const std::filesystem::path textFilePath = GetZoneColorTextFilePath(bitmapPath);
	std::array<uint32_t, 16> defaultZoneColors{};

	if (GetZoneManagerDefaultZoneColors(defaultZoneColors))
	{
		std::ofstream file(textFilePath, std::ofstream::out | std::ofstream::trunc);

		if (file)
		{
			file << "-----------------------------------\n";
			file << "Zone Manager Zone Drag Colors\n";
			file << "-----------------------------------\n";
			file << '\n';
			file << "If you want the Zone colors in the game to match those you used in your network layout BMP, download:\n";
			file << "https://community.simtropolis.com/files/file/33590-scoty-zoning-mod/ and use the following two files:\n";
			file << '\n';
			file << "cori_ZM_tex_img2zones.dat\n";
			file << "cori_ZManag_img2zones.dat\n";
			file << '\n';
			file << "Then in the second file, replace the \"Values as text\" in Zone Drag Color with the following:\n";

			constexpr size_t lastItemIndex = defaultZoneColors.size() - 1;

			for (size_t i = 0; i < defaultZoneColors.size(); i++)
			{
				const auto entry = zoneInfo.zones.find(static_cast<cISC4ZoneManager::ZoneType>(i));

				const uint32_t value = entry != zoneInfo.zones.end() ? entry->second.zoneColorRGBA : defaultZoneColors[i];

				file << std::format("0x{0:08X}", value);
				file << (i < lastItemIndex ? ',' : '\n');
			}
		}
	}
}
