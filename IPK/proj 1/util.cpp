#include "util.h"

bool stringToNum(const std::string &str, int &num)
{
	char *endptr;
	// Work with decadic format
	num = strtol(str.c_str(), &endptr, 10);
	if (endptr != str.c_str() + str.length() * sizeof(char))
		return false;
	return true;
}


