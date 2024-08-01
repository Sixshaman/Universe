#include <iostream>
#include "LOMatrix.hpp"
#include <bit>

enum LOMode
{
	LIGHTS_OUT   = 0,
	TOROID_LOUT  = 1,
	LIGHTS_TROUT = 2
};

bool VerifySolutionPeriod(uint32_t gameSize, const boost::multiprecision::cpp_int& solutionPeriod)
{
	//Verify solution period
	if(solutionPeriod == 1)
	{
		return gameSize == 1; //Only possible for 1x1 board
	}
	else
	{
		LOMatrix mat;
		assert(solutionPeriod % 2 == 0);
		auto solutionPeriodHalf = solutionPeriod / 2;
		mat.Load(L"Maa.bmp", gameSize);

		//Fast multiplication at the expense of space
		std::vector<LOMatrix> solutionPeriodMatricesPerBit;
		if(solutionPeriodHalf & 1)
		{
			solutionPeriodMatricesPerBit.push_back(mat);
		}

		LOMatrix prevMatrix = mat;
		boost::multiprecision::cpp_int solutionPeriodMatrixBit = 2; //Power 1 is already checked
		while(solutionPeriodMatrixBit <= solutionPeriodHalf)
		{
			//Calculate next power of 2 matrix
			LOMatrix currMatrix = prevMatrix;
			currMatrix.Mul(prevMatrix);

			if(solutionPeriodHalf & solutionPeriodMatrixBit)
			{
				solutionPeriodMatricesPerBit.push_back(currMatrix);
			}

			prevMatrix = currMatrix;
			solutionPeriodMatrixBit = (solutionPeriodMatrixBit << 1);
		}

		LOMatrix finalMatrixSqrt;
		finalMatrixSqrt.SetIdentity(gameSize);
		for(const LOMatrix& matrix: solutionPeriodMatricesPerBit)
		{
			finalMatrixSqrt.Mul(matrix);
		}

		solutionPeriodMatricesPerBit.clear();

		boost::dynamic_bitset<uint64_t> vectorTest(gameSize * gameSize, 0);
		vectorTest.set(0, true);

		//Test A*b = A*A*b (correct solution period check both for solvable and unsolvable board sizes)
		//Test Sqrt(A)*b != A*B (to verify this is indeed the minimal solution period)
		LOMatrix finalMatrix = finalMatrixSqrt;
		finalMatrix.Mul(finalMatrixSqrt);

		LOMatrix finalMatrixSquared = finalMatrix;
		finalMatrixSquared.Mul(finalMatrix);

		boost::dynamic_bitset<uint64_t> vectorResSqrt = finalMatrixSqrt.MulBoard(vectorTest);
		boost::dynamic_bitset<uint64_t> vectorRes = finalMatrix.MulBoard(vectorTest);
		boost::dynamic_bitset<uint64_t> vectorResSquared = finalMatrixSquared.MulBoard(vectorTest);
		return (vectorResSqrt != vectorRes) && (vectorRes == vectorResSquared);
	}
}

//FOR N = 999, THE PREDICTED SOLUTION PERIOD IS COMPARATIVELY SMALL!
//JUST 18 QUADRILLION!!!
//FOR N = 100, FOR EXAMPLE, IT'S 2 QUADRILLION, JUST 4 TIMES SMALLER!
//Of course, there are HUGE numbers. For N = 990, THE SOLUTION PERIOD IS 2 novemquadragintillions!!!
//The smallest solution period around 1000 is for N = 991. This solution period is literally 1984.
//ANOTHER INTERESTING FACT!!!!!!!!!!!!
//The values n > 5 such that nxn solution period is THE LARGEST ONE SO FAR: 6, 10, 12, 18, 22, 28, 36, 46, 52, 58, 60, 66, 70, 78, 82, 100, 102...
//If you divide this by 2, YOU'LL GET QUENEAU NUMBERS: https://oeis.org/A054639 
int main(int argc, char *argv[])
{
	int size_matrix  =  0;
	int power_matrix =  1;
	int mode         = -1;

	std::cout << "Enter size. Enter -1 to check for normal solvability and -2 to check for toroidal solvability, and -3 to check SOLUTION PERIODS" << std::endl;
	std::cin >> size_matrix;

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
		else if(size_matrix == -3)
		{
			for (int i = 1; i <= 256; i++)
			{
				LOMatrix mat;
				auto solutionPeriod = mat.FindSolutionPeriod(i);
				std::cout << "SOLUTION PERIOD FOR " << i << "x" << i << ": " << mat.FindSolutionPeriod(i) << std::endl;

				//if(!VerifySolutionPeriod(i, solutionPeriod))
				//{
				//	std::cout << "SOLUTION PERIOD VALIDATION ERROR" << std::endl;
				//}
			}
		}
		else
		{
			std::cout << "Unknown mode!" << std::endl;
		}
	}
	else
	{
		std::cout << "Enter matrix power" << std::endl;
		std::cin >> power_matrix;

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

			LOMatrix mulMat;
			if(power_matrix == 1)
			{
				mulMat = mat;
			}
			else
			{
				uint32_t totalMatrixPower = 0;
				mulMat.SetIdentity(size_matrix);

				//Store 5 matrices at most; at 256x256 board size, the matrix requires 4GB of memory
				uint32_t remainder = power_matrix;
				while(remainder != 0)
				{
					uint32_t currPowerRequired = std::bit_floor(remainder);
					uint32_t currMatrixPower = 1;

					LOMatrix currMatrix = mat;
					while(currMatrixPower < currPowerRequired)
					{
						currMatrix.Mul(currMatrix);
						currMatrixPower *= 2;

						std::cout << "Calculated power of " << currMatrixPower << std::endl;
					}

					mulMat.Mul(currMatrix);
					remainder -= currMatrixPower;
					totalMatrixPower += currMatrixPower;

					std::cout << "Calculated power of " << totalMatrixPower << std::endl;
				}
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
