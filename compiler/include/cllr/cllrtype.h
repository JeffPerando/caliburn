
#pragma once

#include "error.h"
#include "langcore.h"

#include "ast/fn.h"

#include "cllr.h"

namespace caliburn
{
	namespace cllr
	{
		enum class TypeCheckResult
		{
			COMPATIBLE,
			WIDEN,
			BITCAST_TO_INT,
			INT_TO_FLOAT,
			METHOD_CALL,
			INCOMPATIBLE
		};

		struct LowType
		{
			const SSA id;
			const Opcode category;

			FunctionGroup constructors;
			SSA destructor = 0;

			LowType(SSA id, Opcode tc) : id(id), category(tc){}

			virtual uint32_t getBitWidth() const = 0;
			virtual uint32_t getBitAlign() const = 0;

			virtual TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const = 0;

			virtual bool addMember(std::string name, sptr<const LowType> type) = 0;
			virtual bool getMember(std::string name, out<std::pair<uint32_t, sptr<LowType>>> member) const = 0;

			virtual bool setMemberFns(std::string name, ref<sptr<FunctionGroup>> fn) = 0;
			virtual sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const = 0;

		};

		struct TypeChecker
		{
			sptr<const CompilerSettings> settings;

			TypeChecker(sptr<const CompilerSettings> cs) : settings(cs) {}
			~TypeChecker() {}

			TypeCheckResult lookup(sptr<LowType> targetType, in<TypedSSA> rhs, in<Assembler> codeAsm) const;

			bool check(sptr<LowType> targetType, out<TypedSSA> rhs, out<Assembler> codeAsm, Operator op = Operator::NONE) const;

		};

	}
}