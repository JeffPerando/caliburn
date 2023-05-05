
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector : public Type
	{
		const uint32_t elements;
		const ptr<Type> inner;

		TypeVector(uint32_t vecElements, Type* innerType) :
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

		//virtual void getConvertibleTypes(std::set<Type*>* types) override;

		TypeCompat isCompatible(Operator op, Type* rType) const override;

		virtual ptr<Type> makeVariant(ref<std::vector<ptr<Type>>> genArgs) const override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}