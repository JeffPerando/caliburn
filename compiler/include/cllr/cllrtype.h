
#pragma once

#include "cllr.h"
#include "error.h"
#include "langcore.h"

namespace caliburn
{
	namespace cllr
	{
		enum class ConversionResult
		{
			NO_CONVERSION,
			WIDEN,
			BITCAST_TO_INT,
			INT_TO_FLOAT,
			METHOD_CONVERSION,
			INCOMPATIBLE
		};

		struct LowType
		{
			const Opcode category;

			LowType(Opcode tc) : category(tc) {}

			virtual uint32_t getBitWidth() const = 0;

			virtual uint32_t getBitAlign() const = 0;

			virtual ConversionResult isConvertibleTo(cllr::SSA other, sptr<const LowType> otherImpl) const = 0;

			virtual bool addMember(cllr::SSA typeID, sptr<const LowType> typeImpl);

			virtual bool addConverter(cllr::SSA type, cllr::SSA fnID);

		};

		struct TypeCheckResult
		{
			TypedSSA lhs;
			TypedSSA rhs;
		};

		struct TypeChecker
		{
			bool check(ref<TypedSSA> lhs, ref<TypedSSA> rhs, Operator op, ref<cllr::Assembler> codeAsm) const;

		};

	}
}