
#include "cfg.h"

using namespace caliburn::cfg;

bool RuleConst::match(buffer<Token>* tokens) const
{
	if (tokens->current()->str == this->token)
	{
		tokens->consume();
		return true;
	}

	return false;
}

bool RuleOptional::match(buffer<Token>* tokens) const
{
	innerRule->match(tokens);
	return true;
}

bool RuleAll::match(buffer<Token>* tokens) const
{
	auto index = tokens->currentIndex();

	for (auto rule : rules)
	{
		if (!rule->match(tokens))
		{
			tokens->revertTo(index);
			return false;
		}

	}

	return true;
}

bool RuleAny::match(buffer<Token>* tokens) const
{
	auto index = tokens->currentIndex();

	for (auto rule : rules)
	{
		if (rule->match(tokens))
		{
			return true;
		}

		tokens->revertTo(index);

	}

	return false;
}