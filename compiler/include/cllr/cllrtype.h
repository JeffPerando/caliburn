
#pragma once

#include "error.h"
#include "langcore.h"

#include "ast/fn.h"

#include "cllr.h"

namespace caliburn
{
	struct FunctionGroup;

	namespace cllr
	{
		enum class ConvertResult
		{
			LGTM,
			WIDEN,
			BITCAST_TO_INT,
			INT_TO_FLOAT,
			METHOD_CONVERSION,
			INCOMPATIBLE
		};

		enum class OpResult
		{
			COMPATIBLE,
			CONVERT_NEEDED,
			METHOD_CALL,
			INCOMPATIBLE
		};

		struct LowType
		{
			const Opcode category;

			FunctionGroup constructors;
			SSA destructor = 0;

			LowType(Opcode tc) : category(tc) {}

			virtual uint32_t getBitWidth() const = 0;
			virtual uint32_t getBitAlign() const = 0;

			virtual ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const = 0;

			virtual OpResult getOp(Operator op, out<cllr::SSA> fnID) const = 0;

			virtual bool addMember(std::string name, SSA typeID, sptr<const LowType> typeImpl)
			{
				return false;
			}

			virtual bool getMember(std::string name, out<std::pair<uint32_t, sptr<LowType>>> member) const
			{
				return false;
			}

			virtual bool setMemberFns(std::string name, ref<sptr<FunctionGroup>> fn)
			{
				return false;
			}

			virtual sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const
			{
				return nullptr;
			}

		};

		struct TypeChecker
		{
			sptr<const CompilerSettings> settings;

			TypeChecker(sptr<const CompilerSettings> cs) : settings(cs) {}
			~TypeChecker() {}

			ConvertResult lookup(SSA targetType, in<TypedSSA> rhs, in<Assembler> codeAsm) const;

			bool check(SSA targetType, out<TypedSSA> rhs, out<Assembler> codeAsm, Operator op = Operator::UNKNOWN) const;

		};

	}
}