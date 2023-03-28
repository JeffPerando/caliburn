
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat : public ConcreteType
	{
	protected:
		uint32_t const floatBits;
	public:
		TypeFloat() : TypeFloat(32) {}
		TypeFloat(uint32_t s) :
			ConcreteType(TypeCategory::FLOAT, std::string("float" + s)),
			floatBits(s)
		{}

		uint32_t getSizeBytes() const override
		{
			return floatBits / 8;
		}

		uint32_t getAlignBytes() const override
		{
			return floatBits / 8;
		}
		
		ConcreteType* clone() const override
		{
			return (ConcreteType*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>& types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;

		virtual void getSSAs(cllr::Assembler& codeAsm) override;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}