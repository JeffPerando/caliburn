
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

			virtual ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl) const = 0;

			virtual bool addMember(SSA typeID, sptr<const LowType> typeImpl);

			virtual bool addConverter(SSA type, SSA fnID);

			virtual SSA getConverter(SSA type);
			
		};

		struct TypeChecker
		{
			//TODO make cllr::Assembler take a CompilerSettings
			//const CompilerSettings settings;

			//TypeChecker(ref<const CompilerSettings> cs) : settings(cs) {}
			TypeChecker() = default;
			~TypeChecker() {}

			bool check(SSA targetType, ref<TypedSSA> rhs, ref<Assembler> codeAsm) const;

		};

	}
}