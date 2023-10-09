
#pragma once

#include <map>

#include "ast/fn.h"

#include "types/type.h"

namespace caliburn
{
	struct TypeStruct;

	struct RealStruct : RealType
	{
		RealStruct(ptr<TypeStruct> parent, sptr<GenericArguments> gArgs) : RealType((ptr<BaseType>)parent, gArgs) {}

		sptr<cllr::LowType> emitTypeCLLR(sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) override;

	};

	struct TypeStruct : GenericType<RealStruct>
	{
		std::map<std::string, Member> members;
		std::vector<sptr<Function>> constructors;
		sptr<Function> destructor = nullptr;
		
		TypeStruct(in<std::string> name, sptr<GenericSignature> gSig, in<std::map<std::string, Member>> members)
			: GenericType(TypeCategory::STRUCT, name, gSig, lambda_v(sptr<GenericArguments> gArgs)
		{
			return new_sptr<RealStruct>(this, gArgs);
		}), members(members) {}
		virtual ~TypeStruct() {}

		virtual Member getMember(in<std::string> name) const
		{
			return members.at(name);
		}

	};

}
