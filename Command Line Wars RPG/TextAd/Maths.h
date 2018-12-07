#pragma once

int ClampVal(int num, int min, int max)
{
	if (num < min)
		num = min;

	if (num > max)
		num = max;

	return num;
}