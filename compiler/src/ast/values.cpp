
#include "ast/values.h"
#include "ast/fn.h"

using namespace caliburn;

void IntLiteralValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << lit->str;
}

cllr::TypedSSA IntLiteralValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

	if (auto t = pType.resolve(table))
	{
		//Integer literals parse immediately since there's no loss in precision;
		//Float literals defer parsing since they can lose precision during parsing
		//Why does it matter? I dunno
		uint64_t parsedLit = t->base->parseLiteral(lit->str);

		auto tID = t->emitDeclCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_INT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, {}, tID));

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void FloatLiteralValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << lit->str;
}

cllr::TypedSSA FloatLiteralValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	//We defer parsing further! Great success!
	auto sID = codeAsm.addString(lit->str.substr(0, lit->str.find_first_of('_')));
	auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

	if (auto t = pType.resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_FP, { sID }, {}, tID));

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void StringLitValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << lit->str;
}

cllr::TypedSSA StringLitValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto pType = ParsedType("string");

	if (auto t = pType.resolve(table))
	{
		auto sID = codeAsm.addString(lit->str);
		auto tID = t->emitDeclCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_STR, { sID }, {}, tID));

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void BoolLitValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << lit->str;
}

cllr::TypedSSA BoolLitValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto pType = ParsedType("bool");

	if (auto t = pType.resolve(table))
	{
		auto tID = t->emitDeclCLLR(table, codeAsm);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_BOOL, { lit->str == "true" }, {}, tID));

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void ArrayLitValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << '[';

	for (size_t i = 0; i < values.size(); ++i)
	{
		values[i]->prettyPrint(ss);

		if (i + 1 < values.size())
		{
			ss << ", ";
		}

	}

	ss << ']';

}

cllr::TypedSSA ArrayLitValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	//TODO make array type
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_ARRAY, { (uint32_t)values.size() }));

	for (uint32_t i = 0; i < values.size(); ++i)
	{
		auto const& v = values[i];
		auto& [elemTypeID, elemType, elemID] = v->emitValueCLLR(table, codeAsm);

		codeAsm.pushNew(cllr::Instruction(cllr::Opcode::LIT_ARRAY_ELEM, { i }, { vID, elemID }));

	}

	//FIXME
	return cllr::TypedSSA(nullptr, 0, vID);
}

void ExpressionValue::prettyPrint(ref<std::stringstream> ss) const
{
	const auto cat = OP_CATEGORIES.find(op)->second;

	if (cat == OpCategory::UNARY)
	{
		//TODO complain
		return;
	}

	const auto& opStr = findStrForOp(op);

	lValue->prettyPrint(ss);

	ss << ' ' << opStr << ' ';

	rValue->prettyPrint(ss);

}

cllr::TypedSSA ExpressionValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto lhs = lValue->emitValueCLLR(table, codeAsm);
	auto rhs = rValue->emitValueCLLR(table, codeAsm);

	auto opType = OP_CATEGORIES.at(op);

	auto cllrOp = cllr::Opcode::VALUE_EXPR;

	if (opType == OpCategory::LOGICAL)
	{
		cllrOp = cllr::Opcode::COMPARE;
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllrOp, { (uint32_t)op }, { lhs.value, rhs.value }, lhs.type));

	//FIXME
	return cllr::TypedSSA(lhs.typePtr, lhs.type, vID);
}

void IsAValue::prettyPrint(ref<std::stringstream> ss) const
{
	val->prettyPrint(ss);

	ss << " is ";

	chkType->prettyPrint(ss);

}

cllr::TypedSSA IsAValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	/*
	chkType->emitDeclCLLR(table, codeAsm);
	auto vID = val->emitValueCLLR(table, codeAsm);
	auto tID = chkType->id;

	TODO:
	Emit the bool literal of whether or not type A is an instance of type B.

	*/
	return cllr::TypedSSA();
	//return codeAsm.pushNew(cllr::Opcode::VALUE_INSTANCEOF, {}, { vID, tID, 0 });;
}

void SubArrayValue::prettyPrint(ref<std::stringstream> ss) const
{
	array->prettyPrint(ss);

	ss << '[';

	index->prettyPrint(ss);

	ss << ']';

}

cllr::TypedSSA SubArrayValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto& [aType, aTypeID, aID] = array->emitValueCLLR(table, codeAsm);
	auto& [iType, iTypeID, iID] = index->emitValueCLLR(table, codeAsm);

	//FIXME check array type
	auto outType = codeAsm.codeFor(aID)->refs[0];
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_SUBARRAY, {}, { aID, iID }, outType));

	//FIXME
	return cllr::TypedSSA(nullptr, outType, vID);
}

void CastValue::prettyPrint(ref<std::stringstream> ss) const
{
	lhs->prettyPrint(ss);

	ss << " as ";

	castTarget->prettyPrint(ss);

}

cllr::TypedSSA CastValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	if (auto t = castTarget->resolve(table))
	{
		auto& [inType, inTypeID, inID] = lhs->emitValueCLLR(table, codeAsm);
		auto tID = t->emitDeclCLLR(table, codeAsm);

		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_CAST, {}, { inID }, tID));

		return cllr::TypedSSA(t, tID, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void VarReadValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << varTkn->str;
}

cllr::TypedSSA VarReadValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto v = table->find(varTkn->str);

	if (auto varRes = std::get_if<sptr<Variable>>(&v))
	{
		return (*varRes)->emitLoadCLLR(table, codeAsm, 0);
	}

	if (auto valRes = std::get_if<sptr<Value>>(&v))
	{
		return (*valRes)->emitValueCLLR(table, codeAsm);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void MemberReadValue::prettyPrint(ref<std::stringstream> ss) const
{
	target->prettyPrint(ss);

	ss << '.';

	ss << memberName->str;

}

cllr::TypedSSA MemberReadValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto& [targetType, targetTypeID, targetID] = target->emitValueCLLR(table, codeAsm);

	if (auto v = std::get_if<sptr<Variable>>(&targetType->base->getMember(memberName->str)))
	{
		return (*v)->emitLoadCLLR(table/*FIXME probably not the table to use*/, codeAsm, targetID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

void VarChainValue::prettyPrint(ref<std::stringstream> ss) const
{
	if (target != nullptr)
	{
		target->prettyPrint(ss);
		ss << '.';
	}

	for (size_t i = 0; i < (chain.size() - 1); ++i)
	{
		ss << chain[i]->str << '.';
	}

	ss << chain.back()->str;

}

cllr::TypedSSA VarChainValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	size_t start = 0;
	auto finalVal = cllr::TypedSSA();

	if (target != nullptr)
	{
		finalVal = target->emitValueCLLR(table, codeAsm);
	}
	else
	{
		sptr<SymbolTable> tbl = table;

		for (size_t i = 0; i < chain.size(); ++i)
		{
			Symbol sym = tbl->find(chain[i]->str);

			if (std::holds_alternative<std::monostate>(sym))
			{
				//TODO complain
				return finalVal;
			}
			if (auto m = std::get_if<sptr<Module>>(&sym))
			{
				//TODO get symbol table from module
				++start;
				return finalVal;
			}

			if (auto v = std::get_if<sptr<Variable>>(&sym))
			{
				finalVal = (*v)->emitLoadCLLR(table, codeAsm, 0);
				++start;
				break;
			}

		}

	}

	for (size_t i = start; i < chain.size(); ++i)
	{
		auto const& tkn = chain[i];

		auto mem = finalVal.typePtr->base->getMember(tkn->str);

		if (auto v = std::get_if<sptr<Variable>>(&mem))
		{
			finalVal = (*v)->emitLoadCLLR(table, codeAsm, finalVal.value);
		}
		else if (auto val = std::get_if<sptr<Value>>(&mem))
		{
			finalVal = (*val)->emitValueCLLR(table, codeAsm);
		}
		else
		{
			//TODO complain
			break;
		}

	}

	if (finalVal.value == 0)
	{
		//TODO complain
	}

	return finalVal;
}

void UnaryValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << start->str;

	val->prettyPrint(ss);

	if (end != nullptr)
	{
		ss << end->str;
	}

}

cllr::TypedSSA UnaryValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	//TODO sanity check output types
	auto in = val->emitValueCLLR(table, codeAsm);
	auto outID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR_UNARY, { (uint32_t)op }, { in.value }, in.type));

	return cllr::TypedSSA(in.typePtr, in.type, outID);
}

void FnCallValue::prettyPrint(ref<std::stringstream> ss) const
{
	if (target != nullptr)
	{
		target->prettyPrint(ss);

		ss << '.';

	}

	ss << name->str;

	genArgs->prettyPrint(ss);

	ss << '(';

	for (size_t i = 0; i < args.size(); ++i)
	{
		const auto& arg = args[i];

		arg->prettyPrint(ss);

		if (i + 1 < args.size())
		{
			ss << ", ";

		}

	}

	ss << ')';

}

cllr::TypedSSA FnCallValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto sym = table->find(name->str);

	if (auto fn = std::get_if<sptr<Function>>(&sym))
	{
		auto fnImpl = (**fn).makeVariant(genArgs);

		return fnImpl->call(table, codeAsm, args);
	}
	/*
	else if (auto t = std::get_if<sptr<BaseType>>(&sym))
	{
		auto tReal = (**t).getImpl(genArgs);
		auto tID = tReal->emitDeclCLLR(table, codeAsm);
	}
	*/

	//TODO complain
	return cllr::TypedSSA();
}

void SetterValue::prettyPrint(ref<std::stringstream> ss) const
{
	lhs->prettyPrint(ss);

	ss << ' ';

	if (op != Operator::UNKNOWN)
	{
		ss << findStrForOp(op);

	}

	ss << "= ";

	rhs->prettyPrint(ss);

}

cllr::TypedSSA SetterValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	auto lvalue = lhs->emitValueCLLR(table, codeAsm);
	auto rvalue = rhs->emitValueCLLR(table, codeAsm);

	if (op != Operator::UNKNOWN)
	{
		auto rvID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR, { (uint32_t)op }, { lvalue.value, rvalue.value }, lvalue.type));
		rvalue = cllr::TypedSSA(lvalue.typePtr, lvalue.type, rvID);

	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { lvalue.value, rvalue.value }));

	return cllr::TypedSSA();
}

void NullValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << "null";
}

cllr::TypedSSA NullValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	//TODO figure out typing
	return cllr::TypedSSA(nullptr, 0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_NULL)));
}

void ZeroValue::prettyPrint(ref<std::stringstream> ss) const {}

cllr::TypedSSA ZeroValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA(nullptr, 0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_ZERO)));
}

void SignValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << first->str << ' ';
	target->prettyPrint(ss);

}

cllr::TypedSSA SignValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA();
}

void UnsignValue::prettyPrint(ref<std::stringstream> ss) const
{
	ss << first->str << ' ';
	target->prettyPrint(ss);

}

cllr::TypedSSA UnsignValue::emitValueCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA();
}
