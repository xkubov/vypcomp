#include "vypcomp/parser/indexdriver.h"

using namespace vypcomp;

IndexParserDriver::IndexParserDriver():
	ParserDriver()
{
}

IndexParserDriver::IndexParserDriver(const SymbolTable &global):
	ParserDriver(global)
{
}

Class::Ptr IndexParserDriver::newClass(const std::string& name, const std::string& base) const
{
        if (searchGlobal(name)) {
		throw SemanticError("Redefinition of "+name);
        }
	auto newBase = searchTables(base).has_value() ? base : "Object";

	return ParserDriver::newClass(name, newBase);
}

Function::Ptr IndexParserDriver::newFunction(const ir::Function::Signature& sig) const
{
	auto [type, name, args] = sig;
        if (searchCurrent(name)) {
		throw SemanticError("Redefinition of "+name);
        }
	return ParserDriver::newFunction({type, name, args});
}
