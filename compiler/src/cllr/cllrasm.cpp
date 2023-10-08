
#include <algorithm>

#include "cllr/cllrasm.h"
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

uint32_t Assembler::addString(in<std::string> str)
{
	auto index = strs.size();

	strs.push_back(str);

	return (uint32_t)index;
}

SSA Assembler::createSSA(Opcode op)
{
	ssaRefs.push_back(0);
	ssaToOp.push_back(op);
	ssaToIndex.push_back(0);

	return nextSSA++;
}

out<Instruction> Assembler::codeFor(SSA id) const
{
	return code->at(ssaToIndex.at(id));
}

out<Instruction> Assembler::codeAt(size_t off) const
{
	return code->at(off);
}

Opcode Assembler::opFor(SSA id) const
{
	return ssaToOp.at(id);
}

SSA Assembler::push(in<Instruction> ins)
{
	auto ssa = ins.index;

	//NOTE: ID == 0 is not an error

	if (ssa != 0 && ssaToOp[ssa] != Opcode::UNKNOWN)
	{
		//TODO complain
	}

	code->push_back(ins);
	doBookkeeping(ins);

	return ssa;
}

std::pair<SSA, sptr<LowType>> Assembler::pushType(in<Instruction> ins)
{
	auto found = types.find(ins);

	if (found != types.end())
	{
		return found->second;
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

	auto ret = std::pair(id, t);

	types.emplace(ins, ret);

	return ret;
}

void Assembler::pushAll(in<std::vector<Instruction>> ins)
{
	for (auto& i : ins)
	{
		code->push_back(i);
		doBookkeeping(i);

	}

}

sptr<LowType> Assembler::getType(SSA id) const
{
	auto& found = types.find(code->at(ssaToIndex[id]));

	if (found == types.end())
	{
		return nullptr;
	}

	return found->second.second;
}

std::pair<uint32_t, SSA> Assembler::pushInput(in<std::string> name, SSA type)
{
	if (outputs.find(name) != outputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	auto found = inputs.find(name);

	if (found != inputs.end())
	{
		return found->second;
	}

	uint32_t index = (uint32_t)inputs.size();
	SSA in = pushNew(Instruction(Opcode::VAR_SHADER_IN, { index }, { type }));
	
	auto inData = std::pair(index, in);

	inputs.emplace(name, inData);
	inputNames.push_back(std::pair(name, index));

	return inData;
}

std::pair<uint32_t, SSA> Assembler::pushOutput(in<std::string> name, SSA type)
{
	if (inputs.find(name) != inputs.end())
	{
		//TODO complain
		return std::pair(0, 0);
	}

	auto found = outputs.find(name);

	if (found != outputs.end())
	{
		return found->second;
	}

	uint32_t index = (uint32_t)outputs.size();
	SSA out = pushNew(Instruction(Opcode::VAR_SHADER_OUT, { index }, { type }));
	
	auto outData = std::pair(index, out);

	outputs.emplace(name, outData);
	outputNames.push_back(std::pair(name, index));

	return outData;
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
		return 0;
	}

	for (auto& op : *code)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			if (op.refs[i] == in)
			{
				op.refs[i] = out;
				++count;

			}

		}

		if (count == limit)
		{
			break;
		}

		if (op.outType == in)
		{
			op.outType = out;
			++count;
		}

		if (count == limit)
		{
			break;
		}

	}

	if (out != 0)
	{
		ssaToOp[out] = ssaToOp[in];
		ssaRefs[out] += count;
		ssaToIndex[out] = ssaToIndex[in];
		
	}

	ssaRefs[in] = 0;
	ssaToOp[in] = Opcode::UNKNOWN;
	ssaToIndex[in] = 0;

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

	for (size_t i = 0; i < ssaToIndex.size(); ++i)
	{
		if (ssaToOp.at(ssaToIndex[i]) == Opcode::UNKNOWN)
		{
			ssaToIndex.erase(ssaToIndex.begin() + i);
		}

	}

	return replaced;
}

void Assembler::doBookkeeping(in<Instruction> ins)
{
	if (ins.index != 0)
	{
		ssaToIndex[ins.index] = code->size() - 1;
		ssaToOp[ins.index] = ins.op;
	}

	for (auto i = 0; i < 3; ++i)
	{
		auto const& refID = ins.refs[i];

		if (refID != 0)
		{
			ssaRefs[refID] += 1;
		}

	}

	if (ins.outType != 0)
	{
		ssaRefs[ins.outType] += 1;
	}

	if (ins.op == Opcode::STRUCT_MEMBER)
	{
		//FIXME I forgot about members when writing this code
		getType(ins.refs[0])->addMember("", ins.index, getType(ins.refs[1]));
	}

}
