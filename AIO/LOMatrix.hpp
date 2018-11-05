#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <string>

class LOMatrix
{
public:
	LOMatrix();
	~LOMatrix();

	LOMatrix Inverto();
	bool     CheckInv();

	void Load(const std::wstring& filename);
	void Save(const std::wstring& filename);

	void Default(uint32_t size);

private:
	uint32_t                                     mSize;
	uint32_t                                     mQuietPatternBase;
	std::vector<boost::dynamic_bitset<uint64_t>> mRows;
	std::vector<boost::dynamic_bitset<uint64_t>> mRowsInverto;
};