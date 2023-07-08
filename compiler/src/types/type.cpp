
#include "ast/ast.h"
#include "types/type.h"

using namespace caliburn;

sptr<RealType> ParsedType::resolve(sptr<const SymbolTable> table)
{
	if (resultType != nullptr)
	{
		return resultType;
	}

	auto cTypeSym = table->find(name->str);

	if (auto cType = std::get_if<sptr<BaseType>>(&cTypeSym))
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
