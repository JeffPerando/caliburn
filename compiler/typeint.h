
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeInt : public Type
	{
	protected:
		const uint32_t intBits;
		const bool isSigned;
	public:
		TypeInt() : TypeInt(32, true) {}
		TypeInt(uint32_t b, bool s) :
			Type(TypeCategory::INT, std::string((s ? "int" : "uint") + b)),
			intBits(b),
			isSigned(s)
		{}

		uint32_t getSizeBytes() const override
		{
			return intBits / 8;
		}

		uint32_t getAlignBytes() const override
		{
			return intBits / 8;
		}

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_LIT_INT, { 0 }, { this->id });
		}

		TypeCompat isCompatible(Operator op, sptr<Type> rType) const override;
		
		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

	};

}
