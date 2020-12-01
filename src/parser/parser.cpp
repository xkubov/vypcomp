#include <fstream>
#include <stdexcept>
#include <variant>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;

SymbolTable initSymbolTable()
{
	auto table = SymbolTable(true);
	auto object = ir::Class::Ptr(new ir::Class("Object", nullptr));

	table.insert({"Object", object});
	return table;
}

ParserDriver::ParserDriver()
{
	_tables.push_back(initSymbolTable());
}

ParserDriver::ParserDriver(const SymbolTable& global)
{
	_tables.push_back(global);
}

ParserDriver::~ParserDriver()
{
}

const SymbolTable& ParserDriver::table() const
{
	// First table is global.
	return _tables[0];
}

Class::Ptr ParserDriver::getClass(const std::string &name) const
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

	throw SemanticError("class not defined: "+name);
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
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file, Parser::token::PROGRAM_START) );
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, this));

	if (int err = _parser->parse()) {
		throw SyntaxError("parser returned: "+std::to_string(err));
	}
}

void ParserDriver::parseExpression(std::istream& file, bool debug_on)
{
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file, Parser::token::EXPR_PARSE_START));
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, this));
	if (debug_on)
		_parser->set_debug_level(1);

	if (int err = _parser->parse()) {
		throw SyntaxError("parser returned: " + std::to_string(err));
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
	_tables.back().insert({fun->name(), fun});

	pushSymbolTable();
	for (auto arg: fun->args()) {
		//TODO: push args to bottom table.
	}
}

void ParserDriver::parseStart(ir::Class::Ptr cl)
{
	_tables.back().insert({cl->name(), cl});
	pushSymbolTable(true);
	_currClass = cl;
}

Class::Ptr ParserDriver::newClass(const std::string &name, const std::string& base) const
{
        if (auto symbol = searchGlobal(name)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol))
			throw std::runtime_error("Invalid state in ParserDriver:"+std::to_string(__LINE__));

		auto cl = std::get<Class::Ptr>(*symbol);
		cl->setBase(getClass(base));
		return cl;
        }

	return Class::Ptr(new Class(name, getClass(base)));
}

Function::Ptr ParserDriver::newFunction(const ir::Function::Signature& sig) const
{
	auto [type, name, args] = sig;
        if (auto symbol = searchCurrent(name)) {
		if (!std::holds_alternative<Function::Ptr>(*symbol))
			throw std::runtime_error("Invalid state in ParserDriver:"+std::to_string(__LINE__));

		return std::get<Function::Ptr>(*symbol);
        }

        return Function::Ptr(new Function({type, name, args}));
}

void ParserDriver::verify(const ir::AllocaInstruction::Ptr& decl)
{
	if (searchCurrent(decl->name())) {
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

	throw SemanticError("main not defined.");
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

std::optional<SymbolTable::Symbol> ParserDriver::searchGlobal(const SymbolTable::Key& key) const
{
	if (_tables[0].has(key))
		return _tables[0].get(key);

	return {};
}

std::optional<SymbolTable::Symbol> ParserDriver::searchCurrent(const SymbolTable::Key& key) const
{
	if (_tables[_tables.size()-1].has(key))
		return _tables[0].get(key);

	return {};
}

Class::Ptr vypcomp::ParserDriver::getCurrentClass() const
{
	return _currClass;
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
