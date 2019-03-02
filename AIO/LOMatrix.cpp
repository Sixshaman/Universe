#include "LOMatrix.hpp"
#include "LOPictureWriter.hpp"
#include "LOPictureReader.hpp"
#include <iostream>

LOMatrix::LOMatrix(): mSize(0), mQuietPatternBase(mSize*mSize)
{
}

LOMatrix::~LOMatrix()
{
}

LOMatrix LOMatrix::Inverto()
{
	if(mRowsInverto.empty())
	{
		int size_size = mRows.size();

		for (int i = 0; i < size_size; i++)
		{
			mRowsInverto.push_back(boost::dynamic_bitset<uint64_t>(size_size));
			mRowsInverto[i].set(i);
		}

		auto base = mRows;
		for (int i = 0; i < size_size; i++)
		{
			if (!base[i][i])
			{
				for (int j = i + 1; j < size_size; j++)
				{
					if (base[j][i])
					{
						swap(base[i],         base[j]);
						swap(mRowsInverto[i], mRowsInverto[j]);
						break;
					}
				}
			}
			for (int j = i + 1; j < size_size; j++)
			{
				if (base[j][i])
				{
					base[j]         ^= base[i];
					mRowsInverto[j] ^= mRowsInverto[i];
				}
			}
		}

		for (int i = size_size - 1; i >= 0; i--)
		{
			for (int j = i - 1; j >= 0; j--)
			{
				if (base[j][i])
				{
					base[j]         ^= base[i];
					mRowsInverto[j] ^= mRowsInverto[i];
				}
			}
		}

		mQuietPatternBase = 0;
		if(base.size() > 0)
		{
			for(size_t i = base.size() - 1; i >= 0 && i < base.size(); i--)
			{
				if(base[i].none())
				{
					mQuietPatternBase++;
				}
			}
		}
	}

	LOMatrix inverto;
	inverto.mRows             = mRowsInverto;
	inverto.mRowsInverto      = mRows;
	inverto.mSize             = mSize;
	inverto.mQuietPatternBase = mSize * mSize;

	return inverto;
}

uint32_t LOMatrix::CheckInv()
{
	if(mQuietPatternBase == mSize * mSize)
	{
		Inverto();
	}

	return mQuietPatternBase;
}

void LOMatrix::Mul(const LOMatrix& right)
{
	if(right.mSize != mSize)
	{
		return;
	}

	std::vector<boost::dynamic_bitset<uint64_t>> rowsNew;
	rowsNew.resize(mRows.size());
	for(size_t i = 0; i < mRows.size(); i++)
	{
		rowsNew[i].resize(mRows[i].size(), 0);
	}

	for(size_t i = 0; i < rowsNew.size(); i++)
	{
		boost::dynamic_bitset<uint64_t> rowM = mRows[i];
		for(size_t j = 0; j < rowsNew[i].size(); j++)
		{
			boost::dynamic_bitset<uint64_t> colM;
			colM.resize(rowM.size(), false);
			for(size_t k = 0; k < colM.size(); k++)
			{
				colM.set(k, right.mRows[k][j]);
			}

			bool resMul = ((rowM & colM).count() % 2) == 1;
			rowsNew[i][j] = resMul;
		}
	}

	mRows = rowsNew;
}

void LOMatrix::Load(const std::wstring& filename, uint32_t gameSize)
{
	mSize = gameSize;
	mQuietPatternBase = mSize * mSize;

	int si_si = mSize * mSize;
	mRows.clear();
	mRowsInverto.clear();

	for (int i = 0; i < si_si; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(si_si));
	}

	LOPictureReader reader(filename, mSize, PictureLoadMode::BORDERLESS_SMALL);
	reader.ReadMetadata();

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		std::vector<boost::dynamic_bitset<uint64_t>> matrixTrouted;
		matrixTrouted.resize(mRows.size());
		for (size_t i = 0; i < mRows.size(); i++)
		{
			matrixTrouted[i].resize(mRows.size());
		}

		for (int i = 0; i < si_si; i++)
		{
			reader.ReadNextRow(matrixTrouted[i], i);
		}

		for (size_t i = 0; i < matrixTrouted.size(); i++)
		{
			size_t iBig = i / mSize;
			size_t jBig = i % mSize;

			for (size_t j = 0; j < matrixTrouted[i].size(); j++)
			{
				size_t iSm = j / mSize;
				size_t jSm = j % mSize;

				size_t row = iBig * mSize + iSm;
				size_t column = jBig * mSize + jSm;

				mRows[row].set(column, matrixTrouted[i][j]);
			}
		}
	}
	else
	{
		std::cout << "USING DEFAULT MATRIX" << std::endl;
		LoadDefault(gameSize);
	}
}

void LOMatrix::LoadToroid(const std::wstring & filename, uint32_t gameSize)
{
	mSize = gameSize;
	mQuietPatternBase = mSize * mSize;

	int si_si = mSize * mSize;
	mRows.clear();
	mRowsInverto.clear();

	for (int i = 0; i < si_si; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(si_si));
	}

	LOPictureReader reader(filename, mSize, PictureLoadMode::BORDERLESS_SMALL_TOR);
	reader.ReadMetadata();

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		std::vector<boost::dynamic_bitset<uint64_t>> matrixTrouted;
		matrixTrouted.resize(mRows.size());
		for (size_t i = 0; i < mRows.size(); i++)
		{
			matrixTrouted[i].resize(mRows.size());
		}

		for (int i = 0; i < si_si; i++)
		{
			reader.ReadNextRow(matrixTrouted[i], i);
		}

		for (size_t i = 0; i < matrixTrouted.size(); i++)
		{
			size_t iBig = i / mSize;
			size_t jBig = i % mSize;

			for (size_t j = 0; j < matrixTrouted[i].size(); j++)
			{
				size_t iSm = j / mSize;
				size_t jSm = j % mSize;

				size_t row    = iBig * mSize + iSm;
				size_t column = jBig * mSize + jSm;

				mRows[row].set(column, matrixTrouted[i][j]);
			}
		}
	}
	else
	{
		std::cout << "USING DEFAULT MATRIX" << std::endl;
		LoadDefaultTor(gameSize);
	}
}

void LOMatrix::LoadBig(const std::wstring& filename)
{
	LOPictureReader reader(filename, 0, PictureLoadMode::BORDERLESS);
	reader.ReadMetadata();

	mSize             = reader.GetGameSize();
	mQuietPatternBase = mSize * mSize;

	int si_si = mSize * mSize;
	mRows.clear();
	mRowsInverto.clear();

	for (int i = 0; i < si_si; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(si_si));
	}

	if(reader.IsValidImage())
	{
		reader.ReadBeginning();

		std::vector<boost::dynamic_bitset<uint64_t>> matrixTrouted;
		matrixTrouted.resize(mRows.size());
		for (size_t i = 0; i < mRows.size(); i++)
		{
			matrixTrouted[i].resize(mRows.size());
		}

		for (int i = 0; i < si_si; i++)
		{
			reader.ReadNextRow(matrixTrouted[matrixTrouted.size() - i - 1], i);
		}

		for (size_t i = 0; i < matrixTrouted.size(); i++)
		{
			size_t iBig = i / mSize;
			size_t jBig = i % mSize;

			for (size_t j = 0; j < matrixTrouted[i].size(); j++)
			{
				size_t iSm = j / mSize;
				size_t jSm = j % mSize;

				size_t row    = iBig * mSize + iSm;
				size_t column = jBig * mSize + jSm;

				mRows[row].set(column, matrixTrouted[i][j]);
			}
		}
	}
	else
	{
		std::cout << "USING DEFAULT MATRIX 7X7" << std::endl;
		LoadDefault(7);
	}
}

void LOMatrix::Save(const std::wstring& filename)
{
	SaveMatrix(filename, PictureSaveMode::BORDERFUL);
}

void LOMatrix::SaveBorderless(const std::wstring& filename)
{
	SaveMatrix(filename, PictureSaveMode::BORDERLESS);
}

void LOMatrix::LoadDefault(uint32_t size)
{
	mSize = size;
	mQuietPatternBase = mSize * mSize;

	int si_si = size * size;
	mRows.clear();
	mRowsInverto.clear();

	for (int i = 0; i < si_si; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(si_si));
	}

	for (int i = 0; i < si_si; i++)
	{
		for (int j = 0; j < si_si; j++)
		{
			if ((i == j) || ((i == j - 1) && (i + 1) % size) || ((i == j + 1) && (i%size)) || (i == j - size) || (i == j + size))
				mRows[i][j] = 1;
			else mRows[i][j] = 0;
		}
	}
}

void LOMatrix::LoadDefaultTor(uint32_t size)
{
	mSize = size;
	mQuietPatternBase = mSize * mSize;

	int si_si = size * size;
	mRows.clear();
	mRowsInverto.clear();

	for (int i = 0; i < si_si; i++)
	{
		mRows.push_back(boost::dynamic_bitset<uint64_t>(si_si));
		mRows.back().reset();
	}

	for(int i = 0; i < si_si; i++)
	{
		int iBig = i / mSize;
		int iSm  = i % mSize;

		int rightSm = mod(iSm + 1, mSize);
		int leftSm  = mod(iSm - 1, mSize);
		int topBig  = mod(iBig - 1, mSize);
		int botBig  = mod(iBig + 1, mSize);

		int top   = topBig * mSize + iSm;
		int bot   = botBig * mSize + iSm;
		int left  = iBig * mSize + leftSm;
		int right = iBig * mSize + rightSm;

		mRows[i].set(i);
		mRows[i].set(top);
		mRows[i].set(bot);
		mRows[i].set(left);
		mRows[i].set(right);
	}
}

void LOMatrix::SaveMatrix(const std::wstring & filename, PictureSaveMode saveMode)
{
	std::vector<boost::dynamic_bitset<uint64_t>> matrixTrouted;
	matrixTrouted.resize(mRows.size());
	for(size_t i = 0; i < mRows.size(); i++)
	{
		matrixTrouted[i].resize(mRows.size());
	}

	for(size_t i = 0; i < mRows.size(); i++)
	{
		size_t iBig = i / mSize;
		size_t jBig = i % mSize;

		for(size_t j = 0; j < mRows[i].size(); j++)
		{
			size_t iSm = j / mSize;
			size_t jSm = j % mSize;

			size_t row    = iBig * mSize + iSm;
			size_t column = jBig * mSize + jSm;

			matrixTrouted[row].set(column, mRows[i][j]);
		}
	}

	LOPictureWriter writer(filename, mSize, saveMode);
	writer.WriteMetadata();
	writer.WriteBeginning();

	for(size_t i = 0; i < matrixTrouted.size(); i++)
	{
		writer.WriteRow(matrixTrouted[i], i);
	}
}

int LOMatrix::mod(int a, int b)
{
	return ((((a) % b) + b) % b);
}
