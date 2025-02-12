
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
/*
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

	id = cllr::TypedSSA(type, vID);

	return id;
}
*/
/*
cllr::TypedSSA LocalVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	//emitVarCLLR(table, codeAsm);

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
*/
//======================================
//==========	GlobalVariable	========
//======================================

void GlobalVariable::prettyPrint(out<std::stringstream> ss) const
{

}

/*
cllr::TypedSSA GlobalVariable::emitVarCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
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
*/

/*
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
/*
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
*/
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

cllr::TypedSSA ShaderIOVariable::emitLoadCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm)
{
	if (auto t = typeHint->resolve(table, codeAsm))
	{
		//TODO type check

		auto var = codeAsm.pushIOVar(name, ShaderIOVarType::INPUT, t);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_READ_VAR, {}, { var.value }, var.type->id));

		return cllr::TypedSSA(t, vID);
	}

	auto e = codeAsm.errors->err("Unable to resolve type for shader I/O variable", *typeHint);
	return cllr::TypedSSA();
}

void ShaderIOVariable::emitStoreCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm, cllr::TypedSSA rhs)
{
	if (auto t = typeHint->resolve(table, codeAsm))
	{
		//TODO type check

		auto var = codeAsm.pushIOVar(name, ShaderIOVarType::OUTPUT, t);
		codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { var.value, rhs.value }));
		
	}
	else
	{
		auto e = codeAsm.errors->err("Unable to resolve type for shader I/O variable", *typeHint);
	}

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
