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

	uint32_t GetQuietPatternsCount();

	void SetIdentity(uint32_t boardWidth, uint32_t boardHeight);

	LOMatrix Mul(const LOMatrix& right);
	boost::dynamic_bitset<uint64_t> MulBoard(boost::dynamic_bitset<uint64_t>& board);

	LOMatrix InvertMatrix();

	LOMatrix CalcMatrixPower(const boost::multiprecision::cpp_int& matrixPower, bool bVerbose);

	void LoadSquareClickRule(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight);
	void LoadToroidClickRule(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight);
	void LoadMatrix(const std::string& filename);

	void SaveMatrix(const std::string& filename);
	void SaveMatrixBorderless(const std::string& filename);

	boost::multiprecision::cpp_int FindSolutionPeriod();

	boost::multiprecision::cpp_int FindSolutionPeriodHeuristic(uint32_t boardSize);

private:
	void LoadDefault(uint32_t boardWidth, uint32_t boardHeight);
	void LoadDefaultToroid(uint32_t boardWidth, uint32_t boardHeight);

	void SaveMatrix(const std::string& filename, PictureSaveMode saveMode);

private:
	uint32_t mBoardWidth       = 0;
	uint32_t mBoardHeight      = 0;
	uint32_t mQuietPatternBase = 0;

	std::vector<boost::dynamic_bitset<uint64_t>> mRows;
};