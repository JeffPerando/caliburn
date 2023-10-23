
#include "ast/values.h"

#include <algorithm>

#include "ast/fn.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

cllr::TypedSSA IntLiteralValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto& [intLit, intType] = splitStr(lit->str, "_");

	auto pType = ParsedType(intType);

	if (auto t = pType.resolve(table, codeAsm))
	{
		//Integer literals parse immediately since there's no loss in precision;
		//Float literals defer parsing since they can lose precision during parsing
		//Why does it matter? I dunno

		//TODO write an integer-parsing algorithm
		uint64_t parsedLit = std::stoi(intLit);

		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_INT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, {}, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA FloatLiteralValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//We defer parsing further! Great success!
	auto sID = codeAsm.addString(lit->str.substr(0, lit->str.find_first_of('_')));
	auto pType = ParsedType(lit->str.substr(lit->str.find_first_of('_') + 1));

	if (auto t = pType.resolve(table, codeAsm))
	{
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_FP, { sID }, {}, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA StringLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto pType = ParsedType("string");

	if (auto t = pType.resolve(table, codeAsm))
	{
		auto sID = codeAsm.addString(lit->str);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_STR, { sID }, {}, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA BoolLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto t = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_BOOL, { lit->str == "true" }, {}, t->id));

	return cllr::TypedSSA(t, vID);
}

cllr::TypedSSA ArrayLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO make array type
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_ARRAY, { (uint32_t)values.size() }));

	for (uint32_t i = 0; i < values.size(); ++i)
	{
		auto const& v = values[i];
		auto elem = v->emitValueCLLR(table, codeAsm);

		//TODO typecheck against array inner type

		codeAsm.pushNew(cllr::Instruction(cllr::Opcode::LIT_ARRAY_ELEM, { i }, { vID, elem.value }));

	}

	//FIXME
	return cllr::TypedSSA(0, vID);
}

cllr::TypedSSA ExpressionValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lhs = lValue->emitValueCLLR(table, codeAsm);
	auto rhs = rValue->emitValueCLLR(table, codeAsm);

	auto opType = OP_CATEGORIES.at(op);

	auto cllrOp = cllr::Opcode::VALUE_EXPR;

	if (opType == OpCategory::LOGICAL)
	{
		cllrOp = cllr::Opcode::COMPARE;
	}

	cllr::TypeChecker tc(codeAsm.settings);

	cllr::TypedSSA result;
	if (!tc.check(lhs.type, rhs, result, codeAsm, op))
	{
		//TODO complain
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllrOp, { (uint32_t)op }, { lhs.value, result.value }, lhs.type->id));

	//FIXME
	return cllr::TypedSSA(lhs.type, vID);
}

cllr::TypedSSA IsAValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
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

cllr::TypedSSA SubArrayValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto& [aTypeImpl, aID] = array->emitValueCLLR(table, codeAsm);
	auto& [iTypeImpl, iID] = index->emitValueCLLR(table, codeAsm);

	//FIXME check array type
	auto outType = codeAsm.getType(codeAsm.codeFor(aID).refs[0]);
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_SUBARRAY, {}, { aID, iID }, outType->id));

	return cllr::TypedSSA(outType, vID);
}

cllr::TypedSSA CastValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (auto t = castTarget->resolve(table, codeAsm))
	{
		auto& [inTypeID, inID] = lhs->emitValueCLLR(table, codeAsm);

		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_CAST, {}, { inID }, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA VarReadValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto v = table->find(varTkn->str);

	if (auto varRes = std::get_if<sptr<Variable>>(&v))
	{
		return (*varRes)->emitLoadCLLR(table, codeAsm);
	}

	if (auto valRes = std::get_if<sptr<Value>>(&v))
	{
		return (*valRes)->emitValueCLLR(table, codeAsm);
	}

	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA MemberReadValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	cllr::TypedSSA targetValue = target->emitValueCLLR(table, codeAsm);
	auto mem = targetValue.type->getMember(targetValue.value, memberName->str, codeAsm);

	if (std::holds_alternative<std::monostate>(mem))
	{
		//TODO complain
		return cllr::TypedSSA();
	}

	if (auto v = std::get_if<cllr::TypedSSA>(&mem))
	{
		return *v;
	}

	if (auto m = std::get_if<std::pair<uint32_t, sptr<cllr::LowType>>>(&mem))
	{
		auto& [memIndex, memType] = *m;
		auto v = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memIndex }, { targetValue.type->id }, memType->id));

		return cllr::TypedSSA(memType, v);
	}

	return cllr::TypedSSA();
}

cllr::TypedSSA VarChainValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	size_t start = 0;
	cllr::TypedSSA finalVal;

	if (target != nullptr)
	{
		finalVal = target->emitValueCLLR(table, codeAsm);
	}
	else
	{
		sptr<const SymbolTable> tbl = table;

		for (size_t i = 0; i < chain.size(); ++i)
		{
			Symbol sym = tbl->find(chain[i]->str);

			if (std::holds_alternative<std::monostate>(sym))
			{
				//TODO complain
				return finalVal;
			}
			else if (auto m = std::get_if<sptr<Module>>(&sym))
			{
				//TODO get symbol table from module
				++start;
				return finalVal;
			}
			else if (auto v = std::get_if<sptr<Variable>>(&sym))
			{
				finalVal = (*v)->emitLoadCLLR(table, codeAsm);
				++start;
				break;
			}

		}

	}

	for (size_t i = start; i < chain.size(); ++i)
	{
		auto const& tkn = chain[i];
		auto mem = finalVal.type->getMember(finalVal.value, tkn->str, codeAsm);

		if (std::holds_alternative<std::monostate>(mem))
		{
			//TODO complain
			break;
		}

		if (auto v = std::get_if<cllr::TypedSSA>(&mem))
		{
			finalVal = *v;
		}

		if (auto m = std::get_if<std::pair<uint32_t, sptr<cllr::LowType>>>(&mem))
		{
			auto& [memIndex, memType] = *m;
			auto v = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memIndex }, { finalVal.type->id }, memType->id));

			finalVal = cllr::TypedSSA(memType, v);
		}

	}

	if (finalVal.value == 0)
	{
		//TODO complain
	}

	return finalVal;
}

cllr::TypedSSA UnaryValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO sanity check output types
	auto in = val->emitValueCLLR(table, codeAsm);
	auto outID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR_UNARY, { (uint32_t)op }, { in.value }, in.type->id));

	return cllr::TypedSSA(in.type, outID);
}

cllr::TypedSSA FnCallValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//Emit the values now to typecheck later
	std::vector<cllr::TypedSSA> argIDs(args.size());
	std::transform(args.begin(), args.end(), std::back_inserter(argIDs), lambda(sptr<Value> v) { return v->emitValueCLLR(table, codeAsm); });

	if (target != nullptr)
	{
		auto val = target->emitValueCLLR(table, codeAsm);
		auto memFn = val.type->getMemberFn(name->str, argIDs);
		auto memFnImpl = memFn->getImpl(genArgs);

		return memFnImpl->call(table, codeAsm, argIDs, name);
	}

	auto sym = table->find(name->str);

	if (std::holds_alternative<std::monostate>(sym))
	{
		auto e = codeAsm.errors->err({ "Function name not found:", name->str }, *this);
		return cllr::TypedSSA();
	}

	if (auto fn = std::get_if<sptr<Function>>(&sym))
	{
		auto fnImpl = (**fn).getImpl(genArgs);

		return fnImpl->call(table, codeAsm, argIDs, name);
	}
	else if (auto bt = std::get_if<sptr<BaseType>>(&sym))
	{
		auto t = (**bt).resolve(genArgs, table, codeAsm);
		auto tCon = t->constructors.find(argIDs, codeAsm);
		auto conImpl = tCon->getImpl(nullptr);

		return conImpl->call(table, codeAsm, argIDs, name);
	}
	
	//TODO complain
	return cllr::TypedSSA();
}

cllr::TypedSSA SetterValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lvalue = lhs->emitValueCLLR(table, codeAsm);
	auto rvalue = rhs->emitValueCLLR(table, codeAsm);

	cllr::TypeChecker tc(codeAsm.settings);

	cllr::TypedSSA result;
	if (!tc.check(lvalue.type, rvalue, result, codeAsm, op))
	{
		//TODO complain
	}

	if (op != Operator::NONE)
	{
		auto rvID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR, { (uint32_t)op }, { lvalue.value, result.value }, lvalue.type->id));
		result = cllr::TypedSSA(lvalue.type, rvID);

	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { lvalue.value, result.value }));

	return cllr::TypedSSA();
}

cllr::TypedSSA NullValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO figure out typing
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_NULL)));
}

cllr::TypedSSA ZeroValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_ZERO)));
}

cllr::TypedSSA SignValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA();
}

cllr::TypedSSA UnsignValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA();
}

void IntLiteralValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit->str.substr(0, lit->str.find('_'));
}

void FloatLiteralValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit->str;
}

void StringLitValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit->str;
}

void BoolLitValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit->str;
}

void ArrayLitValue::prettyPrint(out<std::stringstream> ss) const
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

void ExpressionValue::prettyPrint(out<std::stringstream> ss) const
{
	const auto cat = OP_CATEGORIES.find(op)->second;

	if (cat == OpCategory::UNARY)
	{
		//TODO complain
		return;
	}

	const auto& opStr = INFIX_OPS_STR.at(op);

	auto constexpr postfix = false;

	if (postfix)
	{
		lValue->prettyPrint(ss);

		ss << ' ';

		rValue->prettyPrint(ss);

		ss << ' ' << opStr;

	}
	else
	{
		//un-comment the parentheses if you're struggling
		ss << '(';
		lValue->prettyPrint(ss);

		ss << ' ' << opStr << ' ';

		rValue->prettyPrint(ss);
		ss << ')';
	}
	
}

void IsAValue::prettyPrint(out<std::stringstream> ss) const
{
	val->prettyPrint(ss);

	ss << " is ";

	chkType->prettyPrint(ss);

}

void CastValue::prettyPrint(out<std::stringstream> ss) const
{
	lhs->prettyPrint(ss);

	ss << " as ";

	castTarget->prettyPrint(ss);

}

void SubArrayValue::prettyPrint(out<std::stringstream> ss) const
{
	array->prettyPrint(ss);

	ss << '[';

	index->prettyPrint(ss);

	ss << ']';

}

void VarReadValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << varTkn->str;
}

void MemberReadValue::prettyPrint(out<std::stringstream> ss) const
{
	target->prettyPrint(ss);

	ss << '.';

	ss << memberName->str;

}

void VarChainValue::prettyPrint(out<std::stringstream> ss) const
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

void UnaryValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << start->str;

	val->prettyPrint(ss);

	if (end != nullptr)
	{
		ss << end->str;
	}
	
}

void FnCallValue::prettyPrint(out<std::stringstream> ss) const
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

void SetterValue::prettyPrint(out<std::stringstream> ss) const
{
	lhs->prettyPrint(ss);

	ss << ' ';

	if (op != Operator::NONE)
	{
		ss << INFIX_OPS_STR.at(op);

	}

	ss << "= ";

	rhs->prettyPrint(ss);

}

void NullValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << "null";
}

void ZeroValue::prettyPrint(out<std::stringstream> ss) const {}

void SignValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << first->str << ' ';
	target->prettyPrint(ss);

}

void UnsignValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << first->str << ' ';
	target->prettyPrint(ss);

}
