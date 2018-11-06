#include "LOPictureReader.hpp"
#include <iostream>

//LOMatrix ReadMatrixFromMemory(const std::vector<byte>& bytes, int width, int height, int losize, int bytesPerData, int stride)
//{
//	int si_si = losize * losize;
//
//	LOMatrix matrix;
//	matrix.resize(si_si);
//	for(size_t i = 0; i < si_si; i++)
//	{
//		matrix[i].resize(si_si, 0);
//	}
//
//	if(bytesPerData != 3 && bytesPerData != 4)
//	{
//		return matrix;
//	}
//
//	for(int i = 0; i < height; i++)
//	{
//		const byte* memoryStart = bytes.data() + i * stride;
//
//		for(int j = 0; j < width; j++)
//		{
//			const byte* pixelStart = memoryStart + j * bytesPerData;
//
//			const byte b = *(pixelStart + 0);
//			const byte g = *(pixelStart + 1);
//			const byte r = *(pixelStart + 2);
//
//			int matIBig = i / losize;
//			int matJBig = j / losize;
//			int matISm  = i % losize;
//			int matJSm  = j % losize;
//
//			int indexBig = matIBig * losize + matJBig;
//			int indexSm  = matISm  * losize + matJSm;
//			matrix[indexBig].set(indexSm, g < 120);
//		}
//	}
//
//	return matrix;
//}
//
//LOMatrix ReadMatrixSmFromMemory(const std::vector<byte>& bytes, int width, int height, int loSize, int bytesPerData, int stride)
//{
//	int si_si = loSize * loSize;
//
//	LOMatrix matrix;
//	matrix.resize(si_si);
//	for(size_t i = 0; i < si_si; i++)
//	{
//		matrix[i].resize(si_si, 0);
//	}
//
//	if(bytesPerData != 3 && bytesPerData != 4)
//	{
//		return matrix;
//	}
//
//	std::vector<boost::dynamic_bitset<>> factor;
//	factor.resize(height);
//	for(int i = 0; i < factor.size(); i++)
//	{
//		factor[i].resize(width, 0);
//	}





//
//	
//
//	return matrix;
//}

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
					break;
				case PictureLoadMode::BORDERLESS_SMALL:
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
	default:
		break;
	}
}

bool LOPictureReader::IsValidImage()
{
	return mFileHandle != INVALID_HANDLE_VALUE && mPixelByteSize != 0;
}

void LOPictureReader::ReadNextRowRegular(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex)
{
	if (!ReadFile(mFileHandle, mTempBuf.data(), mTempBuf.size(), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
	else
	{
		std::cout << "NOT IMPLEMENTED" << std::endl;
		/*if (bmpInfoHeader.biHeight > 0)
		{
			std::vector<byte> bitmapDataReversed(bitmapData.size());
			for (size_t i = 0; i < height; i++)
			{
				int iR = height - i - 1;
				memcpy_s(bitmapDataReversed.data() + i * stride, bitmapDataReversed.size() - (i * stride), bitmapData.data() + iR * stride, stride);
			}

			bitmapData = bitmapDataReversed;
		}

		result = ReadMatrixSmFromMemory(bitmapData, width, height, loSize, bitCount / 8, stride);*/
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

void LOPictureReader::ReadSmallPicture()
{
	if(!ReadFile(mFileHandle, mTempBuf.data(), mTempBuf.size(), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
}