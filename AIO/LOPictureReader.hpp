#pragma once

#include <Windows.h>
#include <string>
#include <boost\dynamic_bitset.hpp>
#include "FileModes.hpp"

class LOPictureReader
{
public:
	LOPictureReader(const std::string& filename, uint32_t boardWidth, uint32_t boardHeight, PictureLoadMode loadMode);
	~LOPictureReader();

	void ReadMetadata();
	void ReadBeginning();
	void ReadNextRow(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);

	bool IsValidImage();

	uint32_t GetBoardSize() const;

private:
	void ReadNextRowRegular(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);
	void ReadNextRowSmall(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);
	void ReadNextRowSmallTor(boost::dynamic_bitset<uint64_t>& row, uint32_t rowIndex);

	void ReadSmallPicture();

private:
	HANDLE          mFileHandle;
	uint32_t        mBoardWidth;
	uint32_t        mBoardHeight;
	PictureLoadMode mLoadMode;

	uint32_t mImageWidth;
	uint32_t mImageHeight;
	uint32_t mImageWidthSm;
	uint32_t mImageHeightSm;

	uint32_t mImageStrideBytes;

	uint8_t mPixelByteSize;

	std::vector<byte> mTempBuf;
};