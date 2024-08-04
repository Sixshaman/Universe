#include "LOPictureReader.hpp"
#include <iostream>

LOPictureReader::LOPictureReader(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight, PictureLoadMode loadMode): mFileHandle(nullptr), mBoardWidth(boardWidth), mBoardHeight(boardHeight), mLoadMode(loadMode)
{
	mImageWidth    = 0;
	mImageHeight   = 0;
	mImageWidthSm  = 0;
	mImageHeightSm = 0;

	mImageStrideBytes = 0;

	mPixelByteSize = 0;

	mFileHandle = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(mFileHandle == INVALID_HANDLE_VALUE && !filename.empty())
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
					mBoardWidth    = mImageWidthSm;
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

uint32_t LOPictureReader::GetBoardSize() const
{
	return mBoardWidth;
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
	const uint32_t matrixSize = mBoardWidth * mBoardHeight;
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

	int rowIndexBig = rowIndex / mBoardHeight;
	int rowIndexSm  = rowIndex % mBoardHeight;

	row.resize(mBoardWidth * mBoardWidth);
	if(abs(rowIndexSm - rowIndexBig) > smallHeightHalf)
	{
		row.reset();
	}
	else
	{
		uint32_t lineIndex      = (rowIndexSm - rowIndexBig) + iFacCenter;
		uint32_t cellIndexFirst = lineIndex * mImageWidthSm;

		for (int i = 0; i < mBoardWidth; i++)
		{
			int colIndexBig = i;

			for (int j = 0; j < mBoardWidth; j++)
			{
				int colIndexSm = j;

				if(abs(colIndexSm - colIndexBig) > smallWidthHalf)
				{
					row.set(i * mBoardWidth + j, 0);
				}
				else
				{
					row.set(i * mBoardWidth + j, factor[cellIndexFirst + jFacCenter + (colIndexSm - colIndexBig)]);
				}
			}
		}
	}
}

void LOPictureReader::ReadNextRowSmallTor(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	const uint32_t si_si = mBoardWidth * mBoardHeight;
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

	int rowIndexBig = rowIndex / mBoardHeight;
	int rowIndexSm  = rowIndex % mBoardHeight;


	int topmostRow       = (rowIndexBig + mBoardHeight - smallHeightHalf) % mBoardHeight;
	int newSmallRowIndex = (rowIndexSm + mBoardHeight - topmostRow) % mBoardHeight;

	row.resize(mBoardWidth * mBoardWidth);
	if(mImageHeightSm <= mBoardHeight)
	{
		if(newSmallRowIndex >= mImageHeightSm)
		{
			row.reset();
		}
		else
		{
			for(int colIndexBig = 0; colIndexBig < mBoardWidth; colIndexBig++)
			{
				for(int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
				{
					int leftmostCol = (colIndexBig + mBoardWidth - smallWidthHalf) % mBoardWidth;
					if(mImageWidthSm <= mBoardWidth)
					{
						int newSmallColIndex = (colIndexSm + mBoardWidth - leftmostCol) % mBoardWidth;
						if(newSmallColIndex >= mImageWidthSm)
						{
							row.set(colIndexBig * mBoardWidth + colIndexSm, 0);
						}
						else
						{
							row.set(colIndexBig * mBoardWidth + colIndexSm, factor[newSmallRowIndex * mImageWidthSm + newSmallColIndex]);
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
		int fullRowsTied = mImageHeightSm / mBoardHeight;

		std::vector<boost::dynamic_bitset<uint64_t>> tiedRows;
		tiedRows.resize(fullRowsTied);

		for(int r = 0; r < fullRowsTied; r++)
		{
			int rowIndexSmTied = newSmallRowIndex + r * mBoardHeight;
			tiedRows[r].resize(row.size());

			if(mImageWidthSm <= mBoardWidth)
			{
				for (int colIndexBig = 0; colIndexBig < mBoardWidth; colIndexBig++)
				{
					for (int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
					{
						int leftmostCol = (colIndexBig + mBoardWidth - smallWidthHalf) % mBoardWidth;

						int newSmallColIndex = (colIndexSm + mBoardWidth - leftmostCol) % mBoardWidth;
						if (newSmallColIndex >= mImageWidthSm)
						{
							tiedRows[r].flip(colIndexBig * mBoardWidth + colIndexSm, 0);
						}
						else
						{
							tiedRows[r].flip(colIndexBig * mBoardWidth + colIndexSm, factor[rowIndexSmTied * mImageWidthSm + newSmallColIndex]);
						}
					}
				}
			}
			else
			{
				for(int colIndexBig = 0; colIndexBig < mBoardWidth; colIndexBig++)
				{
					int fullColsTied = mImageWidthSm / mBoardWidth;
					int leftmostCol  = (colIndexBig + mBoardWidth - smallWidthHalf) % mBoardWidth;

					std::vector<boost::dynamic_bitset<uint64_t>> tiedBigCols;
					tiedBigCols.resize(fullColsTied);

					for(int c = 0; c < fullColsTied; c++)
					{
						tiedBigCols[c].resize(mBoardWidth);

						for(int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
						{
							int newSmallColIndex = (colIndexSm + mBoardWidth - leftmostCol) % mBoardWidth;

							int overlappedColIndex = colIndexSm + c * mBoardWidth;
							tiedBigCols[c].flip(newSmallColIndex, factor[rowIndexSmTied * mImageWidthSm + overlappedColIndex]);
						}
					}

					boost::dynamic_bitset<uint64_t> finalTiedBigCol;
					finalTiedBigCol.resize(mBoardWidth);
					finalTiedBigCol.reset();
					for(auto dbs : tiedBigCols)
					{
						finalTiedBigCol ^= dbs;
					}

					int lastColTieStart = fullColsTied * mBoardWidth;
					for(int tieColIndex = lastColTieStart; tieColIndex < mImageWidthSm; tieColIndex++)
					{
						finalTiedBigCol[tieColIndex - lastColTieStart] ^= factor[rowIndexSmTied * mImageWidthSm + tieColIndex];
					}

					for(int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
					{
						tiedRows[r].flip(colIndexBig * mBoardWidth + colIndexSm, finalTiedBigCol[colIndexSm]);
					}
				}
			}
		}

		boost::dynamic_bitset<uint64_t> finalTiedRow;
		finalTiedRow.resize(mBoardWidth * mBoardHeight);
		finalTiedRow.reset();
		for (auto dbs: tiedRows)
		{
			finalTiedRow ^= dbs;
		}

		int lastRowTieLength = mImageHeightSm - fullRowsTied * mBoardWidth;
		if(newSmallRowIndex < lastRowTieLength)
		{
			int lastRowTieIndex = fullRowsTied * mBoardWidth + newSmallRowIndex;
			if (mImageWidthSm <= mBoardWidth)
			{
				for (int colIndexBig = 0; colIndexBig < mBoardWidth; colIndexBig++)
				{
					for (int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
					{
						int leftmostCol = (colIndexBig + mBoardWidth - smallWidthHalf) % mBoardWidth;

						int newSmallColIndex = (colIndexSm + mBoardWidth - leftmostCol) % mBoardWidth;
						if (newSmallColIndex < mImageWidthSm)
						{
							finalTiedRow[colIndexBig * mBoardWidth + colIndexSm] ^= factor[lastRowTieIndex * mImageWidthSm + newSmallColIndex];
						}
					}
				}
			}
			else
			{
				for (int colIndexBig = 0; colIndexBig < mBoardWidth; colIndexBig++)
				{
					int fullColsTied = mImageWidthSm / mBoardWidth;
					int leftmostCol = (colIndexBig + mBoardWidth - smallWidthHalf) % mBoardWidth;

					std::vector<boost::dynamic_bitset<uint64_t>> tiedBigCols;
					tiedBigCols.resize(fullColsTied);

					for (int c = 0; c < fullColsTied; c++)
					{
						tiedBigCols[c].resize(mBoardWidth);

						for (int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
						{
							int newSmallColIndex = (colIndexSm - leftmostCol) % mBoardWidth;

							int overlappedColIndex = newSmallColIndex + c * mBoardWidth;
							tiedBigCols[c].flip(colIndexSm, factor[lastRowTieIndex * mImageWidthSm + overlappedColIndex]);
						}
					}

					boost::dynamic_bitset<uint64_t> finalTiedBigCol;
					finalTiedBigCol.resize(mBoardWidth);
					finalTiedBigCol.reset();
					for (auto dbs : tiedBigCols)
					{
						finalTiedBigCol ^= dbs;
					}

					int lastColTieStart = leftmostCol + fullColsTied * mBoardWidth;
					for (int tieColIndex = lastColTieStart; tieColIndex < mImageWidthSm; tieColIndex++)
					{
						finalTiedBigCol[tieColIndex - lastColTieStart] ^= factor[lastRowTieIndex * mImageWidthSm + tieColIndex];
					}

					for (int colIndexSm = 0; colIndexSm < mBoardWidth; colIndexSm++)
					{
						finalTiedRow[colIndexBig * mBoardWidth + colIndexSm] ^= finalTiedBigCol[colIndexSm];
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
