#include <fstream>
#include <stdexcept>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

LangParser::~LangParser()
{
}

void LangParser::parse(const std::string &filename)
{
	std::ifstream input(filename);
	if (!input.good())
		throw std::runtime_error("invalid file: "+filename);

	parse(input);
}

void LangParser::parse(std::istream &file)
{
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file) );
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, *this));

	if (int err = _parser->parse()) {
		throw ParserError("parser returned: "+std::to_string(err));
	}
}

void LangParser::generateOutput(const std::string &filename) const
{
	std::ofstream output(filename);
	if (!output.good())
		throw std::runtime_error("invalid file: "+filename);

	generateOutput(filename);
}

void LangParser::generateOutput(std::ostream &output) const
{
	throw std::runtime_error("Not implemented.");
}

ParserError::ParserError(const std::string& msg):
	msg(msg)
{
}

const char* ParserError::what() const throw()
{
	return msg.c_str();
}
