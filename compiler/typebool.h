
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeBool : public ConcreteType
	{
		TypeBool() : ConcreteType(TypeCategory::BOOLEAN, "bool"){}

		uint32_t getSizeBytes() const override
		{
			return 1;
		}

		uint32_t getAlignBytes() const override
		{
			return 1;
		}

		virtual ConcreteType* clone() const override
		{
			//To future me: This cast is necessary; it complains otherwise. Yes it looks goofy. TypeBool* != ConcreteType*. smh
			return (ConcreteType*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		virtual void getSSAs(cllr::Assembler& codeAsm) override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}
