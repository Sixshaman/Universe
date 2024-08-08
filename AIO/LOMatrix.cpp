#include "LOMatrix.hpp"
#include "LOPictureWriter.hpp"
#include "LOPictureReader.hpp"
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>
#include <format>
#include <bit>

uint32_t FindTrajectoryCycleLength(uint32_t n)
{
	//https://oeis.org/A309786
	//If n = 2^n or 3*2^n, return 1.
	//If n = 5*2^n, return 2.
	//Otherwise, if n is even, return cycle length of n/2.
	//Finally, if n is odd, return the value of https://oeis.org/A003558 at floor(n/2).

	if(std::has_single_bit(n))
	{
		return 1;
	}
	else if(n % 3 == 0 && std::has_single_bit(n / 3))
	{
		return 1;
	}
	else if(n % 5 == 0 && std::has_single_bit(n / 5))
	{
		return 2;
	}
	else if(n % 2 == 0)
	{
		return FindTrajectoryCycleLength(n / 2);
	}
	else
	{
		uint32_t nn = n / 2;

		uint32_t m = 1;
		while(true)
		{
			boost::multiprecision::cpp_int twoPowered = 2;
			twoPowered = twoPowered << m;

			uint32_t twoNplus1 = 2 * nn + 1;

			uint32_t remainder = (twoPowered % twoNplus1).convert_to<uint32_t>();
			if(remainder == 1 || remainder == 2 * nn)
			{
				return m + 1;
			}
			
			m++;
		}
	}
}

uint32_t FindEditingStepsPower(uint32_t n)
{
	//Finds 2^E, where E is the number of editing steps (delete, insert, or substitute) to transform n into n+1 in binary
	//If n + 1 = 2^m, then E = m
	//Otherwise, let m be the highest number such that 2^m divides n + 1. Then E = m + 1.
	if(std::has_single_bit(n))
	{
		return n;
	}
	else
	{
		return 2 * (1 << std::countr_zero(n));
	}
}

LOMatrix::LOMatrix()
{
}

LOMatrix::~LOMatrix()
{
}

LOMatrix LOMatrix::InvertMatrix()
{
	#pragma warning(disable: 6293) //For loops til unsigned overflow are not ill-defined

	LOMatrix result;
	result.SetIdentity(mBoardWidth, mBoardHeight);

	uint32_t matrixSize = mBoardWidth * mBoardHeight;
	for(uint32_t i = 0; i < matrixSize; i++)
	{
		if(!mRows[i][i])
		{
			for(uint32_t j = i + 1; j < matrixSize; j++)
			{
				if(mRows[j][i])
				{
					std::swap(mRows[i],        mRows[j]);
					std::swap(result.mRows[i], result.mRows[j]);
					break;
				}
			}
		}

	#pragma omp parallel for
		for(int j = i + 1; j < (int)matrixSize; j++)
		{
			if(mRows[j][i])
			{
				mRows[j]        ^= mRows[i];
				result.mRows[j] ^= result.mRows[i];
			}
		}
	}

	for(uint32_t i = matrixSize - 1; i < matrixSize; i--)
	{
	#pragma omp parallel for
		for(int j = i - 1; j >= 0; j--)
		{
			if(mRows[j][i])
			{
				mRows[j]        ^= mRows[i];
				result.mRows[j] ^= result.mRows[i];
			}
		}
	}

	result.mQuietPatternBase = 0;
	if(mRows.size() > 0)
	{
		for(size_t i = mRows.size() - 1; i < mRows.size(); i--)
		{
			if(mRows[i].none())
			{
				result.mQuietPatternBase++;
			}
		}
	}

	return result;
}

uint32_t LOMatrix::GetQuietPatternsCount()
{
	return mQuietPatternBase;
}

void LOMatrix::SetIdentity(uint32_t boardWidth, uint32_t boardHeight)
{
	mBoardWidth  = boardWidth;
	mBoardHeight = boardHeight;

	mRows.resize(mBoardWidth * mBoardHeight);
	for(uint32_t i = 0; i < mRows.size(); i++)
	{
		mRows[i].resize(mRows.size());
		mRows[i].set(i, 1);
	}
}

LOMatrix LOMatrix::Mul(const LOMatrix& right)
{
	if(right.mBoardWidth != mBoardWidth || right.mBoardHeight != mBoardHeight)
	{
		return *this;
	}

	LOMatrix result;
	result.mBoardWidth  = mBoardWidth;
	result.mBoardHeight = mBoardHeight;

	result.mRows.resize(mRows.size());
	for(size_t i = 0; i < mRows.size(); i++)
	{
		result.mRows[i].resize(mRows[i].size(), 0);
	}

	assert(right.mRows.size() > 0 && right.mRows.size() == right.mRows[0].size());
	
	std::vector<boost::dynamic_bitset<uint64_t>> rightColumnMajor;
	rightColumnMajor.resize(right.mRows.size());
	for(size_t j = 0; j < right.mRows[0].size(); j++)
	{
		rightColumnMajor[j].resize(right.mRows.size());
	}

	for(size_t i = 0; i < right.mRows.size(); i++)
	{
		for(size_t j = 0; j < mRows.size(); j++)
		{
			rightColumnMajor[j].set(i, right.mRows[i][j]);
		}
	}

#pragma omp parallel for
	for(int i = 0; i < (int)mRows.size(); i++)
	{
		const boost::dynamic_bitset<uint64_t>& leftMatrixRow = mRows[i];

		for(size_t j = 0; j < result.mRows[i].size(); j++)
		{
			const boost::dynamic_bitset<uint64_t>& rightMatrixColumn = rightColumnMajor[j];

			bool resMul = ((leftMatrixRow & rightMatrixColumn).count() % 2) == 1;
			result.mRows[i][j] = resMul;
		}
	}

	return result;
}

boost::dynamic_bitset<uint64_t> LOMatrix::MulBoard(boost::dynamic_bitset<uint64_t>& board)
{
	assert(board.size() == mBoardWidth * mBoardHeight);
	boost::dynamic_bitset<uint64_t> result(board.size());

	for(uint32_t i = 0; i < mBoardWidth * mBoardHeight; i++)
	{
		result.set(i, (mRows[i] & board).count() % 2);
	}

	return result;
}

LOMatrix LOMatrix::CalcMatrixPower(const boost::multiprecision::cpp_int& matrixPower, bool bVerbose)
{
	LOMatrix mulMat;

	if(matrixPower == 1)
	{
		mulMat = *this;
	}
	else
	{
		boost::multiprecision::cpp_int totalMatrixPower = 0;
		mulMat.SetIdentity(mBoardWidth, mBoardHeight);

		if(matrixPower & 1)
		{
			mulMat = mulMat.Mul(*this);

			if(bVerbose)
			{
				std::cout << "Calculated matrix power 1..." << std::endl;
			}
		}

		boost::multiprecision::cpp_int currMatrixPower = 1;

		//For each "1" bit of matrixPower, multiply the result matrix by the current matrix
		LOMatrix prevMatrix = *this;
		boost::multiprecision::cpp_int currMatrixPowerBit = 2; //Power 1 is already checked
		while(currMatrixPowerBit <= matrixPower)
		{
			LOMatrix currMatrix = prevMatrix.Mul(prevMatrix);
			if(matrixPower & currMatrixPowerBit)
			{
				mulMat = mulMat.Mul(currMatrix);

				currMatrixPower = currMatrixPower | currMatrixPowerBit;
				if(bVerbose)
				{
					std::cout << "Calculated matrix power " << currMatrixPower << "..." << std::endl;
				}
			}

			std::swap(prevMatrix, currMatrix);
			currMatrixPowerBit = (currMatrixPowerBit << 1);
		}
	}

	return mulMat;
}

void LOMatrix::LoadSquareClickRule(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight)
{
	mBoardWidth = boardWidth;
	mBoardHeight = boardHeight;
	mQuietPatternBase = 0;

	int matrixSize = mBoardWidth * mBoardHeight;
	mRows.clear();

	for (int i = 0; i < matrixSize; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(matrixSize));
	}

	LOPictureReader reader(filename, mBoardWidth, mBoardHeight, PictureLoadMode::BORDERLESS_SMALL);
	reader.ReadMetadata();

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		boost::dynamic_bitset<uint64_t> matrixRowSplat;
		matrixRowSplat.resize(mBoardWidth * mBoardWidth);
		for (size_t i = 0; i < mBoardHeight * mBoardHeight; i++)
		{
			reader.ReadNextRow(matrixRowSplat, i);

			size_t iBig = i / mBoardHeight;
			size_t jBig = i % mBoardHeight;

			for(size_t j = 0; j < matrixRowSplat.size(); j++)
			{
				size_t iSm = j / mBoardWidth;
				size_t jSm = j % mBoardWidth;

				size_t row    = iBig * mBoardWidth + iSm;
				size_t column = jBig * mBoardWidth + jSm;

				mRows[row].set(column, matrixRowSplat[j]);
			}
		}
	}
	else
	{
		//std::cout << "USING DEFAULT MATRIX" << std::endl;
		LoadDefault(mBoardWidth, mBoardHeight);
	}
}

void LOMatrix::LoadToroidClickRule(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight)
{
	mBoardWidth = boardWidth;
	mBoardHeight = boardHeight;
	mQuietPatternBase = 0;

	int matrixSize = mBoardWidth * mBoardHeight;
	mRows.clear();

	for (int i = 0; i < matrixSize; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(matrixSize));
	}

	LOPictureReader reader(filename, mBoardWidth, mBoardHeight, PictureLoadMode::BORDERLESS_SMALL_TOR);
	reader.ReadMetadata();

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		boost::dynamic_bitset<uint64_t> matrixRowSplat;
		matrixRowSplat.resize(mBoardWidth * mBoardWidth);
		for(size_t i = 0; i < mBoardHeight * mBoardHeight; i++)
		{
			reader.ReadNextRow(matrixRowSplat, i);

			size_t iBig = i / mBoardHeight;
			size_t jBig = i % mBoardHeight;

			for (size_t j = 0; j < matrixRowSplat.size(); j++)
			{
				size_t iSm = j / mBoardWidth;
				size_t jSm = j % mBoardWidth;

				size_t row    = iBig * mBoardWidth + iSm;
				size_t column = jBig * mBoardWidth + jSm;

				mRows[row].flip(column, matrixRowSplat[j]);
			}
		}
	}
	else
	{
		//std::cout << "USING DEFAULT MATRIX" << std::endl;
		LoadDefaultToroid(boardWidth, boardHeight);
	}
}

void LOMatrix::LoadMatrix(const std::string& filename)
{
	LOPictureReader reader(filename, 0, 0, PictureLoadMode::BORDERLESS);
	reader.ReadMetadata();

	mBoardWidth       = reader.GetBoardSize();
	mBoardHeight      = reader.GetBoardSize();
	mQuietPatternBase = 0;

	int matrixSize = mBoardWidth * mBoardHeight;
	mRows.clear();

	for (int i = 0; i < matrixSize; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(matrixSize));
	}

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		boost::dynamic_bitset<uint64_t> matrixRowSplat;
		matrixRowSplat.resize(mBoardWidth * mBoardWidth);
		for(size_t i = 0; i < mBoardHeight * mBoardHeight; i++)
		{
			reader.ReadNextRow(matrixRowSplat, i);

			size_t iBig = i / mBoardHeight;
			size_t jBig = i % mBoardHeight;

			for (size_t j = 0; j < matrixRowSplat.size(); j++)
			{
				size_t iSm = j / mBoardWidth;
				size_t jSm = j % mBoardWidth;

				size_t row    = iBig * mBoardHeight + iSm;
				size_t column = jBig * mBoardWidth  + jSm;

				mRows[row].set(column, matrixRowSplat[j]);
			}
		}
	}
	else
	{
		//std::cout << "USING DEFAULT MATRIX 7X7" << std::endl;
		LoadDefault(7, 7);
	}
}

void LOMatrix::SaveMatrix(const std::string& filename)
{
	SaveMatrix(filename, PictureSaveMode::BORDERFUL);
}

void LOMatrix::SaveMatrixBorderless(const std::string& filename)
{
	SaveMatrix(filename, PictureSaveMode::BORDERLESS);
}

boost::multiprecision::cpp_int LOMatrix::FindSolutionPeriod(uint32_t gameSize)
{
	//(Hypothesis) A solution period of ANY default Lights Out nxn has form:
	//2^p * (2^q - 1), where q is the (n+1)th value of https://oeis.org/A309786,
	//and p is the nth value of https://oeis.org/A091090.

	boost::multiprecision::cpp_int solutionPeriod = 1;
	solutionPeriod *= ((boost::multiprecision::cpp_int(1) << FindTrajectoryCycleLength(gameSize + 1)) - 1);
	solutionPeriod *= FindEditingStepsPower(gameSize + 1);

	return solutionPeriod;
}

void LOMatrix::LoadDefault(uint32_t boardWidth, uint32_t boardHeight)
{
	mBoardWidth = boardWidth;
	mBoardHeight = boardHeight;
	mQuietPatternBase = mBoardWidth * mBoardHeight;

	uint32_t matrixSize = mBoardWidth * mBoardHeight;
	mRows.clear();

	for(uint32_t i = 0; i < matrixSize; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(matrixSize));
	}

	for(uint32_t i = 0; i < matrixSize; i++)
	{
		mRows[i][i] = 1;

		uint32_t widthDiv = i / boardWidth;
		uint32_t widthRem = i - (widthDiv * boardWidth);

		if(widthRem != 0)
		{
			mRows[i][i - 1] = 1;
		}

		if(widthRem + 1 != boardWidth)
		{
			mRows[i][i + 1] = 1;
		}

		if(widthDiv != 0)
		{
			mRows[i][i - boardWidth] = 1;
		}

		if(widthDiv + 1 != boardHeight)
		{
			mRows[i][i + boardWidth] = 1;
		}
	}
}

void LOMatrix::LoadDefaultToroid(uint32_t boardWidth, uint32_t boardHeight)
{
	mBoardWidth = boardWidth;
	mBoardHeight = boardHeight;
	mQuietPatternBase = mBoardWidth * mBoardHeight;

	uint32_t matrixSize = mBoardWidth * mBoardHeight;
	mRows.clear();

	for(uint32_t i = 0; i < matrixSize; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(matrixSize));
	}

	for(uint32_t i = 0; i < matrixSize; i++)
	{
		uint32_t rowIndex = i / boardWidth;
		uint32_t colIndex = i % boardWidth;

		mRows[i].flip(i);

		mRows[i].flip((i + matrixSize - boardWidth) % matrixSize);
		mRows[i].flip((i + matrixSize + boardWidth) % matrixSize);

		mRows[i].flip(rowIndex * boardWidth + (colIndex + boardWidth - 1) % boardWidth);
		mRows[i].flip(rowIndex * boardWidth + (colIndex + boardWidth + 1) % boardWidth);
	}
}

void LOMatrix::SaveMatrix(const std::string& filename, PictureSaveMode saveMode)
{
	std::vector<boost::dynamic_bitset<uint64_t>> matrixUnsplat;
	matrixUnsplat.resize(mBoardHeight * mBoardHeight);
	for(size_t i = 0; i < matrixUnsplat.size(); i++)
	{
		matrixUnsplat[i].resize(mBoardWidth * mBoardWidth);
	}

	for(size_t i = 0; i < mRows.size(); i++)
	{
		size_t iBig = i / mBoardWidth;
		size_t jBig = i % mBoardWidth;

		for(size_t j = 0; j < mRows[i].size(); j++)
		{
			size_t iSm = j / mBoardWidth;
			size_t jSm = j % mBoardWidth;

			size_t row    = iBig * mBoardHeight + iSm;
			size_t column = jBig * mBoardWidth  + jSm;

			matrixUnsplat[row].set(column, mRows[i][j]);
		}
	}

	LOPictureWriter writer(filename, mBoardWidth, mBoardHeight, saveMode);
	writer.WriteMetadata();
	writer.WriteBeginning();

	for(size_t i = 0; i < matrixUnsplat.size(); i++)
	{
		writer.WriteRow(matrixUnsplat[i], i);
	}
}