#include <vypcomp/generator/generator.h>
#include <vypcomp/ir/expression.h>

using namespace vypcomp;

void Generator::generate(vypcomp::ir::BasicBlock::Ptr input, Generator::OutputStream& out)
{
    if (!input) return;
    out << "LABEL " << input->name() << std::endl;
    auto alloca_instructions = get_alloca_instructions(input->first());
    auto variable_count = alloca_instructions.size();
    if (variable_count != 0)
    {
        // if there are any variables in the possible instruction stream, reserve stack space for them
        out << "ADDI $SP, $SP, " << variable_count << std::endl;
        if (verbose)
        {
            for (std::size_t i = 0; i < variable_count; i++)
            {
                auto& alloca = alloca_instructions[i];
                std::int64_t offset = variable_count - i;
                out << "# " << alloca->name() << " [$SP-" << offset << "]" << std::endl;
            }
        }
    }

    for (auto instruction = input->first(); instruction != nullptr; instruction = instruction->next())
    {
        generate_instruction(instruction, out);
    }
}

void vypcomp::Generator::generate_instruction(vypcomp::ir::Instruction::Ptr input, OutputStream& out)
{
    if (auto instr = dynamic_cast<ir::AllocaInstruction*>(input.get()))
    {
        return; // these are handled elsewhere
    }
    else if (auto instr = dynamic_cast<ir::Assignment*>(input.get()))
    {
        auto destination = instr->getAlloca();
        auto expr = instr->getExpr();
        auto result_register = generate_expression(expr, out); // may need to get return register, or give the generate_expression destination address
        // SET (get stack position of the destination) << ", " << result_register << std::endl;
    }
    else
    {
        throw std::runtime_error("Generator encountered unsupported IR instruction type.");
    }
}

vypcomp::Generator::RegisterName vypcomp::Generator::generate_expression(ir::Expression::ValueType input, OutputStream& out)
{
    if (auto lit_expr = dynamic_cast<ir::LiteralExpression*>(input.get()))
    {

        throw std::runtime_error("Generator encountered unsupported expression type.");
    }
    else
    {
        throw std::runtime_error("Generator encountered unsupported expression type.");
    }
}

bool vypcomp::Generator::is_alloca(vypcomp::ir::Instruction::Ptr instr)
{
    return dynamic_cast<ir::AllocaInstruction*>(instr.get()) != nullptr;
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