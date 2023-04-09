
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

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			return codeAsm.push(0, cllr::Opcode::VALUE_LITERAL, { 0 }, { this->id }, true);
		}

		ConcreteType* clone() const override
		{
			return (ConcreteType*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>& types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;
		/*
		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			id = codeAsm.createSSA(cllr::Opcode::TYPE_INT);
		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			codeAsm.push(id, cllr::Opcode::TYPE_INT, { intBits, isSigned, 0 }, {});
		}

	};

}