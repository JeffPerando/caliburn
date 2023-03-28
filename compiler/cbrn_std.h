
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
		TypeBool* boolType;
		std::vector<TypeFloat*> floats;
		std::vector<TypeInt*> ints;
		std::vector<TypeVector*> vectors;

	public:
		DefaultLib()
		{
			auto constexpr aType = SymbolType::TYPE;

			boolType = new TypeBool();
			add("bool", aType, boolType);

			for (auto b = MIN_FLOAT_BITS; b < MAX_FLOAT_BITS; b *= 2)
			{
				auto f = new TypeFloat(b);
				floats.push_back(f);
				add("float" + b, aType, f);
			}

			for (auto b = MIN_INT_BITS; b < MAX_INT_BITS; b *= 2)
			{
				auto i = new TypeInt(b, true);
				auto u = new TypeInt(b, false);
				
				ints.push_back(i);
				ints.push_back(u);

				add("int" + b, aType, i);
				add("uint" + b, aType, u);

			}

		}

		~DefaultLib()
		{
			SymbolTable::~SymbolTable();

			if (boolType)
			{
				delete boolType;
				boolType = nullptr;
			}

			for (auto f : floats)
			{
				delete f;
			}

			for (auto i : ints)
			{
				delete i;
			}

			for (auto v : vectors)
			{
				delete v;
			}

			floats.clear();
			ints.clear();
			vectors.clear();

		}

	};

}