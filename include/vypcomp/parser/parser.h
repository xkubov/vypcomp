#pragma once

#include <memory>
#include <string>

#include "vypcomp/parser/scanner.h"
#include "vypcomp/parser/symbol_table.h"

#include "location.hh"
#include "bison_parser.tab.hpp"

namespace vypcomp {

/**
 * Provides wrapper around Bison Parser that implements interface
 * for pasing.
 */
class LangParser {
public:
	LangParser();
	LangParser(const SymbolTable& global);

	virtual ~LangParser();

public:
	/**
	 * @brief Parse file provided by path as argument.
	 */
	void parse(const std::string &filename);
	void parse(std::istream &file);

	void generateOutput(const std::string &output) const;
	void generateOutput(std::ostream &output) const;

	Class::Ptr getBaseClass(const std::string& name);

	void parseStart(ir::Function::Ptr fun);
	void parseStart(ir::Class::Ptr fun);

	void verify(const ir::AllocaInstruction::Ptr& decl);
	void add(const ir::AllocaInstruction::Ptr& decl);

	void ensureMainDefined() const;

	void pushSymbolTable(bool storeFunctions=false);
	void popSymbolTable();

	void parseEnd();

	std::optional<SymbolTable::Symbol> searchTables(const SymbolTable::Key& key) const;

private:
	std::unique_ptr<vypcomp::Parser> _parser;
	std::unique_ptr<vypcomp::Scanner> _scanner;

	bool _indexRun = false;

	std::vector<vypcomp::SymbolTable> _tables;
	Class::Ptr _currClass = nullptr;
};

class SyntaxError: public std::exception {
public:
	SyntaxError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

class SemanticError: public std::exception {
public:
	SemanticError(const std::string& msg);
	const char * what() const throw() override;

private:
	std::string msg;
};

}
