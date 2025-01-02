#include <iostream>

int main()
{
	int position = 8;
	char* str = new char[9];
	std::cout << sizeof str << '\n';
	std::cout << position << '\n';
	delete[] str;
	return 0;
}
