
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
			return codeAsm.pushNew(cllr::Opcode::VALUE_LITERAL, { false }, { this->id });
		}

		virtual Type* clone() const override
		{
			//To future me: This cast is necessary; it complains otherwise. Yes it looks goofy. TypeBool* != ConcreteType*. smh
			return (Type*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, Type* rType) const override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}
