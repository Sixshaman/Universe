#include "GenPicture.hpp"
#include <iostream>

#define MAKE_ARGB(A, R, G, B) ((UINT)((A << 24) | (R << 16) | (G << 8) | (B << 0)))

#define RED_ARGB   MAKE_ARGB(0xff, 0xff, 0x00, 0x00)
#define BLACK_ARGB MAKE_ARGB(0xff, 0x00, 0x00, 0x00)
#define WHITE_ARGB MAKE_ARGB(0xff, 0xff, 0xff, 0xff)

#define CELL_WID 5

void WriteWhiteLine(HANDLE hFile, UINT width, UINT* buf, BYTE* Palette, DWORD &writtenBytes)
{
	UINT white = WHITE_ARGB;

	UINT* buf_prev = buf;
	for(int ix = 0; ix < width; ix++)
	{
		*(buf_prev++) = white;
	}

	WriteFile(hFile, buf, sizeof(UINT) * width, &writtenBytes, nullptr);
	WriteFile(hFile, Palette, (sizeof(UINT)* width) % 4, &writtenBytes, nullptr);
}

void WriteEdgeLine(HANDLE hFile, UINT width, UINT widthSm, UINT size, UINT* buf, BYTE* Palette, DWORD &writtenBytes)
{
	UINT white = WHITE_ARGB;
	UINT black = BLACK_ARGB;

	UINT* buf_prev = buf;
	for(int ix = 0; ix < size; ix++)
	{
		*(buf_prev++) = white;

		for(int jx = 0; jx < widthSm; jx++)
		{
			*(buf_prev++) = black;
		}
	}
	*(buf_prev++) = white;

	WriteFile(hFile, buf, sizeof(UINT) * width, &writtenBytes, nullptr);
	WriteFile(hFile, Palette, (sizeof(UINT)* width) % 4, &writtenBytes, nullptr);
}

void WriteCellLine(HANDLE hFile, UINT width, UINT size, LOMatrix& mat, UINT majY, UINT minY, UINT* buf, BYTE* Palette, DWORD &writtenBytes)
{
	UINT white = WHITE_ARGB;
	UINT black = BLACK_ARGB;
	UINT red   = RED_ARGB;

	UINT* buf_prev = buf;
	for(int ix = 0; ix < size; ix++)
	{
		UINT row = majY*size + ix;

		*(buf_prev++) = white;

		for(int jx = 0; jx < size; jx++)
		{
			UINT column = minY*size + jx;

			*(buf_prev++) = black;

			for(int i = 0; i < CELL_WID; i++)
			{
				if(mat[row][column])
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

	WriteFile(hFile, buf, sizeof(UINT) * width, &writtenBytes, nullptr);
	WriteFile(hFile, Palette, (sizeof(UINT)* width) % 4, &writtenBytes, nullptr);
}

void WriteBorderlessLine(HANDLE hFile, UINT width, UINT size, LOMatrix& mat, UINT majY, UINT minY, UINT* buf, BYTE* Palette, DWORD &writtenBytes)
{
	UINT white = WHITE_ARGB;
	UINT red   = RED_ARGB;

	UINT* buf_prev = buf;
	for(int ix = 0; ix < size; ix++)
	{
		UINT row = majY*size + ix;

		for(int jx = 0; jx < size; jx++)
		{
			UINT column = minY*size + jx;

			if (mat[row][column])
			{
				*(buf_prev++) = red;
			}
			else
			{
				*(buf_prev++) = white;
			}
		}
	}

	WriteFile(hFile, buf, sizeof(UINT) * width, &writtenBytes, nullptr);
	WriteFile(hFile, Palette, (sizeof(UINT)* width) % 4, &writtenBytes, nullptr);
}

LOMatrix ReadMatrixFromMemory(const std::vector<byte>& bytes, int width, int height, int losize, int bytesPerData, int stride)
{
	int si_si = losize * losize;

	LOMatrix matrix;
	matrix.resize(si_si);
	for(size_t i = 0; i < si_si; i++)
	{
		matrix[i].resize(si_si, 0);
	}

	if(bytesPerData != 3 && bytesPerData != 4)
	{
		return matrix;
	}

	for(int i = 0; i < height; i++)
	{
		const byte* memoryStart = bytes.data() + i * stride;

		for(int j = 0; j < width; j++)
		{
			const byte* pixelStart = memoryStart + j * bytesPerData;

			const byte b = *(pixelStart + 0);
			const byte g = *(pixelStart + 1);
			const byte r = *(pixelStart + 2);

			int matIBig = i / losize;
			int matJBig = j / losize;
			int matISm  = i % losize;
			int matJSm  = j % losize;

			int indexBig = matIBig * losize + matJBig;
			int indexSm  = matISm  * losize + matJSm;
			matrix[indexBig].set(indexSm, g < 120);
		}
	}

	return matrix;
}

LOMatrix ReadMatrixSmFromMemory(const std::vector<byte>& bytes, int width, int height, int loSize, int bytesPerData, int stride)
{
	int si_si = loSize * loSize;

	LOMatrix matrix;
	matrix.resize(si_si);
	for(size_t i = 0; i < si_si; i++)
	{
		matrix[i].resize(si_si, 0);
	}

	if(bytesPerData != 3 && bytesPerData != 4)
	{
		return matrix;
	}

	std::vector<boost::dynamic_bitset<>> factor;
	factor.resize(height);
	for(int i = 0; i < factor.size(); i++)
	{
		factor[i].resize(width, 0);
	}

	for(int i = 0; i < height; i++)
	{
		const byte* memoryStart = bytes.data() + i * stride;

		for(int j = 0; j < width; j++)
		{
			const byte* pixelStart = memoryStart + j * bytesPerData;

			const byte b = *(pixelStart + 0);
			const byte g = *(pixelStart + 1);
			const byte r = *(pixelStart + 2);

			factor[i].set(j, g < 120);
		}
	}

	int iFacCenter  = height / 2;
	int jFacCenter  = width / 2;
	int indexCenter = iFacCenter * width + jFacCenter;

	for(int i = 0; i < si_si; i++)
	{
		for(int j = 0; j < si_si; j++)
		{
			int iBig = i / loSize;
			int jBig = j / loSize;
			int iSm  = i % loSize;
			int jSm  = j % loSize;

			int indexBig = iBig * loSize + jBig;
			int indexSm  = iSm  * loSize + jSm;
		
			if(abs(jSm - jBig) > jFacCenter || abs(iSm - iBig) > iFacCenter)
			{
				matrix[indexBig].set(indexSm, 0);
			}
			else
			{
				int iReset = iSm - iBig + iFacCenter;
				int jReset = jSm - jBig + jFacCenter;
				matrix[indexBig].set(indexSm, factor[iReset][jReset]);
			}
		}
	}

	return matrix;
}

void SaveBorder(LOMatrix mask, int size)
{
	HANDLE hFile;
	int widSm = (CELL_WID * size + size + 1);
	int wid   = widSm*size + size + 1;
	int hei   = wid;

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; 
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + sizeof(UINT) * wid * hei + hei * (sizeof (UINT)* wid) % 4;
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

	hFile = CreateFile(L"Am.bmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Cannot create file", L"Error", MB_OK);
		return;
	}

	BYTE Palette[1024];  
	memset(Palette, 0, 1024);

	DWORD writtenBytes;
	WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &writtenBytes, nullptr);
	WriteFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &writtenBytes, nullptr);

	WriteFile(hFile, Palette, 1024, &writtenBytes, nullptr);

	UINT* buf = new UINT[wid];

	UINT si_si = size*size;
	for(int iy = 0; iy < size; iy++)
	{
		WriteWhiteLine(hFile, wid, buf, Palette, writtenBytes);

		for(int jy = 0; jy < size; jy++)
		{
			std::cout << (iy*size + jy) * 100.0f / si_si << "%..." << std::endl;

			WriteEdgeLine(hFile, wid, widSm, size, buf, Palette, writtenBytes);

			for(int i = 0; i < CELL_WID; i++)
			{
				WriteCellLine(hFile, wid, size, mask, iy, jy, buf, Palette, writtenBytes);
			}
		}

		WriteEdgeLine(hFile, wid, widSm, size, buf, Palette, writtenBytes);
	}
	WriteWhiteLine(hFile, wid, buf, Palette, writtenBytes);

	CloseHandle(hFile);
	delete[] buf;
}

void SaveBorderless(LOMatrix mask, int size)
{
	HANDLE hFile;
	int widSm = size;
	int wid   = widSm*size;
	int hei   = wid;

	BITMAPFILEHEADER bmpFileHeader;
	bmpFileHeader.bfType = 0x4D42;
	bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 1024; 
	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + sizeof(UINT) * wid * hei + hei * (sizeof (UINT)* wid) % 4;
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

	hFile = CreateFile(L"Am.bmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(nullptr, L"Cannot create file", L"Error", MB_OK);
		return;
	}

	BYTE Palette[1024];  
	memset(Palette, 0, 1024);

	DWORD writtenBytes;
	WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &writtenBytes, nullptr);
	WriteFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &writtenBytes, nullptr);

	WriteFile(hFile, Palette, 1024, &writtenBytes, nullptr);

	UINT* buf = new UINT[wid];

	UINT si_si = size*size;
	for(int iy = 0; iy < size; iy++)
	{
		for(int jy = 0; jy < size; jy++)
		{
			std::cout << (iy*size + jy) * 100.0f / si_si << "%..." << std::endl;
			WriteBorderlessLine(hFile, wid, size, mask, iy, jy, buf, Palette, writtenBytes);
		}
	}

	CloseHandle(hFile);
	delete[] buf;
}

LOMatrix ReadBorderless()
{
	LOMatrix result;

	HANDLE hFile = CreateFile(L"Ma.bmp", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "File doesn\'t exist!" << std::endl;
	}

	BITMAPFILEHEADER bmpFileHeader;
	if(!ReadFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
	else
	{
		BITMAPINFOHEADER bmpInfoHeader;
		if(!ReadFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), nullptr, nullptr))
		{
			std::cout << "Error reading file!" << std::endl;
		}
		else
		{
			int bitCount = bmpInfoHeader.biBitCount;
			if(bitCount != 32 && bitCount != 24)
			{
				std::cout << "Wrong file type!" << std::endl;
			}
			else
			{
				int width  = abs(bmpInfoHeader.biWidth);
				int height = abs(bmpInfoHeader.biHeight);

				int loSize = int(sqrtf(width));
				if(width != height || loSize * loSize != width)
				{
					std::cout << "Wrong file size!" << std::endl;
				}
				else
				{
					size_t offBits = bmpFileHeader.bfOffBits - sizeof(BITMAPINFOHEADER) - sizeof(BITMAPFILEHEADER);
					std::vector<char> ignoreBytes(offBits);

					if(offBits != 0 && !ReadFile(hFile, ignoreBytes.data(), offBits, nullptr, nullptr))
					{
						std::cout << "Error reading file!" << std::endl;
					}
					else
					{
						int widthSized = (bitCount / 8) * width;
						int stride     = (widthSized + 3) & (~3);
						std::vector<byte> bitmapData(stride * height);
						
						if(!ReadFile(hFile, bitmapData.data(), bitmapData.size(), nullptr, nullptr))
						{
							std::cout << "Error reading file!" << std::endl;
						}
						else
						{
							if(bmpInfoHeader.biHeight > 0)
							{
								std::vector<byte> bitmapDataReversed(bitmapData.size());
								for(size_t i = 0; i < height; i++)
								{
									int iR = height - i - 1;
									memcpy_s(bitmapDataReversed.data() + i * stride, bitmapDataReversed.size() - (i * stride), bitmapData.data() + iR * stride, stride);
								}

								bitmapData = bitmapDataReversed;
							}

							result = ReadMatrixFromMemory(bitmapData, width, height, loSize, bitCount / 8, stride);
						}
					}
				}
			}
		}
	}

	CloseHandle(hFile);
	return result;
}

LOMatrix ReadBorderlessSm(int loSize)
{
	LOMatrix result;

	HANDLE hFile = CreateFile(L"Maa.bmp", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "File doesn\'t exist!" << std::endl;
	}

	BITMAPFILEHEADER bmpFileHeader;
	if(!ReadFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), nullptr, nullptr))
	{
		std::cout << "Error reading file!" << std::endl;
	}
	else
	{
		BITMAPINFOHEADER bmpInfoHeader;
		if(!ReadFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), nullptr, nullptr))
		{
			std::cout << "Error reading file!" << std::endl;
		}
		else
		{
			int bitCount = bmpInfoHeader.biBitCount;
			if(bitCount != 32 && bitCount != 24)
			{
				std::cout << "Wrong file type!" << std::endl;
			}
			else
			{
				int width  = abs(bmpInfoHeader.biWidth);
				int height = abs(bmpInfoHeader.biHeight);

				if(width != height && width % 2 == 0)
				{
					std::cout << "Wrong file size!" << std::endl;
				}
				else
				{
					size_t offBits = bmpFileHeader.bfOffBits - sizeof(BITMAPINFOHEADER) - sizeof(BITMAPFILEHEADER);
					std::vector<char> ignoreBytes(offBits);

					if(offBits != 0 && !ReadFile(hFile, ignoreBytes.data(), offBits, nullptr, nullptr))
					{
						std::cout << "Error reading file!" << std::endl;
					}
					else
					{
						int widthBytes = (bitCount / 8) * width;
						int stride     = (widthBytes + 3) & (~3);
						std::vector<byte> bitmapData(stride * height);
						
						if(!ReadFile(hFile, bitmapData.data(), bitmapData.size(), nullptr, nullptr))
						{
							std::cout << "Error reading file!" << std::endl;
						}
						else
						{
							if(bmpInfoHeader.biHeight > 0)
							{
								std::vector<byte> bitmapDataReversed(bitmapData.size());
								for(size_t i = 0; i < height; i++)
								{
									int iR = height - i - 1;
									memcpy_s(bitmapDataReversed.data() + i * stride, bitmapDataReversed.size() - (i * stride), bitmapData.data() + iR * stride, stride);
								}

								bitmapData = bitmapDataReversed;
							}

							result = ReadMatrixSmFromMemory(bitmapData, width, height, loSize, bitCount / 8, stride);
						}
					}
				}
			}
		}
	}

	CloseHandle(hFile);
	return result;
}