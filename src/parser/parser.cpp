#include "vypcomp/parser/parser.h"

using namespace vypcomp;

LangParser::~LangParser()
{
}

void LangParser::parse(const std::string &filename)
{
	throw std::runtime_error("Not implemented.");
}

void LangParser::parse(std::istream &file)
{
	throw std::runtime_error("Not implemented.");
}
