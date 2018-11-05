#pragma once

#include <boost/dynamic_bitset.hpp>
#include <Windows.h>

typedef std::vector<boost::dynamic_bitset<>> LOMatrix;

LOMatrix ReadBorderless();

void SaveBorder(LOMatrix mask, int size);