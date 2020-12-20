#pragma once

#include <memory>
#include <string>

#include "vypcomp/parser/scanner.h"
#include "vypcomp/parser/symbol_table.h"
#include "vypcomp/errors/errors.h"

#include "location.hh"
#include "bison_parser.tab.hpp"

namespace vypcomp {

/**
 * Provides wrapper around Bison Parser that implements interface
 * for pasing.
 */
class ParserDriver {
public:
	using Ptr = std::unique_ptr<ParserDriver>;

	ParserDriver(const SymbolTable& global);
	ParserDriver();

	virtual ~ParserDriver();

public:
	const SymbolTable& table() const;

	/**
	 * @brief Parse file provided by path as argument.
	 */
	void parse(const std::string &filename);
	void parse(std::istream &file);
	void parseExpression(std::istream& file, bool debug_on = false);

	void generateOutput(const std::string &output) const;
	void generateOutput(std::ostream &output) const;

	Class::Ptr getClass(const std::string& name) const;

	void parseStart(ir::Function::Ptr fun);
	void parseStart(ir::Class::Ptr cl);

	/**
	 * Creates new class. This parser does not perform redefinition check. This is left
	 * for index run. When class exists returns referecne to this class. This is solution
	 * for not modifying global sybol table after first run and referencing same symbols.
	 */
	virtual Class::Ptr newClass(const std::string& name, const std::string& base) const;

	virtual AllocaInstruction::Ptr newDeclaration(const ir::Datatype& t, const std::string& name);

	/**
	 * Creates new function based on provided signature. This function does not perform
	 * redefinition check. This is left for index run and expects that all names are behaving
	 * well.
	 */
	virtual Function::Ptr newFunction(const ir::Function::Signature& sig) const;

	virtual Datatype customDatatype(const std::string& dt) const;
	virtual Instruction::Ptr assign(ir::Expression::ValueType dest_expr, const ir::Expression::ValueType& val) const;
	virtual Instruction::Ptr assign(const std::string& name, const ir::Expression::ValueType& val) const;
	void checkAssignmentTypes(const Datatype& dest_type, const Datatype& value_type) const;
	void checkArgTypes(const Function::Ptr& function_ptr, const FunctionExpression::ArgExpressions& real_args) const;
	virtual std::vector<Instruction::Ptr> call_func(ir::Expression::ValueType func_expr, std::vector<ir::Expression::ValueType>& args) const;
	virtual Return::Ptr createReturn(const ir::Expression::ValueType& val) const;
	virtual Instruction::Ptr createIf(
		const ir::Expression::ValueType& val,
		const ir::BasicBlock::Ptr& if_block,
		const ir::BasicBlock::Ptr& else_block) const;
	virtual Instruction::Ptr createWhile(
		const ir::Expression::ValueType& val,
		const ir::BasicBlock::Ptr& block) const;
	void verify(const ir::AllocaInstruction::Ptr& decl) const;
	void add(const ir::AllocaInstruction::Ptr& decl);

	void ensureMainDefined() const;

	void pushSymbolTable(bool storeFunctions=false);
	void popSymbolTable();

	void parseClassEnd();
	void parseFunctionEnd();

	std::optional<SymbolTable::Symbol> searchTables(const SymbolTable::Key& key) const;
	std::optional<SymbolTable::Symbol> searchGlobal(const SymbolTable::Key& key) const;
	std::optional<SymbolTable::Symbol> searchCurrent(const SymbolTable::Key& key) const;

	Class::Ptr getCurrentClass() const;

	// Expressions

	virtual ir::Expression::ValueType identifierExpr(const std::string& name) const;
	virtual ir::Expression::ValueType functionCall(
		const ir::Expression::ValueType& identifier,
		std::vector<ir::Expression::ValueType>& args) const;

	virtual ir::Expression::ValueType notExpr(const ir::Expression::ValueType& expr) const;
	virtual std::shared_ptr<CastExpression> createCastExpr(const Datatype& dest_datatype, Expression::ValueType expr) const;
	virtual ir::Expression::ValueType thisExpr() const;
	virtual ir::Expression::ValueType superExpr() const;

	virtual ir::Expression::ValueType addExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType subExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType mulExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType divExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType geqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType gtExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType leqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType ltExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType eqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType neqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType andExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType orExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const;

	virtual ir::Expression::ValueType dotExpr(
		const ir::Expression::ValueType& e1,
		const std::string& id) const;

private:
	std::unique_ptr<vypcomp::Parser> _parser;
	std::unique_ptr<vypcomp::Scanner> _scanner;

	std::vector<vypcomp::SymbolTable> _tables;
	Class::Ptr _currClass = nullptr;
	Function::Ptr _currFunction = nullptr;
};

}
