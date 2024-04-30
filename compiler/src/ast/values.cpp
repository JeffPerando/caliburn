
#include "ast/values.h"

#include <algorithm>

#include "ast/fn.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

ValueResult IntLiteralValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto& [intLit, intType] = splitStr(lit->str, "_");

	auto pType = ParsedType(intType);

	if (auto t = pType.resolve(table, codeAsm))
	{
		//Integer literals parse immediately since there's no loss in precision;
		//Float literals defer parsing since they can lose precision during parsing
		//Why does it matter? I dunno

		//TODO write an integer-parsing algorithm
		uint64_t parsedLit = std::stol(intLit);

		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_INT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, {}, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return ValueResult();
}

ValueResult FloatLiteralValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
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
	return ValueResult();
}

ValueResult StringLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (auto t = ParsedType("string").resolve(table, codeAsm))
	{
		auto sID = codeAsm.addString(lit->str);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_STR, { sID }, {}, t->id));

		return cllr::TypedSSA(t, vID);
	}

	//TODO complain
	return ValueResult();
}

ValueResult BoolLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto t = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_BOOL, { lit->str == "true" }, {}, t->id));

	return cllr::TypedSSA(t, vID);
}

ValueResult ArrayLitValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO make array type
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_ARRAY, { (uint32_t)values.size() }));

	for (uint32_t i = 0; i < values.size(); ++i)
	{
		auto const& v = values[i];
		auto elem = v->emitValueCLLR(table, codeAsm);

		MATCH(elem, cllr::TypedSSA, elemVal)
		{
			codeAsm.pushNew(cllr::Instruction(cllr::Opcode::LIT_ARRAY_ELEM, { i }, { vID, elemVal->value }));
		}
		else
		{
			//TODO complain
		}

		//TODO typecheck against array inner type

	}

	//FIXME
	return cllr::TypedSSA(0, vID);
}

ValueResult ExpressionValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lhs = lValue->emitValueCLLR(table, codeAsm);
	auto rhs = rValue->emitValueCLLR(table, codeAsm);

	cllr::TypedSSA lhsVal, rhsVal;

	MATCH(lhs, cllr::TypedSSA, lhsPtr)
	{
		lhsVal = *lhsPtr;
	}
	else
	{
		//TODO complain
	}

	MATCH(rhs, cllr::TypedSSA, rhsPtr)
	{
		rhsVal = *rhsPtr;
	}
	else
	{
		//TODO complain
	}

	if (lhsVal.value == 0 || rhsVal.value == 0)
	{
		return ValueResult();
	}

	auto cllrOp = cllr::Opcode::VALUE_EXPR;

	if (OP_CATEGORIES.at(op) == OpCategory::LOGICAL)
	{
		cllrOp = cllr::Opcode::COMPARE;
	}

	cllr::TypeChecker tc(codeAsm.settings);

	cllr::TypedSSA result;
	if (!tc.check(lhsVal.type, rhsVal, result, codeAsm, op))
	{
		//TODO complain
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllrOp, { (uint32_t)op }, { lhsVal.value, result.value }, lhsVal.type->id));

	return cllr::TypedSSA(lhsVal.type, vID);
}

ValueResult SubArrayValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	MATCH(array->emitValueCLLR(table, codeAsm), cllr::TypedSSA, arrVal)
	{
		MATCH(index->emitValueCLLR(table, codeAsm), cllr::TypedSSA, indexVal)
		{
			//FIXME check array type

			auto outType = codeAsm.getType(codeAsm.getIns(arrVal->type->id).refs[0]);
			auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_SUBARRAY, {}, { arrVal->value, indexVal->value }, outType->id));

			return cllr::TypedSSA(outType, vID);
		}
		else
		{
			//TODO complain
		}

	}
	else
	{
		//TODO complain
	}

	return ValueResult();
}

ValueResult CastValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (auto t = castTarget->resolve(table, codeAsm))
	{
		MATCH(lhs->emitValueCLLR(table, codeAsm), cllr::TypedSSA, lhsVal)
		{
			auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_CAST, {}, { lhsVal->value }, t->id));

			return cllr::TypedSSA(t, vID);
		}

	}

	//TODO complain
	return ValueResult();
}

ValueResult VarReadValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto sym = table->find(varTkn->str);

	MATCH_EMPTY(sym)
	{
		codeAsm.errors->err({ "Symbol not found:", varTkn->str }, varTkn);
		return ValueResult();
	}

	MATCH(sym, sptr<Variable>, var)
	{
		return (*var)->emitLoadCLLR(table, codeAsm);
	}

	MATCH(sym, sptr<FunctionGroup>, fn)
	{
		return *fn;
	}

	MATCH(sym, sptr<Value>, val)
	{
		return (*val)->emitValueCLLR(table, codeAsm);
	}

	MATCH(sym, sptr<Module>, mod)
	{
		return *mod;
	}

	MATCH(sym, sptr<BaseType>, t)
	{
		return *t;
	}

	MATCH(sym, sptr<cllr::LowType>, lt)
	{
		return *lt;
	}

	//TODO complain
	return ValueResult();
}

ValueResult VarChainValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	ValueResult finalRes;
	cllr::TypedSSA targetValue;
	size_t start = 0;

	auto tgtRes = target->emitValueCLLR(table, codeAsm);

	MATCH_WHILE(tgtRes, sptr<Module>, mod)
	{
		if (start == chain.size())
		{
			return *mod;
		}

		sptr<Token> chainStart = chain[start];
		Symbol targetSym = (*mod)->getTable()->find(chainStart->str);

		MATCH(targetSym, sptr<Module>, mod)
		{
			++start;
			//Modules within modules! what a crazy world!
			tgtRes = *mod;
			continue;
		}

		MATCH(targetSym, sptr<Variable>, var)
		{
			++start;
			targetValue = (*var)->emitLoadCLLR(table, codeAsm);
			break;
		}

		size_t next = start + 1;
		
		if (chain.size() > next)
		{
			codeAsm.errors->err("Will be unable to access this member", chain[next]);
			return ValueResult();
		}

		MATCH(targetSym, sptr<FunctionGroup>, fn)
		{
			return *fn;
		}
		
		MATCH(targetSym, sptr<BaseType>, t)
		{
			return *t;
		}
		
		MATCH(targetSym, sptr<cllr::LowType>, lt)
		{
			return *lt;
		}

		codeAsm.errors->err({ "Could not find symbol", chainStart->str, "in module" }, *target);
		return ValueResult();
	}
	
	MATCH(tgtRes, cllr::TypedSSA, val)
	{
		targetValue = *val;
	}
	else
	{
		codeAsm.errors->err("Cannot retrieve member from target", *target);
		return ValueResult();
	}

	for (size_t i = start; i < chain.size(); ++i)
	{
		sptr<Token> nameTkn = chain[i];
		cllr::LowMember mem = targetValue.type->getMember(targetValue.value, nameTkn->str, codeAsm);

		MATCH(mem, cllr::TypedSSA, valPtr)
		{
			targetValue = *valPtr;
		}
		else MATCH(mem, cllr::IndexedMember, memPtr)
		{
			auto& [memIndex, memType] = *memPtr;
			auto val = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memIndex }, { targetValue.type->id }, memType->id));
			targetValue = cllr::TypedSSA(memType, val);

		}
		else
		{
			codeAsm.errors->err("Member not found:", nameTkn);
			return ValueResult();
		}

	}

	return targetValue;
}

ValueResult UnaryValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO sanity check output types
	auto in = val->emitValueCLLR(table, codeAsm);

	MATCH(in, cllr::TypedSSA, inVal)
	{
		auto outID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR_UNARY, { (uint32_t)op }, { inVal->value }, inVal->type->id));

		return cllr::TypedSSA(inVal->type, outID);
	}
	else
	{
		//TODO complain
	}

	return ValueResult();
}

ValueResult FnCallValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//Emit the values now to typecheck later
	std::vector<cllr::TypedSSA> argIDs(args.size());
	//FIXME I need some CINQ in this
	std::transform(args.begin(), args.end(), std::back_inserter(argIDs), LAMBDA(sptr<Value> v) {
		auto arg = v->emitValueCLLR(table, codeAsm);

		MATCH(arg, cllr::TypedSSA, argPtr)
		{
			return *argPtr;
		}

		codeAsm.errors->err("Invalid function argument", *v);
		return cllr::TypedSSA();
	});

	if (target != nullptr)
	{
		MATCH(target->emitValueCLLR(table, codeAsm), cllr::TypedSSA, tgtVal)
		{
			argIDs.emplace(argIDs.begin(), *tgtVal);

		}
		
		/*
		TODO test and see if target is always a valid value

		We don't care about invalid targets since they're probably the result of bad parsing anyway.
		In theory, at least...
		*/

	}

	auto fnName = name->emitValueCLLR(table, codeAsm);

	sptr<Function> fn = nullptr;

	MATCH_EMPTY(fnName)
	{
		codeAsm.errors->err("Function not found", *name);
		return ValueResult();
	}

	MATCH(fnName, sptr<FunctionGroup>, fnGroup)
	{
		fn = (*fnGroup)->find(argIDs, table, codeAsm);

		if (fn == nullptr)
		{
			//TODO complain
			return ValueResult();
		}

	}
	else
	{
		sptr<cllr::LowType> lType = nullptr;

		MATCH(fnName, sptr<BaseType>, baseType)
		{
			lType = (*baseType)->resolve(genArgs, table, codeAsm);

			if (lType == nullptr)
			{
				codeAsm.errors->err({ "Could not resolve type", (*baseType)->canonName }, *name);
				return ValueResult();
			}

		}
		else MATCH(fnName, sptr<cllr::LowType>, typePtr)
		{
			lType = *typePtr;
		}
		else
		{
			codeAsm.errors->err("Not a valid function name", *name);
			return ValueResult();
		}

		auto ctor = lType->constructors.find(argIDs, table, codeAsm);

		if (ctor == nullptr)
		{
			codeAsm.errors->err("Could not find constructor", *this);
			return ValueResult();
		}

		fn = ctor;

	}

	auto fnImpl = fn->getImpl(genArgs);

	if (fnImpl == nullptr)
	{
		//TODO complain
		return ValueResult();
	}

	return fnImpl->call(table, codeAsm, argIDs, name->firstTkn());
}

ValueResult SetterValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lvalue = lhs->emitValueCLLR(table, codeAsm);
	auto rvalue = rhs->emitValueCLLR(table, codeAsm);

	cllr::TypedSSA lhsVal, rhsVal;

	MATCH(lvalue, cllr::TypedSSA, lhsPtr)
	{
		lhsVal = *lhsPtr;
	}
	else
	{
		//TODO complain
	}

	MATCH(rvalue, cllr::TypedSSA, rhsPtr)
	{
		rhsVal = *rhsPtr;
	}
	else
	{
		//TODO complain
	}

	if (lhsVal.value == 0 || rhsVal.value == 0)
	{
		return ValueResult();
	}

	cllr::TypeChecker tc(codeAsm.settings);

	cllr::TypedSSA result;
	if (!tc.check(rhsVal.type, rhsVal, result, codeAsm, op))
	{
		//TODO complain
	}

	if (op != Operator::NONE)
	{
		auto rvID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_EXPR, { (uint32_t)op }, { lhsVal.value, result.value }, lhsVal.type->id));
		result = cllr::TypedSSA(lhsVal.type, rvID);

	}

	codeAsm.push(cllr::Instruction(cllr::Opcode::ASSIGN, {}, { lhsVal.value, result.value }));

	return ValueResult();
}

ValueResult NullValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO figure out typing
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_NULL)));
}

ValueResult ZeroValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_ZERO)));
}

ValueResult SignValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return ValueResult();
}

ValueResult UnsignValue::emitValueCLLR(sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return ValueResult();
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

	name->prettyPrint(ss);
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
