
#pragma once

#include "cllrasm.h"
#include "cllrtype.h"

namespace caliburn
{
	namespace cllr
	{
		struct LowPrimitive : LowType
		{
			const uint32_t width;

			LowPrimitive(Opcode cat, uint32_t bits) : LowType(cat), width(bits) {}

			uint32_t getBitWidth() const override
			{
				return width;
			}

			uint32_t getBitAlign() const override
			{
				return width;
			}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override = 0;

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override = 0;

			bool addMember(std::string name, SSA typeID, sptr<const LowType> typeImpl) override
			{
				return false;
			}

			bool getMember(std::string name, out<std::pair<uint32_t, sptr<LowType>>> member) const override
			{
				return false;
			}

			bool setMemberFns(std::string name, ref<sptr<FunctionGroup>> fn) override
			{
				return false;
			}

			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override
			{
				return nullptr;
			}

		};

		struct LowVoid : LowPrimitive
		{
			LowVoid() : LowPrimitive(Opcode::TYPE_VOID, 0) {}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return ConvertResult::INCOMPATIBLE;
			}

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override
			{
				return OpResult::INCOMPATIBLE;
			}

		};

		struct LowFloat : LowPrimitive
		{
			LowFloat(uint32_t w) : LowPrimitive(Opcode::TYPE_FLOAT, w) {}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override;

		};

		struct LowInt : LowPrimitive
		{
			LowInt(Opcode cat, uint32_t w) : LowPrimitive(cat, w) {}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const;

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override;

		};

		struct LowBool : LowPrimitive
		{
			LowBool() : LowPrimitive(Opcode::TYPE_BOOL, 8) {}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override;

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

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return ConvertResult::INCOMPATIBLE;
			}

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override
			{
				return OpResult::INCOMPATIBLE;
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

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return innerType->isConvertibleTo(other, otherImpl, op);
			}

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override
			{
				return innerType->getOp(op, fnID);
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

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override
			{
				return innerType->isConvertibleTo(other, otherImpl, op);
			}

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override
			{
				return innerType->getOp(op, fnID);
			}

		};

		struct LowStruct : LowType
		{
			uint32_t totalLength = 0;
			std::vector<sptr<LowType>> memberVars;
			std::map<SSA, SSA> conversions;

			HashMap<std::string, std::pair<SSA, sptr<LowType>>> members;
			HashMap<std::string, uptr<FunctionGroup>> memberFns;

			LowStruct() : LowType(Opcode::TYPE_STRUCT) {}

			uint32_t getBitWidth() const override
			{
				return totalLength;
			}

			uint32_t getBitAlign() const override
			{
				return memberVars.back()->getBitAlign();
			}

			ConvertResult isConvertibleTo(SSA other, sptr<const LowType> otherImpl, Operator op) const override;

			OpResult getOp(Operator op, out<cllr::SSA> fnID) const override
			{
				return OpResult::INCOMPATIBLE;
			}

			bool addMember(std::string name, SSA typeID, sptr<const LowType> typeImpl) override;
			bool getMember(std::string name, out<std::pair<uint32_t, sptr<LowType>>> member) const override;

			bool setMemberFns(std::string name, ref<sptr<FunctionGroup>> fn) override;
			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override;

		};

	}

}