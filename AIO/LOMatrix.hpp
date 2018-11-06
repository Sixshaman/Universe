#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include "FileModes.hpp"

class LOMatrix
{
public:
	LOMatrix();
	~LOMatrix();

	LOMatrix Inverto();
	uint32_t CheckInv();

	void Load(const std::wstring& filename, uint32_t gameSize);
	void LoadBig(const std::wstring& filename);

	void Save(const std::wstring& filename);
	void SaveBorderless(const std::wstring& filename);

private:
	void LoadDefault(uint32_t size);

	void SaveMatrix(const std::wstring& filename, PictureSaveMode saveMode);

private:
	uint32_t                                     mSize;
	uint32_t                                     mQuietPatternBase;
	std::vector<boost::dynamic_bitset<uint64_t>> mRows;
	std::vector<boost::dynamic_bitset<uint64_t>> mRowsInverto;
};