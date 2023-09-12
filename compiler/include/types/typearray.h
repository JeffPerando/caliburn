
#pragma once

#include "type.h"

namespace caliburn
{
	struct TypeArray;

	struct RealArray : RealType
	{
		RealArray(ptr<TypeArray> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeArray : GenericType<RealArray>
	{
		TypeArray() :
			GenericType(TypeCategory::ARRAY, "array", new_sptr<GenericSignature>(std::initializer_list{
				GenericName(GenericSymType::TYPE, "T"),
				GenericName(GenericSymType::CONST, "N")
				}),
				lambda_v(sptr<GenericArguments> gArgs) {
					return new_sptr<RealArray>(this, gArgs);
				}
			)
		{}

		Member getMember(ref<const std::string> name) const override
		{
			//TODO get length
			return Member();
		}

	};

}