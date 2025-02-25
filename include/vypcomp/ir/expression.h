/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

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

class DummyExpression : public Expression
{
public:
	virtual std::string to_string() const override;
};

class LiteralExpression : public Expression 
{
public:
	LiteralExpression(vypcomp::ir::Literal value);

	virtual std::string to_string() const override;
	virtual bool is_simple() const override;
	vypcomp::ir::Literal getValue() const;
private:
	vypcomp::ir::Literal _value;
};

class NullObject : public LiteralExpression
{
public:
	NullObject(Datatype::ClassName class_name) 
		: LiteralExpression(Literal(0ull))
	{
		_type = Datatype(class_name);
	}
};

class SymbolExpression : public Expression 
{
public:
	SymbolExpression(AllocaInstruction::Ptr value);

	virtual std::string to_string() const override;
	virtual bool is_simple() const override;
	AllocaInstruction::Ptr getValue() const;
protected:
	AllocaInstruction::Ptr _value;
};

class SuperExpression : public SymbolExpression
{
public:
	SuperExpression(AllocaInstruction::Ptr value, Class::Ptr child_ptr);
	Class::Ptr getClass() const;
private:
	Class::Ptr _child_ptr;
};


class ObjectCastExpression : public Expression
{
public:
	ObjectCastExpression(Class::Ptr target_class, ValueType operand);
	virtual std::string to_string() const override;
	Class::Ptr getTargetClass() const;
	ValueType getOperand() const;
private:
	Class::Ptr _target_class;
	ValueType _operand;
};

class StringCastExpression : public Expression
{
public:
	StringCastExpression(ValueType operand);
	virtual std::string to_string() const override;
	ValueType getOperand() const;
private:
	ValueType _operand;
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
	void setArgs(const ArgExpressions& args);
protected:
	Function::Ptr _value;
	ArgExpressions _args;
};

// This expression type represents function that does not exist in the symbol table (such as object creation routine preceding constructor)
class ConstructorExpression : public FunctionExpression
{
public:
	using ArgExpressions = std::vector<Expression::ValueType>;
	ConstructorExpression(Class::Ptr class_ptr);

	virtual std::string to_string() const override;
	std::string getFunctionName() const;
	ArgExpressions getArgs() const;
private:
	std::string _class_name;
};

class MethodExpression : public FunctionExpression
{
public:
	MethodExpression(Function::Ptr function, ValueType context_object);

	virtual std::string to_string() const override;
	ValueType getContextObj() const;
private:
	ValueType _object;
};

class BinaryOpExpression : public Expression
{
public:
	using Ptr = std::shared_ptr<BinaryOpExpression>;
protected:
	BinaryOpExpression(ValueType op1, ValueType op2);
	BinaryOpExpression(Datatype dt, ValueType op1, ValueType op2);
public:
	ValueType getOp1() const;
	ValueType getOp2() const;
protected:
	ValueType _op1;
	ValueType _op2;
};

class AddExpression : public BinaryOpExpression
{
public:
	AddExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

class SubtractExpression : public BinaryOpExpression
{
public:
	SubtractExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

class MultiplyExpression : public BinaryOpExpression
{
public:
	MultiplyExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

class DivideExpression : public BinaryOpExpression
{
public:
	DivideExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

class ComparisonExpression : public BinaryOpExpression
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

	ComparisonExpression(Operation operation, ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
	Operation getOperation() const;
private:
	std::string op_string() const;
private:
	Operation _operation;
};

class AndExpression : public BinaryOpExpression
{
public:
	AndExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

class OrExpression : public BinaryOpExpression
{
public:
	OrExpression(ValueType op1, ValueType op2);

	virtual std::string to_string() const override;
};

//
// Logical Not Expression
//
class NotExpression : public Expression
{
public:
	NotExpression(ValueType operand);

	virtual std::string to_string() const override;
	ValueType getOperand() const;
private:
	ValueType _operand;
};

class ObjectAttributeExpression : public Expression
{
public:
	ObjectAttributeExpression(AllocaInstruction::Ptr object, AllocaInstruction::Ptr attribute, Class::Ptr class_ptr);

	AllocaInstruction::Ptr getObject() const;
	Class::Ptr getClass() const;
	AllocaInstruction::Ptr getAttribute() const;
	virtual std::string to_string() const override;
private:
	AllocaInstruction::Ptr _object;
	Class::Ptr _class;
	AllocaInstruction::Ptr _attribute;
};

}
}
