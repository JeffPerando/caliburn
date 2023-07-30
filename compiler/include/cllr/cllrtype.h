
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

		enum class OpResult
		{
			COMPATIBLE,
			CONVERT_NEEDED,
			METHOD_OP,
			INCOMPATIBLE
		};

		struct LowType
		{
			const Opcode category;

			LowType(Opcode tc) : category(tc) {}

			virtual uint32_t getBitWidth() const = 0;

			virtual uint32_t getBitAlign() const = 0;

			virtual ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const = 0;

			//virtual OpResult getOp(Operator op) const = 0;

			virtual bool addMember(SSA typeID, sptr<const LowType> typeImpl);

			virtual bool addConverter(SSA type, SSA fnID);

			virtual SSA getConverter(SSA type);
			
		};

		struct TypeChecker
		{
			sptr<const CompilerSettings> settings;

			TypeChecker(sptr<const CompilerSettings> cs) : settings(cs) {}
			~TypeChecker() {}

			bool check(SSA targetType, ref<TypedSSA> rhs, ref<Assembler> codeAsm, Operator op = Operator::UNKNOWN) const;

		};

	}
}