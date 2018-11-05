#include "LOMatrix.hpp"
#include "LOPictureWriter.hpp"

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

bool LOMatrix::CheckInv()
{
	if(mQuietPatternBase == mSize * mSize)
	{
		Inverto();
	}

	return mQuietPatternBase == 0;
}

void LOMatrix::Save(const std::wstring& filename)
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

	LOPictureWriter writer(filename, mSize, PictureSaveMode::BORDERFUL);
	writer.WriteMetadata();
	writer.WriteBeginning();

	for(size_t i = 0; i < matrixTrouted.size(); i++)
	{
		writer.WriteRow(matrixTrouted[i], i);
	}
}

void LOMatrix::Default(uint32_t size)
{
	mSize = size;

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
