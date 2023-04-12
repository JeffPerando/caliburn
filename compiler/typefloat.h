
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeFloat : public Type
	{
	protected:
		uint32_t const floatBits;
	public:
		TypeFloat() : TypeFloat(32) {}
		TypeFloat(uint32_t s) :
			Type(TypeCategory::FLOAT, std::string("float" + s)),
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
			id = codeAsm.createSSA(cllr::Opcode::TYPE_FLOAT);
		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			codeAsm.push(id, cllr::Opcode::TYPE_FLOAT, { floatBits }, {});
		}

	};

}