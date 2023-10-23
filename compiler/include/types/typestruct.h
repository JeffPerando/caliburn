
#pragma once

#include <map>

#include "ast/fn.h"

#include "types/type.h"

namespace caliburn
{
	struct TypeStruct : BaseType
	{
		const uptr<GenericSignature> genSig;

		std::map<std::string, Member> members;
		GenArgMap<cllr::LowType> variants;
		std::vector<sptr<Function>> constructors;
		sptr<Function> destructor = nullptr;
		
		TypeStruct(in<std::string> name, out<uptr<GenericSignature>> sig, in<std::map<std::string, Member>> members)
			: BaseType(TypeCategory::STRUCT, name), genSig(std::move(sig)), members(members) {}
		virtual ~TypeStruct() {}

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;

		virtual Member getMember(in<std::string> name) const
		{
			return members.at(name);
		}

	};

}
