#include <fstream>
#include <iostream>
#include <algorithm>

#include <vypcomp/generator/generator.h>
#include <vypcomp/ir/instructions.h>

using namespace vypcomp;

Generator::Generator(std::string out_filename, bool verbose) 
    : verbose(verbose) 
{
    out = std::make_unique<std::ofstream>(out_filename);
}

Generator::Generator(std::unique_ptr<std::ostream> out, bool verbose)
    : out(std::move(out)), verbose(verbose)
{
}

const Generator::OutputStream& Generator::get_output() const
{
    return *out.get();
}

void Generator::generate(const vypcomp::SymbolTable& symbol_table)
{
    *out << "CALL [$SP] main" << "\n" << "JUMP ENDOFPROGRAM" << std::endl;

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
                *out << "LABEL readInt\n";
                *out << "READI $0\n";
                *out << "RETURN [$SP]\n" << std::endl;
            }
            else if (function->name() == "readString")
            {
                *out << "LABEL readString\n";
                *out << "READS $0\n";
                *out << "RETURN [$SP]\n" << std::endl;
            }
            else if (function->name() == "length")
            {
                *out << "LABEL length\n";
                *out << "GETSIZE $0, [$SP-1]\n";
                *out << "SET $1, [$SP]\n";
                *out << "SUBI $SP, $SP, 1\n"; // length has one parameter
                *out << "RETURN $1\n" << std::endl;
            }
            else if (function->name() == "subStr")
            {

            }
            else
            {
                generate(function);
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

    *out << "LABEL ENDOFPROGRAM";
}

void Generator::generate(vypcomp::ir::Function::Ptr input)
{
    if (!input) return;
    auto first_block = input->first();
    *out << "LABEL " << input->name() << std::endl;
    auto local_variables = get_alloca_instructions(first_block->first());
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
        *out << "ADDI $SP, $SP, " << variable_count << std::endl;
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
            *out << "# " << alloca_instr->name() << " [$SP-" << offset << "]" << std::endl;
        }
        for (auto& alloca_instr : local_variables)
        {
            auto offset = variable_offsets[alloca_instr.get()];
            *out << "# " << alloca_instr->name() << " [$SP-" << offset << "]" << std::endl;
        }
    }

    for (auto instruction = first_block->first(); instruction != nullptr; instruction = instruction->next())
    {
        generate_instruction(instruction, variable_offsets);
    }

    if (!is_return(input->first()->last()))
    {
        *out << "SET $0, 0" << std::endl;
        generate_return();
    }
}

void vypcomp::Generator::generate_instruction(vypcomp::ir::Instruction::Ptr input, OffsetMap& variable_offsets)
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
            generate_expression(expr, "", variable_offsets);
        }
        else
        {
            auto variable_offset_iter = variable_offsets.find(destination.get());
            if (variable_offset_iter == variable_offsets.end()) throw std::runtime_error("Assignment destination was not found while generating assignment");
            auto& [_, variable_offset] = *variable_offset_iter;
            auto result_register = std::string("$0");
            generate_expression(expr, result_register, variable_offsets);
            *out << "SET [$SP-" << variable_offset << "], " << result_register << std::endl;
        }
    }
    else if (auto instr = dynamic_cast<ir::Return*>(input.get()))
    {
        if (!instr->isVoid())
        {
            auto expr = instr->getExpr();
            generate_expression(expr, "$0", variable_offsets);
        }
        generate_return();
    }
    else
    {
        std::cerr << "skipping past instruction:\n" << input->str("") << std::endl;
        //throw std::runtime_error("Generator encountered unsupported IR instruction type.");
    }
}

void Generator::generate_return()
{
    //  high address
    // |  ...    |
    // |  loc2   |
    // |  loc1   |
    // | return  |
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
        *out << "SUBI $SP, $SP, " << variable_count;
        if (verbose)
            *out << " # [$SP] is now return address\n";
        else
            *out << "\n";
    }
    if (arg_count != 0)
    {
        // reclaim stack of arguments
        *out << "SET $1, [$SP]" << "\n";
        *out << "SUBI $SP, $SP, " << arg_count << "\n";
        *out << "RETURN $1" << std::endl;
    }
    else
    {
        *out << "RETURN [$SP]" << std::endl;
    }
    *out << std::endl;
}

void vypcomp::Generator::generate_expression(ir::Expression::ValueType input, RegisterName destination, OffsetMap& variable_offsets)
{   
    if (auto lit_expr = dynamic_cast<ir::LiteralExpression*>(input.get()))
    {
        auto lit_value = lit_expr->getValue();
        if (destination.size() == 0) throw std::runtime_error("Can't assign literal expression to null.");
        *out << "SET " << destination << ", " << lit_value.vypcode_representation() << std::endl;
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
                generate_expression(argument, "$0", variable_offsets);
                switch (prim_type)
                {
                case ir::PrimitiveDatatype::Int:
                    *out << "WRITEI $0" << std::endl;
                    break;
                case ir::PrimitiveDatatype::String:
                    *out << "WRITES $0" << std::endl;
                    break;
                case ir::PrimitiveDatatype::Float:
                    *out << "WRITEF $0" << std::endl;
                    break;
                default:
                    throw std::runtime_error("Unexpected primitive type in print.");
                }

            }
        }
        else
        {
            // reserve stack space
            *out << "ADDI $SP, $SP, " << args_count << std::endl;
            std::for_each(variable_offsets.begin(), variable_offsets.end(), [args_count](auto& ptr_offset_pair) { ptr_offset_pair.second += args_count;  });
            for (std::size_t i = 0; i < args_count; i++)
            {
                const ir::Expression::ValueType& argument = function_args[i];
                generate_expression(argument, "$0", variable_offsets);
                std::int64_t offset = args_count - i; // first argument has lowest stack address, last is $SP-1
                *out << "SET " << "[$SP-" << offset << "], $0" << std::endl;
            }
            std::for_each(variable_offsets.begin(), variable_offsets.end(), [args_count](auto& ptr_offset_pair) { ptr_offset_pair.second -= args_count;  });
            *out << "CALL [$SP], " << func_name << std::endl;
        }
    }
    else if (auto symb_expr = dynamic_cast<ir::SymbolExpression*>(input.get()))
    {
        if (destination.size() == 0) throw std::runtime_error("Can't assign symbol expression to null.");
        auto alloca_src = symb_expr->getValue();
        auto find_result = variable_offsets.find(alloca_src.get());
        if (find_result == variable_offsets.end()) throw std::runtime_error("Did not find assigned offset to alloca instruction.");
        auto& [_, offset] = *find_result;
        *out << "SET " << destination << ", " << "[$SP-" << offset << "]" << std::endl;
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

std::vector<ir::AllocaInstruction::Ptr> vypcomp::Generator::get_alloca_instructions(vypcomp::ir::Instruction::Ptr first)
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
            auto allocas_if = get_alloca_instructions(branch_instr->getIf()->first());
            auto allocas_else = get_alloca_instructions(branch_instr->getElse()->first());
            result.insert(result.end(), allocas_if.begin(), allocas_if.end());
            result.insert(result.end(), allocas_else.begin(), allocas_else.end());
        }
        else if (auto loop_instr = std::dynamic_pointer_cast<ir::LoopInstruction>(current))
        {
            auto allocas_body = get_alloca_instructions(loop_instr->getBody()->first());
            result.insert(result.end(), allocas_body.begin(), allocas_body.end());
        }
    }
    return result;
}