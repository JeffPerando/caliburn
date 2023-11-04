
#include "cllr/cllrasm.h"
#include "cllr/cllrtypes.h"

using namespace caliburn::cllr;

void Assembler::beginSect()
{
	codeSects.emplace(new Section());
}

bool Assembler::hasSect() const
{
	return !codeSects.empty();
}

void Assembler::endSect()
{
	auto& sect = codeSects.top();

	allCode.insert(allCode.end(), sect->code.begin(), sect->code.end());

	codeSects.pop();
}

SSA Assembler::createSSA(in<Instruction> ins)
{
	ssaToIns.push_back(ins);
	ssaRefs.push_back(0);
	ssaToOp.push_back(ins.op);
	
	return nextSSA++;
}

Instruction Assembler::getIns(SSA id) const
{
	if (id == 0 || ssaToIns.size() >= id)
	{
		return Instruction();
	}

	return ssaToIns.at(id - 1);
}

Opcode Assembler::getOp(SSA id) const
{
	return ssaToOp.at(id - 1);
}

sptr<LowType> Assembler::getType(SSA id) const
{
	if (auto& found = ssaToType.find(id); found != ssaToType.end())
	{
		return found->second;
	}

	return nullptr;
}

void Assembler::push(in<Instruction> ins)
{
	if (!hasSect())
	{
		//TODO complain
		return;
	}

	codeSects.top()->code.push_back(ins);

	doBookkeeping(ins);

}

void Assembler::pushAll(in<std::vector<Instruction>> ins)
{
	if (!hasSect())
	{
		//TODO complain
		return;
	}

	auto& code = codeSects.top()->code;

	code.insert(code.end(), ins.begin(), ins.end());

	for (auto& i : ins)
	{
		doBookkeeping(i);
	}

}

SSA Assembler::pushNew(out<Instruction> ins)
{
	ins.index = createSSA(ins);
	push(ins);
	return ins.index;
}

sptr<LowType> Assembler::pushType(out<Instruction> ins)
{
	//Structs can have identical operands, so ignore them
	//All other types have unique operands for a given type
	if (ins.op != Opcode::TYPE_STRUCT)
	{
		auto found = types.find(ins);

		if (found != types.end())
		{
			return found->second;
		}

	}
	
	auto id = createSSA(ins.op);
	ins.index = id;

	//TODO consider just using a type section like in the SPIR-V assembler
	allCode.push_back(ins);
	doBookkeeping(ins);

	sptr<LowType> t = nullptr;

	switch (ins.op)
	{
		case Opcode::TYPE_VOID: t = new_sptr<LowVoid>(id); break;
		case Opcode::TYPE_FLOAT: t = new_sptr<LowFloat>(id, ins.operands[0]); break;
		case Opcode::TYPE_INT_SIGN: PASS;
		case Opcode::TYPE_INT_UNSIGN: t = new_sptr<LowInt>(id, ins.op, ins.operands[0]); break;
		case Opcode::TYPE_ARRAY: t = new_sptr<LowArray>(id, ins.operands[0], getType(ins.refs[0])); break;
		case Opcode::TYPE_VECTOR: t = new_sptr<LowVector>(id, ins.operands[0], getType(ins.refs[0])); break;
		case Opcode::TYPE_MATRIX: t = new_sptr<LowMatrix>(id, ins.operands[0], ins.operands[1], getType(ins.refs[0])); break;
		case Opcode::TYPE_STRUCT: t = new_sptr<LowStruct>(id); break;
		case Opcode::TYPE_BOOL: t = new_sptr<LowBool>(id); break;
		//case Opcode::TYPE_PTR: t = new_sptr<LowPointer>(id);
		//case Opcode::TYPE_TUPLE: t = new_sptr<LowTuple>(id);
		default: break;//TODO complain
	}

	types.emplace(ins, t);
	ssaToType.emplace(id, t);

	return t;
}

void Assembler::beginLoop(SSA start, SSA end)
{
	if (!hasSect())
	{
		return;
	}

	codeSects.top()->beginLoop(start, end);

}

SSA Assembler::getLoopStart() const
{
	if (!hasSect())
	{
		return 0;
	}

	return codeSects.top()->getLoopStart();
}

SSA Assembler::getLoopEnd() const
{
	if (!hasSect())
	{
		return 0;
	}

	return codeSects.top()->getLoopEnd();
}

void Assembler::endLoop()
{
	if (!hasSect())
	{
		return;
	}

	codeSects.top()->endLoop();
}

uint32_t Assembler::addString(in<std::string> str)
{
	auto index = strs.size();

	strs.push_back(str);

	return (uint32_t)index;
}

std::string Assembler::getString(uint32_t index) const
{
	if (strs.empty() || index >= strs.size())
	{
		return "";
	}

	return strs.at(index);
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

	for (auto& op : allCode)
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
		ssaToIns[out] = ssaToIns[in];
		
	}

	ssaRefs[in] = 0;
	ssaToOp[in] = Opcode::UNKNOWN;
	ssaToIns[in] = Instruction();

	return count;
}

//TODO audit implementation
uint32_t Assembler::flatten()
{
	uint32_t replaced = 0;
	uint32_t lastSSA = nextSSA - 1;

	for (uint32_t ssa = 1; ssa < nextSSA; ++ssa)
	{
		if (ssaRefs[ssa] != 0)
		{
			continue;
		}
		
		for (uint32_t off = ssa + 1; off < nextSSA; ++off)
		{
			if (ssaRefs[off] != 0)
			{
				replace(off, ssa);
				++replaced;
				lastSSA = ssa;
				break;
			}

		}

	}

	nextSSA = lastSSA + 1;

	ssaToOp.resize(lastSSA);
	ssaToIns.resize(lastSSA);

	return replaced;
}

void Assembler::doBookkeeping(in<Instruction> ins)
{
	for (auto i = 0; i < MAX_REFS; ++i)
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

}

void Section::beginLoop(SSA start, SSA end)
{
	loops.push(Loop{ start, end });
}

SSA Section::getLoopStart() const
{
	if (loops.empty())
	{
		return 0;
	}

	return loops.top().start;
}

SSA Section::getLoopEnd() const
{
	if (loops.empty())
	{
		return 0;
	}

	return loops.top().end;
}

void Section::endLoop()
{
	loops.pop();
}
