#include <fstream>
#include <iostream>
#include <algorithm>

#include <vypcomp/generator/generator.h>
#include <vypcomp/ir/instructions.h>

using namespace vypcomp;
using namespace std::string_literals;

vypcomp::Generator::Generator(std::string out_filename, bool verbose)
    : verbose(verbose) 
{
    _main_out = std::make_unique<std::ofstream>(out_filename);
}

vypcomp::Generator::Generator(std::unique_ptr<std::ostream> out, bool verbose)
    : _main_out(std::move(out)), verbose(verbose)
{
}

const vypcomp::Generator::OutputStream& vypcomp::Generator::get_output() const
{
    return *_main_out.get();
}

void vypcomp::Generator::generate(const vypcomp::SymbolTable& symbol_table)
{
    OutputStream& out = *_main_out;
    out << "#! /bin/vypint\n# VYPcode: 1.0\n# Generated by: xmicka11 & xkubov06\n";
    // program prolog, makes the order of functions meaningless
    out << "CALL [$SP] main" << "\n" << "JUMP ENDOFPROGRAM" << std::endl;

    for (auto [_, symbol] : symbol_table.data()) {
        if (std::holds_alternative<ir::Function::Ptr>(symbol))
        {
            auto function = std::get<ir::Function::Ptr>(symbol);
            // handle embedded intrinsic functions
            if (function->name() == "print")
            {
                // just do nothing and generate WRITEI, WRITES, WRITEF directly on call site
            }
            else if (function->name() == "readInt")
            {
                out << "LABEL readInt\n";
                out << "READI $0\n";
                out << "SET $1, [$SP]\n";
                out << "SUBI $SP, $SP, 1\n"; // length has no parameters
                out << "RETURN $1\n" << std::endl;
            }
            else if (function->name() == "readString")
            {
                out << "LABEL readString\n";
                out << "READS $0\n";
                out << "SET $1, [$SP]\n";
                out << "SUBI $SP, $SP, 1\n"; // length has no parameters
                out << "RETURN $1\n" << std::endl;
            }
            else if (function->name() == "length")
            {
                out << "LABEL length\n";
                out << "GETSIZE $0, [$SP-1]\n";
                out << "SET $1, [$SP]\n";
                out << "SUBI $SP, $SP, 2\n"; // length has one parameter
                out << "RETURN $1\n" << std::endl;
            }
            else if (function->name() == "subStr")
            {

            }
            else
            {
                generate(function, out);
            }
        }
        else if (std::holds_alternative<ir::Class::Ptr>(symbol))
        {
            auto class_symbol = std::get<ir::Class::Ptr>(symbol);
            std::cerr << "class generation is not supported yet, skipping " << class_symbol->name() << std::endl;
        }
        else
        {
            throw std::runtime_error("unexpected symbol on top level symbol table");
        }
    }
    // program epilog
    out << "LABEL ENDOFPROGRAM";
}

void vypcomp::Generator::generate(vypcomp::ir::Function::Ptr input, OutputStream& out)
{
    if (!input) return;
    auto first_block = input->first();
    out << "LABEL " << input->name() << std::endl;
    // TempVarMap holds destination for each expression result 
    // (currently each expression producing new value gets separate stack location aka "local variable" with lifetime of the whole function execution)
    TempVarMap temporary_variables_mapping; 
    // local_variables consists of all possible local variables with variable in sub-scopes as well
    auto local_variables = get_alloca_instructions(first_block->first(), temporary_variables_mapping);
    auto& args = input->args();
    arg_count = args.size();
    variable_count = local_variables.size();
    OffsetMap variable_offsets{};
    if (arg_count != 0)
    {
        // assign $SP offsets of arguments
        for (std::size_t i = 0; i < arg_count; i++)
        {
            auto& alloca_instr = args[i];
            std::int64_t offset = arg_count - i; // first arg has lowest stack position, last arg is $SP-1
            variable_offsets[alloca_instr.get()] = offset;
        }
    }
    if (variable_count != 0)
    {
        // if there are any variables in the possible instruction stream, reserve stack space for them
        out << "ADDI $SP, $SP, " << variable_count << std::endl;
        // shift the offsets of function arguments
        std::for_each(variable_offsets.begin(), variable_offsets.end(), [this](auto& ptr_offset_pair) { ptr_offset_pair.second += variable_count;  });
        // insert $SP offsets of local variables
        for (std::size_t i = 0; i < variable_count; i++)
        {
            auto& alloca_instr = local_variables[i];
            std::int64_t offset = variable_count - i - 1; // last variable is [$SP]
            variable_offsets[alloca_instr.get()] = offset;
        }
    }
    if (verbose)
    {
        // dump offsets of all local symbols into code
        for (auto& alloca_instr : args)
        {
            auto offset = variable_offsets[alloca_instr.get()];
            out << "# [$SP-" << offset << "] " << alloca_instr->name() << std::endl;
        }
        for (auto& alloca_instr : local_variables)
        {
            auto offset = variable_offsets[alloca_instr.get()];
            out << "# [$SP-" << offset << "] " << alloca_instr->name() << std::endl;
        }
    }

    generate_block(input->first(), variable_offsets, temporary_variables_mapping, out);

    if (!is_return(input->first()->last()))
    {
        out << "SET $0, 0" << std::endl;
        generate_return(out);
    }
}

void vypcomp::Generator::generate_block(vypcomp::ir::BasicBlock::Ptr in_block, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out)
{
    for (auto instruction = in_block->first(); instruction != nullptr; instruction = instruction->next())
    {
        generate_instruction(instruction, variable_offsets, temporary_variables_mapping, out);
    }
}

void vypcomp::Generator::generate_return(OutputStream& out)
{
    //  high address
    // |  ...    | < SP after prolog
    // |  loc2   |
    // |  loc1   |
    // | return  | < SP at entry
    // |  arg3   |
    // |  arg2   |
    // |  arg1   |
    // |  ...    |
    //  low address
    // stack layout
    // generate implicit 0 return
    if (variable_count != 0)
    {
        // reclaim stack of local variables
        out << "SUBI $SP, $SP, " << variable_count;
        if (verbose)
            out << " # [$SP] is now return address\n";
        else
            out << "\n";
    }
    // reclaim stack of arguments, move by at least one (return address)
    out << "SET $1, [$SP]" << "\n";
    out << "SUBI $SP, $SP, " << arg_count+1 << "\n";
    out << "RETURN $1\n" << std::endl;
}

void vypcomp::Generator::generate_instruction(vypcomp::ir::Instruction::Ptr input, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out)
{
    if (auto instr = dynamic_cast<ir::AllocaInstruction*>(input.get()))
    {
        return; // these are handled elsewhere
    }
    else if (auto instr = dynamic_cast<ir::Assignment*>(input.get()))
    {
        auto destination = instr->getAlloca();
        auto expr = instr->getExpr();
        if (!destination) 
        {
            // statement level function call that discards the result
            generate_expression(expr, "", variable_offsets, temporary_variables_mapping, out);
        }
        else
        {
            auto variable_offset_iter = variable_offsets.find(destination.get());
            if (variable_offset_iter == variable_offsets.end()) throw std::runtime_error("Assignment destination was not found while generating assignment");
            auto& [_, variable_offset] = *variable_offset_iter;
            auto result_register = std::string("$0");
            generate_expression(expr, result_register, variable_offsets, temporary_variables_mapping, out);
            out << "SET [$SP-" << variable_offset << "], " << result_register << std::endl;
        }
    }
    else if (auto instr = dynamic_cast<ir::Return*>(input.get()))
    {
        if (!instr->isVoid())
        {
            auto expr = instr->getExpr();
            generate_expression(expr, "$0", variable_offsets, temporary_variables_mapping, out);
        }
        generate_return(out);
    }
    else if (auto instr = dynamic_cast<ir::BranchInstruction*>(input.get()))
    {
        static std::uint64_t if_label_index = 0;
        auto str_label_index = std::to_string(if_label_index++);
        auto expr = instr->getExpr();
        auto if_block = instr->getIf();
        auto else_block = instr->getElse();
        auto label_if = "if_branch_"s + str_label_index;
        auto label_else = "else_branch_"s + str_label_index;
        auto label_end = "endif_label_"s + str_label_index;
        std::stringstream if_instruction_stream, else_instruction_stream;

        generate_block(if_block, variable_offsets, temporary_variables_mapping, if_instruction_stream);
        generate_block(else_block, variable_offsets, temporary_variables_mapping, else_instruction_stream);

        generate_expression(expr, "$0", variable_offsets, temporary_variables_mapping, out);
        out << "JUMPZ " << label_else << ", $0\n";

        out << "LABEL " << label_if << "\n"; // TODO: this is not necessary
        if (if_instruction_stream.rdbuf()->in_avail())
            out << if_instruction_stream.rdbuf();
        out << "JUMP " << label_end << "\n";

        out << "LABEL " << label_else << "\n";
        if (else_instruction_stream.rdbuf()->in_avail())
            out << else_instruction_stream.rdbuf();
        out << "JUMP " << label_end << "\n"; // TODO: this is not necessary

        out << "LABEL " << label_end << std::endl;
    }
    else if (auto instr = dynamic_cast<ir::LoopInstruction*>(input.get()))
    {
        static std::uint64_t while_label = 0;
        auto str_while_label = std::to_string(while_label);
        auto expr = instr->getExpr();
        auto body_block = instr->getBody();
        auto condition_label = "while_cond_"s + str_while_label;
        auto end_label = "while_end_"s + str_while_label;

        std::stringstream body_instruction_stream;
        generate_block(body_block, variable_offsets, temporary_variables_mapping, body_instruction_stream);

        out << "LABEL " << condition_label << "\n";
        generate_expression(expr, "$0", variable_offsets, temporary_variables_mapping, out);
        out << "JUMPZ " << end_label << ", $0\n";
        if (body_instruction_stream.rdbuf()->in_avail())
            out << body_instruction_stream.rdbuf();
        out << "JUMP " << condition_label << "\n";
        out << "LABEL " << end_label << std::endl;
    }
    else
    {
        std::cerr << "skipping past instruction:\n" << input->str("") << std::endl;
        //throw std::runtime_error("Generator encountered unsupported IR instruction type.");
    }
}

void vypcomp::Generator::generate_expression(ir::Expression::ValueType input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out)
{   
    if (auto lit_expr = dynamic_cast<ir::LiteralExpression*>(input.get()))
    {
        auto lit_value = lit_expr->getValue();
        if (destination.size() == 0) throw std::runtime_error("Can't assign literal expression to null.");
        out << "SET " << destination << ", " << lit_value.vypcode_representation() << std::endl;
    }
    else if (auto func_expr = dynamic_cast<ir::FunctionExpression*>(input.get()))
    {
        auto func_name = func_expr->getFunction()->name();
        auto function_args = func_expr->getArgs();
        const auto args_count = function_args.size();
        if (func_name == "print")
        {
            // each argument is a standalone call
            for (std::size_t i = 0; i < args_count; i++)
            {
                const ir::Expression::ValueType& argument = function_args[i];
                ir::Datatype argument_type = argument->type();
                // should be safe because primitive types are checked by parser
                ir::PrimitiveDatatype prim_type = argument_type.get<ir::PrimitiveDatatype>();
                generate_expression(argument, "$0", variable_offsets, temporary_variables_mapping, out);
                switch (prim_type)
                {
                case ir::PrimitiveDatatype::Int:
                    out << "WRITEI $0" << std::endl;
                    break;
                case ir::PrimitiveDatatype::String:
                    out << "WRITES $0" << std::endl;
                    break;
                case ir::PrimitiveDatatype::Float:
                    out << "WRITEF $0" << std::endl;
                    break;
                default:
                    throw std::runtime_error("Unexpected primitive type in print.");
                }
            }
        }
        else
        {
            // reserve stack space
            out << "ADDI $SP, $SP, " << args_count + 1; // at least one for return address, last arg is $SP-1
            if (verbose)
                out << " # reserved stack for " << args_count << " function parameters + return address" << std::endl;
            else
                out << std::endl;
            // shift local variable offsets by the amount stack increased
            std::for_each(variable_offsets.begin(), variable_offsets.end(), [args_count](auto& ptr_offset_pair) { ptr_offset_pair.second += args_count + 1ll;  });
            for (std::size_t i = 0; i < args_count; i++)
            {
                const ir::Expression::ValueType& argument = function_args[i];
                generate_expression(argument, "$0", variable_offsets, temporary_variables_mapping, out);
                std::int64_t offset = args_count - i; // first argument has lowest stack address, last is $SP-1
                out << "SET " << "[$SP-" << offset << "], $0" << std::endl;
            }
            out << "CALL [$SP], " << func_name << std::endl;
            // return value register is always $0
            if (destination.size() && destination != "$0")
                out << "SET " << destination << ", $0" << std::endl;
            // shift local variable offsets back, since callee cleaned up the stack
            std::for_each(variable_offsets.begin(), variable_offsets.end(), [args_count](auto& ptr_offset_pair) { ptr_offset_pair.second -= args_count + 1ll;  });
        }
    }
    else if (auto symb_expr = dynamic_cast<ir::SymbolExpression*>(input.get()))
    {
        if (destination.size() == 0) throw std::runtime_error("Can't assign symbol expression to null.");
        auto alloca_src = symb_expr->getValue();
        auto find_result = variable_offsets.find(alloca_src.get());
        if (find_result == variable_offsets.end()) throw std::runtime_error("Did not find assigned offset to alloca instruction.");
        auto& [_, offset] = *find_result;
        out << "SET " << destination << ", " << "[$SP-" << offset << "]" << std::endl;
    }
    else if (auto binop = dynamic_cast<ir::BinaryOpExpression*>(input.get()))
    {
        // TODO: replace "" with destination searched for here
        // search for it in temporary_variables_mapping
        // set destination to result in proper alloca
        // As a result L370ish-400ish should get easier to read
        auto result_destination = get_expr_destination(binop, temporary_variables_mapping, variable_offsets);
        generate_binaryop(std::dynamic_pointer_cast<ir::BinaryOpExpression>(input), result_destination, variable_offsets, temporary_variables_mapping, out);
    }
    else
    {
        throw std::runtime_error("Generator encountered unsupported expression type: " + input->to_string());
    }
}

void vypcomp::Generator::generate_binaryop(ir::BinaryOpExpression::Ptr input, DestinationName destination, OffsetMap& variable_offsets, TempVarMap& temporary_variables_mapping, OutputStream& out)
{
    // prepare operands
    auto op1 = input->getOp1();
    std::string op1_location;
    if (op1->is_simple())
    {
        op1_location = "$1";
    }
    else
    {
        op1_location = get_expr_destination(op1.get(), temporary_variables_mapping, variable_offsets);
    }

    auto op2 = input->getOp2();
    std::string op2_location;
    if (op2->is_simple())
    {
        op2_location = "$2";
    }
    else
    {
        op2_location = get_expr_destination(op2.get(), temporary_variables_mapping, variable_offsets);
    }
            
    if (!op1->is_simple())
        generate_expression(op1, op1_location, variable_offsets, temporary_variables_mapping, out);
    generate_expression(op2, op2_location, variable_offsets, temporary_variables_mapping, out);
    if (op1->is_simple()) // it's going to be just a simple register set, set it after computing op2, because it can utilize $1 register and overwrite the result
        generate_expression(op1, op1_location, variable_offsets, temporary_variables_mapping, out);
    // execute operation
    if (auto addop = dynamic_cast<ir::AddExpression*>(input.get()))
    {

        if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Int))
            out << "ADDI $0, " << op1_location << ", " << op2_location << "\n";
        else if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Float))
            out << "ADDF $0, " << op1_location << ", " << op2_location << "\n";
        else if (input->type() == ir::Datatype(ir::PrimitiveDatatype::String))
        {
            // COPY dst, op1
            // GETSIZE sz1, op1
            // GETSIZE sz2, op2
            // ADDI sz, sz1, sz2
            // RESIZE dst, sz
            //
            // SET ctr, 0
            // LABEL strcpy_loop
            // LTI jmp, ctr, sz2
            // JUMPZ strcpy_end
            // ADDI off, sz1, ctr
            // GETWORD value, op2, ctr
            // SETWORD op1, off, value
            // ADDI ctr, ctr1, 1
            // JUMP strcpy_loop
            // LABEL strcpy_end
            throw std::runtime_error("String concat not implemented yet.");
        }
        else
            throw std::runtime_error("Unexpected operand in add operation: "s + input->to_string());
        out << "SET " << destination << ", $0" << std::endl;
    }
    else if (auto minop = dynamic_cast<ir::SubtractExpression*>(input.get()))
    {
        // TODO: test minus operation
        if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Int))
        {
            out << "SUBI $0, " << op1_location << ", " << op2_location << "\n";
        }
        else if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Float))
            out << "SUBF $0, " << op1_location << ", " << op2_location << "\n";
        else
        {
            throw std::runtime_error("Unexpected operand in subtract opertaion: "s + input->to_string());
        }
    }
    else if (auto eqop = dynamic_cast<ir::ComparisonExpression*>(input.get()))
    {
        switch (eqop->getOperation())
        {
        case ir::ComparisonExpression::EQUALS:
            if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Int))
                out << "EQI $0, " << op1_location << ", " << op2_location << "\n";
            else if (input->type() == ir::Datatype(ir::PrimitiveDatatype::Float))
                out << "EQF $0, " << op1_location << ", " << op2_location << "\n";
            else if (input->type() == ir::Datatype(ir::PrimitiveDatatype::String))
                out << "EQS $0, " << op1_location << ", " << op2_location << "\n";
            else
            {
                // TODO: != and == should support object types as well for chunk comparison (see vypa20.pdf)
                throw std::runtime_error("Unexpected operand type in subtract opertaion: "s + input->to_string());
            }
            break;
        // TODO: all other cases
        default:
            throw std::runtime_error("Unexpected comparison type in comparison: "s + input->to_string());
        }
    }
    else
    {
        throw std::runtime_error("Generator encountered unsupported expression type: " + input->to_string());
    }
}

bool vypcomp::Generator::is_alloca(vypcomp::ir::Instruction::Ptr instr) const
{
    return dynamic_cast<ir::AllocaInstruction*>(instr.get()) != nullptr;
}

bool vypcomp::Generator::is_return(vypcomp::ir::Instruction::Ptr instr) const
{
    return dynamic_cast<ir::Return*>(instr.get()) != nullptr;
}

vypcomp::Generator::AllocaVector vypcomp::Generator::get_alloca_instructions(vypcomp::ir::Instruction::Ptr first, TempVarMap& exp_temporary_mapping)
{
    std::vector<ir::AllocaInstruction::Ptr> result;

    for (auto current = first; current != nullptr; current = current->next())
    {
        if (auto alloca_instr = std::dynamic_pointer_cast<ir::AllocaInstruction>(current))
        {
            result.push_back(alloca_instr);
        }
        else if (auto branch_instr = std::dynamic_pointer_cast<ir::BranchInstruction>(current))
        {
            auto allocas_cond = get_temporary_allocas(branch_instr->getExpr(), exp_temporary_mapping);
            auto allocas_if = get_alloca_instructions(branch_instr->getIf()->first(), exp_temporary_mapping);
            auto allocas_else = get_alloca_instructions(branch_instr->getElse()->first(), exp_temporary_mapping);
            result.insert(result.end(), allocas_cond.begin(), allocas_cond.end());
            result.insert(result.end(), allocas_if.begin(), allocas_if.end());
            result.insert(result.end(), allocas_else.begin(), allocas_else.end());
        }
        else if (auto loop_instr = std::dynamic_pointer_cast<ir::LoopInstruction>(current))
        {
            auto allocas_cond = get_temporary_allocas(loop_instr->getExpr(), exp_temporary_mapping);
            auto allocas_body = get_alloca_instructions(loop_instr->getBody()->first(), exp_temporary_mapping);
            result.insert(result.end(), allocas_cond.begin(), allocas_cond.begin());
            result.insert(result.end(), allocas_body.begin(), allocas_body.end());
        }
        else if (auto assignment = std::dynamic_pointer_cast<ir::Assignment>(current))
        {
            // if assignment, analyze if expressions needs temporaries
            auto expr = assignment->getExpr();
            auto allocas = get_temporary_allocas(expr, exp_temporary_mapping);
            result.insert(result.end(), allocas.begin(), allocas.end());
        }
        else if (auto ret_instr = std::dynamic_pointer_cast<ir::Return>(current))
        {
            if (!ret_instr->isVoid())
            {
                // analyze if return expression needs temporaries
                auto expr = ret_instr->getExpr();
                auto allocas = get_temporary_allocas(expr, exp_temporary_mapping);
                result.insert(result.end(), allocas.begin(), allocas.end());
            }
        }
    }
    return result;
}

std::vector<ir::AllocaInstruction::Ptr> vypcomp::Generator::get_temporary_allocas(ir::Expression::ValueType expr, TempVarMap& exp_temporary_mapping)
{
    std::vector<ir::AllocaInstruction::Ptr> result;
    if (!expr->is_simple())
    {
        auto required_temps = get_required_temporaries(expr, exp_temporary_mapping);
        result.insert(result.end(), required_temps.begin(), required_temps.end());
    }
    return result;
}

std::vector<ir::AllocaInstruction::Ptr> vypcomp::Generator::get_required_temporaries(ir::Expression::ValueType expr, TempVarMap& exp_temporary_mapping)
{
    std::vector<ir::AllocaInstruction::Ptr> result;
    // very conservative approach to generating temporaries, possible improvement would reuse old temporaries and free registers
    // for now, every binary expression result is stored in a new stack variable
    if (expr->is_simple()) 
        return {};
    else if (auto func_expr = dynamic_cast<ir::FunctionExpression*>(expr.get()))
    {
        auto arg_expressions = func_expr->getArgs();
        for (auto& arg_expression : arg_expressions)
        {
            auto temps = get_required_temporaries(arg_expression, exp_temporary_mapping);
            result.insert(result.end(), temps.begin(), temps.end());
        }
        auto func_result_temp = std::make_shared<ir::AllocaInstruction>(std::make_pair(func_expr->type(), func_expr->to_string()));
        exp_temporary_mapping[func_expr] = func_result_temp.get();
        result.push_back(func_result_temp);
    }
    else if (auto binop_exp = dynamic_cast<ir::BinaryOpExpression*>(expr.get()))
    {
        auto op1 = binop_exp->getOp1();
        auto op2 = binop_exp->getOp2();
        auto op1_temps = get_required_temporaries(op1, exp_temporary_mapping);
        result.insert(result.end(), op1_temps.begin(), op1_temps.end());
        auto op2_temps = get_required_temporaries(op2, exp_temporary_mapping);
        result.insert(result.end(), op2_temps.begin(), op2_temps.end());

        auto new_temporary = std::make_shared<ir::AllocaInstruction>(std::make_pair(binop_exp->type(), binop_exp->to_string()));
        exp_temporary_mapping[binop_exp] = new_temporary.get();
        result.push_back(new_temporary);
    }
    else
    {
        throw std::runtime_error("Unexpected expression type in get_required_temporaries. expr is "s + expr->to_string());
    }
    return result;
}

std::optional<std::size_t> vypcomp::Generator::find_offset(AllocaRawPtr alloca_ptr, OffsetMap& variable_offsets) const
{
    if (auto search_result = variable_offsets.find(alloca_ptr); search_result != variable_offsets.end())
    {
        return search_result->second;
    }
    return std::nullopt;
}

std::optional<vypcomp::Generator::AllocaRawPtr> vypcomp::Generator::find_expr_destination(ExprRawPtr expr, TempVarMap& temporary_variables_mapping) const
{
    if (auto mapping_result = temporary_variables_mapping.find(expr); mapping_result != temporary_variables_mapping.end())
    {
        return mapping_result->second;
    }
    return std::nullopt;
}

// for non-simple expressions only, gets the stack position for given expression to store result into
vypcomp::Generator::DestinationName vypcomp::Generator::get_expr_destination(ExprRawPtr expr, TempVarMap& temporary_variables_mapping, OffsetMap& variable_offsets) const
{
    auto exp_destination = find_expr_destination(expr, temporary_variables_mapping);
    if (!exp_destination) throw std::runtime_error("Binary operation expression destination was not a temporary variable: "s + expr->to_string());
    auto destination_offset = find_offset(exp_destination.value(), variable_offsets);
    if (!destination_offset) throw std::runtime_error("Binary operation expression destination's offset not found: "s + expr->to_string());
    return "[$SP-"s + std::to_string(destination_offset.value()) + "]"s;
}
