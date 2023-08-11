
#include <algorithm>

#include "cllr/cllrasm.h"
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

uint32_t Assembler::addString(ref<const std::string> str)
{
	auto index = strs.size();

	strs.push_back(str);

	return (uint32_t)index;
}

SSA Assembler::createSSA(Opcode op)
{
	ssaRefs.push_back(0);
	ssaToOp.push_back(op);
	ssaToCode.push_back(nullptr);

	return nextSSA++;
}

sptr<Instruction> Assembler::codeFor(SSA id) const
{
	return ssaToCode.at(id);
}

Opcode Assembler::opFor(SSA id) const
{
	return ssaToOp.at(id);
}

SSA Assembler::push(ref<Instruction> ins)
{
	auto ssa = ins.index;

	//NOTE: ID == 0 is not an error

	if (ssa != 0 && ssaToOp[ssa] != Opcode::UNKNOWN)
	{
		//TODO complain
	}

	auto ins_sptr = new_sptr<Instruction>(ins);

	code->push_back(ins_sptr);

	doBookkeeping(ins_sptr);

	return ssa;
}

SSA Assembler::pushType(ref<Instruction> ins)
{
	auto found = types.find(ins);

	if (found != types.end())
	{
		return found->second.first;
	}

	auto id = push(ins);

	sptr<LowType> t = nullptr;

	switch (ins.op)
	{
		case Opcode::TYPE_VOID: t = new_sptr<LowVoid>(); break;
		case Opcode::TYPE_FLOAT: t = new_sptr<LowFloat>(ins.operands[0]); break;
		case Opcode::TYPE_INT_SIGN: pass;
		case Opcode::TYPE_INT_UNSIGN: t = new_sptr<LowInt>(ins.op, ins.operands[0]); break;
		case Opcode::TYPE_ARRAY: t = new_sptr<LowArray>(ins.operands[0], getType(ins.refs[0])); break;
		case Opcode::TYPE_VECTOR: t = new_sptr<LowVector>(ins.operands[0], getType(ins.refs[0])); break;
		case Opcode::TYPE_MATRIX: t = new_sptr<LowMatrix>(ins.operands[0], ins.operands[1], getType(ins.refs[0])); break;
		case Opcode::TYPE_STRUCT: t = new_sptr<LowStruct>(); break;
		case Opcode::TYPE_BOOL: t = new_sptr<LowBool>(); break;
		//case Opcode::TYPE_PTR: t = new_sptr<LowPointer>();
		//case Opcode::TYPE_TUPLE: t = new_sptr<LowTuple>();
		default: break;//TODO complain
	}

	types.emplace(ins, std::pair(id, t));

	return id;
}

void Assembler::pushAll(std::vector<Instruction> ins)
{
	code->reserve(code->size() + ins.size());

	for (auto& i : ins)
	{
		auto ins = new_sptr<Instruction>(i);
		code->push_back(ins);
		doBookkeeping(ins);

	}

}

sptr<LowType> Assembler::getType(SSA id) const
{
	auto& found = types.find(*ssaToCode[id]);

	if (found == types.end())
	{
		return nullptr;
	}

	return found->second.second;
}

std::pair<SSA, uint32_t> Assembler::pushInput(std::string name, SSA type)
{
	if (ioNames.find(name) == ioNames.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}
	
	if (outputs.find(name) != outputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	if (inputs.find(name) != inputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	uint32_t index = (uint32_t)inputs.size();
	SSA nextIn = pushNew(Instruction(Opcode::VAR_SHADER_IN, { index }, { type }));
	
	inputs.emplace(name, nextIn);
	inputNames.push_back(std::pair(name, index));

	return std::pair(nextIn, index);
}

std::pair<SSA, uint32_t> Assembler::pushOutput(std::string name, SSA type)
{
	if (ioNames.find(name) == ioNames.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	if (inputs.find(name) != inputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	if (outputs.find(name) != outputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	uint32_t index = (uint32_t)outputs.size();
	SSA nextOut = pushNew(Instruction(Opcode::VAR_SHADER_OUT, { index }, { type }));
	
	outputs.emplace(name, nextOut);
	outputNames.push_back(std::pair(name, index));

	return std::pair(nextOut, index);
}

void Assembler::addIOName(std::string name)
{
	auto& [itr, success] = ioNames.insert(name);

	if (!success)
	{
		//TODO complain
	}

}

uint32_t Assembler::replace(SSA in, SSA out)
{
	if (in == 0)
	{
		//TODO complain
		return 0;
	}

	uint32_t count = 0;
	uint32_t limit = ssaRefs[in];

	if (limit == 0)
	{
		//TODO complain
		return 0;
	}

	for (auto const& op : *code)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (op->refs[i] == in)
			{
				op->refs[i] = out;
				++count;

			}

		}

		if (count == limit)
		{
			break;
		}

		if (op->outType == in)
		{
			op->outType = out;
			++count;
		}

		if (count == limit)
		{
			break;
		}

	}

	if (out != 0)
	{
		ssaRefs[out] += count;
		ssaToCode[out] = ssaToCode[in];
		ssaToCode[out]->index = out;

	}

	ssaRefs[in] = 0;
	ssaToCode[in] = nullptr;//TODO find alternative

	return count;
}

uint32_t Assembler::flatten()
{
	uint32_t replaced = 0;
	uint32_t lastSSA = nextSSA - 1;

	for (uint32_t i = 1; i < nextSSA; ++i)
	{
		if (ssaRefs[i] != 0)
		{
			continue;
		}
		
		for (uint32_t off = i + 1; off < nextSSA; ++off)
		{
			if (ssaRefs[off] != 0)
			{
				replace(off, i);
				++replaced;
				lastSSA = i;
				break;
			}

		}

	}

	nextSSA = lastSSA + 1;

	for (size_t i = 0; i < ssaToCode.size(); ++i)
	{
		if (ssaToCode[i] == nullptr)
		{
			ssaToCode.erase(ssaToCode.begin() + i);
		}

	}

	return replaced;
}

void Assembler::doBookkeeping(sptr<Instruction> ins)
{
	if (ins->index != 0)
	{
		ssaToCode[ins->index] = ins;
	}

	for (auto i = 0; i < 3; ++i)
	{
		auto refID = ins->refs[i];

		if (refID != 0)
		{
			ssaRefs[refID] += 1;
		}

	}

	if (ins->outType != 0)
	{
		ssaRefs[ins->outType] += 1;
	}

	if (ins->op == Opcode::STRUCT_MEMBER)
	{
		getType(ins->refs[0])->addMember(ins->index, getType(ins->refs[1]));
	}

}
