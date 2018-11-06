#include <iostream>
#include "LOPictureWriter.hpp"

#define MAKE_ARGB(A, R, G, B) ((UINT)((A << 24) | (R << 16) | (G << 8) | (B << 0)))

#define RED_ARGB   MAKE_ARGB(0xff, 0xff, 0x00, 0x00)
#define BLACK_ARGB MAKE_ARGB(0xff, 0x00, 0x00, 0x00)
#define WHITE_ARGB MAKE_ARGB(0xff, 0xff, 0xff, 0xff)

#define CELL_WID   5
#define BORDER_WID 1
#define SPACE_WID  1

LOPictureWriter::LOPictureWriter(const std::wstring& filename, uint32_t gamesize, PictureSaveMode mode): mFileHandle(nullptr), mGameSize(gamesize), mSaveMode(mode)
{
	mImageWidth    = 0;
	mImageHeight   = 0;
	mImageWidthSm  = 0;
	mImageHeightSm = 0;

	mImageStrideBytes = 0;

	mFileHandle = CreateFile(filename.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
}

LOPictureWriter::~LOPictureWriter()
{
	CloseHandle(mFileHandle);
}

void LOPictureWriter::WriteMetadata()
{
	int widSm = 0;
	int heiSm = 0;
	int wid   = 0;
	int hei   = 0;

	switch (mSaveMode)
	{
	case PictureSaveMode::BORDERFUL:
		widSm = (CELL_WID * mGameSize + BORDER_WID * (mGameSize + 1));
		heiSm = (CELL_WID * mGameSize + BORDER_WID * (mGameSize + 1));
		wid   = widSm * mGameSize + SPACE_WID * (mGameSize + 1);
		hei   = heiSm * mGameSize + SPACE_WID * (mGameSize + 1);
		break;
	case PictureSaveMode::BORDERFUL_WHITE:
		widSm = CELL_WID * mGameSize;
		heiSm = CELL_WID * mGameSize;
		wid   = widSm * mGameSize + SPACE_WID * (mGameSize + 1);
		hei   = heiSm * mGameSize + SPACE_WID * (mGameSize + 1);
		break;
	case PictureSaveMode::BORDERLESS:
		widSm = mGameSize;
		heiSm = mGameSize;
		wid   = widSm * mGameSize;
		hei   = heiSm * mGameSize;
		break;
	case PictureSaveMode::BORDERLESS_BIG:
		widSm = CELL_WID * mGameSize;
		heiSm = CELL_WID * mGameSize;
		wid   = widSm * mGameSize;
		hei   = wid;
		break;
	default:
		break;
	}

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024;
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + sizeof(UINT) * wid * hei + hei * (sizeof(UINT)* wid) % 4;
	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;

	BITMAPINFOHEADER bmpInfoHeader;
	ZeroMemory(&bmpInfoHeader, sizeof(BITMAPINFOHEADER));
	bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHeader.biBitCount = 32;
	bmpInfoHeader.biClrUsed = 0;
	bmpInfoHeader.biCompression = BI_RGB;
	bmpInfoHeader.biWidth = wid;
	bmpInfoHeader.biHeight = -1 * hei;
	bmpInfoHeader.biPlanes = 1;

	mImageWidth    = (uint32_t)wid;
	mImageHeight   = (uint32_t)hei;
	mImageWidthSm  = (uint32_t)widSm;
	mImageHeightSm = (uint32_t)heiSm;

	if(mFileHandle != INVALID_HANDLE_VALUE)
	{
		BYTE Palette[1024];
		memset(Palette, 0, 1024);

		DWORD writtenBytes;
		WriteFile(mFileHandle, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &writtenBytes, nullptr);
		WriteFile(mFileHandle, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &writtenBytes, nullptr);

		WriteFile(mFileHandle, Palette, 1024, &writtenBytes, nullptr);

		mImageStrideBytes = (mImageWidth * sizeof(uint32_t) + 0x03) & (~(0x03)); //Next number divisible by 4
		mTempBuf.resize(mImageStrideBytes / sizeof(uint32_t) + 1);	
	}
}

void LOPictureWriter::WriteBeginning()
{
	switch(mSaveMode)
	{
	case PictureSaveMode::BORDERFUL:
		WriteWhiteLine();
		break;
	case PictureSaveMode::BORDERFUL_WHITE:
		WriteWhiteLine();
		break;
	case PictureSaveMode::BORDERLESS:
		break;
	case PictureSaveMode::BORDERLESS_BIG:
		break;
	}
}

void LOPictureWriter::WriteRow(const boost::dynamic_bitset<uint64_t>& row, int rowIndex)
{
	UINT si_si = mGameSize * mGameSize;
	std::cout << (rowIndex) * 100.0f / si_si << "%..." << std::endl;

	switch (mSaveMode)
	{
	case PictureSaveMode::BORDERFUL:
		WriteRowBorderful(row, rowIndex);
		break;
	case PictureSaveMode::BORDERFUL_WHITE:
		WriteRowBorderfulWhite(row, rowIndex);
		break;
	case PictureSaveMode::BORDERLESS:
		WriteRowBorderless(row);
		break;
	case PictureSaveMode::BORDERLESS_BIG:
		WriteRowBorderlessBig(row);
		break;
	}
}

void LOPictureWriter::WriteRowBorderful(const boost::dynamic_bitset<uint64_t>& row, int rowIndex)
{
	WriteEdgeLine();
	for(int i = 0; i < CELL_WID; i++)
	{
		WriteCellLine(row);
	}

	if((rowIndex + 1) % mGameSize == 0)
	{
		WriteEdgeLine();
		WriteWhiteLine();
	}
}

void LOPictureWriter::WriteRowBorderfulWhite(const boost::dynamic_bitset<uint64_t>& row, int rowIndex)
{
	for(int i = 0; i < CELL_WID; i++)
	{
		WriteCellLine(row);
	}

	if((rowIndex + 1) % mGameSize == 0)
	{
		WriteWhiteLine();
	}
}

void LOPictureWriter::WriteRowBorderless(const boost::dynamic_bitset<uint64_t>& row)
{
	WriteBorderlessCellLine(row);
}

void LOPictureWriter::WriteRowBorderlessBig(const boost::dynamic_bitset<uint64_t>& row)
{
	for(int i = 0; i < CELL_WID; i++)
	{
		WriteBorderlessCellLineBig(row);
	}
}

DWORD LOPictureWriter::WriteWhiteLine()
{
	if(mTempBuf.empty())
	{
		return 0;
	}

	uint32_t white = WHITE_ARGB;

	uint32_t* buf_prev = mTempBuf.data();
	for(uint32_t ix = 0; ix < mImageWidth; ix++)
	{
		*(buf_prev++) = white;
	}

	byte* buf_bytes = reinterpret_cast<byte*>(buf_prev);
	for(uint32_t pad = mImageWidth * sizeof(uint32_t); pad < mImageStrideBytes; pad++)
	{
		*(buf_bytes++) = 0;
	}

	DWORD writtenBytes = 0;
	WriteFile(mFileHandle, mTempBuf.data(), mImageStrideBytes, &writtenBytes, nullptr);

	return writtenBytes;
}

DWORD LOPictureWriter::WriteEdgeLine()
{
	if(mTempBuf.empty())
	{
		return 0;
	}

	uint32_t white = WHITE_ARGB;
	uint32_t black = BLACK_ARGB;

	uint32_t* buf_prev = mTempBuf.data();
	for (int ix = 0; ix < mGameSize; ix++)
	{
		*(buf_prev++) = white;

		for (int jx = 0; jx < mImageWidthSm; jx++)
		{
			*(buf_prev++) = black;
		}
	}
	*(buf_prev++) = white;

	byte* buf_bytes = reinterpret_cast<byte*>(buf_prev);
	for (uint32_t pad = mImageWidth * sizeof(uint32_t); pad < mImageStrideBytes; pad++)
	{
		*(buf_bytes++) = 0;
	}

	DWORD writtenBytes = 0;
	WriteFile(mFileHandle, mTempBuf.data(), mImageStrideBytes, &writtenBytes, nullptr);

	return writtenBytes;
}

DWORD LOPictureWriter::WriteCellLine(const boost::dynamic_bitset<uint64_t>& row)
{
	if(mTempBuf.empty())
	{
		return 0;
	}

	UINT white = WHITE_ARGB;
	UINT black = BLACK_ARGB;
	UINT red   = RED_ARGB;

	uint32_t* buf_prev = mTempBuf.data();
	for (int ix = 0; ix < mGameSize; ix++)
	{
		*(buf_prev++) = white;

		for (int jx = 0; jx < mGameSize; jx++)
		{
			*(buf_prev++) = black;

			for (int i = 0; i < CELL_WID; i++)
			{
				if(row[ix * mGameSize + jx])
				{
					*(buf_prev++) = red;
				}
				else
				{
					*(buf_prev++) = white;
				}
			}
		}
		*(buf_prev++) = black;
	}
	*(buf_prev++) = white;

	byte* buf_bytes = reinterpret_cast<byte*>(buf_prev);
	for(uint32_t pad = mImageWidth * sizeof(uint32_t); pad < mImageStrideBytes; pad++)
	{
		*(buf_bytes++) = 0;
	}

	DWORD writtenBytes = 0;
	WriteFile(mFileHandle, mTempBuf.data(), mImageStrideBytes, &writtenBytes, nullptr);

	return writtenBytes;
}

DWORD LOPictureWriter::WriteBorderlessCellLine(const boost::dynamic_bitset<uint64_t>& row)
{
	UINT white = WHITE_ARGB;
	UINT red   = RED_ARGB;

	uint32_t* buf_prev = mTempBuf.data();
	for(int ix = 0; ix < mGameSize; ix++)
	{
		for(int jx = 0; jx < mGameSize; jx++)
		{
			if(row[ix * mGameSize + jx])
			{
				*(buf_prev++) = red;
			}
			else
			{
				*(buf_prev++) = white;
			}
		}
	}

	byte* buf_bytes = reinterpret_cast<byte*>(buf_prev);
	for(uint32_t pad = mImageWidth * sizeof(uint32_t); pad < mImageStrideBytes; pad++)
	{
		*(buf_bytes++) = 0;
	}

	DWORD writtenBytes = 0;
	WriteFile(mFileHandle, mTempBuf.data(), mImageStrideBytes, &writtenBytes, nullptr);

	return writtenBytes;
}

DWORD LOPictureWriter::WriteBorderlessCellLineBig(const boost::dynamic_bitset<uint64_t>& row)
{
	UINT white = WHITE_ARGB;
	UINT red   = RED_ARGB;

	uint32_t* buf_prev = mTempBuf.data();
	for(int ix = 0; ix < mGameSize; ix++)
	{
		for(int jx = 0; jx < mGameSize; jx++)
		{
			for (int i = 0; i < CELL_WID; i++)
			{
				if (row[ix * mGameSize + jx])
				{
					*(buf_prev++) = red;
				}
				else
				{
					*(buf_prev++) = white;
				}
			}
		}
	}

	byte* buf_bytes = reinterpret_cast<byte*>(buf_prev);
	for(uint32_t pad = mImageWidth * sizeof(uint32_t); pad < mImageStrideBytes; pad++)
	{
		*(buf_bytes++) = 0;
	}

	DWORD writtenBytes = 0;
	WriteFile(mFileHandle, mTempBuf.data(), mImageStrideBytes, &writtenBytes, nullptr);

	return writtenBytes;
}
