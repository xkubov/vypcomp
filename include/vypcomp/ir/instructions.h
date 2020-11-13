#pragma once

#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace vypcomp {

namespace ir {

/**
 * Provides datatype options.
 */
enum class Datatype : int {
	String,
	Float,
	Int
};

using Declaration = std::pair<Datatype, std::string>;
using Arglist = std::vector<Declaration>;
using PossibleDatatype = std::optional<Datatype>;

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

	void setNext(Instruction::Ptr);

protected:
	Instruction::Ptr _next = nullptr;
};

class Function: public Instruction {
public:
	using Ptr = std::shared_ptr<Function>;
	using Signature = std::tuple<PossibleDatatype, std::string, Arglist>;

	Function(const Signature& sig);

	void setBody(Instruction::Ptr body);

private:
	PossibleDatatype _type;
	std::string _name;
	Arglist _args;

	Instruction::Ptr _body;
};

}
}
