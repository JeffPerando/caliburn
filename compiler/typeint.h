
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeInt : public ConcreteType
	{
	protected:
		const uint32_t intBits;
		const bool isSigned;
	public:
		TypeInt() : TypeInt(32, true) {}
		TypeInt(uint32_t b, bool s) :
			ConcreteType(TypeCategory::INT, std::string((s ? "int" : "uint") + b)),
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