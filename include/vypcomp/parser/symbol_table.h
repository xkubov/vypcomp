#pragma once

#include <map>
#include <variant>

#include "vypcomp/ir/instructions.h"

namespace vypcomp {

class SymbolTable {
public:
	using Symbol = std::variant<
			ir::Function::Ptr,
			ir::Class::Ptr,
			ir::AllocaInstruction::Ptr>;

	using Key = std::string;

	SymbolTable(bool storesFunctions = false);

	bool insert(const std::pair<Key, Symbol>& element);

	bool has(const Key& symb) const;
	Symbol get(const Key& symb) const;

private:
	std::map<Key, Symbol> _table;
	bool _storesFunctions = false;
};

}
