
#pragma once

#include <variant>

#include "basic.h"
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

		struct LowType;

		using IndexedMember = std::pair<uint32_t, sptr<LowType>>;
		using LowMember = std::variant<std::monostate, IndexedMember, TypedSSA>;

		struct LowType
		{
			const SSA id;
			const Opcode category;

			FunctionGroup constructors;
			sptr<Method> destructor = nullptr;

			LowType(SSA id, Opcode tc) : id(id), category(tc) {}
			virtual ~LowType() = default;

			virtual uint32_t getBitWidth() const = 0;
			virtual uint32_t getBitAlign() const = 0;

			virtual TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const = 0;

			virtual bool addMember(std::string_view name, sptr<const LowType> type)
			{
				return false;
			}

			virtual LowMember getMember(SSA objID, std::string_view name, out<cllr::Assembler> codeAsm) const
			{
				return LowMember();
			}

			virtual std::vector<std::string> getMembers() const
			{
				return {};
			}

			virtual bool addMemberFn(sptr<Method> fn)
			{
				return false;
			}

			virtual sptr<FunctionGroup> getMemberFns(std::string_view name) const
			{
				return nullptr;
			}

		};

		struct TypeChecker
		{
			sptr<const CompilerSettings> settings;

			TypeChecker(sptr<const CompilerSettings> cs) : settings(cs) {}
			virtual ~TypeChecker() = default;

			TypeCheckResult lookup(sptr<LowType> targetType, in<TypedSSA> rhs, in<Assembler> codeAsm) const;

			bool check(sptr<LowType> targetType, in<TypedSSA> rhs, out<TypedSSA> result, out<Assembler> codeAsm, Operator op = Operator::NONE) const;

		};

	}

}
