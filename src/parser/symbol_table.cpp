/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

#include "vypcomp/parser/symbol_table.h"
#include <stdexcept>

using namespace vypcomp;

SymbolTable::SymbolTable(bool storesFunctions):
	_storesFunctions(storesFunctions)
{
}

bool SymbolTable::insert(const std::pair<Key, Symbol>& element)
{
	auto [k,v] = element;
	if (!_storesFunctions) {
		if (std::holds_alternative<ir::Function::Ptr>(v)) {
			return false;
		}
	}

	_table[k] = v;

        return true;
}

bool SymbolTable::has(const Key& symb) const
{
	return _table.count(symb);
}

SymbolTable::Symbol SymbolTable::get(const Key& key) const
{
	if (!has(key))
		throw std::runtime_error("Symbol table does not contain value: "+key);

	return _table.at(key);
}

const std::map<SymbolTable::Key, SymbolTable::Symbol>& SymbolTable::data() const
{
	return _table;
}
