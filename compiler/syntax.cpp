
#include "basic.h"

#include "syntax.h"

using namespace caliburn;

std::string caliburn::findStrForOp(Operator op)
{
	return std::find_if(std::begin(infixOps), std::end(infixOps), lambda(const auto& pair) { return pair.second == op; })->first;
}
