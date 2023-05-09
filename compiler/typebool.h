
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeBool : public Type
	{
		TypeBool() : Type(TypeCategory::BOOLEAN, "bool"){}

		uint32_t getSizeBytes() const override
		{
			return 1;
		}

		uint32_t getAlignBytes() const override
		{
			return 1;
		}

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			return codeAsm.pushNew(cllr::Opcode::VALUE_BOOL_LIT, { false }, { this->id });
		}

		//void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, sptr<Type> rType) const override;

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

	};

}
