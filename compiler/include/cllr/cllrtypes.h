
#pragma once

#include "cllrtype.h"

namespace caliburn
{
	namespace cllr
	{
		struct LowVoid : LowType
		{
			LowVoid() : LowType(Opcode::TYPE_VOID) {}

			uint32_t getBitWidth() const override
			{
				return 0;
			}

			uint32_t getBitAlign() const override
			{
				return 0;
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return ConversionResult::INCOMPATIBLE;
			}

		};

		struct LowFloat : LowType
		{
			const uint32_t width;

			LowFloat(uint32_t w) : LowType(Opcode::TYPE_FLOAT), width(w) {}

			uint32_t getBitWidth() const override
			{
				return width;
			}

			uint32_t getBitAlign() const override
			{
				return width;
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;

		};

		struct LowInt : LowType
		{
			const uint32_t width;

			LowInt(Opcode cat, uint32_t w) :
				LowType(cat), width(w) {}

			uint32_t getBitWidth() const override
			{
				return width;
			}

			uint32_t getBitAlign() const override
			{
				return width;
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const;

		};

		struct LowArray : LowType
		{
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowArray(uint32_t l, sptr<LowType> inner) :
				LowType(Opcode::TYPE_ARRAY), length(l), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return ConversionResult::INCOMPATIBLE;
			}

		};

		struct LowVector : LowType
		{
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowVector(uint32_t l, sptr<LowType> inner) :
				LowType(Opcode::TYPE_ARRAY), length(l), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return innerType->isConvertibleTo(other, otherImpl, op);
			}

		};

		struct LowMatrix : LowType
		{
			const uint32_t width;
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowMatrix(uint32_t x, uint32_t y, sptr<LowType> inner) :
				LowType(Opcode::TYPE_MATRIX), width(x), length(y), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return width * length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return ConversionResult::INCOMPATIBLE;
			}

		};

		struct LowStruct : LowType
		{
			uint32_t totalLength = 0;
			std::vector<sptr<LowType>> memberVars;
			std::map<SSA, SSA> conversions;

			LowStruct() : LowType(Opcode::TYPE_STRUCT) {}

			uint32_t getBitWidth() const override
			{
				return totalLength;
			}

			uint32_t getBitAlign() const override
			{
				return memberVars.back()->getBitAlign();
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;
		};

		struct LowBool : LowType
		{
			LowBool() : LowType(Opcode::TYPE_BOOL) {}

			uint32_t getBitWidth() const override
			{
				return 8;
			}

			uint32_t getBitAlign() const override
			{
				return 8;
			}

			ConversionResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;

		};

	}

}