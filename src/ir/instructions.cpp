#include "vypcomp/ir/instructions.h"

using namespace vypcomp;
using namespace vypcomp::ir;

Instruction::Instruction()
{
}

void Instruction::setNext(Instruction::Ptr next)
{
	// TODO: link the other way too?
	_next = next;
}

Function::Function(const Function::Signature& sig)
{
	std::tie(_type, _name, _args) = sig;
}

void Function::setBody(Instruction::Ptr body)
{
	_body = body;
}
