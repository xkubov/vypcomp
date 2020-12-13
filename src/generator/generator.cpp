#include <fstream>
#include <iostream>
#include <algorithm>

#include <vypcomp/generator/generator.h>
#include <vypcomp/generator/generator.h>

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
                // WRITES / WRITEI (maybe split into printf printi in the print call parsing)
            }
            else if (function->name() == "readInt")
            {
                // READI
            }
            else if (function->name() == "readString")
            {
                // READS
            }
            else if (function->name() == "length")
            {
                // GETSIZE
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
        std::for_each(variable_offsets.begin(), variable_offsets.end(), [this](auto& ptr_offset_pair) { ptr_offset_pair.second -= variable_count;  });
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
        // may need to get return register, or give the generate_expression destination address
        // TODO: some long expressions may run out of free registers for intermediate results
        auto variable_offset_iter = variable_offsets.find(destination.get());
        if (variable_offset_iter == variable_offsets.end()) throw std::runtime_error("Assignment destination was not found while generating assignment");
        auto& [_, variable_offset] = *variable_offset_iter;
        auto result_register = std::string("$0");
        generate_expression(expr, result_register);
        *out << "SET [$SP-" << variable_offset << "], " << result_register << std::endl;
    }
    else if (auto instr = dynamic_cast<ir::Return*>(input.get()))
    {
        if (!instr->isVoid())
        {
            auto expr = instr->getExpr();
            generate_expression(expr, "$0");
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
}

void vypcomp::Generator::generate_expression(ir::Expression::ValueType input, RegisterName destination)
{
    if (auto lit_expr = dynamic_cast<ir::LiteralExpression*>(input.get()))
    {
        auto lit_value = lit_expr->getValue();
        *out << "SET " << destination << ", " << lit_value.vypcode_representation() << std::endl;
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