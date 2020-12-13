#pragma once
#include <string>
#include <variant>
#include <memory>

#include "vypcomp/ir/ir.h"
#include "vypcomp/ir/instructions.h"

namespace vypcomp
{

namespace ir
{

class LiteralExpression : public Expression 
{
public:
	LiteralExpression(vypcomp::ir::Literal value);

	virtual std::string to_string() const override;
	vypcomp::ir::Literal getValue() const;
private:
	vypcomp::ir::Literal _value;
};

class SymbolExpression : public Expression 
{
public:
	SymbolExpression(AllocaInstruction::Ptr value);

	virtual std::string to_string() const override;
	AllocaInstruction::Ptr getValue() const;
private:
	AllocaInstruction::Ptr _value;
};

// This expression type has type Datatype::FunctionType when it is referenced by an identifier.
// When a call operation with arguments is provided, the expression gets the proper type
// from the function return type.
class FunctionExpression : public Expression
{
public:
	using ArgExpressions = std::vector<Expression::ValueType>;
	FunctionExpression(Function::Ptr value);
	FunctionExpression(Function::Ptr value, ArgExpressions args);

	virtual std::string to_string() const override;
	Function::Ptr getFunction() const;
	ArgExpressions getArgs() const;
private:
	Function::Ptr _value;
	ArgExpressions _args;
};

class AddExpression : public Expression 
{
public:
	AddExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

class SubtractExpression : public Expression
{
public:
	SubtractExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

class MultiplyExpression : public Expression
{
public:
	MultiplyExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

class DivideExpression : public Expression
{
public:
	DivideExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

class ComparisonExpression : public Expression
{
public:
	enum Operation : std::uint8_t
	{
		GREATER,
		GEQ,
		LESS,
		LEQ,
		EQUALS,
		NOTEQUALS
	};

	ComparisonExpression(Operation operation, ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	std::string op_string() const;
private:
	Operation _operation;
	ValueType _op1;
	ValueType _op2;
};

class AndExpression : public Expression
{
public:
	AndExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

class OrExpression : public Expression
{
public:
	OrExpression(ValueType&& op1, ValueType&& op2);

	virtual std::string to_string() const override;
private:
	ValueType _op1;
	ValueType _op2;
};

}
}
