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

#include "ZoneBitmapReader.h"
#include "cGZPersistResourceKey.h"
#include "cIGZBuffer.h"
#include "cIGZCOM.h"
#include "cIGZFrameWork.h"
#include "cIGZGimexFactory.h"
#include "cIGZVariant.h"
#include "cIGZPersistResourceManager.h"
#include "cISCProperty.h"
#include "cISCPropertyHolder.h"
#include "cRZAutoRefCount.h"
#include "cRZCOMDllDirector.h"
#include "GZServPtrs.h"
#include <functional>
#include <stdexcept>
#include <unordered_map>

static constexpr int32_t SmallTileSize = 64;  // 64x64 cells.
static constexpr int32_t MediumTileSize = 128; // 128x128 cells.
static constexpr int32_t LargeTileSize = 256; // 256x256 cells.

struct ColorBgrx32
{
	uint32_t bgrx;

	ColorBgrx32(): ColorBgrx32(0, 0, 0)
	{
	}

	ColorBgrx32(uint8_t b, uint8_t g, uint8_t r)
		: bgrx(0x00000000 | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b))
	{
	}

	uint32_t ToRGBA() const
	{
		// Convert from 0xXXRRGGBB to 0xRRGGBBAA.
		// Shift the red, green, and blue values to the right by 8 bits and add an opaque alpha value.

		return ((bgrx & 0x00FFFFFF) << 8) | 0x000000FF;
	}

	bool operator ==(const ColorBgrx32& other) const
	{
		// Mask the values so that only the colors are compared.
		return (bgrx & 0x00FFFFFF) == (other.bgrx & 0x00FFFFFF);
	}

	bool operator !=(const ColorBgrx32& other) const
	{
		return !operator==(other);
	}

	bool operator<(const ColorBgrx32& other) const
	{
		// Mask the values so that only the colors are compared.
		return (bgrx & 0x00FFFFFF) < (other.bgrx & 0x00FFFFFF);
	}

	bool operator<=(const ColorBgrx32& other) const
	{
		// Mask the values so that only the colors are compared.
		return (bgrx & 0x00FFFFFF) <= (other.bgrx & 0x00FFFFFF);
	}

	bool operator>(const ColorBgrx32& other) const
	{
		// Mask the values so that only the colors are compared.
		return (bgrx & 0x00FFFFFF) > (other.bgrx & 0x00FFFFFF);
	}

	bool operator>=(const ColorBgrx32& other) const
	{
		// Mask the values so that only the colors are compared.
		return (bgrx & 0x00FFFFFF) >= (other.bgrx & 0x00FFFFFF);
	}
};

template<>
struct std::hash<ColorBgrx32>
{
	std::size_t operator()(const ColorBgrx32& c) const noexcept
	{
		// Mask the value so that only the colors are included.
		return std::hash<uint32_t>{}(c.bgrx & 0x00FFFFFF);
	}
};

namespace
{
	void LoadZoneColorInfoFromImage(
		const uint8_t* pScan0,
		uint32_t stride,
		int32_t height,
		std::unordered_map<ColorBgrx32, cISC4ZoneManager::ZoneType>& zoneColors,
		ZoneCreationOptions& creationOptions)
	{
		// The last row of the bitmap contains 16 pixels that indicate the RGB colors that correspond
		// to the game's 16 zone types and control values for the zone creation options.

		const ColorBgrx32* pRow = reinterpret_cast<const ColorBgrx32*>(pScan0 + (static_cast<size_t>(height - 1) * stride));

		for (size_t i = 0; i < 16; i++)
		{
			zoneColors.emplace(pRow[i], static_cast<cISC4ZoneManager::ZoneType>(i));
		}

		// We use RGB 255,165,0 as the active/enabled color for the control options.
		// This is intended to avoid any issues that may be caused by image editing software filling
		// the row with black or white as a default color.

		const ColorBgrx32 activeColor(0, 165, 255);

		if (pRow[16] == activeColor)
		{
			creationOptions |= ZoneCreationOptions::CustomSize;
		}

		if (pRow[17] == activeColor)
		{
			creationOptions |= ZoneCreationOptions::AlternateOrientation;
		}

		if (pRow[18] == activeColor)
		{
			creationOptions |= ZoneCreationOptions::PlaceStreets;
		}
	}
}

ZoneInfo ZoneBitmapReader::Read(const cRZBaseString& path)
{
	cIGZCOM* const pCOM = RZGetFrameWork()->GetCOMObject();

	cRZAutoRefCount<cIGZGimexFactory> pGimexFactory;

	if (!pCOM->GetClassObject(GZCLSID_cIGZGimexFactory, GZIID_cIGZGimexFactory, pGimexFactory.AsPPVoid()))
	{
		throw std::runtime_error("Failed to get the Gimex factory.");
	}

	cRZAutoRefCount<cIGZBuffer> pImage;

	// Load the image into a 32-bit BGRA buffer, SC4 doesn't have a 24-bit buffer format.

	if (!pGimexFactory->LoadFromFile(path, pImage.AsPPObj(), cGZBufferType::ThirtyTwoBitBgra))
	{
		throw std::runtime_error("Failed to load the zone bitmap image.");
	}

	const int32_t width = pImage->Width();
	const int32_t height = pImage->Height();

	if (width == SmallTileSize)
	{
		if (height != (SmallTileSize + 1))
		{
			throw std::runtime_error("A small city tile image must have a width of 64 pixels and a height of 65 pixels.");
		}
	}
	else if (width == MediumTileSize)
	{
		if (height != (MediumTileSize + 1))
		{
			throw std::runtime_error("A medium city tile image must have a width of 128 pixels and a height of 129 pixels.");
		}
	}
	else if (width == LargeTileSize)
	{
		if (height != (LargeTileSize + 1))
		{
			throw std::runtime_error("A large city tile image must have a width of 256 pixels and a height of 257 pixels.");
		}
	}
	else
	{
		throw std::runtime_error("Unknown city tile size. Must be 64x65 (small), 128x129 (medium) or 256x257 (large).");
	}

	const uint8_t* pScan0 = static_cast<const uint8_t*>(pImage->GetColorSurfaceBits());
	const uint32_t stride = pImage->GetColorSurfaceStride();

	std::unordered_map<ColorBgrx32, cISC4ZoneManager::ZoneType> zoneColors;
	ZoneCreationOptions creationOptions = ZoneCreationOptions::None;

	// The zone colors and creation options are stored as pixels in the last row of the image.
	// This control row is ignored when reading the actual zone data.

	LoadZoneColorInfoFromImage(pScan0, stride, height, zoneColors, creationOptions);

	const int32_t heightWithoutControlRow = height - 1;

	ZoneInfo zoneInfo(creationOptions);

	for (int32_t y = 0; y < heightWithoutControlRow; y++)
	{
		const ColorBgrx32* row = reinterpret_cast<const ColorBgrx32*>(pScan0 + (static_cast<size_t>(y) * stride));

		for (ptrdiff_t x = 0; x < width; x++)
		{
			const auto& zoneItem = zoneColors.find(row[x]);

			if (zoneItem != zoneColors.end())
			{
				const cISC4ZoneManager::ZoneType zoneType = zoneItem->second;

				auto existingEntry = zoneInfo.zones.find(zoneType);

				if (existingEntry != zoneInfo.zones.end())
				{
					existingEntry->second.region.cellMap.SetValue(x, y, true);
				}
				else
				{
					auto newEntry = zoneInfo.zones.emplace(
						zoneType,
						ZoneEntry(width, heightWithoutControlRow, false, zoneItem->first.ToRGBA()));
					newEntry.first->second.region.cellMap.SetValue(x, y, true);
				}
			}
		}
	}

	return zoneInfo;
}
