#include <iostream>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

int main(int argc, char *argv[])
{
	LangParser parser;
	parser.parse(std::cin);

	return 0;
}
