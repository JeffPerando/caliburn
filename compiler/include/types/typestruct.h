
#pragma once

#include "ast/fn.h"

#include "types/type.h"

namespace caliburn
{
	struct TypeStruct;

	struct RealStruct : RealType
	{
		RealStruct(ptr<Generic<RealStruct>> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override;

	};

	struct TypeStruct : GenericType<RealStruct>
	{
		friend struct RealStruct;

		std::vector<sptr<Variable>> memberVars;
		std::vector<sptr<Function>> constructors;
		//std::vector<sptr<Function>> memberFns;

		TypeStruct(std::string name, sptr<GenericSignature> gSig) : GenericType(TypeCategory::STRUCT, name, gSig) {}

		virtual Member getMember(ref<const std::string> name) const
		{
			for (auto const& v : memberVars)
			{
				if (v->nameTkn->str == name)
				{
					return v;
				}

			}
			
			return Member();
		}

		virtual sptr<Function> getConstructor(ref<std::vector<RealType>> args) const
		{
			//TODO implement constructor finder
			return nullptr;
		}

		virtual sptr<Function> getDestructor() const
		{
			return nullptr;
		}

	};

}
