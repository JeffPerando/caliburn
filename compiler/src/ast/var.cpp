
#include "ast/var.h"

#include "ast/values.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

/*
TODO: Type checking
*/

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

	ss << ' ' << name;

	if (initValue != nullptr)
	{
		ss << " = ";
		initValue->prettyPrint(ss);
	}

}

cllr::TypedSSA LocalVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id.value != 0)
	{
		return id;
	}

	if (initValue == nullptr)
	{
		initValue = new_sptr<ZeroValue>();
	}

	auto initRes = initValue->emitValueCLLR(table, codeAsm);
	cllr::TypedSSA v;

	MATCH(initRes, cllr::TypedSSA, valPtr)
	{
		v = *valPtr;
	}
	else
	{
		codeAsm.errors->err("Invalid variable initializer", *initValue);
		return cllr::TypedSSA();
	}

	sptr<cllr::LowType> type = nullptr;

	if (typeHint == nullptr)
	{
		type = v.type;

	}
	else if (auto t = typeHint->resolve(table, codeAsm))
	{
		//TODO check for compatibility with initial value

		type = t;

	}
	else
	{
		//TODO complain
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_LOCAL, { (uint32_t)mods }, { type->id, v.value }));

	id = cllr::TypedSSA(type, vID);

	return id;
}

cllr::TypedSSA LocalVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	emitVarCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { id.value }, t->id));

		return cllr::TypedSSA(t, vID);
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

void LocalVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	emitVarCLLR(table, codeAsm);
	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { id.type->id, rhs.value }));

}

//======================================
//==========	GlobalVariable	========
//======================================

void GlobalVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::TypedSSA GlobalVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id.value != 0)
	{
		return id;
	}

	sptr<cllr::LowType> type = 0;

	auto initRes = initValue->emitValueCLLR(table, codeAsm);
	cllr::TypedSSA v;

	MATCH(initRes, cllr::TypedSSA, valPtr)
	{
		v = *valPtr;
	}
	else
	{
		codeAsm.errors->err("Invalid variable initializer", *initValue);
		return cllr::TypedSSA();
	}

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		//TODO type check

		type = t;

	}
	else
	{
		type = v.type;
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_GLOBAL, {}, { type->id, v.value }));

	id = cllr::TypedSSA(type, vID);

	return id;
}

cllr::TypedSSA GlobalVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//TODO why do we even bother using global variables properly?

	emitVarCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { id.value }, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void GlobalVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	//TODO complain
}

//======================================
//==========	FnArgVariable	========
//======================================

void FnArgVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << name;

}

cllr::TypedSSA FnArgVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id.value != 0)
	{
		return id;
	}

	if (typeHint == nullptr)
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_FUNC_ARG, { argIndex }, { t->id }));

		id = cllr::TypedSSA(t, vID);

		return id;
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA FnArgVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}

void FnArgVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
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

cllr::TypedSSA ShaderIOVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (id.value != 0)
	{
		return id;
	}

	if (ioType == ShaderIOVarType::UNKNOWN)
	{
		return cllr::TypedSSA();
	}

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		std::pair<uint32_t, cllr::SSA> ioData;

		if (ioType == ShaderIOVarType::INPUT)
		{
			ioData = codeAsm.pushInput(name, t->id);
		}
		else
		{
			ioData = codeAsm.pushOutput(name, t->id);
		}

		ioIndex = ioData.first;
		id = cllr::TypedSSA(t, ioData.second);

		return id;
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA ShaderIOVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (ioType == ShaderIOVarType::OUTPUT)
	{
		return cllr::TypedSSA();
	}

	ioType = ShaderIOVarType::INPUT;

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { emitVarCLLR(table, codeAsm).value }, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void ShaderIOVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	if (ioType == ShaderIOVarType::INPUT)
	{
		//TODO complain
		return;
	}

	ioType = ShaderIOVarType::OUTPUT;

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { emitVarCLLR(table, codeAsm).value, rhs.value }));

}

//======================================
//========	DescriptorVariable	========
//======================================

void DescriptorVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::TypedSSA DescriptorVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}

cllr::TypedSSA DescriptorVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}

void DescriptorVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{

}
