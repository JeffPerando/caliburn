
#pragma once

#include "caliburn.h"
#include "symbols.h"

#include "types/typearray.h"
#include "types/typebool.h"
#include "types/typefloat.h"
#include "types/typeint.h"
#include "types/typetex.h"
#include "types/typevector.h"
#include "types/typevoid.h"

namespace caliburn
{
	static sptr<SymbolTable> makeStdLib(sptr<CompilerSettings> options)
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

		root->addType(new_sptr<TypeTex2D>());

		return root;
	}

}