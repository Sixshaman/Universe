#pragma once

#include <vector>
#include <boost/dynamic_bitset.hpp>

typedef std::vector<boost::dynamic_bitset<>> LOMatrix;

LOMatrix GetMatrixLO(int size);
LOMatrix GetMatrixLT(int size);