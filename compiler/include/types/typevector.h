
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector : public Type
	{
		const uint32_t elements;
		const sptr<Type> inner;

		TypeVector(uint32_t vecElements, sptr<Type> innerType) :
			Type(TypeCategory::VECTOR, "vec" + vecElements, 1),
			elements(vecElements), inner(innerType)
		{
			setGeneric(0, innerType);

		}

		uint32_t getSizeBytes() const override;

		uint32_t getAlignBytes() const override;

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			return 0;
			//TODO figure out vector semantics
			//return codeAsm.push(0, cllr::Opcode::VALUE_LITERAL, { 0, 0, 0 }, { this->id }, true);
		}

		//void getConvertibleTypes(std::set<sptr<Type>>* types) override;

		TypeCompat isCompatible(Operator op, sptr<Type> rType) const override;

		sptr<Type> makeVariant(ref<std::vector<sptr<Type>>> genArgs) const override;

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override;

	};

}