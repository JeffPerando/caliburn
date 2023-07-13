
#include "types/type.h"

#include "ast/ast.h"

using namespace caliburn;

void ParsedType::prettyPrint(ref<std::stringstream> ss) const
{
	if (name == "")
	{
		ss << "INVALID TYPE PLS FIX";
		return;
	}

	if (fullName.length() == 0)
	{
		std::stringstream s;

		s << name;
		genericArgs->prettyPrint(s);

		s.str(fullName);

	}

	ss << fullName;

}

sptr<RealType> ParsedType::resolve(sptr<const SymbolTable> table)
{
	if (resultType != nullptr)
	{
		return resultType;
	}

	auto cTypeSym = table->find(name);

	if (auto cType = std::get_if<sptr<RealType>>(&cTypeSym))
	{
		if (!genericArgs->empty())
		{
			//TODO complain
		}

		resultType = *cType;
		return resultType;
	}
	else if (auto cType = std::get_if<sptr<BaseType>>(&cTypeSym))
	{
		resultType = (**cType).getImpl(genericArgs);
		return resultType;
	}

	//TODO complain
	return nullptr;
}

void Variable::resolveSymbols(sptr<const SymbolTable> table)
{
	/*
	if (initValue != nullptr)
	{
		initValue->resolveSymbols(table);
	}
	*/
}
