#include <iostream>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

int main(int argc, char *argv[])
{
	try {
		LangParser parser;
		parser.parse(std::cin);
	}
	catch (const LexicalError& le) {
		std::cerr << "lexical error: " << le.what() << std::endl;
		return 11;
	}
	catch (const ParserError& pe) {
		std::cerr << "syntax error: " << pe.what() << std::endl;
		return 12;
	}
	catch (const std::exception& e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 19;
	}

	return 0;
}
