
#pragma once

#include "ast/basetypes.h"

#include "caliburn.h"
#include "symbols.h"

namespace caliburn
{
	static sptr<SymbolTable> makeStdLib(sptr<const CompilerSettings> options)
	{
		auto root = new_sptr<SymbolTable>();

		root->addType(new_sptr<TypeArray>());
		root->addType(new_sptr<TypeBool>());
		root->addType(new_sptr<TypeVoid>());

		for (auto bits = MIN_INT_BITS; bits <= MAX_INT_BITS; bits *= 2)
		{
			root->addType(new_sptr<TypeInt>(bits, true));
			root->addType(new_sptr<TypeInt>(bits, false));
		}

		for (auto bits = MIN_FLOAT_BITS; bits <= MAX_FLOAT_BITS; bits *= 2)
		{
			root->addType(new_sptr<TypeFloat>(bits));
		}

		for (auto len = MIN_VECTOR_LEN; len <= MAX_VECTOR_LEN; ++len)
		{
			root->addType(new_sptr<TypeVector>(len));
		}

		for (auto& [tt, _] : TEX_TYPES)
		{
			root->addType(new_sptr<TypeTexture>(tt));
		}

		return root;
	}

}