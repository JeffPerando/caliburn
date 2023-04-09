
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

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			return codeAsm.push(0, cllr::Opcode::VALUE_LITERAL, { false }, { this->id }, true);
		}

		virtual ConcreteType* clone() const override
		{
			//To future me: This cast is necessary; it complains otherwise. Yes it looks goofy. TypeBool* != ConcreteType*. smh
			return (ConcreteType*)this;
		}

		//void getConvertibleTypes(std::set<ConcreteType*>* types) override;

		TypeCompat isCompatible(Operator op, ConcreteType* rType) const override;
		/*
		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			id = codeAsm.createSSA(cllr::Opcode::TYPE_BOOL);
		}
		*/
		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			codeAsm.push(id, cllr::Opcode::TYPE_BOOL, {}, {});
		}

	};

}
