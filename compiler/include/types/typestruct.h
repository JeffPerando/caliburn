
#pragma once

#include <map>

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
		std::map<std::string, Member> members;
		std::vector<sptr<Function>> constructors;
		sptr<Function> destructor = nullptr;
		
		TypeStruct(ref<const std::string> name, sptr<GenericSignature> gSig, ref<std::map<std::string, Member>> members)
			: GenericType(TypeCategory::STRUCT, name, gSig, lambda_v(sptr<GenericArguments> gArgs) {
			return new_sptr<RealStruct>(this, gArgs);
		}), members(members) {}
		virtual ~TypeStruct() {}

		virtual Member getMember(ref<const std::string> name) const
		{
			return members.at(name);
		}

		cllr::TypedSSA callConstructor(ref<cllr::Assembler> codeAsm, ref<std::vector<Value>> args) const override
		{
			return cllr::TypedSSA();
		}

		void callDestructor(cllr::SSA val) const override
		{

		}
		
	};

}
