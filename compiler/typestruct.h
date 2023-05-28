
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeStruct : public Type
	{
		TypeStruct(std::string name, size_t genMax) :
			Type(TypeCategory::STRUCT, name, 0, genMax) {}

		//At this point, I feel the need for a getOptimalSize() method.
		uint32_t getSizeBytes() const override
		{
			uint32_t size = 0;
			uint32_t lastAlign = 1;

			for (auto const& m : members)
			{
				auto t = m->type;
				size += t->getSizeBytes();

				auto align = t->getAlignBytes();

				//calculate padding
				if (size % align != lastAlign)
				{
					size += ((size / lastAlign) + 1) * lastAlign;
				}

				lastAlign = align;

			}

			return size;
		}

		uint32_t getAlignBytes() const override
		{
			if (members.empty())
			{
				return 1;
			}

			return members.back()->type->getAlignBytes();
		}

		sptr<Type> makeVariant(ref<std::vector<sptr<Type>>> genArgs) const override
		{
			auto ret = std::make_unique<TypeStruct>(canonName, maxGenerics);

			//TODO check compatibility

			for (size_t i = 0; i < genArgs.size(); ++i)
			{
				ret->setGeneric(i, genArgs.at(i));
			}

			return ret;
		}

		virtual cllr::SSA emitDefaultInitValue(ref<cllr::Assembler> codeAsm)
		{
			//TODO consider
			return codeAsm.pushNew(cllr::Opcode::UNKNOWN, {}, { this->id });
		}

		TypeCompat isCompatible(Operator op, sptr<Type> rType) const override
		{
			return TypeCompat::INCOMPATIBLE_TYPE;
		}

		void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			if (id != 0)
			{
				return;
			}

			id = codeAsm.pushNew(cllr::Opcode::TYPE_STRUCT, {}, {});

			for (auto const& m : members)
			{
				m->emitDeclCLLR(codeAsm);

			}

		}

	};

}
