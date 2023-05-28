
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat : public Type
	{
	protected:
		uint32_t const floatBits;
	public:
		TypeFloat() : TypeFloat(32) {}
		TypeFloat(uint32_t s) :
			Type(TypeCategory::FLOAT, std::string("float" + s)),
			floatBits(s)
		{}
		virtual ~TypeFloat() {}

		uint32_t getSizeBytes() const override
		{
			return floatBits / 8;
		}

		uint32_t getAlignBytes() const override
		{
			return floatBits / 8;
		}

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			auto sID = codeAsm.addString("0f");

			return codeAsm.pushNew(cllr::Opcode::VALUE_FP_LIT, { }, { sID, this->id });
		}

		TypeCompat isCompatible(Operator op, sptr<Type> rType) const override;
		
		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			codeAsm.push(id, cllr::Opcode::TYPE_FLOAT, { floatBits }, {});
		}

	};

}