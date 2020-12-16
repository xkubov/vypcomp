#include <fstream>
#include <stdexcept>
#include <variant>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;
using namespace std::string_literals;

SymbolTable initSymbolTable()
{
	auto table = SymbolTable(true);
	auto object = ir::Class::Ptr(new ir::Class("Object", nullptr));
	table.insert({ "Object", object });
	// add functions here
	// int readInt(void)
	table.insert({ "readInt", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::Int), "readInt", Arglist())) });
	// string readString(void)
	table.insert({ "readString", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::String), "readString", Arglist())) });
	// int length(string s)
	table.insert({ "length", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::Int), std::string("length"), Arglist{ std::make_pair(Datatype(PrimitiveDatatype::String), "s")} )) });
	auto subStr_ptr = std::make_shared<ir::Function>(Function::Signature(
		PrimitiveDatatype::String, 
		std::string("subStr"), 
		Arglist{ { 
			std::make_pair(Datatype(PrimitiveDatatype::String), "s"s),
			std::make_pair(Datatype(PrimitiveDatatype::Int), "i"s),
			std::make_pair(Datatype(PrimitiveDatatype::Int), "n"s)
		} }
	));
	// string subStr(string s, int i, int n)
	table.insert({ "subStr", subStr_ptr });
	// void print(PrimitiveDatatype i, ...)
	table.insert({ "print", std::make_shared<ir::Function>(std::make_tuple(std::nullopt, "print", Arglist())) }); // a special one handled differently in parser
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
		_tables.back().insert({ arg->name(), arg });  // Richard's hotfix, maybe a better way possible
		//TODO: if _currClass is not none provide implicit first param.
	}
	_currFunction = fun;
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
		cl->clear();
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

Instruction::Ptr ParserDriver::assign(const std::string& name,
                       const ir::Expression::ValueType &val) const
{
        if (auto symbol = searchTables(name)) {
		if (!std::holds_alternative<AllocaInstruction::Ptr>(*symbol))
			throw SemanticError("Cannot assign to function.");

		auto var = std::get<AllocaInstruction::Ptr>(*symbol);
		if (var->type() != val->type()) {
			throw SemanticError("Invalid type.");
		}

		return Assignment::Ptr(
			new Assignment(
				std::get<AllocaInstruction::Ptr>(*symbol),
				val
			)
		);
        }

	throw SemanticError("Assignment to undefined variable "+name);
}

std::vector<Instruction::Ptr> ParserDriver::call_func(const std::string& name, const std::vector<ir::Expression::ValueType>& args) const
{
	auto search_result = searchTables(name);
	if (search_result)
	{
		SymbolTable::Symbol symbol = search_result.value();
		if (std::holds_alternative<Function::Ptr>(symbol))
		{
			auto function = std::get<Function::Ptr>(symbol);
			if (function->name() == "print")
			{
				if (args.size() < 1) throw SemanticError("print has to have at least 1 parameter");
				for (const ir::Expression::ValueType& argument : args)
				{
					auto arg_type = argument->type();
					if (!arg_type.isPrimitive())
					{
						throw SemanticError("print called with non-primitive datatype parameter.");
					}
				}
			}
			else
			{
				if (args.size() != function->args().size()) 
					throw SemanticError("Provided argument count does not match the declared parameter count.");

				for (std::size_t i = 0; i < args.size(); i++)
				{
					auto formal_type = function->argTypes()[i];
					auto actual_type = args[i]->type();
					if (formal_type != actual_type)
						throw SemanticError("Provided argument type does not match declared type.");
				}
			}
			return { std::make_shared<Assignment>(nullptr, std::make_shared<FunctionExpression>(function, args)) };
		}
		else
		{
			throw SemanticError("Identifier in function call is not a function.");
		}
	}
	else
	{
		throw SemanticError("Identifier does not exist.");
	}
}

Return::Ptr ParserDriver::createReturn(const ir::Expression::ValueType& val) const
{
	if (_currFunction == nullptr) {
		throw SemanticError("Return statement out of a function");
	}

	if (val == nullptr) {
		if (!_currFunction->isVoid())
			throw SemanticError("Invalid return for function "+_currFunction->name()+" with type: "+_currFunction->type()->to_string());
		return Return::Ptr(new Return());
	}

	if (val->type() != *_currFunction->type())
		throw SemanticError("Return type mismatch.");

	return Return::Ptr(new Return(val));
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
	_tables.back().insert({decl->name(), decl});
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

void ParserDriver::parseClassEnd()
{
	if (_currClass == nullptr)
		throw std::runtime_error("Invalid usage of parseClassEnd");

	popSymbolTable();
	_currClass = nullptr;
}

void ParserDriver::parseFunctionEnd()
{
	if (_currFunction == nullptr)
		throw std::runtime_error("Invalid usage of parseFunctionEnd");

	popSymbolTable();
	_currFunction = nullptr;
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
		return _tables[_tables.size()-1].get(key);

	return {};
}

Class::Ptr ParserDriver::getCurrentClass() const
{
	return _currClass;
}

Datatype ParserDriver::customDatatype(const std::string& dt) const
{
	if (auto symbol = searchTables(dt)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol))
			throw SemanticError("not a type: "+dt);

		return Datatype(dt);
	}

	throw SemanticError("Invalid datatype.");
}
