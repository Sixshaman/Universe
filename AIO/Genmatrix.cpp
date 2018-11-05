#include "Genmatrix.hpp"

LOMatrix GetMatrixLO(int size)
{
	int si_si = size*size;
	LOMatrix lightMatrix;

	for(int i = 0; i<si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<>(si_si));
	}

	for(int i = 0; i<si_si; i++)
	{
		for(int j = 0; j<si_si; j++)
		{
			if((i == j) || ((i == j - 1) && (i + 1) % size) || ((i == j + 1) && (i%size)) || (i == j - size) || (i == j + size))
				lightMatrix[i][j] = 1;
			else lightMatrix[i][j] = 0;
		}
	}
	return lightMatrix;
}

LOMatrix GetMatrixLT(int size) //Lights trout
{
	int si_si = size*size;
	LOMatrix lightMatrix;

	for(int i = 0; i<si_si; i++)
	{
		lightMatrix.push_back(boost::dynamic_bitset<>(si_si));
	}

	for(int i = 0; i < si_si; i++)
	{
		for(int j = 0; j < si_si; j++)
		{
			if((i % size == j % size) || (i / size == j / size))
				lightMatrix[i][j] = 1;
			else lightMatrix[i][j] = 0;
		}
	}
	return lightMatrix;
}