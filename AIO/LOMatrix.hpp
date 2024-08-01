#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include "FileModes.hpp"

class LOMatrix
{
public:
	LOMatrix();
	~LOMatrix();

	LOMatrix Inverto();
	uint32_t CheckInv();

	void SetIdentity(uint32_t gameSize);

	void Mul(const LOMatrix& right);
	boost::dynamic_bitset<uint64_t> MulBoard(boost::dynamic_bitset<uint64_t>& board);

	void Load(const std::wstring& filename, uint32_t gameSize);
	void LoadToroid(const std::wstring& filename, uint32_t gameSize);
	void LoadBig(const std::wstring& filename);

	void Save(const std::wstring& filename);
	void SaveBorderless(const std::wstring& filename);

	boost::multiprecision::cpp_int FindSolutionPeriod(uint32_t gameSize);

private:
	void LoadDefault(uint32_t size);
	void LoadDefaultTor(uint32_t size);

	void SaveMatrix(const std::wstring& filename, PictureSaveMode saveMode);

	int mod(int a, int b);

private:
	uint32_t                                     mSize;
	uint32_t                                     mQuietPatternBase;
	std::vector<boost::dynamic_bitset<uint64_t>> mRows;
	std::vector<boost::dynamic_bitset<uint64_t>> mRowsInverto;
};