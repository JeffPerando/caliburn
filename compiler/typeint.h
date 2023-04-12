
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeInt : public Type
	{
	protected:
		const uint32_t intBits;
		const bool isSigned;
	public:
		TypeInt() : TypeInt(32, true) {}
		TypeInt(uint32_t b, bool s) :
			Type(TypeCategory::INT, std::string((s ? "int" : "uint") + b)),
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
			return codeAsm.pushNew(cllr::Opcode::VALUE_LITERAL, { 0 }, { this->id });
		}

		Type* clone() const override
		{
			return (Type*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>& types) override;

		TypeCompat isCompatible(Operator op, Type* rType) const override;
		/*
		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			id = codeAsm.createSSA(cllr::Opcode::TYPE_INT);
		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override;

	};

}