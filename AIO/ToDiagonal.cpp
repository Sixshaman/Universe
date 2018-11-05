#include "ToDiagonal.hpp"
#include <iostream>

LOMatrix ToDiag(LOMatrix base)
{
	int size_size = base.size();
	LOMatrix elementary;

	for(int i=0; i<size_size; i++)
	{
		elementary.push_back(boost::dynamic_bitset<>(size_size));
		elementary[i][i] = 1;
	}

	for(int i=0; i<size_size; i++)
	{
		if(!base[i][i])
		{
			for(int j=i+1; j<size_size; j++)
			{
				if(base[j][i])
				{
					swap(base[i], base[j]);
					swap(elementary[i], elementary[j]);
					break;
				}
			}
		}
		for(int j=i+1; j<size_size; j++)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
				elementary[j] ^= elementary[i];
			}
		}
	}

	for(int i=size_size-1; i>=0; i--)
	{
		for(int j=i-1; j>=0; j--)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
				elementary[j] ^= elementary[i];
			}
		}
	}

	return elementary;
}

bool CheckInv(LOMatrix base)
{
	int size_size = base.size();

	for(int i=0; i<size_size; i++)
	{
		if(!base[i][i])
		{
			for(int j=i+1; j<size_size; j++)
			{
				if(base[j][i])
				{
					swap(base[i], base[j]);
					break;
				}
			}
		}
		for(int j=i+1; j<size_size; j++)
		{
			if(base[j][i])
			{
				base[j] ^= base[i];
			}
		}
	}

	for (int i = size_size - 1; i >= 0; i--)
	{
		for (int j = i - 1; j >= 0; j--)
		{
			if (base[j][i])
			{
				base[j] ^= base[i];
			}
		}
	}

	return base.back().any();
}
