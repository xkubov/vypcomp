#include <fstream>
#include <stdexcept>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

LangParser::LangParser():
	_indexRun(false)
{
	// Global symbol table.
	_tables.push_back(SymbolTable(true));
}

LangParser::LangParser(const SymbolTable& global):
	_indexRun(true)
{
	_tables.push_back(global);
}

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
		throw SyntaxError("parser returned: "+std::to_string(err));
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

void LangParser::addFunction(ir::Function::Ptr fun)
{
	if (searchTables(fun->name())) {
		// All redefinitions will be sorted out after index run.
		// In non-indexRun we must ignore "redefinitions" as we are slowly
		// replacing each function.
		if (_indexRun)
			throw SemanticError("Symbol "+fun->name()+" already defined.");
	}

	_tables.back().insert({fun->name(), fun});

	pushSymbolTable();
	for (auto arg: fun->args()) {
		//TODO: push args to bottom table.
	}
}

void LangParser::verify(const ir::AllocaInstruction::Ptr& decl)
{
	if (_indexRun) {
		return;
	}

	//TODO: if not index run we have all info needed in global table.
}

void LangParser::ensureMainDefined() const
{
	throw std::runtime_error("Not implemented.");
}

void LangParser::pushSymbolTable()
{
	throw std::runtime_error("Not implemented.");
}

void LangParser::popSymbolTable()
{
	// We must preserve global table.
	if (_tables.size() <= 1)
		return;

	throw std::runtime_error("Not implemented.");
}

void LangParser::leaveFunction()
{
	throw std::runtime_error("Not implemented.");
}

std::optional<SymbolTable::Symbol> LangParser::searchTables(const SymbolTable::Key& key) const
{
	for (auto it = _tables.rbegin(); it != _tables.rend(); ++it) {
		if (it->has(key))
			return it->get(key);
	}

	return {};
}

SyntaxError::SyntaxError(const std::string& msg):
	msg(msg)
{
}

const char* SyntaxError::what() const throw()
{
	return msg.c_str();
}


SemanticError::SemanticError(const std::string& msg):
	msg(msg)
{
}

const char* SemanticError::what() const throw()
{
	return msg.c_str();
}
