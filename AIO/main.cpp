#include <iostream>
#include "LOMatrix.hpp"

enum LOMode
{
	LIGHTS_OUT   = 0,
	TOROID_LOUT  = 1,
	LIGHTS_TROUT = 2
};

int main(int argc, char *argv[])
{
	int size_matrix  =  0;
	int power_matrix =  1;
	int mode         = -1;

	std::cout << "Enter size. Enter -1 to check for normal solvability and -2 to check for toroidal solvability" << std::endl;
	std::cin >> size_matrix;

	std::cout << "Enter matrix power" << std::endl;
	std::cin >> power_matrix;

	if(size_matrix < 0)
	{
		if (size_matrix == -1)
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
		else if (size_matrix == -2)
		{
			for (int i = 1; i <= 250; i++)
			{
				LOMatrix mat;
				mat.LoadToroid(L"Maa.bmp", i);
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
		std::cout << "Enter mode."                                                          << "\n"
			      << "LO - Normal Lights Out, TO - Toroidal Lights Out, LT - Lights Trout." << "\n"
			      << "I - Inverse, A - Direct."                                             << "\n"
			      << "VE - With Borders, EL - Borderless."                                  << "\n"
			      << "0,   1,   2,   3 - LOIVE, LOIEL, LOAVE, LOAEL respectively."          << "\n"
			      << "4,   5,   6,   7 - TOIVE, TOIEL, TOAVE, TOAEL respectively."          << "\n"
			      << "8,   9,  10,  11 - LTIVE, LTIEL, LTAVE, LTAEL respectively."          << std::endl;

		std::cin >> mode;

		if (mode < 0 || mode > 11)
		{
			std::cout << "Unknown mode!" << std::endl;
		}
		else
		{
			LOMatrix mat;

			LOMode loMode = (LOMode)(mode / 4);
			bool   useA   = ((mode % 4) >> 1) & 0x01;
			bool   useEL  = ((mode % 4) >> 0) & 0x01;

			if(loMode == LIGHTS_OUT)
			{
				mat.Load(L"Maa.bmp", size_matrix);
			}
			else if(loMode == TOROID_LOUT)
			{
				mat.LoadToroid(L"Maa.bmp", size_matrix);
			}
			else if(loMode == LIGHTS_TROUT)
			{
				mat.LoadBig(L"Ma.bmp");
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

			LOMatrix mulMat = mat;
			for(int i = 0; i < power_matrix - 1; i++)
			{
				mulMat.Mul(mat);
			}

			if(!useEL)
			{
				mulMat.Save(filename);
			}
			else
			{
				mulMat.SaveBorderless(filename);
			}

			std::cout << "Saved succesfully! Completed." << std::endl;
		}
	}

	system("pause");
	return 0;
}
