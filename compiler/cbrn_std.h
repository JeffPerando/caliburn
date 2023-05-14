
#pragma once

#include "langcore.h"
#include "symbols.h"

#include "typebool.h"
#include "typefloat.h"
#include "typeint.h"
#include "typevector.h"
//#include "typevoid.h"

namespace caliburn
{
	class DefaultLib : public SymbolTable
	{
		sptr<TypeBool> boolType;
		std::vector<sptr<TypeFloat>> floats;
		std::vector<sptr<TypeInt>> ints;
		std::vector<sptr<TypeVector>> vectors;

	public:
		DefaultLib()
		{
			boolType = std::make_shared<TypeBool>();
			add("bool", boolType);

			for (auto b = MIN_FLOAT_BITS; b < MAX_FLOAT_BITS; b *= 2)
			{
				auto f = std::make_shared<TypeFloat>(b);
				floats.push_back(f);
				add("float" + b, f);
			}

			for (auto b = MIN_INT_BITS; b < MAX_INT_BITS; b *= 2)
			{
				auto i = std::make_shared<TypeInt>(b, true);
				auto u = std::make_shared<TypeInt>(b, false);
				
				ints.push_back(i);
				ints.push_back(u);

				add("int" + b, i);
				add("uint" + b, u);

			}

		}

		virtual ~DefaultLib() {}

	};

}