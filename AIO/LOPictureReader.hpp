#pragma once

#include <Windows.h>
#include <string>
#include <boost\dynamic_bitset.hpp>
#include "FileModes.hpp"

class LOPictureReader
{
public:
	LOPictureReader(const std::wstring& filename, uint32_t gamesize, PictureLoadMode loadMode);
	~LOPictureReader();

	void ReadMetadata();
	void ReadBeginning();
	void ReadNextRow(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);

	bool IsValidImage();

	uint32_t GetGameSize() const;

private:
	void ReadNextRowRegular(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);
	void ReadNextRowSmall(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);
	void ReadNextRowSmallTor(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);

	void ReadSmallPicture();

	int mod(int a, int b);

private:
	HANDLE          mFileHandle;
	uint32_t        mGameSize;
	PictureLoadMode mLoadMode;

	uint32_t mImageWidth;
	uint32_t mImageHeight;
	uint32_t mImageWidthSm;
	uint32_t mImageHeightSm;

	uint32_t mImageStrideBytes;

	uint8_t mPixelByteSize;

	std::vector<byte> mTempBuf;
};