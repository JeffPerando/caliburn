
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

cllr::TypedSSA LocalVariable::emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm)
{
	if (varData.value != 0)
	{
		return varData;
	}

	if (initValue == nullptr)
	{
		initValue = new_sptr<ZeroValue>();
	}

	auto localScope = new_sptr<SymbolTable>(table);

	auto initRes = initValue->emitCodeCLLR(localScope, codeAsm);
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
	else if (auto t = typeHint->resolve(localScope, codeAsm))
	{
		//TODO check for compatibility with initial value

		type = t;

	}
	else
	{
		//TODO complain
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_LOCAL, { (uint32_t)mods }, { type->id, v.value }));

	varData = cllr::TypedSSA(type, vID);

	return varData;
}

cllr::TypedSSA LocalVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	auto v = emitVarCLLR(table, false, codeAsm);

	if (v.value == 0)
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { v.value }, v.type->id));

	return cllr::TypedSSA(v.type, vID);
}

void LocalVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	auto v = emitVarCLLR(table, true, codeAsm);
	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { v.type->id, rhs.value }));

}

//======================================
//==========	GlobalVariable	========
//======================================
/*
void GlobalVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::TypedSSA GlobalVariable::emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm)
{
	if (id.value != 0)
	{
		return id;
	}

	sptr<cllr::LowType> type = 0;

	auto localScope = new_sptr<SymbolTable>(table);

	auto initRes = initValue->emitCodeCLLR(localScope, codeAsm);
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

	if (auto t = typeHint->resolve(localScope, codeAsm))
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
*/
//======================================
//==========	FnArgVariable	========
//======================================

void FnArgVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << name;

}

cllr::TypedSSA FnArgVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (typeHint == nullptr)
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	if (auto t = typeHint->resolve(table, codeAsm))
	{
		if (id == 0)
		{
			id = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VAR_FUNC_ARG, { argIndex }, { t->id }));
		}
		
		return cllr::TypedSSA(t, id);
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

void FnArgVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	//TODO complain (maybe)
}

cllr::TypedSSA FnArgVariable::emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}

//======================================
//========	ShaderIOVariable	========
//======================================

void ShaderIOVariable::prettyPrint(out<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << name;

}

cllr::TypedSSA ShaderIOVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//TODO type check

	auto var = emitVarCLLR(table, false, codeAsm);
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { var.value }, var.type->id));

	return cllr::TypedSSA(var.type, vID);
}

void ShaderIOVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	//TODO type check

	auto var = emitVarCLLR(table, true, codeAsm);
	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { var.value, rhs.value }));

}

cllr::TypedSSA ShaderIOVariable::emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm)
{
	if (auto t = typeHint->resolve(table, codeAsm))
	{
		return codeAsm.pushIOVar(name, isBeingWritten ? ShaderIOVarType::OUTPUT : ShaderIOVarType::INPUT, t);
	}

	auto e = codeAsm.errors->err("Unable to resolve type for shader I/O variable", *typeHint);
	return cllr::TypedSSA();
}

//======================================
//========	DescriptorVariable	========
//======================================

void DescriptorVariable::prettyPrint(out<std::stringstream> ss) const
{

}

cllr::TypedSSA DescriptorVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}

void DescriptorVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{

}

cllr::TypedSSA DescriptorVariable::emitVarCLLR(sptr<const SymbolTable> table, bool isBeingWritten, out<cllr::Assembler> codeAsm)
{
	return cllr::TypedSSA();
}
