#include "LOPictureReader.hpp"
#include <iostream>

LOPictureReader::LOPictureReader(const std::wstring& filename, uint32_t gamesize, PictureLoadMode loadMode): mFileHandle(nullptr), mGameSize(gamesize), mLoadMode(loadMode)
{
	mImageWidth    = 0;
	mImageHeight   = 0;
	mImageWidthSm  = 0;
	mImageHeightSm = 0;

	mImageStrideBytes = 0;

	mPixelByteSize = 0;

	mFileHandle = CreateFile(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(mFileHandle == INVALID_HANDLE_VALUE)
	{
		std::cout << "File doesn\'t exist!" << std::endl;
	}
}

LOPictureReader::~LOPictureReader()
{
	CloseHandle(mFileHandle);
}

void LOPictureReader::ReadMetadata()
{
	if(mFileHandle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	BITMAPFILEHEADER bmpFileHeader;
	if(!ReadFile(mFileHandle, &bmpFileHeader, sizeof(BITMAPFILEHEADER), nullptr, nullptr))
	{
		mPixelByteSize = 0;
	}
	else
	{
		BITMAPINFOHEADER bmpInfoHeader;
		if(!ReadFile(mFileHandle, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), nullptr, nullptr))
		{
			mPixelByteSize = 0;
		}
		else
		{
			int bitCount = bmpInfoHeader.biBitCount;
			if(bitCount != 32 && bitCount != 24)
			{
				mPixelByteSize = 0;
			}
			else
			{
				switch (mLoadMode)
				{
				case PictureLoadMode::BORDERLESS:
					mImageWidth    = abs(bmpInfoHeader.biWidth);
					mImageHeight   = abs(bmpInfoHeader.biHeight);
					mImageWidthSm  = (uint32_t)sqrt(mImageWidth);
					mImageHeightSm = (uint32_t)sqrt(mImageHeight);
					mGameSize      = mImageWidthSm;
					break;
				case PictureLoadMode::BORDERLESS_SMALL:
				case PictureLoadMode::BORDERLESS_SMALL_TOR:
					mImageWidthSm  = abs(bmpInfoHeader.biWidth);
					mImageHeightSm = abs(bmpInfoHeader.biHeight);
					mImageWidth    = mImageWidthSm * mImageWidthSm;
					mImageHeight   = mImageHeightSm * mImageHeightSm;
					break;
				default:
					break;
				}

				if(mImageWidth != mImageHeight ||
			      (mLoadMode == PictureLoadMode::BORDERLESS_SMALL && mImageWidth % 2 == 0) ||
				  (mLoadMode == PictureLoadMode::BORDERLESS_SMALL_TOR && mImageWidth % 2 == 0) ||
			      (mLoadMode == PictureLoadMode::BORDERLESS && (mImageWidthSm * mImageWidthSm != mImageWidth || mImageHeightSm * mImageHeightSm != mImageHeight)))
				{
					std::cout << "Wrong file size!" << std::endl;
					mPixelByteSize = 0;
				}
				else
				{
					size_t offBits = bmpFileHeader.bfOffBits - sizeof(BITMAPINFOHEADER) - sizeof(BITMAPFILEHEADER);
					std::vector<byte> ignoreBytes(offBits);

					if(offBits != 0 && !ReadFile(mFileHandle, ignoreBytes.data(), offBits, nullptr, nullptr))
					{
						mPixelByteSize = 0;
					}
					else
					{
						mPixelByteSize = bitCount / 8;

						uint32_t widthBytes = 0;
						switch (mLoadMode)
						{
						case PictureLoadMode::BORDERLESS:
							widthBytes = mPixelByteSize * mImageWidth;
							mImageStrideBytes = (widthBytes + 3) & (~3);
							mTempBuf.resize(mImageStrideBytes);
							break;
						case PictureLoadMode::BORDERLESS_SMALL:
						case PictureLoadMode::BORDERLESS_SMALL_TOR:
							widthBytes = mPixelByteSize * mImageWidthSm;
							mImageStrideBytes = (widthBytes + 3) & (~3);
							mTempBuf.resize(mImageStrideBytes * mImageHeightSm);
							break;
						}
					}
				}
			}
		}
	}
}

void LOPictureReader::ReadBeginning()
{
	switch (mLoadMode)
	{
	case PictureLoadMode::BORDERLESS:
		break;
	case PictureLoadMode::BORDERLESS_SMALL:
	case PictureLoadMode::BORDERLESS_SMALL_TOR:
		ReadSmallPicture();
		break;
	default:
		break;
	}
}

void LOPictureReader::ReadNextRow(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	switch (mLoadMode)
	{
	case PictureLoadMode::BORDERLESS:
		ReadNextRowRegular(row, rowIndex);
		break;
	case PictureLoadMode::BORDERLESS_SMALL:
		ReadNextRowSmall(row, rowIndex);
		break;
	case PictureLoadMode::BORDERLESS_SMALL_TOR:
		ReadNextRowSmallTor(row, rowIndex);
		break;
	default:
		break;
	}
}

bool LOPictureReader::IsValidImage()
{
	return mFileHandle != INVALID_HANDLE_VALUE && mPixelByteSize != 0;
}

uint32_t LOPictureReader::GetGameSize() const
{
	return mGameSize;
}

void LOPictureReader::ReadNextRowRegular(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	if (!ReadFile(mFileHandle, mTempBuf.data(), mTempBuf.size(), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
	else
	{
		row.clear();
		row.resize(mImageWidth);

		for (int i = 0; i < mImageWidth; i++)
		{
			const byte* pixelStart = mTempBuf.data() + i * mPixelByteSize;

			const byte b = *(pixelStart + 0);
			const byte g = *(pixelStart + 1);
			const byte r = *(pixelStart + 2);

			row.set(i, g < 120);
		}
	}
}

void LOPictureReader::ReadNextRowSmall(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	const uint32_t si_si = mGameSize * mGameSize;
	boost::dynamic_bitset<uint64_t> factor;
	factor.resize(mImageWidthSm * mImageHeightSm);

	for (int i = 0; i < mImageHeightSm; i++)
	{
		const byte* imgLinePtr = mTempBuf.data() + i * mImageStrideBytes;

		for (int j = 0; j < mImageWidthSm; j++)
		{
			const byte* pixelStart = imgLinePtr + j * mPixelByteSize;

			const byte b = *(pixelStart + 0);
			const byte g = *(pixelStart + 1);
			const byte r = *(pixelStart + 2);

			size_t cellIndex = i * mImageWidthSm + j;
			factor.set(cellIndex, g < 120);
		}
	}

	uint32_t iFacCenter  = mImageHeightSm / 2;
	uint32_t jFacCenter  = mImageWidthSm  / 2;

	uint32_t smallWidthHalf  = mImageWidthSm  / 2;
	uint32_t smallHeightHalf = mImageHeightSm / 2;

	int rowIndexBig = rowIndex / mGameSize;
	int rowIndexSm  = rowIndex % mGameSize;

	if(abs(rowIndexSm - rowIndexBig) > smallHeightHalf)
	{
		row.reset();
	}
	else
	{
		uint32_t lineIndex      = (rowIndexSm - rowIndexBig) + iFacCenter;
		uint32_t cellIndexFirst = lineIndex * mImageWidthSm;

		for (int i = 0; i < mGameSize; i++)
		{
			int colIndexBig = i;

			for (int j = 0; j < mGameSize; j++)
			{
				int colIndexSm = j;

				if(abs(colIndexSm - colIndexBig) > smallWidthHalf)
				{
					row.set(i * mGameSize + j, 0);
				}
				else
				{
					row.set(i * mGameSize + j, factor[cellIndexFirst + jFacCenter + (colIndexSm - colIndexBig)]);
				}
			}
		}
	}
}

void LOPictureReader::ReadNextRowSmallTor(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	const uint32_t si_si = mGameSize * mGameSize;
	boost::dynamic_bitset<uint64_t> factor;
	factor.resize(mImageWidthSm * mImageHeightSm);

	for (int i = 0; i < mImageHeightSm; i++)
	{
		const byte* imgLinePtr = mTempBuf.data() + i * mImageStrideBytes;

		for (int j = 0; j < mImageWidthSm; j++)
		{
			const byte* pixelStart = imgLinePtr + j * mPixelByteSize;

			const byte b = *(pixelStart + 0);
			const byte g = *(pixelStart + 1);
			const byte r = *(pixelStart + 2);

			size_t cellIndex = i * mImageWidthSm + j;
			factor.set(cellIndex, g < 120);
		}
	}

	int smallWidthHalf  = mImageWidthSm  / 2;
	int smallHeightHalf = mImageHeightSm / 2;

	int rowIndexBig = rowIndex / mGameSize;
	int rowIndexSm  = rowIndex % mGameSize;

	int topmostRow       = mod(rowIndexBig - smallHeightHalf, mGameSize);
	int newSmallRowIndex = mod(rowIndexSm - topmostRow, mGameSize);

	if(mImageHeightSm <= mGameSize)
	{
		if(newSmallRowIndex >= mImageHeightSm)
		{
			row.reset();
		}
		else
		{
			for(int colIndexBig = 0; colIndexBig < mGameSize; colIndexBig++)
			{
				for(int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
				{
					int leftmostCol = mod(colIndexBig - smallWidthHalf, mGameSize);
					if(mImageWidthSm <= mGameSize)
					{
						int newSmallColIndex = mod(colIndexSm - leftmostCol, mGameSize);
						if(newSmallColIndex >= mImageWidthSm)
						{
							row.set(colIndexBig * mGameSize + colIndexSm, 0);
						}
						else
						{
							row.set(colIndexBig * mGameSize + colIndexSm, factor[newSmallRowIndex * mImageWidthSm + newSmallColIndex]);
						}
					}
					else
					{
						//TODO
					}
				}
			}
		}
	}
	else
	{
		int fullRowsTied = mImageHeightSm / mGameSize;

		std::vector<boost::dynamic_bitset<uint64_t>> tiedRows;
		tiedRows.resize(fullRowsTied);

		for(int r = 0; r < fullRowsTied; r++)
		{
			int rowIndexSmTied = newSmallRowIndex + r * mGameSize;
			tiedRows[r].resize(row.size());

			if(mImageWidthSm <= mGameSize)
			{
				for (int colIndexBig = 0; colIndexBig < mGameSize; colIndexBig++)
				{
					for (int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
					{
						int leftmostCol = mod(colIndexBig - smallWidthHalf, mGameSize);

						int newSmallColIndex = mod(colIndexSm - leftmostCol, mGameSize);
						if (newSmallColIndex >= mImageWidthSm)
						{
							tiedRows[r].set(colIndexBig * mGameSize + colIndexSm, 0);
						}
						else
						{
							tiedRows[r].set(colIndexBig * mGameSize + colIndexSm, factor[rowIndexSmTied * mImageWidthSm + newSmallColIndex]);
						}
					}
				}
			}
			else
			{
				for(int colIndexBig = 0; colIndexBig < mGameSize; colIndexBig++)
				{
					int fullColsTied = mImageWidthSm / mGameSize;
					int leftmostCol  = mod(colIndexBig - smallWidthHalf, mGameSize);

					std::vector<boost::dynamic_bitset<uint64_t>> tiedBigCols;
					tiedBigCols.resize(fullColsTied);

					for(int c = 0; c < fullColsTied; c++)
					{
						tiedBigCols[c].resize(mGameSize);

						for(int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
						{
							int newSmallColIndex = mod(colIndexSm - leftmostCol, mGameSize);

							int overlappedColIndex = colIndexSm + c * mGameSize;
							tiedBigCols[c].set(newSmallColIndex, factor[rowIndexSmTied * mImageWidthSm + overlappedColIndex]);
						}
					}

					boost::dynamic_bitset<uint64_t> finalTiedBigCol;
					finalTiedBigCol.resize(mGameSize);
					finalTiedBigCol.reset();
					for(auto dbs : tiedBigCols)
					{
						finalTiedBigCol ^= dbs;
					}

					int lastColTieStart = fullColsTied * mGameSize;
					for(int tieColIndex = lastColTieStart; tieColIndex < mImageWidthSm; tieColIndex++)
					{
						finalTiedBigCol[tieColIndex - lastColTieStart] ^= factor[rowIndexSmTied * mImageWidthSm + tieColIndex];
					}

					for(int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
					{
						tiedRows[r].set(colIndexBig * mGameSize + colIndexSm, finalTiedBigCol[colIndexSm]);
					}
				}
			}
		}

		boost::dynamic_bitset<uint64_t> finalTiedRow;
		finalTiedRow.resize(mGameSize * mGameSize);
		finalTiedRow.reset();
		for (auto dbs: tiedRows)
		{
			finalTiedRow ^= dbs;
		}

		int lastRowTieLength = mImageHeightSm - fullRowsTied * mGameSize;
		if(newSmallRowIndex < lastRowTieLength)
		{
			int lastRowTieIndex = fullRowsTied * mGameSize + newSmallRowIndex;
			if (mImageWidthSm <= mGameSize)
			{
				for (int colIndexBig = 0; colIndexBig < mGameSize; colIndexBig++)
				{
					for (int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
					{
						int leftmostCol = mod(colIndexBig - smallWidthHalf, mGameSize);

						int newSmallColIndex = mod(colIndexSm - leftmostCol, mGameSize);
						if (newSmallColIndex < mImageWidthSm)
						{
							finalTiedRow[colIndexBig * mGameSize + colIndexSm] ^= factor[lastRowTieIndex * mImageWidthSm + newSmallColIndex];
						}
					}
				}
			}
			else
			{
				for (int colIndexBig = 0; colIndexBig < mGameSize; colIndexBig++)
				{
					int fullColsTied = mImageWidthSm / mGameSize;
					int leftmostCol = mod(colIndexBig - smallWidthHalf, mGameSize);

					std::vector<boost::dynamic_bitset<uint64_t>> tiedBigCols;
					tiedBigCols.resize(fullColsTied);

					for (int c = 0; c < fullColsTied; c++)
					{
						tiedBigCols[c].resize(mGameSize);

						for (int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
						{
							int newSmallColIndex = mod(colIndexSm - leftmostCol, mGameSize);

							int overlappedColIndex = newSmallColIndex + c * mGameSize;
							tiedBigCols[c].set(colIndexSm, factor[lastRowTieIndex * mImageWidthSm + overlappedColIndex]);
						}
					}

					boost::dynamic_bitset<uint64_t> finalTiedBigCol;
					finalTiedBigCol.resize(mGameSize);
					finalTiedBigCol.reset();
					for (auto dbs : tiedBigCols)
					{
						finalTiedBigCol ^= dbs;
					}

					int lastColTieStart = leftmostCol + fullColsTied * mGameSize;
					for (int tieColIndex = lastColTieStart; tieColIndex < mImageWidthSm; tieColIndex++)
					{
						finalTiedBigCol[tieColIndex - lastColTieStart] ^= factor[lastRowTieIndex * mImageWidthSm + tieColIndex];
					}

					for (int colIndexSm = 0; colIndexSm < mGameSize; colIndexSm++)
					{
						finalTiedRow[colIndexBig * mGameSize + colIndexSm] ^= finalTiedBigCol[colIndexSm];
					}
				}
			}
		}

		row = finalTiedRow;
	}
}

void LOPictureReader::ReadSmallPicture()
{
	if(!ReadFile(mFileHandle, mTempBuf.data(), mTempBuf.size(), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
}

int LOPictureReader::mod(int a, int b)
{
	return ((((a) % b) + b) % b);
}
