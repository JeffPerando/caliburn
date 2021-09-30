
#pragma once

#include "type.h"
#include "valuestmnt.h"

namespace caliburn
{
	struct TypeStruct : public SpecializedType
	{
	protected:
		std::vector<VarSymbol*> fieldsList;
		std::map<std::string, VarSymbol*> fieldsMap;
	public:
		TypeStruct(Token* name, std::initializer_list<TypeAttrib> attribs, size_t genMin, size_t genMax) :
			SpecializedType(TypeCategory::CUSTOM, name->str, attribs, genMin, genMax)
		{

		}

		void pushField(VarSymbol* sym)
		{
			fieldsList.push_back(sym);
			fieldsMap.emplace(sym->name, sym);
			
		}

	};

}
