/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

#include <iostream>
#include <string>

#include "vypcomp/parser/parser.h"
#include "vypcomp/parser/indexdriver.h"
#include "vypcomp/generator/generator.h"

using namespace vypcomp;

struct Args {
	std::string inputFile = "";
	std::string outputFile = "out.vc";
	bool verbose = false;

        static std::string usage(const std::string& name) {
		return name+": [-v|--verbose] FILE [FILE]";
	}

        static Args parse(int argc, char** argv) {
		Args args;
		if (argc == 1)
			throw std::runtime_error("expected arguments\n"+ Args::usage(std::string(argv[0])));

		int base = 1;
		if (std::string(argv[1]) == "-v" || std::string(argv[1]) == "--verbose") {
			args.verbose = true;
			base = 2;
		}

		if (argc < base+1)
			throw std::runtime_error("invalid arguments\n"+Args::usage(std::string(argv[0])));

		if (argc > base+1)
			args.outputFile = argv[base+1];
		args.inputFile = argv[base];
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

		// Debug: print intermediet representation to the
		// stdout.
		if (args.verbose) {
			for (auto [_, v]: parser.table().data()) {
				std::visit([](auto&& arg) {
					std::cout << arg->str("");
				}, v);
				std::cout << std::endl;
			}
		}

		Generator gen(args.outputFile, args.verbose);
		gen.generate(parser.table());
	} catch (const LexicalError &le) {
		std::cerr << "lexical error: " << le.what() << std::endl;
		return 11;
	} catch (const SyntaxError &pe) {
		std::cerr << "syntax error: " << pe.what() << std::endl;
		return 12;
	} catch (const IncompabilityError &pe) {
		std::cerr << "semantic error: " << pe.what() << std::endl;
		return 13;
	} catch (const SemanticError &pe) {
		std::cerr << "semantic error: " << pe.what() << std::endl;
		return 14;
	} catch (const std::exception &e) {
		std::cerr << "error: " << e.what() << std::endl;
		return 19;
	}

	return 0;
}
