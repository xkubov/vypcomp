#include <sstream>
#include <variant>
#include <algorithm>
#include <stdexcept>

#include "vypcomp/ir/instructions.h"

using namespace vypcomp;
using namespace vypcomp::ir;

// ------------------------------
// BasicBlock
// ------------------------------

BasicBlock::BasicBlock(const std::string& name, const std::string& suf)
{
	static uint64_t ID = 0;

	std::string suffix(suf);
	if (suffix.empty()) {
		suffix = "_"+std::to_string(ID);
		ID++;
	}

	_name = name+suffix;
}

BasicBlock::Ptr BasicBlock::create()
{
	return BasicBlock::Ptr(new BasicBlock("label", ""));
}

void BasicBlock::setNext(BasicBlock::Ptr instr)
{
	_next = instr;
}

void BasicBlock::addFirst(Instruction::Ptr first)
{
	if (_first) {
		first->setNext(_first);
	}

	_first = first;
}

BasicBlock::Ptr BasicBlock::next() const
{
	return _next;
}

Instruction::Ptr BasicBlock::first() const
{
	return _first;
}

Instruction::Ptr BasicBlock::last() const
{
	Instruction::Ptr prev = nullptr;
	for (auto it = _first; it != nullptr; it = it->next()) {
		prev = it;
	}

	return prev;
}

std::string BasicBlock::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "block: " << _name << std::endl;
	for (auto it = _first; it != nullptr; it = it->next()) {
		out << it->str(prefix+"| ");
	}

	return out.str();
}

std::string BasicBlock::name() const
{
	return _name;
}

// ------------------------------
// BasicBlock
// ------------------------------

std::string DummyInstruction::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "dummpy" << std::endl;
	return out.str();
}

// ------------------------------
// Function
// ------------------------------

Function::Function(const Function::Signature& sig)
{
	Arglist args;
	std::tie(_type, _name, args) = sig;
	for (auto decl: args) {
		_args.push_back(
			AllocaInstruction::Ptr(new AllocaInstruction(decl))
		);
	}
}

void Function::addPrefix(const std::string& prefix)
{
	_prefix += prefix;
}

void Function::setFirst(const BasicBlock::Ptr body)
{
	_first = body;
}

BasicBlock::Ptr Function::first() const
{
	return _first;
}

BasicBlock::Ptr Function::last() const
{
	auto prev = _first;
	for (auto it = _first; it != nullptr; it = it->next()) {
		prev = it;
	}

	return prev;
}

bool Function::isVoid() const
{
	return !_type.has_value();
}

std::string Function::name() const
{
	return _name;
}

PossibleDatatype Function::type() const
{
	return _type;
}

const std::vector<AllocaInstruction::Ptr> Function::args() const
{
	return _args;
}

std::vector<Datatype> Function::argTypes() const
{
	std::vector<Datatype> types;
	for (auto arg: _args) {
		types.push_back(arg->type());
	}

	return types;
}

std::string Function::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "function: " << _name << std::endl;
	for (auto bb = _first; bb != nullptr; bb = bb->next()) {
		out << bb->str(prefix+"  ");
	}

	return out.str();
}

// ------------------------------
// BranchInstruction
// ------------------------------

BranchInstruction::BranchInstruction(
		Expression::ValueType expr,
		BasicBlock::Ptr ifBlock,
		BasicBlock::Ptr elseBlock):
	_expr(expr),
	_if(ifBlock),
	_else(elseBlock)
{
}

std::string BranchInstruction::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "condition: " << _expr->to_string() << std::endl;
	out << _if->str(prefix+"  ");
	out << prefix << "else: " << std::endl;
	out << _else->str(prefix+"  ");

	return out.str();
}

// ------------------------------
// Return
// ------------------------------

Return::Return(Expression::ValueType expr):
	_expr(expr)
{
}

bool Return::isVoid() const
{
	return _expr == nullptr;
}


std::string Return::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "return " << (_expr ? _expr->to_string() : "VOID") << std::endl;
	return out.str();
}

// ------------------------------
// LoopInstruction
// ------------------------------

LoopInstruction::LoopInstruction(
		Expression::ValueType expr,
		BasicBlock::Ptr body):
	_expr(expr),
	_body(body)
{
}

std::string LoopInstruction::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "while " << _expr->to_string() << std::endl;
	out << _body->str(prefix+"  ");
	return out.str();
}

// ------------------------------
// AllocaInstruction
// ------------------------------

AllocaInstruction::AllocaInstruction(const Declaration& decl)
{
	std::tie(_type, _varName) = decl;
}

void AllocaInstruction::addPrefix(const std::string& prefix)
{
	_prefix += prefix;
}

Datatype AllocaInstruction::type() const
{
	return _type;
}

std::string AllocaInstruction::name() const
{
	return _varName;
}

std::string AllocaInstruction::str(const std::string& prefix) const
{
	std::ostringstream out;

	out << prefix << "alloca " << _type.to_string() << " " << _varName
		<< " (prefix: " << _prefix << " )" << std::endl;

	return out.str();
}

// ------------------------------
// Assignment
// ------------------------------

Assignment::Assignment(AllocaInstruction::Ptr ptr, Expression::ValueType expr):
	_ptr(ptr), _expr(expr)
{
}

std::string Assignment::str(const std::string& prefix) const
{
	std::ostringstream out;

	out << prefix << "assignment: " << _expr->to_string() << std::endl;
	out << _ptr->str(prefix+" -> ");
	
	return out.str();
}

// ------------------------------
// Class
// ------------------------------

Class::Class(const std::string& name, Class::Ptr parent):
	_name(name),
	_parent(parent)
{
}

void Class::setBase(Class::Ptr base)
{
	_parent = base;
}

Class::Ptr Class::getBase() const
{
	return _parent;
}

void Class::add(Function::Ptr method, const Visibility& v)
{
	method->addPrefix(_name);

	switch (v) {
	case Visibility::Private:
		_privateMethods.push_back(method);
		break;
	case Visibility::Protected:
		_protectedMethods.push_back(method);
		break;
	default:
		_publicMethods.push_back(method);
	}
}

void Class::add(AllocaInstruction::Ptr attr, const Visibility& v)
{
	attr->addPrefix(_name);

	switch (v) {
	case Visibility::Private:
		_privateAttrs.push_back(attr);
		break;
	case Visibility::Protected:
		_protectedAttrs.push_back(attr);
		break;
	default:
		_publicAttrs.push_back(attr);
	}
}

Function::Ptr Class::getMethod(const std::string& name, const std::vector<Datatype>& argtypes, const Visibility& v) const
{
	switch (v) {
		case Visibility::Protected:
		case Visibility::Private: {
			auto it = std::find_if(_privateMethods.begin(), _privateMethods.end(), [name, argtypes](const auto& method) {
				return method->name() == name && method->argTypes() == argtypes;
			});
			if (it != _publicMethods.end())
				return *it;

			auto pit = std::find_if(_protectedMethods.begin(), _protectedMethods.end(), [name, argtypes](const auto& method) {
				return method->name() == name && method->argTypes() == argtypes;
			});
			if (pit != _publicMethods.end())
				return *pit;
		}
		case Visibility::Public:
		default: {
			auto it = std::find_if(_publicMethods.begin(), _publicMethods.end(), [name, argtypes](const auto& method) {
				return method->name() == name && method->argTypes() == argtypes;
			});
			if (it != _publicMethods.end())
				return *it;

			if (_parent)
				return _parent->getMethod(name, argtypes, v);
		}
	}

	return nullptr;
}

Function::Ptr Class::getMethod(const std::string& name, const Visibility& v) const
{
	switch (v) {
		case Visibility::Protected:
		case Visibility::Private: {
			auto it = std::find_if(_privateMethods.begin(), _privateMethods.end(), [name](const auto& method) {
				return method->name() == name;
			});
			if (it != _publicMethods.end())
				return *it;

			auto pit = std::find_if(_protectedMethods.begin(), _protectedMethods.end(), [name](const auto& method) {
				return method->name() == name;
			});
			if (pit != _publicMethods.end())
				return *pit;
		}
		case Visibility::Public:
		default: {
			auto it = std::find_if(_publicMethods.begin(), _publicMethods.end(), [name](const auto& method) {
				return method->name() == name;
			});
			if (it != _publicMethods.end())
				return *it;

			if (_parent)
				return _parent->getMethod(name, v);
		}
	}

	return nullptr;
}

AllocaInstruction::Ptr Class::getAttribute(const std::string& name, const Visibility& v) const
{
	if ((v == Visibility::Protected) || (v == Visibility::Private)) {
		auto it = std::find_if(_privateAttrs.begin(), _privateAttrs.end(), [name](const auto& attr) {
			return attr->name() == name;
		});
		if (it != _privateAttrs.end())
			return *it;

		auto pit = std::find_if(_protectedAttrs.begin(), _protectedAttrs.end(), [name](const auto& attr) {
			return attr->name() == name;
		});
		if (pit != _protectedAttrs.end())
			return *it;
	}

	auto it = std::find_if(_publicAttrs.begin(), _publicAttrs.end(), [name](const auto& attr) {
		return attr->name() == name;
	});
	if (it != _publicAttrs.end())
		return *it;

	if (_parent)
		return _parent->getAttribute(name, v);

	return nullptr;
}

std::string Class::name() const
{
	return _name;
}

std::string Class::str(const std::string& prefix) const
{
	std::ostringstream out;
	out << prefix << "class: " << _name << " : " << (_parent ? _parent->name() : "nullptr") << std::endl;
	out << "public methods:" << std::endl;
	for (auto& m: _publicMethods) {
		out << m->str("  ");
	}
	if (_publicMethods.empty())
		out << prefix << "  -- None" << std::endl;

	out << "private methods:" << std::endl;
	for (auto& m: _privateMethods) {
		out << m->str("  ");
	}
	if (_privateMethods.empty())
		out << prefix << "  -- None" << std::endl;

	out << "protected methods:" << std::endl;
	for (auto& m: _protectedMethods) {
		out << m->str("  ");
	}
	if (_protectedMethods.empty())
		out << prefix << "  -- None" << std::endl;

	out << "public attributes:" << std::endl;
	for (auto& m: _publicAttrs) {
		out << m->str("  ");
	}
	if (_publicAttrs.empty())
		out << prefix << "  -- None" << std::endl;

	out << "private attributes:" << std::endl;
	for (auto& m: _privateAttrs) {
		out << m->str("  ");
	}
	if (_privateAttrs.empty())
		out << prefix << "  -- None" << std::endl;

	out << "protected attributes:" << std::endl;
	for (auto& m: _protectedAttrs) {
		out << m->str("  ");
	}
	if (_protectedAttrs.empty())
		out << prefix << "  -- None" << std::endl;

	return out.str();
}
