
#include "ast/values.h"

#include <algorithm>

#include "ast/basetypes.h"
#include "ast/fn.h"

#include "cllr/cllrtype.h"

using namespace caliburn;

ValueResult IntLiteralValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto const& intLit = lit.str;

	std::stringstream type;
	size_t len = intLit.length();

	auto name = "int";
	auto bits = "32";

	if (std::toupper(intLit[len - 1]) == 'L')
	{
		bits = "64";
		--len;
	}

	if (std::toupper(intLit[len - 1]) == 'U')
	{
		name = "uint";
		--len;
	}

	type << name;
	type << bits;

	auto pType = ParsedType(type.str());
	auto t = pType.resolve(table, codeAsm);

	if (!t)
	{
		//TODO complain
		return ValueResult();
	}

	uint64_t parsedLit = 0;
	size_t idx = 0;

	if (intLit[0] == '0')
	{
		if (std::toupper(intLit[1]) == 'X')
		{
			for (idx = 2; idx < len; ++idx)
			{
				char digit = intLit[idx];
				if (digit == '_')
				{
					continue;
				}

				parsedLit <<= 4;
				parsedLit |= std::toupper(digit) - 'A';

			}

		}
		else if (std::toupper(intLit[1]) == 'B')
		{
			for (idx = 2; idx < len; ++idx)
			{
				char digit = intLit[idx];
				if (digit == '_')
				{
					continue;
				}

				parsedLit <<= 1;
				parsedLit |= (digit - '0');

			}

		}
		else if (std::toupper(intLit[1]) == 'C')
		{
			for (idx = 2; idx < len; ++idx)
			{
				char digit = intLit[idx];
				if (digit == '_')
				{
					continue;
				}

				parsedLit <<= 3;
				parsedLit |= (digit - '0');

			}

		}

	}

	if (idx == 0)
	{
		parsedLit = parseInt(intLit, len);
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_INT, { (uint32_t)(parsedLit & 0xFFFFFFFF), (uint32_t)((parsedLit >> 32) & 0xFFFFFFFF) }, {}, t->id));

	return cllr::TypedSSA(t, vID);
}

ValueResult FloatLiteralValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto fpLit = lit.str;
	size_t len = fpLit.length();

	auto bits = 32;

	switch (std::toupper(fpLit[len - 1]))
	{
		//TODO maybe add a half-float type?
	case 'D': bits = 64; PASS;
	case 'F': --len; break;
	}

	fpLit = fpLit.substr(0, len);

	std::stringstream type;
	type << "fp" << bits;

	auto pType = ParsedType(type.str());
	auto t = pType.resolve(table, codeAsm);

	if (!t)
	{
		//TODO complain
		return ValueResult();
	}

	cllr::SSA vID = 0;

	if (bits == 32)
	{
		auto data = std::stof(std::string(fpLit));
		uint32_t bitData = *reinterpret_cast<uint32_t*>(&data);

		vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_FP, { bitData }, {}, t->id));

	}
	else if (bits == 64)
	{
		auto data = std::stod(std::string(fpLit));
		uint64_t bitData = *reinterpret_cast<uint64_t*>(&data);

		vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_FP, { (uint32_t)(bitData & 0xFFFFFFFF), (uint32_t)((bitData >> 32) & 0xFFFFFFFF) }, {}, t->id));

	}
	else
	{
		return ValueResult();
	}

	return cllr::TypedSSA(t, vID);
}

ValueResult StringLitValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (auto t = ParsedType("string").resolve(table, codeAsm))
	{
		/* FIXME
		auto sID = codeAsm.addString(lit->str);
		auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_STR, { sID }, {}, t->id));

		return cllr::TypedSSA(t, vID);
		*/
	}

	//TODO complain
	return ValueResult();
}

ValueResult BoolLitValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto t = codeAsm.pushType(cllr::Instruction(cllr::Opcode::TYPE_BOOL));
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_BOOL, { lit.str == "true" }, {}, t->id));

	return cllr::TypedSSA(t, vID);
}

ValueResult ArrayLitValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO make array type
	auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_LIT_ARRAY, { (uint32_t)values.size() }));

	for (uint32_t i = 0; i < values.size(); ++i)
	{
		auto const& v = values[i];
		auto elem = v->emitCodeCLLR(table, codeAsm);

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

ValueResult ExpressionValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto lhs = lValue->emitCodeCLLR(table, codeAsm);
	auto rhs = rValue->emitCodeCLLR(table, codeAsm);

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
		return ValueResult();
	}

	auto vID = codeAsm.pushNew(cllr::Instruction(cllrOp, { (uint32_t)op }, { lhsVal.value, result.value }, lhsVal.type->id));

	return cllr::TypedSSA(lhsVal.type, vID);
}

ValueResult SubArrayValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	MATCH(array->emitCodeCLLR(table, codeAsm), cllr::TypedSSA, arrVal)
	{
		MATCH(index->emitCodeCLLR(table, codeAsm), cllr::TypedSSA, indexVal)
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

ValueResult CastValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	if (auto t = castTarget->resolve(table, codeAsm))
	{
		MATCH(lhs->emitCodeCLLR(table, codeAsm), cllr::TypedSSA, lhsVal)
		{
			auto vID = codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_CAST, {}, { lhsVal->value }, t->id));

			return cllr::TypedSSA(t, vID);
		}

	}

	//TODO complain
	return ValueResult();
}

ValueResult VarReadValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto const sym = table->find(varStr);

	MATCH(sym, sptr<Variable>, var)
	{
		return (*var)->emitLoadCLLR(table, codeAsm);
	}

	MATCH(sym, sptr<FunctionGroup>, fn)
	{
		return *fn;
	}

	MATCH(sym, sptr<Expr>, val)
	{
		return (*val)->emitCodeCLLR(table, codeAsm);
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

	codeAsm.errors->err({ "Symbol not found:", varStr }, *this);
	return ValueResult();
}

ValueResult MemberReadDirectValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto tgtRes = target->emitCodeCLLR(table, codeAsm);

	MATCH(tgtRes, cllr::TypedSSA, tgtVal)
	{
		auto memRes = tgtVal->type->getMember(tgtVal->value, mem, codeAsm);

		MATCH(memRes, cllr::IndexedMember, iMem)
		{
			auto& [memIndex, memType] = *iMem;
			return codeAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memIndex }, { tgtVal->value }), memType);
		}

		MATCH(memRes, cllr::TypedSSA, memVal)
		{
			return *memVal;
		}

	}

	codeAsm.errors->err({ "Member not found:", mem }, *target);
	return ValueResult();
}

ValueResult MemberReadChainValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	ValueResult finalRes;
	cllr::TypedSSA targetValue;
	size_t start = 0;

	auto tgtRes = target->emitCodeCLLR(table, codeAsm);

	MATCH_WHILE(tgtRes, sptr<Module>, mod)
	{
		if (start == mems.size())
		{
			return *mod;
		}

		const Token chainStart = mems[start];
		const Symbol targetSym = (*mod)->getTable()->find(chainStart.str);

		MATCH(targetSym, sptr<Module>, mod)
		{
			++start;
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

		if (mems.size() > next)
		{
			codeAsm.errors->err("Will be unable to access this member", mems[next]);
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

		codeAsm.errors->err({ "Could not find symbol", chainStart.str, "in module" }, *target);
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

	for (size_t i = start; i < mems.size(); ++i)
	{
		auto& memTkn = mems[i];

		auto mem = targetValue.type->getMember(targetValue.value, memTkn.str, codeAsm);

		MATCH(mem, cllr::IndexedMember, memPtr)
		{
			auto& [memIndex, memType] = *memPtr;
			targetValue = codeAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_MEMBER, { memIndex }, { targetValue.value }), memType);

		}
		else MATCH(mem, cllr::TypedSSA, valPtr)
		{
			targetValue = *valPtr;
		}
		else
		{
			codeAsm.errors->err({ "Member not found:", memTkn.str }, *this);
			return ValueResult();
		}

	}

	return targetValue;
}

ValueResult UnaryValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO sanity check output types
	auto in = val->emitCodeCLLR(table, codeAsm);

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

ValueResult FnCallValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//Emit the values now to typecheck later
	std::vector<cllr::TypedSSA> argIDs(args.size());
	//FIXME I need some CINQ in this
	std::transform(args.begin(), args.end(), std::back_inserter(argIDs), LAMBDA(sptr<Expr> v) {
		auto arg = v->emitCodeCLLR(table, codeAsm);

		MATCH(arg, cllr::TypedSSA, argPtr)
		{
			return *argPtr;
		}

		codeAsm.errors->err("Invalid function argument", *v);
		return cllr::TypedSSA();
	});

	ValueResult fnResult = name->emitCodeCLLR(table, codeAsm);
	sptr<cllr::LowType> lType = nullptr;

	MATCH(fnResult, cllr::TypedSSA, val)
	{
		//TODO check for a function pointer
		if (args.size() != 1)
		{
			codeAsm.errors->err("Invalid function", *name);
			return cllr::TypedSSA();
		}

		cllr::TypeChecker tc(codeAsm.settings);
		cllr::TypedSSA rhs;

		if (!tc.check(argIDs[0].type, *val, rhs, codeAsm, Operator::MUL))
		{
			//TODO complain
			return ValueResult();
		}

		return codeAsm.pushValue(cllr::Instruction(cllr::Opcode::VALUE_EXPR, { SCAST<uint32_t>(Operator::MUL) }, { argIDs[0].value, rhs.value }), argIDs[0].type);
	}

	MATCH(fnResult, sptr<FunctionGroup>, fnGroup)
	{
		return (*fnGroup)->call(argIDs, genArgs, codeAsm);
	}
	else MATCH(fnResult, sptr<BaseType>, baseType)
	{
		lType = (*baseType)->resolve(genArgs, table, codeAsm);
	}
	else MATCH(fnResult, sptr<cllr::LowType>, typePtr)
	{
		lType = *typePtr;
	}

	if (lType != nullptr)
	{
		auto ctorVal = lType->ctors.call(argIDs, genArgs, codeAsm);

		if (ctorVal.value == 0)
		{
			codeAsm.errors->err("Could not find constructor appropriate for these arguments", firstTkn(), lastTkn());
			return ValueResult();
		}

		return ctorVal;
	}

	codeAsm.errors->err("Function not found", *name);
	return ValueResult();
}

ValueResult MethodCallValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	auto targetVal = target->emitCodeCLLR(table, codeAsm);

	MATCH_EMPTY(targetVal)
	{
		auto e = codeAsm.errors->err("Not a value", *target);
		return ValueResult();
	}

	//Emit the values now to typecheck later
	std::vector<cllr::TypedSSA> argIDs(args.size());
	//FIXME I need some CINQ in this
	std::transform(args.begin(), args.end(), std::back_inserter(argIDs), LAMBDA(sptr<Expr> v) {
		auto arg = v->emitCodeCLLR(table, codeAsm);

		MATCH(arg, cllr::TypedSSA, argPtr)
		{
			return *argPtr;
		}

		codeAsm.errors->err("Invalid function argument", *v);
		return cllr::TypedSSA();
	});

	MATCH(targetVal, cllr::TypedSSA, val)
	{
		argIDs.push_back(*val);
		auto m = val->type->getMemberFns(name.str);

		if (m == nullptr)
		{
			auto e = codeAsm.errors->err({ "Unable to find method", name.str }, *this);
			return ValueResult();
		}

		return m->call(argIDs, genArgs, codeAsm);
	}

	MATCH(targetVal, sptr<Module>, mod)
	{
		auto sym = (*mod)->getTable()->find(name.str);

		MATCH_EMPTY(sym)
		{
			auto e = codeAsm.errors->err({ "Unable to find function", name.str, "in module" }, *target);
			return ValueResult();
		}

		MATCH(sym, sptr<FunctionGroup>, fg)
		{
			return (**fg).call(argIDs, genArgs, codeAsm);
		}

		//TODO construct type if sptr<BaseType> or sptr<LowType> found

		auto e = codeAsm.errors->err("Not a function:", name);
		return ValueResult();
	}

	auto e = codeAsm.errors->err({ "Unable to find function or method: ", name.str }, *this);
	return ValueResult();
}

ValueResult NullValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	//TODO figure out expected type
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_NULL)));
}

ValueResult ZeroValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return cllr::TypedSSA(0, codeAsm.pushNew(cllr::Instruction(cllr::Opcode::VALUE_ZERO)));
}

ValueResult SignValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return ValueResult();
}

ValueResult UnsignValue::emitCodeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const
{
	return ValueResult();
}

void IntLiteralValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit.str.substr(0, lit.str.find('_'));
}

void FloatLiteralValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit.str;
}

void StringLitValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit.str;
}

void BoolLitValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << lit.str;
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
	ss << varStr;
}

void MemberReadDirectValue::prettyPrint(out<std::stringstream> ss) const
{
	target->prettyPrint(ss);
	ss << '.' << mem;

}

void MemberReadChainValue::prettyPrint(out<std::stringstream> ss) const
{
	target->prettyPrint(ss);
	
	for (auto const& mem : mems)
	{
		ss << '.' << mem.str;
	}

}

void UnaryValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << start.str;

	val->prettyPrint(ss);

	if (end.exists())
	{
		ss << end.str;
	}
	
}

void FnCallValue::prettyPrint(out<std::stringstream> ss) const
{
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

void MethodCallValue::prettyPrint(out<std::stringstream> ss) const
{
	target->prettyPrint(ss);
	ss << '.' << name.str;
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

void NullValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << "null";
}

void ZeroValue::prettyPrint(out<std::stringstream> ss) const {}

void SignValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << first.str << ' ';
	target->prettyPrint(ss);

}

void UnsignValue::prettyPrint(out<std::stringstream> ss) const
{
	ss << first.str << ' ';
	target->prettyPrint(ss);

}
