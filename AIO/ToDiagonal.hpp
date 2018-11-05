#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include "Genmatrix.hpp"

LOMatrix ToDiag(LOMatrix base);

bool CheckInv(LOMatrix base);