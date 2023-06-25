
#include "syntax.h"

#include "basic.h"

using namespace caliburn;

std::string caliburn::findStrForOp(Operator op)
{
	return std::find_if(std::begin(INFIX_OPS), std::end(INFIX_OPS), lambda(const auto& pair) { return pair.second == op; })->first;
}
