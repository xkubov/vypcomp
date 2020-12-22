/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

#include <fstream>
#include <stdexcept>
#include <variant>

#include "vypcomp/parser/parser.h"

using namespace vypcomp;
using namespace std::string_literals;

SymbolTable initSymbolTable()
{
	auto table = SymbolTable(true);
	auto object = ir::Class::Ptr(new ir::Class("Object", nullptr));
	table.insert({ "Object", object });
	// Object built-in functions
	// string toString(void)
	{
		auto toString_fn = Function::Ptr(new Function(Function::Signature(
			PrimitiveDatatype::String,
			std::string("toString"),
			Arglist{ {
				std::make_pair(Datatype(object->name()), "this"s),
			} }
		)));
		object->add(toString_fn);
	}
	// string getClass(void)
	{
		auto getClass_fn = Function::Ptr(new Function(Function::Signature(
			PrimitiveDatatype::String,
			std::string("getClass"),
			Arglist{ {
				std::make_pair(Datatype(object->name()), "this"s),
			} }
		)));
		object->add(getClass_fn);
	}
	
	// free built-in functions
	// int readInt(void)
	table.insert({ "readInt", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::Int), "readInt", Arglist())) });
	// int readFloat(void)
	table.insert({ "readFloat", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::Float), "readFloat", Arglist())) });
	// string readString(void)
	table.insert({ "readString", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::String), "readString", Arglist())) });
	// int length(string s)
	table.insert({ "length", std::make_shared<ir::Function>(std::make_tuple(Datatype(PrimitiveDatatype::Int), std::string("length"), Arglist{ std::make_pair(Datatype(PrimitiveDatatype::String), "s")} )) });
	auto subStr_ptr = std::make_shared<ir::Function>(Function::Signature(
		PrimitiveDatatype::String, 
		std::string("subStr"), 
		Arglist{ { 
			std::make_pair(Datatype(PrimitiveDatatype::String), "s"s),
			std::make_pair(Datatype(PrimitiveDatatype::Int), "i"s),
			std::make_pair(Datatype(PrimitiveDatatype::Int), "n"s)
		} }
	));
	// string subStr(string s, int i, int n)
	table.insert({ "subStr", subStr_ptr });
	// void print(PrimitiveDatatype i, ...)
	table.insert({ "print", std::make_shared<ir::Function>(std::make_tuple(std::nullopt, "print", Arglist())) }); // a special one handled differently in parser
	return table;
}

ParserDriver::ParserDriver()
{
	_tables.push_back(initSymbolTable());
}

ParserDriver::ParserDriver(const SymbolTable& global)
{
	_tables.push_back(global);
}

ParserDriver::~ParserDriver()
{
}

const SymbolTable& ParserDriver::table() const
{
	// First table is global.
	return _tables[0];
}

Class::Ptr ParserDriver::getClass(const std::string &name) const
{
	if (auto symbol = searchTables(name)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol)) {
			std::string msg = std::holds_alternative<Function::Ptr>(*symbol) ?
					"cannot derive from function" :
					"invalid derivation of class";

			throw IncompabilityError(msg);
		}

		return std::get<Class::Ptr>(*symbol);
	}

	throw SemanticError("class not defined: "+name);
}

void ParserDriver::parse(const std::string &filename)
{
	std::ifstream input(filename);
	if (!input.good())
		throw std::runtime_error("invalid file: "+filename);

	parse(input);
}

void ParserDriver::parse(std::istream &file)
{
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file, Parser::token::PROGRAM_START) );
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, this));

	if (int err = _parser->parse()) {
		throw SyntaxError("parser returned: "+std::to_string(err));
	}
}

void ParserDriver::parseExpression(std::istream& file, bool debug_on)
{
	_scanner = std::unique_ptr<Scanner>(new vypcomp::Scanner(file, Parser::token::EXPR_PARSE_START));
	_parser = std::unique_ptr<Parser>(new vypcomp::Parser(*_scanner, this));
	if (debug_on)
		_parser->set_debug_level(1);

	if (int err = _parser->parse()) {
		throw SyntaxError("parser returned: " + std::to_string(err));
	}
}

void ParserDriver::parseStart(ir::Function::Ptr fun)
{
	_tables.back().insert({fun->name(), fun});

	pushSymbolTable();
	for (auto arg: fun->args()) {
		add(arg);
	}
	
	// If method add implicit this parameter
	if (_currClass) {
		auto& args = fun->args();
		if (args.size() == 0 || (args.front()->name() != "this")) {
			auto thisArg = AllocaInstruction::Ptr(new AllocaInstruction({
						Datatype(_currClass->name()), "this"}));
			args.insert(args.begin(), thisArg);
		}

		if (auto orig = _currClass->getBase()->getOriginalMethod(fun->name(), ir::Class::Visibility::Private)) {
			if (orig->type() != fun->type()) {
				throw SemanticError("override on type is not supported");
			}
			if (orig->args().size() != fun->args().size()) {
				throw SemanticError("override not supported size");
			}
			auto funTypes = fun->argTypes();
			auto it = funTypes.begin();
			bool first = true;
			for (auto origType: orig->argTypes()) {
				if (first) {
					first = false;
					it++;
					continue;
				}
				if (origType != *it)
					if (!origType.is<ir::Datatype::InvalidDatatype>() && !it->is<ir::Datatype::InvalidDatatype>())
						throw SemanticError("override not supported, expected: "+origType.to_string()+" got: "+it->to_string());
				it++;
			}
		}
	}

	_currFunction = fun;
}

void ParserDriver::parseStart(ir::Class::Ptr cl)
{
	_tables.back().insert({cl->name(), cl});
	pushSymbolTable(true);
	_currClass = cl;

	cl->clear();
	for (auto a: cl->publicMethods()) {
		_tables.back().insert({a->name(), a});
	}
	for (auto a: cl->privateMethods()) {
		_tables.back().insert({a->name(), a});
	}
	for (auto a: cl->protectedMethods()) {
		_tables.back().insert({a->name(), a});
	}
	for (auto a: cl->publicAttributes()) {
		_tables.back().insert({a->name(), a});
	}
	for (auto a: cl->privateAttributes()) {
		_tables.back().insert({a->name(), a});
	}
	for (auto a: cl->protectedAttributes()) {
		_tables.back().insert({a->name(), a});
	}
}

Class::Ptr ParserDriver::newClass(const std::string &name, const std::string& base) const
{
        if (auto symbol = searchGlobal(name)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol))
			throw std::runtime_error("Invalid state in ParserDriver:"+std::to_string(__LINE__));

		auto cl = std::get<Class::Ptr>(*symbol);
		
		cl->setBase(getClass(base));
		auto parent = getClass(base);
		while (parent->name() != "Object") {
			if (parent->name() == name) {
				throw SemanticError("cyclic derivation of class "+name);
			}
			parent = parent->getBase();
		}
		return cl;
        }

	return Class::Ptr(new Class(name, getClass(base)));
}

Function::Ptr ParserDriver::newFunction(const ir::Function::Signature& sig) const
{
	auto [type, name, args] = sig;
        if (auto symbol = searchCurrent(name)) {
		if (!std::holds_alternative<Function::Ptr>(*symbol))
			throw std::runtime_error("Invalid state in ParserDriver:"+std::to_string(__LINE__));
		auto fun = std::get<Function::Ptr>(*symbol);
		fun->setSignature(sig);

		return fun;
        }

        return Function::Ptr(new Function({type, name, args}));
}

Instruction::Ptr ParserDriver::assign(ir::Expression::ValueType dest_expr,
                       const ir::Expression::ValueType &val) const
{
		std::string name;
		if (auto symbexp = std::dynamic_pointer_cast<SymbolExpression>(dest_expr))
		{
			name = symbexp->getValue()->name();
		}
		else if (auto obj_attr = std::dynamic_pointer_cast<ObjectAttributeExpression>(dest_expr))
		{
			return std::make_shared<ObjectAssignment>(dest_expr, val);
		}
		else
		{
			throw IncompabilityError("Only symbol expression or object attribute allowed as assignment target: " + dest_expr->to_string());
		}
        if (auto symbol = searchTables(name)) {
		if (!std::holds_alternative<AllocaInstruction::Ptr>(*symbol))
			throw IncompabilityError("Cannot assign to function.");

		auto var = std::get<AllocaInstruction::Ptr>(*symbol);
		checkAssignmentTypes(var->type(), val->type());

		return Assignment::Ptr(
			new Assignment(
				std::get<AllocaInstruction::Ptr>(*symbol),
				val
			)
		);
        }

	throw SemanticError("Assignment to undefined variable "+name);
}

Instruction::Ptr ParserDriver::assign(const std::string& name,
	const ir::Expression::ValueType& val) const
{
	if (auto symbol = searchTables(name)) {
		if (!std::holds_alternative<AllocaInstruction::Ptr>(*symbol))
			throw IncompabilityError("Cannot assign to function.");

		auto var = std::get<AllocaInstruction::Ptr>(*symbol);
		checkAssignmentTypes(var->type(), val->type());

		return Assignment::Ptr(
			new Assignment(
				std::get<AllocaInstruction::Ptr>(*symbol),
				val
			)
		);
	}

	throw SemanticError("Assignment to undefined variable " + name);
}

void ParserDriver::checkAssignmentTypes(const Datatype& dest_type, const Datatype& value_type) const
{
	if (dest_type != value_type) 
	{
		// value_type is an object type, test whether dest_type is not a parent of value_type
		if (dest_type.is<Datatype::ClassName>() && value_type.is<Datatype::ClassName>())
		{
			auto dest_class_name = dest_type.get<Datatype::ClassName>();
			auto value_class_name = value_type.get<Datatype::ClassName>();
			auto dest_class_search_result = searchGlobal(dest_class_name);
			if (!dest_class_search_result) throw std::runtime_error("Destination in assignment of type \"" + dest_class_name + "\" has class type of class that is missing from SymbolTable");
			auto value_class_search_result = searchGlobal(value_class_name);
			if (!value_class_search_result) throw std::runtime_error("Value in assignment of type \"" + value_class_name + "\" has class type of class that is missing from SymbolTable");
			if (!std::holds_alternative<Class::Ptr>(dest_class_search_result.value()) || !std::holds_alternative<Class::Ptr>(value_class_search_result.value()))
				throw std::runtime_error("Type of destination or value in assignment is not a class.");
			auto dest_class_ptr = std::get<Class::Ptr>(dest_class_search_result.value());
			auto val_class_ptr = std::get<Class::Ptr>(value_class_search_result.value());
			if (Class::canAssign(dest_class_ptr, val_class_ptr))
				return;
		}
		throw IncompabilityError("Invalid type, can't assign " + value_type.to_string() + " to " + dest_type.to_string());
	}
}

void ParserDriver::checkArgTypes(const Function::Ptr& function_ptr, const FunctionExpression::ArgExpressions& real_args) const
{
	// function object was found, verify arguments
	if (function_ptr->name() == "print")
	{
		if (real_args.size() < 1) throw SemanticError("print has to have at least 1 parameter");
		for (const ir::Expression::ValueType& argument : real_args)
		{
			auto arg_type = argument->type();
			if (!arg_type.isPrimitive())
			{
				throw IncompabilityError("print called with non-primitive datatype parameter.");
			}
		}
	}
	else
	{
		if (real_args.size() != function_ptr->args().size())
			throw SemanticError("Provided argument count does not match the declared parameter count.");

		for (std::size_t i = 0; i < real_args.size(); i++)
		{
			auto formal_type = function_ptr->argTypes()[i];
			auto actual_type = real_args[i]->type();
			checkAssignmentTypes(formal_type, actual_type);
		}
	}
}

std::vector<Instruction::Ptr> ParserDriver::call_func(ir::Expression::ValueType func_expr, std::vector<ir::Expression::ValueType>& args) const
{
	std::string name;
	Function::Ptr function;
	if (auto methodexp = std::dynamic_pointer_cast<MethodExpression>(func_expr))
	{
		function = methodexp->getFunction();
		// push the preceding expression as the implicit `this` argument
		args.insert(args.begin(), methodexp->getContextObj());
	}
	else if (auto funcexp = std::dynamic_pointer_cast<FunctionExpression>(func_expr))
	{
		name = funcexp->getFunction()->name();
		auto search_result = searchTables(name);
		if (search_result)
		{
			SymbolTable::Symbol symbol = search_result.value();
			if (std::holds_alternative<Function::Ptr>(symbol))
			{
				function = std::get<Function::Ptr>(symbol);
			}
			else
			{
				throw IncompabilityError("Identifier in function call is not a function.");
			}
		}
		else
		{
			throw SemanticError("Identifier does not exist.");
		}
	}
	else
	{
		throw SyntaxError("Only function or assignment allowed on statement level, got: " + func_expr->to_string());
	}
	auto funcexp = std::dynamic_pointer_cast<FunctionExpression>(func_expr);
	checkArgTypes(funcexp->getFunction(), args);
	funcexp->setArgs(args);
	return { std::make_shared<Assignment>(nullptr, func_expr) };
}

Instruction::Ptr ParserDriver::createIf(
	const ir::Expression::ValueType& val,
	const ir::BasicBlock::Ptr& if_block,
	const ir::BasicBlock::Ptr& else_block) const
{
	if (val->type() != Datatype(PrimitiveDatatype::Int) && !val->type().is<Datatype::ClassName>())
		throw IncompabilityError("Expression in if statement has to be either int or object type.");
	return BranchInstruction::Ptr(new BranchInstruction(val, if_block, else_block));
}

Instruction::Ptr ParserDriver::createWhile(
	const ir::Expression::ValueType& val,
	const ir::BasicBlock::Ptr& block) const
{
	if (val->type() != Datatype(PrimitiveDatatype::Int) && !val->type().is<Datatype::ClassName>())
		throw IncompabilityError("Expression in while statement has to be either int or object type.");
	return LoopInstruction::Ptr(new LoopInstruction(val, block));
}


Return::Ptr ParserDriver::createReturn(const ir::Expression::ValueType& val) const
{
	if (_currFunction == nullptr) {
		throw SyntaxError("Return statement out of a function");
	}

	if (val == nullptr) {
		if (!_currFunction->isVoid())
			throw IncompabilityError("Invalid return for function "+_currFunction->name()+" with type: "+_currFunction->type()->to_string());
		return Return::Ptr(new Return());
	}
	else if (_currFunction->isVoid()) {
		throw IncompabilityError("Returning non-void expression from function that is void.");
	}

	checkAssignmentTypes(*_currFunction->type(), val->type());

	return Return::Ptr(new Return(val));
}

Expression::ValueType ParserDriver::createCastExpr(const Datatype& dest_datatype, Expression::ValueType expr) const
{
	if ((!dest_datatype.is<Datatype::ClassName>() || !expr->type().is<Datatype::ClassName>()) && (dest_datatype != Datatype(PrimitiveDatatype::String) || expr->type() != Datatype(PrimitiveDatatype::Int)))
		throw IncompabilityError("Cast of object to object type or int to string is allowed.");

	if (dest_datatype.is<Datatype::ClassName>())
	{
		auto class_name = dest_datatype.get<Datatype::ClassName>();
		auto target_search_result = searchTables(class_name);
		if (!target_search_result) throw SemanticError("Target class " + class_name + " does not exist for cast expression.");
		if (!std::holds_alternative<Class::Ptr>(target_search_result.value())) throw SemanticError("Target class name " + class_name + " is not a class in cast expression.");
		auto class_ptr = std::get<Class::Ptr>(target_search_result.value());
		return std::make_shared<ObjectCastExpression>(class_ptr, expr);
	}
	else
	{
		return std::make_shared<StringCastExpression>(expr);
	}
}

void ParserDriver::verify(const ir::AllocaInstruction::Ptr& decl) const
{
	if (searchCurrent(decl->name())) {
		throw SemanticError("Redefinition of "+decl->name());
	}
	if (auto symbol = searchTables(decl->name())) {
		if (std::holds_alternative<Function::Ptr>(*symbol))
			throw SemanticError("redefinition: same name as function "+decl->name());
		if (std::holds_alternative<Class::Ptr>(*symbol))
			throw SemanticError("redefinition: same name as class "+decl->name());
	}
}

void ParserDriver::add(const AllocaInstruction::Ptr& decl)
{
	verify(decl);
	_tables.back().insert({decl->name(), decl});
}

ir::AllocaInstruction::Ptr ParserDriver::newDeclaration(const Datatype& t, const std::string& id)
{
	if (auto symbol = searchCurrent(id)) {
		if (!std::holds_alternative<AllocaInstruction::Ptr>(*symbol))
			throw std::runtime_error("Invalid state of parser.");

		auto al = std::get<AllocaInstruction::Ptr>(*symbol);
		al->setType(t);
		return al;
	}

	auto decl = AllocaInstruction::Ptr(new AllocaInstruction({t, id}));
	_tables.back().insert({decl->name(), decl});
	return decl;
}

void ParserDriver::ensureMainDefined() const
{
	if (auto symbol = searchTables("main")) {
		if (!std::holds_alternative<Function::Ptr>(*symbol)) {
			throw SemanticError("main must be function");
		}
		auto main = std::get<Function::Ptr>(*symbol);
		if (main->type())
			throw SemanticError("main must be void");

		if (main->args().size())
			throw SemanticError("main must have no args");

		return;
	}

	throw SemanticError("main not defined.");
}

void ParserDriver::pushSymbolTable(bool storeFunctions)
{
	_tables.push_back(SymbolTable(storeFunctions));
}

void ParserDriver::popSymbolTable()
{
	// We must preserve global table.
	if (_tables.size() <= 1)
		return;

	_tables.pop_back();
}

void ParserDriver::parseClassEnd()
{
	if (_currClass == nullptr)
		throw std::runtime_error("Invalid usage of parseClassEnd");

	popSymbolTable();
	_currClass = nullptr;
}

void ParserDriver::parseFunctionEnd()
{
	if (_currFunction == nullptr)
		throw std::runtime_error("Invalid usage of parseFunctionEnd");

	popSymbolTable();
	_currFunction = nullptr;
}

std::optional<SymbolTable::Symbol> ParserDriver::searchTables(const SymbolTable::Key& key) const
{
	for (auto it = _tables.rbegin(); it != _tables.rend(); ++it) {
		if (it->has(key))
			return it->get(key);
	}

	return {};
}

std::optional<SymbolTable::Symbol> ParserDriver::searchGlobal(const SymbolTable::Key& key) const
{
	if (_tables[0].has(key))
		return _tables[0].get(key);

	return {};
}

std::optional<SymbolTable::Symbol> ParserDriver::searchCurrent(const SymbolTable::Key& key) const
{
	if (_tables[_tables.size()-1].has(key))
		return _tables[_tables.size()-1].get(key);

	return {};
}

Class::Ptr ParserDriver::getCurrentClass() const
{
	return _currClass;
}

Datatype ParserDriver::customDatatype(const std::string& dt) const
{
	if (auto symbol = searchTables(dt)) {
		if (!std::holds_alternative<Class::Ptr>(*symbol))
			throw SemanticError("not a type: "+dt);

		return Datatype(dt);
	}

	throw SemanticError("Invalid datatype.");
}

// Expressions

ir::Expression::ValueType ParserDriver::identifierExpr(const std::string& name) const
{
	auto search_result = searchTables(name);
	if (search_result)
	{
		SymbolTable::Symbol symbol = search_result.value();
		if (std::holds_alternative<AllocaInstruction::Ptr>(symbol))
		{
			auto instruction = std::get<AllocaInstruction::Ptr>(symbol);
			return std::make_shared<SymbolExpression>(instruction);
		}
		else if (std::holds_alternative<Function::Ptr>(symbol))
		{
			auto func = std::get<Function::Ptr>(symbol);
			return std::make_shared<FunctionExpression>(func);
		}
		else
		{
			throw SemanticError("Unsupported identifier type in expression.");
		}
	}
	else
	{
		throw SemanticError("Undeclared identifier in expression.");
	}
}

ir::Expression::ValueType ParserDriver::functionCall(
	const ir::Expression::ValueType& function_expr,
	std::vector<ir::Expression::ValueType>& args) const
{
	if (!function_expr->type().is<Datatype::FunctionType>())
	{
		throw IncompabilityError("Function call attempted on non-function expression.");
	}
	else
	{
		if (auto methodexp = dynamic_cast<MethodExpression*>(function_expr.get()))
		{
			// push the preceding expression as the implicit `this` argument
			args.insert(args.begin(), methodexp->getContextObj());
		}
		auto function_expr_childptr = dynamic_cast<FunctionExpression*>(function_expr.get());
		checkArgTypes(function_expr_childptr->getFunction(), args);
		function_expr_childptr->setArgs(args);
		return function_expr;
	}
}

ir::Expression::ValueType ParserDriver::notExpr(const ir::Expression::ValueType& expr) const
{
	return std::make_shared<NotExpression>(expr);
}

ir::Expression::ValueType ParserDriver::thisExpr() const
{
	auto current_class = getCurrentClass();
	if (!current_class)
	{
		throw SemanticError("\"this\" used outside of a method context.");
	}
	else
	{
		return std::make_shared<SymbolExpression>(_currFunction->args()[0]);
	}
}

ir::Expression::ValueType ParserDriver::superExpr() const
{
	auto current_class = getCurrentClass();
	if (!current_class)
	{
		throw SemanticError("\"super\" used outside of a method context.");
	}
	else
	{
		auto parent_class = current_class->getBase();
		if (!parent_class)
		{
			throw SemanticError("\"super\" used in method context of parentless class.");
		}
		else
		{
			return std::make_shared<SuperExpression>(_currFunction->args()[0], current_class);
		}
	}
}

ir::Expression::ValueType ParserDriver::newExpr(const std::string& class_name) const
{
	auto search_result = searchTables(class_name);
	if (!search_result) throw SemanticError("class " + class_name + " in constructor not found.");
	if (!std::holds_alternative<Class::Ptr>(search_result.value())) throw IncompabilityError("Identifier " + class_name + " is not a class.");
	auto class_ptr = std::get<Class::Ptr>(search_result.value());
	return std::make_shared<ConstructorExpression>(class_ptr);
}

ir::Expression::ValueType ParserDriver::addExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::AddExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::subExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::SubtractExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::mulExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::MultiplyExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::divExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::DivideExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::geqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::GEQ, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::gtExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::GREATER, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::leqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::LEQ, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::ltExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::LESS, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::eqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::EQUALS, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::neqExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::ComparisonExpression>(
		ComparisonExpression::NOTEQUALS, e1, e2
	);
}

ir::Expression::ValueType ParserDriver::andExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::AndExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::orExpr(
	const ir::Expression::ValueType& e1,
	const ir::Expression::ValueType& e2) const
{
	return std::make_shared<ir::OrExpression>(e1, e2);
}

ir::Expression::ValueType ParserDriver::dotExpr(
	const ir::Expression::ValueType& context_object,
	const std::string& identifier) const
{
	if (!context_object->type().is<ir::Datatype::ClassName>())
	{
		throw IncompabilityError("left hand operand of . operator is not an object variable");
	}
	auto context_class_name = context_object->type().get<ir::Datatype::ClassName>();
	std::optional<SymbolTable::Symbol> search_result = searchGlobal(context_class_name);
	if (!search_result)
	{
		// Don't think this can happen since expression can only get a class type
		// if the class search succeeded in the expr -> IDENTIFIER rule.
		throw SemanticError("left hand operand of . operator has an undefined type");
	}
	else if (!std::holds_alternative<Class::Ptr>(*search_result))
	{
		throw IncompabilityError("left hand operand of . operator is not an object type");
	}
	else
	{
		Class::Ptr expr_class = std::get<Class::Ptr>(search_result.value());
		ir::Class::Visibility vis = ir::Class::Visibility::Public;
		if (_currClass && _currClass->name() == context_class_name)
			vis = ir::Class::Visibility::Private;
		AllocaInstruction::Ptr attribute = expr_class->getAttribute(identifier, vis);
		if (attribute)
		{
			if (auto context_object_symexp = dynamic_cast<SymbolExpression*>(context_object.get()))
			{
				return std::make_shared<ObjectAttributeExpression>(context_object_symexp->getValue(), attribute, expr_class);
			}
			else
			{
				throw IncompabilityError("Object attribute access on a non-symbol expression: " + context_object->to_string());
			}
		}
		else
		{
			// try method
			Function::Ptr method = expr_class->getMethod(identifier, vis);
			if (method)
			{
				return std::make_shared<MethodExpression>(method, context_object);
			}
			else
			{
				throw SemanticError("given object has does not have a visible member called " + identifier);
			}
		}
	}
}
