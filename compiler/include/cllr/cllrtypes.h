
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

			LowPrimitive(SSA id, Opcode cat, uint32_t bits) : LowType(id, cat), width(bits) {}

			uint32_t getBitWidth() const override
			{
				return width;
			}

			uint32_t getBitAlign() const override
			{
				return width;
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override = 0;

			bool addMember(std::string name, sptr<const LowType> type) override
			{
				return false;
			}

			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override
			{
				return LowMember();
			}

			std::vector<std::string> getMembers() const override
			{
				return std::vector<std::string>();
			}

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override
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
			LowVoid(SSA id) : LowPrimitive(id, Opcode::TYPE_VOID, 0) {}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override
			{
				return TypeCheckResult::INCOMPATIBLE;
			}

		};

		struct LowFloat : LowPrimitive
		{
			LowFloat(SSA id, uint32_t w) : LowPrimitive(id, Opcode::TYPE_FLOAT, w) {}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override;

		};

		struct LowInt : LowPrimitive
		{
			LowInt(SSA id, Opcode cat, uint32_t w) : LowPrimitive(id, cat, w) {}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override;

		};

		struct LowBool : LowPrimitive
		{
			LowBool(SSA id) : LowPrimitive(id, Opcode::TYPE_BOOL, 8) {}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override;

		};

		struct LowArray : LowType
		{
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowArray(SSA id, uint32_t l, sptr<LowType> inner) :
				LowType(id, Opcode::TYPE_ARRAY), length(l), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override
			{
				return TypeCheckResult::INCOMPATIBLE;
			}

			bool addMember(std::string name, sptr<const LowType> type) override
			{
				return false;
			}

			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override
			{
				return LowMember();
			}

			std::vector<std::string> getMembers() const override
			{
				return std::vector<std::string>();
			}

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override
			{
				return false;
			}

			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override
			{
				return nullptr;
			}

		};

		struct LowVector : LowType
		{
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowVector(SSA id, uint32_t l, sptr<LowType> inner) :
				LowType(id, Opcode::TYPE_ARRAY), length(l), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override
			{
				return innerType->typeCheck(target, fnID, op);
			}

			bool addMember(std::string name, sptr<const LowType> type) override
			{
				return false;
			}

			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override;

			std::vector<std::string> getMembers() const override
			{
				return {"x", "y", "z", "w"};
			}

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override
			{
				return false;
			}

			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override
			{
				return nullptr;
			}

		};

		struct LowMatrix : LowType
		{
			const uint32_t width;
			const uint32_t length;
			
			sptr<LowType> innerType;

			LowMatrix(SSA id, uint32_t x, uint32_t y, sptr<LowType> inner) :
				LowType(id, Opcode::TYPE_MATRIX), width(x), length(y), innerType(inner) {}

			uint32_t getBitWidth() const override
			{
				return width * length * innerType->getBitWidth();
			}

			uint32_t getBitAlign() const override
			{
				return innerType->getBitAlign();
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override
			{
				return innerType->typeCheck(target, fnID, op);
			}

			bool addMember(std::string name, sptr<const LowType> type) override
			{
				return false;
			}

			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override
			{
				return LowMember();
			}

			std::vector<std::string> getMembers() const override
			{
				return std::vector<std::string>();
			}

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override
			{
				return false;
			}

			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override
			{
				return nullptr;
			}

		};

		struct LowStruct : LowType
		{
			uint32_t totalLength = 0;
			std::vector<sptr<LowType>> memberVars;
			std::map<SSA, SSA> conversions;

			HashMap<std::string, std::pair<SSA, sptr<LowType>>> members;
			HashMap<std::string, uptr<FunctionGroup>> memberFns;

			LowStruct(SSA id) : LowType(id, Opcode::TYPE_STRUCT) {}

			uint32_t getBitWidth() const override
			{
				return totalLength;
			}

			uint32_t getBitAlign() const override
			{
				return memberVars.back()->getBitAlign();
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override;

			bool addMember(std::string name, sptr<const LowType> type) override;
			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override;
			std::vector<std::string> getMembers() const override;

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override;
			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override;

		};

		struct LowTexture : LowType
		{
			const TextureKind tex;

			HashMap<std::string, std::pair<SSA, sptr<LowType>>> members;
			HashMap<std::string, uptr<FunctionGroup>> memberFns;

			LowTexture(SSA id, TextureKind tk) : LowType(id, Opcode::TYPE_TEXTURE), tex(tk) {}

			uint32_t getBitWidth() const override
			{
				return 8;
			}

			uint32_t getBitAlign() const override
			{
				return 8;
			}

			TypeCheckResult typeCheck(sptr<const LowType> target, out<cllr::SSA> fnID, Operator op = Operator::NONE) const override
			{
				return TypeCheckResult::INCOMPATIBLE;
			}

			bool addMember(std::string name, sptr<const LowType> type) override
			{
				return false;
			}

			LowMember getMember(SSA objID, std::string name, out<cllr::Assembler> codeAsm) const override
			{
				return LowMember();
			}

			std::vector<std::string> getMembers() const override
			{
				return std::vector<std::string>();
			}

			bool setMemberFns(std::string name, sptr<FunctionGroup> fn) override;
			sptr<Function> getMemberFn(std::string name, in<std::vector<TypedSSA>> argTypes) const override;

		};

	}

}