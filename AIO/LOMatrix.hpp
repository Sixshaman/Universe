#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include "FileModes.hpp"

enum class SaveMode
{
	SaveNoBorders,
	SaveWithBorders,
	SaveWithSmallBorders
};

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

	LOMatrix CalcMatrixPower(const boost::multiprecision::cpp_int& matrixPower);

	void LoadSquareClickRule(const std::string& filename, uint32_t gameSize);
	void LoadToroidClickRule(const std::string& filename, uint32_t gameSize);
	void LoadMatrix(const std::string& filename);

	void SaveMatrix(const std::string& filename);
	void SaveMatrixBorderless(const std::string& filename);

	boost::multiprecision::cpp_int FindSolutionPeriod(uint32_t gameSize);

private:
	void LoadDefault(uint32_t size);
	void LoadDefaultTor(uint32_t size);

	void SaveMatrix(const std::string& filename, PictureSaveMode saveMode);

	int mod(int a, int b);

private:
	uint32_t                                     mSize;
	uint32_t                                     mQuietPatternBase;
	std::vector<boost::dynamic_bitset<uint64_t>> mRows;
	std::vector<boost::dynamic_bitset<uint64_t>> mRowsInverto;
};