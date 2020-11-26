#include <fstream>
#include <stdexcept>
#include <variant>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

ParserDriver::ParserDriver():
	_indexRun(false)
{
	// Global symbol table.
	_tables.push_back(SymbolTable(true));
}

ParserDriver::ParserDriver(const SymbolTable& global):
	_indexRun(true)
{
	_tables.push_back(global);
}

ParserDriver::~ParserDriver()
{
}

Class::Ptr ParserDriver::getClass(const std::string &name)
{
	if (auto symbol = searchTables(name)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol)) {
			std::string msg = std::holds_alternative<Function::Ptr>(*symbol) ?
					"cannot derive from function" :
					"invalid derivation of class";

			throw SemanticError(msg);
		}

		return std::get<Class::Ptr>(*symbol);
	}

	if (_indexRun) {
	}
	throw std::runtime_error("Not implemented");

	return nullptr;
}

void ParserDriver::parse(const std::string &filename)
{
	std::ifstream input(filename);
	if (!input.good())
		throw std::runtime_error("invalid file: "+filename);

	parse(input);
}

void ParserDriver::parse(std::istream &file)
{
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file) );
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, *this));

	if (int err = _parser->parse()) {
		throw SyntaxError("parser returned: "+std::to_string(err));
	}
}

void ParserDriver::generateOutput(const std::string &filename) const
{
	std::ofstream output(filename);
	if (!output.good())
		throw std::runtime_error("invalid file: "+filename);

	generateOutput(filename);
}

void ParserDriver::generateOutput(std::ostream &output) const
{
	throw std::runtime_error("Not implemented.");
}

void ParserDriver::parseStart(ir::Function::Ptr fun)
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

void ParserDriver::parseStart(ir::Class::Ptr cl)
{
	if (searchTables(cl->name())) {
		// All redefinitions will be sorted out after index run.
		// In non-indexRun we must ignore "redefinitions" as we are slowly
		// replacing each function.
		if (_indexRun)
			throw SemanticError("Symbol "+cl->name()+" already defined.");
	}

	_tables.back().insert({cl->name(), cl});
	pushSymbolTable(true);
	_currClass = cl;
}

void ParserDriver::verify(const ir::AllocaInstruction::Ptr& decl)
{
	if (_indexRun) {
		return;
	}

	//TODO: if not index run we have all info needed in global table.
	if (searchTables(decl->name())) {
		throw SyntaxError("Redefinition of "+decl->name());
	}
}

void ParserDriver::add(const ir::AllocaInstruction::Ptr &decl)
{
	verify(decl);
	_tables.front().insert({decl->name(), decl});
}

void ParserDriver::ensureMainDefined() const
{
	if (searchTables("main"))
		return;

	throw SemanticError("Main not defined.");
}

void ParserDriver::pushSymbolTable(bool storeFunctions)
{
	_tables.push_back(SymbolTable(storeFunctions));
}

void ParserDriver::popSymbolTable()
{
	// We must preserve global table.
	if (_tables.size() <= 1)
		return;

	_tables.pop_back();
}

void ParserDriver::parseEnd()
{
	popSymbolTable();
	_currClass = nullptr;
}

std::optional<SymbolTable::Symbol> ParserDriver::searchTables(const SymbolTable::Key& key) const
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
