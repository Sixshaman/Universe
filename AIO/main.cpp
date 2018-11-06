#include <iostream>
#include "LOMatrix.hpp"

int main(int argc, char *argv[])
{
	int size_matrix = 0;
	int mode        = -1;

	std::cout << "Enter size. Enter -1 to check for solvability" << std::endl;
	std::cin >> size_matrix;

	//LO for Lights Out, LT for Lights Trout, I for Inverse, A for Averse, VE for With Edges, EL for Edgeless
	std::cout << "Enter mode. 0 - LOIVE, 1 - LTIVE, 2 - LOAVE, 3 - LTAVE, 4 - LOIEL, 5 - LTIEL, 6 - LOAEL, 7 - LTAEL" << std::endl;
	std::cin >> mode;

	if (mode < 0 || mode > 7)
	{
		if(mode == 8)
		{
			std::cout << "SECRET mode!" << std::endl;

			//std::vector< boost::dynamic_bitset<> > mat;
			//mat = ReadBorderless();

			//int size = int(sqrtf(mat.size()));

			//mat = ToDiag(mat);

			//if(size_matrix == size)
			//{
			//	SaveBorder(mat, size);
			//}
			//else
			//{
			//	SaveBorderless(mat, size);
			//}
			//
			//std::cout << "Saved succesfully! Completed." << std::endl;
		}
		else if (mode == 11)
		{
			for (int i = 1; i <= 250; i++)
			{
				LOMatrix mat;
				mat.Load(L"Maa.bmp", i);
				uint32_t qPattSize = mat.CheckInv();

				if(qPattSize == 0)
				{
					std::cout << i << " IS SOLVABLE" << std::endl;
				}
				else
				{
					std::cout << i << " IS UNSOLVABLE(" << qPattSize << ")" << std::endl;
				}
			}
		}
		else
		{
			std::cout << "Unknown mode!" << std::endl;
		}
	}
	else
	{
		LOMatrix mat;

		bool useLT = (mode >> 0) & 0x01;
		bool useA  = (mode >> 1) & 0x01;
		bool useEL = (mode >> 2) & 0x01;

		if(!useLT)
		{
			mat.Load(L"Maa.bmp", size_matrix);
		}
		else
		{
			std::cout << "Not supported" << std::endl;
		}

		std::cout << "Generated succesfully..." << std::endl;

		std::wstring filename;

		if(!useA)
		{
			mat = mat.Inverto();
			filename = L"Am.bmp";
			std::cout << "Diagonalized succesfully..." << std::endl;
		}
		else
		{
			filename = L"Ma.bmp";
		}

		if(!useEL)
		{
			mat.Save(filename);
		}
		else
		{
			mat.SaveBorderless(filename);
		}

		std::cout << "Saved succesfully! Completed." << std::endl;
	}

	system("pause");
	return 0;
}
