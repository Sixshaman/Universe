#include <iostream>
#include "ReadPicture.hpp"

int main(int argc, char* argv[])
{
	LOMatrix matrix = ReadBorderless();

	int size = int(sqrtf(matrix.size()));
	SaveBorder(matrix, size);

	return std::cin.get();
}