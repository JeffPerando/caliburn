
#include "ast/var.h"

#include "ast/values.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

//======================================
//==========	LocalVariable	========
//======================================

void LocalVariable::prettyPrint(out<std::stringstream> ss) const
{
	ss << (isConst ? "const" : "var");

	if (typeHint != nullptr)
	{
		ss << ": ";
		typeHint->prettyPrint(ss);

	}

	ss << ' ' << nameTkn->str;

	if (initValue != nullptr)
	{
		ss << " = ";
		initValue->prettyPrint(ss);
	}

}

cllr::SSA LocalVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	if (initValue == nullptr)
	{
		initValue = new_sptr<ZeroValue>();
	}

	auto v = initValue->emitValueCLLR(table, codeAsm);

	sptr<cllr::LowType> type = 0;

	if (typeHint == nullptr)
	{
		type = v.type;

	}
	else if (auto t = typeHint->resolve(table))
	{
		auto hintID = t->emitTypeCLLR(table, codeAsm);

		//TODO check for compatibility with initial value

		type = hintID;

	}
	else
	{
		//TODO complain
	}

	id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_LOCAL, { (uint32_t)mods }, { type->id, v.value }));

	return id;
}

cllr::TypedSSA LocalVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	emitDeclCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { id }, tImpl->id));

		return cllr::TypedSSA(tImpl, vID);
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

void LocalVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { emitDeclCLLR(table, codeAsm), value }));

}

//======================================
//==========	MemberVariable	========
//======================================

void MemberVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ';
	ss << nameTkn->str;

	if (initValue != nullptr)
	{
		ss << " = ";
		initValue->prettyPrint(ss);

	}

	ss << ';';

}

cllr::SSA MemberVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	if (initValue == nullptr)
	{
		initValue = new_sptr<ZeroValue>();
	}

	auto init = initValue->emitValueCLLR(table, codeAsm);

	//TODO type check

	auto parentImpl = parent->emitTypeCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto type = t->emitTypeCLLR(table, codeAsm);
		id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::STRUCT_MEMBER, { memberIndex }, { parentImpl->id, type->id, init.value }));
		return id;
	}

	//TODO complain
	return 0;
}

cllr::TypedSSA MemberVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memberIndex }, { target }, tImpl->id));

		return cllr::TypedSSA(tImpl, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void MemberVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	auto memberLoad = emitLoadCLLR(table, codeAsm, target);

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { memberLoad.value, value }));

}

//======================================
//==========	GlobalVariable	========
//======================================

void GlobalVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::SSA GlobalVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	sptr<cllr::LowType> type = 0;
	auto v = initValue->emitValueCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		//TODO type check

		type = t->emitTypeCLLR(table, codeAsm);

	}
	else
	{
		type = v.type;
	}

	id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_GLOBAL, {}, { type->id, v.value }));

	return id;
}

cllr::TypedSSA GlobalVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	//TODO why do we even bother using global variables properly?

	emitDeclCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { id }, tImpl->id));

		return cllr::TypedSSA(tImpl, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void GlobalVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	//TODO complain
}

//======================================
//==========	FnArgVariable	========
//======================================

void FnArgVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << nameTkn->str;

}

cllr::SSA FnArgVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	if (typeHint == nullptr)
	{
		//TODO complain
		return 0;
	}

	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);

		id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_FUNC_ARG, { argIndex }, { tImpl->id }));

		return id;
	}
	
	//TODO complain
	return 0;
}

cllr::TypedSSA FnArgVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	return cllr::TypedSSA();
}

void FnArgVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	//TODO complain (maybe)
}

//======================================
//========	ShaderIOVariable	========
//======================================

void ShaderIOVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << name;

}

cllr::SSA ShaderIOVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	if (ioType == ShaderIOVarType::UNKNOWN)
	{
		return 0;
	}

	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);

		std::pair<uint32_t, cllr::SSA> ioData;

		if (ioType == ShaderIOVarType::INPUT)
		{
			ioData = codeAsm.pushInput(name, tImpl->id);
		}
		else
		{
			ioData = codeAsm.pushOutput(name, tImpl->id);
		}

		ioIndex = ioData.first;
		id = ioData.second;

		return id;
	}

	//TODO complain
	return 0;
}

cllr::TypedSSA ShaderIOVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	if (ioType == ShaderIOVarType::OUTPUT)
	{
		return cllr::TypedSSA();
	}

	ioType = ShaderIOVarType::INPUT;

	if (auto t = typeHint->resolve(table))
	{
		auto tImpl = t->emitTypeCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { emitDeclCLLR(table, codeAsm) }, tImpl->id));

		return cllr::TypedSSA(tImpl, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void ShaderIOVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	if (ioType == ShaderIOVarType::INPUT)
	{
		//TODO complain
		return;
	}

	ioType = ShaderIOVarType::OUTPUT;

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { emitDeclCLLR(table, codeAsm), value }));

}

//======================================
//========	DescriptorVariable	========
//======================================

void DescriptorVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::SSA DescriptorVariable::emitDeclCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	return 0;
}

cllr::TypedSSA DescriptorVariable::emitLoadCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target)
{
	return cllr::TypedSSA();
}

void DescriptorVariable::emitStoreCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{

}
