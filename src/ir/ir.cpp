#include "vypcomp/ir/ir.h"

using namespace vypcomp::ir;

// ------------------------------
// Instruction
// ------------------------------

Instruction::Instruction()
{
}

Instruction::~Instruction()
{
}

void Instruction::setNext(Instruction::Ptr next)
{
	_next = next;
}

Instruction::Ptr Instruction::next() const
{
	return _next;
}

// ------------------------------
// Datatypes
// ------------------------------

Datatype::Datatype(const Datatype::DT& dt):
	_dt(dt)
{
}

Datatype::Datatype(const Datatype& dt)
{
	*this = dt;
}

const Datatype& Datatype::operator=(const Datatype& other)
{
	_dt = other._dt;
	return *this;
}

bool Datatype::operator==(const Datatype& other) const
{
	if (_dt.index() == other._dt.index())
	{
		if (std::holds_alternative<PrimitiveDatatype>(other._dt) && std::holds_alternative<PrimitiveDatatype>(_dt))
		{
			return std::get<PrimitiveDatatype>(other._dt) == std::get<PrimitiveDatatype>(_dt);
		}
		else if (std::holds_alternative<ClassName>(other._dt) && std::holds_alternative<ClassName>(_dt))
		{
			return std::get<ClassName>(other._dt) == std::get<ClassName>(_dt);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool Datatype::operator!=(const Datatype& other) const
{
	return !(*this == other);
}

std::string Datatype::to_string() const
{
	if (std::holds_alternative<PrimitiveDatatype>(_dt))
	{
		auto enum_val = std::get<PrimitiveDatatype>(_dt);
		if (enum_val == PrimitiveDatatype::Float)
		{
			return "float";
		}
		else if (enum_val == PrimitiveDatatype::Int)
		{
			return "int";
		}
		else
		{
			return "string";
		}
	}
	else if (std::holds_alternative<ClassName>(_dt))
	{
		return "class " + std::get<ClassName>(_dt);
	}
	else if (std::holds_alternative<FunctionType>(_dt))
	{
		return "function";
	}
	else
	{
		return "invalid type";
	}
}

bool Datatype::isPrimitive() const
{
	return std::holds_alternative<PrimitiveDatatype>(_dt);
}

Literal::Literal(const Literal::Impl &val):
	_val(val)
{
}
