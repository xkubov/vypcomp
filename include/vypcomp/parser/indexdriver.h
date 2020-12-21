#pragma once

#include "parser.h"

namespace vypcomp {

/**
 * Provides first index run pareser driver.
 */
class IndexParserDriver : public ParserDriver {
public:
	IndexParserDriver();
	IndexParserDriver(const SymbolTable &global);

public:
	virtual AllocaInstruction::Ptr newDeclaration(const ir::Datatype& t, const std::string& name) override;
	virtual Class::Ptr newClass(const std::string& name, const std::string& base) const override;
	virtual Function::Ptr newFunction(const ir::Function::Signature& sig) const override;
	virtual std::vector<Instruction::Ptr> call_func(ir::Expression::ValueType func_expr, std::vector<ir::Expression::ValueType>& args) const override;
	virtual Return::Ptr createReturn(const ir::Expression::ValueType& val) const override;
	virtual Instruction::Ptr createIf(
		const ir::Expression::ValueType& val,
		const ir::BasicBlock::Ptr& if_block,
		const ir::BasicBlock::Ptr& else_block) const override;
	virtual Instruction::Ptr createWhile(
		const ir::Expression::ValueType& val,
		const ir::BasicBlock::Ptr& block) const override;
	virtual Datatype customDatatype(const std::string& dt) const override;
	virtual Instruction::Ptr assign(const std::string& ptr, const ir::Expression::ValueType& val) const override;
	virtual Instruction::Ptr assign(ir::Expression::ValueType dest_expr, const ir::Expression::ValueType& val) const override;
	virtual Expression::ValueType createCastExpr(const Datatype& dt, Expression::ValueType expr) const override;


// Expressions
public:
	virtual ir::Expression::ValueType identifierExpr(const std::string& name) const override;
	virtual ir::Expression::ValueType functionCall(
		const ir::Expression::ValueType& identifier,
		std::vector<ir::Expression::ValueType>& args) const override;

	virtual ir::Expression::ValueType notExpr(const ir::Expression::ValueType& expr) const override;
	virtual ir::Expression::ValueType thisExpr() const override;
	virtual ir::Expression::ValueType superExpr() const override;
	virtual ir::Expression::ValueType newExpr(const std::string& clas_name) const override;

	virtual ir::Expression::ValueType addExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType subExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType mulExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType divExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType geqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType gtExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType leqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType ltExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType eqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType neqExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType andExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType orExpr(
		const ir::Expression::ValueType& e1,
		const ir::Expression::ValueType& e2) const override;

	virtual ir::Expression::ValueType dotExpr(
		const ir::Expression::ValueType& e1,
		const std::string& id) const override;

private:
	std::vector<vypcomp::SymbolTable> _tables;
};

} // namespace vypcomp
