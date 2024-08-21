#pragma once
#include <cstdint>

class cRZCellMap
{
public:
	cRZCellMap(uint32_t width, uint32_t height, bool initialValue);
	cRZCellMap(cRZCellMap const& other);
	cRZCellMap(cRZCellMap&& other) noexcept;

	cRZCellMap& operator=(cRZCellMap const& other);
	cRZCellMap& operator=(cRZCellMap&& other) noexcept;

	virtual ~cRZCellMap();

	bool GetValue(uint32_t x, uint32_t y) const;
	void SetValue(uint32_t x, uint32_t y, bool value);

private:
	void DestroyData();

	uint32_t rows;
	uint32_t columns;
	uint32_t columnIntegerCount;
	uint32_t** data;
};

static_assert(sizeof(cRZCellMap) == 0x14);