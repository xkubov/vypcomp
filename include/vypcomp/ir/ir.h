#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace vypcomp {
namespace ir {

/**
 * Provides datatype options.
 */
enum class PrimitiveDatatype : int {
	String,
	Float,
	Int
};
using ClassName = std::string;
struct FunctionType {};
struct InvalidDatatype {};
using Datatype = std::variant<PrimitiveDatatype, ClassName, FunctionType, InvalidDatatype>;
bool operator==(const Datatype& dt1, const Datatype& dt2);
bool operator!=(const Datatype& dt1, const Datatype& dt2);
std::string to_string(const Datatype& t);
using Declaration = std::pair<PrimitiveDatatype, std::string>; // TODO: rework all these to work with Datatype instead
using Arglist = std::vector<Declaration>;
using PossibleDatatype = std::optional<PrimitiveDatatype>;

struct Literal {
public:
	using Impl = std::variant<std::string, unsigned long long, float>;
	Literal(const Impl& val);

	template<PrimitiveDatatype = PrimitiveDatatype::String>
	std::string get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Int>
	int get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Float>
	float get() const;

	std::string string_value() const
	{
		if (std::holds_alternative<std::string>(_val)) 
			return "\"" + std::get<std::string>(_val) + "\"";
		else if (std::holds_alternative<unsigned long long>(_val))
			return std::to_string(std::get<unsigned long long>(_val));
		else if (std::holds_alternative<float>(_val))
			return std::to_string(std::get<float>(_val));
		else
			throw std::runtime_error("Unexpected type: "+std::to_string(__LINE__));
	}

	PrimitiveDatatype type() const
	{
		if (std::holds_alternative<std::string>(_val)) return PrimitiveDatatype::String;
		else if (std::holds_alternative<float>(_val)) return PrimitiveDatatype::Float;
		else if (std::holds_alternative<unsigned long long>(_val)) return PrimitiveDatatype::Int;
		else throw std::runtime_error("Unexpected type."+std::to_string(__LINE__));
	}
private:
	Impl _val;
};

using OptLiteral = std::optional<Literal>;

/**
 * Represents abstraction over instruction.
 *
 * Instruction is basically every statement:
 *  - function call,
 *  - variable declaration
 *  - if statement
 *  - loops
 *  - assignmnent
 */
class Instruction {
public:
	using Ptr = std::shared_ptr<Instruction>;

	Instruction();

	void setNext(Instruction::Ptr next);
	Instruction::Ptr next() const;

protected:
	Instruction::Ptr _next = nullptr;
};

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

}
}
