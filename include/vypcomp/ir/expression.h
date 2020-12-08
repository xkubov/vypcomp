#pragma once
#include <string>
#include <variant>
#include <memory>

#include "vypcomp/ir/instructions.h"

namespace vypcomp
{

namespace ir
{

class Expression 
{
public:
	using ValueType = std::shared_ptr<Expression>;
	Expression()
		: _type(InvalidDatatype())
	{}
	Expression(Datatype type)
		: _type(type)
	{}
	Expression(const Expression& other) = default;
	Expression& operator=(const Expression& other) = default;
	Expression(Expression&& other) = default;
	Expression& operator=(Expression&& other) = default;
	virtual ~Expression() = default;

	Datatype type() const
	{
		return _type;
	}

	virtual std::string to_string() const = 0;
protected:
	Datatype _type;
};

class LiteralExpression : public Expression 
{
public:
	LiteralExpression(vypcomp::ir::Literal value);

	virtual std::string to_string() const override;
private:
	vypcomp::ir::Literal _value;
};

class SymbolExpression : public Expression 
{
public:
	SymbolExpression(AllocaInstruction::Ptr value);

	virtual std::string to_string() const override;
private:
	AllocaInstruction::Ptr _value;
};

class FunctionExpression : public Expression
{
public:
	FunctionExpression(Function::Ptr value);

	virtual std::string to_string() const override;
private:
	Function::Ptr _value;
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
