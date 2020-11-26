#include <iostream>
#include <string>

#include "vypcomp/parser/parser.h"
#include "vypcomp/parser/indexdriver.h"

using namespace vypcomp;

struct Args {
	std::string inputFile = "";
	std::string outputFile = "out.vc";

        static std::string usage(const std::string& name) {
		return name+": FILE [FILE]";
	}

        static Args parse(int argc, char** argv) {
		Args args;
		if (argc < 2)
			throw std::runtime_error("invalid arguments.\n"+Args::usage(std::string(argv[0])));

		if (argc > 2)
			args.outputFile = argv[2];
		args.inputFile = argv[1];
		return args;
	}
};

int main(int argc, char** argv)
{
	try {
		auto args = Args::parse(argc, argv);
		IndexParserDriver indexRun;
		indexRun.parse(args.inputFile);
		ParserDriver parser(indexRun.table());
		parser.parse(args.inputFile);
	} catch (const LexicalError &le) {
		std::cerr << "lexical error: " << le.what() << std::endl;
		return 11;
	} catch (const SyntaxError &pe) {
		std::cerr << "syntax error: " << pe.what() << std::endl;
		return 12;
	} catch (const std::exception &e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 19;
	}

	return 0;
}
