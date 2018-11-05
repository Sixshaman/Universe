#pragma once

#include <Windows.h>
#include <string>
#include <boost\dynamic_bitset.hpp>

//LOMatrix ReadBorderless();
//LOMatrix ReadBorderlessSm(int loSize);
//
//void SaveBorder(LOMatrix mask, int size);
//void SaveBorderless(LOMatrix mask, int size);

enum class PictureSaveMode
{
	BORDERFUL,
	BORDERFUL_WHITE,
	BORDERLESS,
	BORDERLESS_BIG
};

class LOPictureWriter
{
public:
	LOPictureWriter(const std::wstring& filename, uint32_t gamesize, PictureSaveMode mode);
	~LOPictureWriter();

	void WriteMetadata();
	void WriteBeginning();
	void WriteRow(const boost::dynamic_bitset<uint64_t>& row, int rowIndex); 

private:
	void WriteRowBorderful(const boost::dynamic_bitset<uint64_t>& row, int rowIndex);
	void WriteRowBorderfulWhite(const boost::dynamic_bitset<uint64_t>& row, int rowIndex);
	void WriteRowBorderless(const boost::dynamic_bitset<uint64_t>& row);
	void WriteRowBorderlessBig(const boost::dynamic_bitset<uint64_t>& row);

	DWORD WriteWhiteLine();
	DWORD WriteEdgeLine();
	DWORD WriteCellLine(const boost::dynamic_bitset<uint64_t>& row);
	DWORD WriteBorderlessCellLine(const boost::dynamic_bitset<uint64_t>& row);
	DWORD WriteBorderlessCellLineBig(const boost::dynamic_bitset<uint64_t>& row);

private:
	HANDLE          mFileHandle;
	uint32_t        mGameSize;
	PictureSaveMode mSaveMode;

	uint32_t mImageWidth;
	uint32_t mImageHeight;
	uint32_t mImageWidthSm;
	uint32_t mImageHeightSm;

	uint32_t mImageStrideBytes;

	std::vector<uint32_t> mTempBuf;
};