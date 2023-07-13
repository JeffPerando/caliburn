
#include "ast/var.h"

#include "ast/values.h"

using namespace caliburn;

void LocalVariable::prettyPrint(ref<std::stringstream> ss) const
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

cllr::SSA LocalVariable::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
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

	cllr::SSA typeID = 0;

	if (typeHint == nullptr)
	{
		typeID = v.type;

	}
	else if (auto t = typeHint->resolve(table))
	{
		auto hintID = t->emitDeclCLLR(table, codeAsm);

		//TODO check for compatibility with initial value

		typeID = hintID;

	}
	else
	{
		//TODO complain
	}

	id = codeAsm.pushNew(cllr::Opcode::VAR_LOCAL, { (uint32_t)mods }, { typeID, v.value });

	return id;
}

cllr::TypedSSA LocalVariable::emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target)
{
	emitDeclCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);

		auto vID = codeAsm.pushNew(cllr::Opcode::VALUE_READ_VAR, {}, { id });

		return cllr::TypedSSA(t, tID, vID);
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

void LocalVariable::emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { emitDeclCLLR(table, codeAsm), value });

}

void MemberVariable::prettyPrint(ref<std::stringstream> ss) const
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

cllr::SSA MemberVariable::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
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

	auto parentID = parent->emitDeclCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto typeID = t->emitDeclCLLR(table, codeAsm);
		id = codeAsm.pushNew(cllr::Opcode::STRUCT_MEMBER, { memberIndex }, { parentID, typeID, init.value });
		return id;
	}

	//TODO complain
	return 0;
}

cllr::TypedSSA MemberVariable::emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target)
{
	if (auto t = typeHint->resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Opcode::VALUE_MEMBER, { memberIndex }, { target });

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void MemberVariable::emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	auto memberLoad = emitLoadCLLR(table, codeAsm, target);

	codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { memberLoad.value, value });

}

void GlobalVariable::prettyPrint(ref<std::stringstream> ss) const
{

}

cllr::SSA GlobalVariable::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	return cllr::SSA();
}

cllr::TypedSSA GlobalVariable::emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target)
{
	return cllr::TypedSSA();
}

void GlobalVariable::emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{

}

void FnArgVariable::prettyPrint(ref<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << nameTkn->str;

}

cllr::SSA FnArgVariable::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
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
		auto tID = t->emitDeclCLLR(table, codeAsm);

		id = codeAsm.pushNew(cllr::Opcode::VAR_FUNC_ARG, { argIndex }, { tID });

		return id;
	}
	
	//TODO complain
	return 0;
}

cllr::TypedSSA FnArgVariable::emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target)
{
	return cllr::TypedSSA();
}

void FnArgVariable::emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	//TODO complain (maybe)
}

void ShaderIOVariable::prettyPrint(ref<std::stringstream> ss) const
{
	typeHint->prettyPrint(ss);

	ss << ' ' << name;

}

cllr::SSA ShaderIOVariable::emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm)
{
	if (id != 0)
	{
		return id;
	}

	if (ioType == ShaderIOVarType::UNKNOWN)
	{
		return 0;
	}

	codeAsm.addIOName(name);

	if (auto t = typeHint->resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);

		if (ioType == ShaderIOVarType::INPUT)
		{
			auto &[ioID, index] = codeAsm.pushInput(name, tID);

			id = ioID;
			ioIndex = index;

		}
		else
		{
			auto& [ioID, index] = codeAsm.pushOutput(name, tID);

			id = ioID;
			ioIndex = index;

		}

		return id;
	}

	//TODO complain
	return 0;
}

cllr::TypedSSA ShaderIOVariable::emitLoadCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target)
{
	if (ioType == ShaderIOVarType::OUTPUT)
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	ioType = ShaderIOVarType::INPUT;

	emitDeclCLLR(table, codeAsm);

	if (auto t = typeHint->resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);

		//TODO outType
		auto vID = codeAsm.pushNew(cllr::Opcode::VALUE_READ_VAR, {}, { id });

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void ShaderIOVariable::emitStoreCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm, cllr::SSA target, cllr::SSA value)
{
	if (ioType == ShaderIOVarType::INPUT)
	{
		//TODO complain
		return;
	}

	ioType = ShaderIOVarType::OUTPUT;

	codeAsm.push(0, cllr::Opcode::ASSIGN, {}, { emitDeclCLLR(table, codeAsm), value });

}
