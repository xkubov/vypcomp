#include <stdexcept>

#include "vypcomp/errors/errors.h"
#include "vypcomp/ir/expression.h"
#include "vypcomp/ir/instructions.h"

using namespace vypcomp::ir;

//
// Expressions
//
//
// Literal Expression
//
LiteralExpression::LiteralExpression(Literal value)
	: Expression(Datatype(value.type())), _value(value)
{}
std::string LiteralExpression::to_string() const
{
	return _value.string_value();
}
vypcomp::ir::Literal LiteralExpression::getValue() const
{
	return _value;
}

//
// Symbol Expression
//
SymbolExpression::SymbolExpression(AllocaInstruction::Ptr value)
	: Expression(value->type()), _value(value)
{}
std::string SymbolExpression::to_string() const
{
	return "(symbol: " + _value->name() + ")";
}

//
// Function Expression
//
FunctionExpression::FunctionExpression(Function::Ptr value)
	: Expression(Datatype(Datatype::FunctionType())), _value(value)
{}
std::string FunctionExpression::to_string() const
{
	return "(function: " + _value->name() + ")";
}

//
// Binary operator expressions
//
AddExpression::AddExpression(ValueType&& op1, ValueType&& op2)
	: Expression(), _op1(std::move(op1)), _op2(std::move(op2))
{
	if (!_op1 || !_op2)
	{
		// Parser done a big bad! Should not happen once every expression type is implemented.
		throw std::runtime_error("one of operands in AddExpression is null");
	}
	if (_op1->type().is<PrimitiveDatatype>() && _op2->type().is<PrimitiveDatatype>())
	{
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype1 != datatype2)
		{
			throw SemanticError("types do not match in + operation");
		}
		else
		{
			_type = _op1->type();
		}
	}
	else
	{
		throw SemanticError("only primitive types are supported in + operation");
	}
}
std::string AddExpression::to_string() const
{
	return "(" + _op1->to_string() + " + " + _op2->to_string() + ")";
}

SubtractExpression::SubtractExpression(ValueType&& op1, ValueType&& op2)
	: Expression(), _op1(std::move(op1)), _op2(std::move(op2))
{
	if (_op1->type().is<PrimitiveDatatype>() && _op2->type().is<PrimitiveDatatype>())
	{
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		// TODO: FLOAT extension here needed
		if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Int)
		{
			_type = _op1->type();
		}
		else
		{
			throw SemanticError("Unsupported type in - operation.");
		}
	}
	else
	{
		throw SemanticError("Unsupported type in - operation.");
	}
}
std::string SubtractExpression::to_string() const
{
	return "(" + _op1->to_string() +" - " + _op2->to_string() + ")";
}

MultiplyExpression::MultiplyExpression(ValueType&& op1, ValueType&& op2)
	: Expression(), _op1(std::move(op1)), _op2(std::move(op2))
{
	try
	{
		// TODO: FLOAT support, modify type checks here
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		if (datatype1 != PrimitiveDatatype::Int)
		{
			throw SemanticError("invalid first operand in * operation, must be int");
		}
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype2 != PrimitiveDatatype::Int)
		{
			throw SemanticError("invalid second operand in * operation, must be int");
		}
		_type = _op1->type();
	}
	catch (const std::bad_variant_access& e)
	{
		throw SemanticError("only int types are supported in * operation");
	}
}
std::string MultiplyExpression::to_string() const
{
	return "(" + _op1->to_string() + " * " + _op2->to_string() + ")";
}

DivideExpression::DivideExpression(ValueType&& op1, ValueType&& op2)
	: Expression(), _op1(std::move(op1)), _op2(std::move(op2))
{
	try
	{
		// TODO: FLOAT support, modify type checks here
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		if (datatype1 != PrimitiveDatatype::Int)
		{
			throw SemanticError("invalid first operand in / operation, must be int");
		}
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype2 != PrimitiveDatatype::Int)
		{
			throw SemanticError("invalid second operand in / operation, must be int");
		}
		_type = _op1->type();
	}
	catch (const std::bad_variant_access& e)
	{
		throw SemanticError("only int types are supported in / operation");
	}
}
std::string DivideExpression::to_string() const
{
	return "(" + _op1->to_string() + " / " + _op2->to_string() + ")";
}

ComparisonExpression::ComparisonExpression(Operation operation, ValueType&& op1, ValueType&& op2)
	: Expression(Datatype(PrimitiveDatatype::Int)), _operation(operation), _op1(std::move(op1)), _op2(std::move(op2))
{
	if (_op1->type() != _op2->type())
	{
		throw SemanticError("types do not match in " + op_string() + " operation");
	}
	if (_operation != EQUALS && _operation != NOTEQUALS)
	{
		// these two operations actually allow arbitrary identical datatypes
		if (!_op1->type().is<PrimitiveDatatype>() || !_op2->type().is<PrimitiveDatatype>())
		{
			throw SemanticError("only primitive types are supported in " + op_string() + " operation");
		}
	}
}
std::string ComparisonExpression::to_string() const
{
	return "(" + _op1->to_string() + " " + op_string() + " " + _op2->to_string() + ")";
}
std::string ComparisonExpression::op_string() const
{
	switch (_operation)
	{
	case GREATER:
		return ">";
	case GEQ:
		return ">=";
	case LESS:
		return "<";
	case LEQ:
		return "<=";
	case EQUALS:
		return "==";
	case NOTEQUALS:
		return "!=";
	}

	throw std::runtime_error("Invalid operator.");
}

AndExpression::AndExpression(ValueType&& op1, ValueType&& op2)
	: Expression(Datatype(PrimitiveDatatype::Int)), _op1(std::move(op1)), _op2(std::move(op2))
{
	if (_op1->type() != _op2->type())
	{
		throw SemanticError("types do not match in && operation");
	}
}
std::string AndExpression::to_string() const
{
	return "(" + _op1->to_string() + " && " + _op2->to_string() + ")";
}

OrExpression::OrExpression(ValueType&& op1, ValueType&& op2)
	: Expression(Datatype(PrimitiveDatatype::Int)), _op1(std::move(op1)), _op2(std::move(op2))
{
	if (_op1->type() != _op2->type())
	{
		throw SemanticError("types do not match in && operation");
	}
}
std::string OrExpression::to_string() const
{
	return "(" + _op1->to_string() + " || " + _op2->to_string() + ")";
}
