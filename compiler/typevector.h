
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVector : public ConcreteType
	{
		uint32_t const elements;
		ConcreteType* const inner;

		TypeVector(uint32_t vecElements, ConcreteType* innerType) :
			ConcreteType(TypeCategory::VECTOR, "vec" + vecElements, 1),
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

		ConcreteType* clone() const override;

		//virtual void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;
		/*
		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			inner->getSSAs(codeAsm);
			id = codeAsm.createSSA(cllr::Opcode::TYPE_VECTOR);
		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			inner->emitDeclCLLR(codeAsm);
			codeAsm.push(id, cllr::Opcode::TYPE_VECTOR, { elements, 0, 0 }, { inner->id });
		}

	};

}