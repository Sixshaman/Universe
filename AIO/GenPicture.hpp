#pragma once

#include "Genmatrix.hpp"
#include "ToDiagonal.hpp"
#include <Windows.h>

LOMatrix ReadBorderless();
LOMatrix ReadBorderlessSm(int loSize);

void SaveBorder(LOMatrix mask, int size);
void SaveBorderless(LOMatrix mask, int size);