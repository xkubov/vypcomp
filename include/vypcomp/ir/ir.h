#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>
#include <stdexcept>

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

class Datatype {
public:
	using ClassName = std::string;
	struct FunctionType {};
	struct InvalidDatatype {};

	using DT = std::variant<
		PrimitiveDatatype,
		ClassName,
		FunctionType,
		InvalidDatatype
	>;

public:
	Datatype(const DT& dt = InvalidDatatype());
	Datatype(const Datatype& dt);

	const Datatype& operator=(const Datatype& other);
	bool operator==(const Datatype& other) const;
	bool operator!=(const Datatype& other) const;
	std::string to_string() const;
	bool isPrimitive() const;
	
	template<class T>
	T get() const {
		return std::get<T>(_dt);
	}

	template<class T>
	bool is() const {
		return std::holds_alternative<T>(_dt);
	}

private:
	DT _dt;
};

using Declaration = std::pair<Datatype, std::string>;
using Arglist = std::vector<Declaration>;
using PossibleDatatype = std::optional<Datatype>;

struct Literal {
public:
	using Impl = std::variant<std::string, unsigned long long, double>;
	Literal(const Impl& val);

	template<PrimitiveDatatype = PrimitiveDatatype::String>
	std::string get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Int>
	int get() const;

	template<PrimitiveDatatype = PrimitiveDatatype::Float>
	double get() const;

	std::string string_value() const
	{
		if (std::holds_alternative<std::string>(_val)) 
			return "\"" + std::get<std::string>(_val) + "\"";
		else if (std::holds_alternative<unsigned long long>(_val))
			return std::to_string(std::get<unsigned long long>(_val));
		else if (std::holds_alternative<double>(_val))
			return std::to_string(std::get<double>(_val));
		else
			throw std::runtime_error("Unexpected type: "+std::to_string(__LINE__));
	}

	std::string vypcode_representation() const
	{
		if (std::holds_alternative<std::string>(_val))
		{
			return "\"" + std::get<std::string>(_val) + "\"";
		}
		else if (std::holds_alternative<unsigned long long>(_val))
			return std::to_string(std::get<unsigned long long>(_val));
		else if (std::holds_alternative<double>(_val))
		{
			char buf[64] = { 0 };
			std::sprintf(buf, "%a", std::get<double>(_val));
			return std::string(buf);
		}
		else
			throw std::runtime_error("Unexpected type: " + std::to_string(__LINE__));
	}

	PrimitiveDatatype type() const
	{
		if (std::holds_alternative<std::string>(_val)) return PrimitiveDatatype::String;
		else if (std::holds_alternative<double>(_val)) return PrimitiveDatatype::Float;
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
	virtual ~Instruction();

	void setNext(Instruction::Ptr next);
	Instruction::Ptr next() const;

	virtual std::string str(const std::string& prefix) const = 0;

protected:
	Instruction::Ptr _next = nullptr;
};

class Expression {
public:
	using ValueType = std::shared_ptr<Expression>;
	Expression()
		: _type(Datatype::InvalidDatatype())
	{}
	Expression(const Datatype& type)
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
	// simple expression means that it can be represented in a single register load
	virtual bool is_simple() const { return false; }
protected:
	Datatype _type;
};

}
}
