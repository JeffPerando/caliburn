
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeVoid : public ConcreteType
	{
		TypeVoid() : ConcreteType(TypeCategory::VOID, "") {}

		uint32_t getSizeBytes() const override
		{
			return 0;
		}

		uint32_t getAlignBytes() const override
		{
			return 0;
		}

		void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		virtual void getSSAs(cllr::Assembler& codeAsm) override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};
}
