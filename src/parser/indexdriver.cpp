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

Datatype IndexParserDriver::customDatatype(const std::string& dt) const
{
	if (auto symbol = searchTables(dt)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol))
			throw SemanticError("not a type: "+dt);

		return Datatype(dt);
	}

	return Datatype(Datatype::InvalidDatatype());
}

Instruction::Ptr IndexParserDriver::assign(ir::Expression::ValueType dest_expr, const ir::Expression::ValueType &val) const
{
	return DummyInstruction::Ptr(new DummyInstruction);
}

Instruction::Ptr IndexParserDriver::assign(const std::string& name, const ir::Expression::ValueType& val) const
{
	return DummyInstruction::Ptr(new DummyInstruction);
}

std::shared_ptr<CastExpression> IndexParserDriver::createCastExpr(std::string class_name, Expression::ValueType expr) const
{
	return nullptr;
}

// Expressions

ir::Expression::ValueType IndexParserDriver::identifierExpr(const std::string& name) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::functionCall(
	const ir::Expression::ValueType& identifier,
	std::vector<ir::Expression::ValueType>& args) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::notExpr(const ir::Expression::ValueType& expr) const
{
	return std::make_shared<NotExpression>(expr);
}

ir::Expression::ValueType IndexParserDriver::thisExpr() const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::superExpr() const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::addExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::subExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::mulExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::divExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::geqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::gtExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::leqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::ltExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::eqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::neqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::andExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::orExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<DummyExpression>();
}

ir::Expression::ValueType IndexParserDriver::dotExpr(
	const ir::Expression::ValueType& e1,
	const std::string& identifier) const
{
	return std::make_shared<DummyExpression>();
}

std::vector<Instruction::Ptr> IndexParserDriver::call_func(ir::Expression::ValueType func_expr, std::vector<ir::Expression::ValueType>& args) const
{
	return {};
}

Return::Ptr IndexParserDriver::createReturn(const ir::Expression::ValueType& val) const
{
	return Return::Ptr(new Return(val));
}

Instruction::Ptr IndexParserDriver::createIf(
	const ir::Expression::ValueType& val,
	const ir::BasicBlock::Ptr& if_block,
	const ir::BasicBlock::Ptr& else_block) const
{
	return DummyInstruction::Ptr(new DummyInstruction);
}

Instruction::Ptr IndexParserDriver::createWhile(
	const ir::Expression::ValueType& val,
	const ir::BasicBlock::Ptr& block) const
{
	return DummyInstruction::Ptr(new DummyInstruction);
}
