
#pragma once

#include <map>

#include "ast/fn.h"

#include "types/type.h"

namespace caliburn
{
	struct TypeStruct : BaseType
	{
		const uptr<GenericSignature> genSig;

		std::map<std::string, sptr<ParsedVar>> members;
		std::vector<uptr<ParsedFn>> memberFns;
		GenArgMap<cllr::LowType> variants;
		
		TypeStruct(in<std::string> name, out<uptr<GenericSignature>> sig, in<std::map<std::string, sptr<ParsedVar>>> members, out<std::vector<uptr<ParsedFn>>> fns)
			: BaseType(TypeCategory::STRUCT, name), genSig(std::move(sig)), members(members), memberFns(std::move(fns)) {}

		virtual ~TypeStruct() = default;

		sptr<cllr::LowType> resolve(sptr<GenericArguments> gArgs, sptr<const SymbolTable> table, out<cllr::Assembler> codeAsm) override;
		
	};

}
