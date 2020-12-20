#include <stdexcept>
#include <sstream>

#include "vypcomp/errors/errors.h"
#include "vypcomp/ir/expression.h"
#include "vypcomp/ir/instructions.h"

using namespace vypcomp::ir;

//
// Expressions
//

std::string DummyExpression::to_string() const
{
	return "dummy_expr";
}

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
bool LiteralExpression::is_simple() const
{
	return true;
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
bool SymbolExpression::is_simple() const
{
	return true;
}
AllocaInstruction::Ptr SymbolExpression::getValue() const
{
	return _value;
}

SuperExpression::SuperExpression(AllocaInstruction::Ptr value, Class::Ptr child_ptr)
	: SymbolExpression(value), _child_ptr(child_ptr)
{}
Class::Ptr SuperExpression::getClass() const
{
	return _child_ptr;
}

//
// Object Cast Expression
//
ObjectCastExpression::ObjectCastExpression(Class::Ptr target_class, ValueType operand) 
	: Expression(Datatype(target_class->name())), _target_class(target_class), _operand(operand)
{
	if (!operand->type().is<Datatype::ClassName>())
		throw SemanticError("Cast expression allowed only from object type: " + operand->to_string());
}
ObjectCastExpression::ValueType ObjectCastExpression::getOperand() const
{
	return _operand;
}
Class::Ptr ObjectCastExpression::getTargetClass() const
{
	return _target_class;
}
std::string ObjectCastExpression::to_string() const
{
	return "((" + _target_class->name() + ")" + _operand->to_string() + ")";
}
//
// String Cast Expression
//
StringCastExpression::StringCastExpression(ValueType value)
	: Expression(Datatype(PrimitiveDatatype::String)), _operand(value)
{
	if (!_operand->type().isPrimitive() || _operand->type().get<PrimitiveDatatype>() != PrimitiveDatatype::Int)
	{
		throw SemanticError("string cast attempted on non-int type: " + _operand->to_string());
	}
}
StringCastExpression::ValueType StringCastExpression::getOperand() const
{
	return _operand;
}
std::string StringCastExpression::to_string() const
{
	return "((string)" + _operand->to_string() + ")";
}

//
// Function Expression
//
FunctionExpression::FunctionExpression(Function::Ptr value)
	: Expression(Datatype(Datatype::FunctionType())), _value(value), _args()
{}
FunctionExpression::FunctionExpression(Function::Ptr value, ArgExpressions args)
	: 
	Expression(value->type() ? value->type().value() : Datatype(Datatype::FunctionType())), 
	_value(value), 
	_args(args)
{}
std::string FunctionExpression::to_string() const
{
	std::ostringstream ss;
	ss << "(function: " + _value->name() + ")(";
	for (auto i = 0ull; i < _args.size(); i++)
	{
		auto& arg_expr = _args[i];
		ss << arg_expr->to_string();
		if (i != _args.size() - 1)
			ss << ", ";
	}
	ss << ")";
	return ss.str();
}
Function::Ptr FunctionExpression::getFunction() const
{
	return _value;
}
FunctionExpression::ArgExpressions FunctionExpression::getArgs() const
{
	return _args;
}
void FunctionExpression::setArgs(const ArgExpressions& args)
{
	_args = args;
	_type = _value->type() ? _value->type().value() : Datatype(Datatype::InvalidDatatype());
}

//
// Constructor Expression
//
ConstructorExpression::ConstructorExpression(Class::Ptr class_ptr)
	: FunctionExpression(class_ptr->constructor()), _class_name(class_ptr->name())
{
	_type = Datatype(class_ptr->name());
}
std::string ConstructorExpression::getFunctionName() const
{
	return _class_name;
}
ConstructorExpression::ArgExpressions ConstructorExpression::getArgs() const
{
	return _args;
}
std::string ConstructorExpression::to_string() const
{
	std::ostringstream ss;
	ss << "(constructor: " + _class_name + ")(";
	for (auto i = 0ull; i < _args.size(); i++)
	{
		auto& arg_expr = _args[i];
		ss << arg_expr->to_string();
		if (i != _args.size() - 1)
			ss << ", ";
	}
	ss << ")";
	return ss.str();
}

//
// Object Method Call Expression
//
MethodExpression::MethodExpression(Function::Ptr function, ValueType context_object)
	: FunctionExpression(function), _object(context_object)
{
	if (!context_object->type().is<Datatype::ClassName>()) throw SemanticError("Method call attempted at non-object type expression result.");
}
std::string MethodExpression::to_string() const
{
	std::ostringstream ss;
	ss << "(method: " + _object->to_string() + "." + _value->name() + ")(";
	for (auto i = 0ull; i < _args.size(); i++)
	{
		auto& arg_expr = _args[i];
		ss << arg_expr->to_string();
		if (i != _args.size() - 1)
			ss << ", ";
	}
	ss << ")";
	return ss.str();
}
MethodExpression::ValueType MethodExpression::getContextObj() const
{
	return _object;
}

//
// Binary operator expressions
//
BinaryOpExpression::BinaryOpExpression(ValueType op1, ValueType op2)
	: _op1(op1), _op2(op2)
{
	if (!_op1 || !_op2)
	{
		// Parser done a big bad! Should not happen once every expression type is implemented.
		throw std::runtime_error("one of operands in BinaryOpExpression is null.");
	}
}
BinaryOpExpression::BinaryOpExpression(Datatype dt, ValueType op1, ValueType op2)
	: Expression(dt), _op1(op1), _op2(op2)
{}
BinaryOpExpression::ValueType BinaryOpExpression::getOp1() const
{
	return _op1;
}
BinaryOpExpression::ValueType BinaryOpExpression::getOp2() const
{
	return _op2;
}

AddExpression::AddExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(std::move(op1), std::move(op2))
{
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

SubtractExpression::SubtractExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(std::move(op1), std::move(op2))
{
	if (_op1->type().is<PrimitiveDatatype>() && _op2->type().is<PrimitiveDatatype>())
	{
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Int)
		{
			_type = _op1->type();
		}
		else if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Float)
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

MultiplyExpression::MultiplyExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(std::move(op1), std::move(op2))
{
	try
	{
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Int)
		{
			_type = _op1->type();
		}
		else if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Float)
		{
			_type = _op1->type();
		}
		else
		{
			throw SemanticError("Unsupported type in * operation.");
		}
	}
	catch (const std::bad_variant_access& e)
	{
		throw SemanticError("only int or float types are supported in * operation");
	}
}
std::string MultiplyExpression::to_string() const
{
	return "(" + _op1->to_string() + " * " + _op2->to_string() + ")";
}

DivideExpression::DivideExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(std::move(op1), std::move(op2))
{
	try
	{
		auto datatype1 = _op1->type().get<PrimitiveDatatype>();
		auto datatype2 = _op2->type().get<PrimitiveDatatype>();
		if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Int)
		{
			_type = _op1->type();
		}
		else if (datatype1 == datatype2 && datatype1 == PrimitiveDatatype::Float)
		{
			_type = _op1->type();
		}
		else
		{
			throw SemanticError("Unsupported type in / operation.");
		}
	}
	catch (const std::bad_variant_access& e)
	{
		throw SemanticError("only int or float types are supported in / operation");
	}
}
std::string DivideExpression::to_string() const
{
	return "(" + _op1->to_string() + " / " + _op2->to_string() + ")";
}

ComparisonExpression::ComparisonExpression(Operation operation, ValueType op1, ValueType op2)
	: BinaryOpExpression(Datatype(PrimitiveDatatype::Int), std::move(op1), std::move(op2)), _operation(operation)
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
ComparisonExpression::Operation ComparisonExpression::getOperation() const
{
	return _operation;
}

AndExpression::AndExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(Datatype(PrimitiveDatatype::Int), std::move(op1), std::move(op2))
{
	if (_op1->type() != _op2->type())
	{
		throw SemanticError("types do not match in && operation");
	}
	if (_op1->type() == Datatype(PrimitiveDatatype::Float) || _op1->type() == Datatype(PrimitiveDatatype::String))
	{
		throw SemanticError("Only int and object types allowed in && operator.");
	}
}
std::string AndExpression::to_string() const
{
	return "(" + _op1->to_string() + " && " + _op2->to_string() + ")";
}

OrExpression::OrExpression(ValueType op1, ValueType op2)
	: BinaryOpExpression(Datatype(PrimitiveDatatype::Int), std::move(op1), std::move(op2))
{
	if (_op1->type() != _op2->type())
	{
		throw SemanticError("types do not match in || operation");
	}
	if (_op1->type() == Datatype(PrimitiveDatatype::Float) || _op1->type() == Datatype(PrimitiveDatatype::String))
	{
		throw SemanticError("Only int and object types allowed in || operator.");
	}
}
std::string OrExpression::to_string() const
{
	return "(" + _op1->to_string() + " || " + _op2->to_string() + ")";
}

//
// Logical Not Expression
//
NotExpression::NotExpression(ValueType operand)
	: Expression(Datatype(PrimitiveDatatype::Int)), _operand(operand)
{
	if (operand->type() != Datatype(PrimitiveDatatype::Int) && !operand->type().is<Datatype::ClassName>())
		throw SemanticError("Only int and object type allowed in ! operator: " + operand->to_string());
}
NotExpression::ValueType NotExpression::getOperand() const
{
	return _operand;
}
std::string NotExpression::to_string() const
{
	return "(!" + _operand->to_string() + ")";
}

//
// Object Attribute Expression
//
ObjectAttributeExpression::ObjectAttributeExpression(AllocaInstruction::Ptr object, AllocaInstruction::Ptr attribute, Class::Ptr class_ptr)
	: Expression(attribute->type()), _object(object), _attribute(attribute), _class(class_ptr)
{
	// TODO: check type, although may not be needed since it's only created by '.' operator
}
std::string ObjectAttributeExpression::to_string() const
{
	return "(attribute of " + _class->name() + ": " + _object->name() + "." + _attribute->name() + ")";
}
AllocaInstruction::Ptr ObjectAttributeExpression::getObject() const
{
	return _object;
}
Class::Ptr ObjectAttributeExpression::getClass() const
{
	return _class;
}
AllocaInstruction::Ptr ObjectAttributeExpression::getAttribute() const
{
	return _attribute;
}
