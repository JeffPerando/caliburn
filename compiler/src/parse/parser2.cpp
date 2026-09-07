
#include "parse/parser2.h"
#include <ast/scopestmt.h>

using namespace caliburn::parse;

ParserV2::ParserV2(in<std::vector<Token>> tkns) : tkns(Buffer<Token>(tkns))
{
	addRule("scope", {
		{"", new Tkn(TokenType::START_SCOPE)},
		{"body", new List(",", new_uptr<Name>("logic"))},
		{"", new Tkn(TokenType::END_SCOPE)}
		},
		LAMBDA(Token start, Token end, in<ParseMap> result) {
			return new_sptr<ScopeStmt>(start, end, result);
		});
		
}

void ParserV2::addRule(in<std::string> name, std::initializer_list<std::pair<std::string, Subrule*>> rs, ParseResultFn outFn)
{
	rules[name].emplace(new_uptr<Rule>(rs, outFn));
}

Status ParserV2::test(in<std::string> name)
{
	size_t start = tkns.offset();

	auto ruleImpls = rules.find(name);
	if (ruleImpls == rules.end())
	{
		return Status::NONE;
	}

	for (auto& rule : ruleImpls->second)
	{
		auto const res = rule->test(*this);

		if (res == Status::COMPLETE)
		{
			return res;
		}

		tkns.revertTo(start);

	}

	return Status::INCOMPLETE;
}

ParseResult ParserV2::parse(in<std::string> ruleName)
{
	auto& pRules = rules[ruleName];

	for (auto const& rule : pRules)
	{
		auto const status = rule->test(*this);

		if (status == Status::INCOMPLETE)
		{

		}

		auto res = rule->parse(*this);

		MATCH_EXISTS(res)
		{
			return std::move(res);
		}

	}

	//TODO complain
	return ParseResult();
}

Status Rule::test(out<ParserV2> p) const
{
	size_t start = p.tkns.offset();

	for (size_t i = 0; i < rules.size(); ++i)
	{
		size_t ruleTknOff = p.tkns.offset();

		auto const& [name, rule] = rules[i];
		auto const status = rule->test(p);

		if (status != Status::COMPLETE)
		{
			if (rule->optional)
			{
				p.tkns.revertTo(ruleTknOff);
				continue;
			}

			p.tkns.revertTo(start);

			if (i > 0)
			{
				return Status::INCOMPLETE;
			}

			return status;
		}

	}

	return Status::COMPLETE;
}

ParseResult Rule::parse(out<ParserV2> p) const
{
	auto& tkns = p.tkns;
	ParseMap pMap;

	Token first = tkns.cur();

	for (auto const& [name, rule] : rules)
	{
		const size_t startOff = p.tkns.offset();

		auto result = rule->parse(p);

		MATCH_EMPTY(result)
		{
			continue;
		}

		if (name.length() > 0)
		{
			pMap[name] = std::move(result);
		}

	}

	Token last = tkns.peekBack(1);

	return parseFn(first, last, pMap);
}

Status Str::test(out<ParserV2> p) const
{
	if (p.tkns.cur().str == str)
	{
		p.tkns.consume();
		return Status::COMPLETE;
	}

	return Status::NONE;
}

ParseResult Str::parse(out<ParserV2> p) const
{
	auto& res = p.tkns.cur();

	if (res.str == str)
	{
		p.tkns.consume();
		return res;
	}

	return ParseResult();
}

Status Tkn::test(out<ParserV2> p) const
{
	if (p.tkns.cur().type == tknType)
	{
		p.tkns.consume();
		return Status::COMPLETE;
	}

	return Status::NONE;
}

ParseResult Tkn::parse(out<ParserV2> p) const
{
	auto const res = p.tkns.cur();

	if (res.type == tknType)
	{
		p.tkns.consume();
		return res;
	}

	return ParseResult();
}

Status Name::test(out<ParserV2> p) const
{
	return p.test(name);
}

ParseResult Name::parse(out<ParserV2> p) const
{
	return p.parse(name);
}

Status Any::test(out<ParserV2> p) const
{
	Status status = Status::NONE;
	
	for (auto& r : rules)
	{
		auto const ruleStatus = r->test(p);

		if (ruleStatus == Status::COMPLETE)
		{
			return Status::COMPLETE;
		}
		
		if (ruleStatus == Status::INCOMPLETE)
		{
			status = Status::INCOMPLETE;
		}

	}

	return status;
}

ParseResult Any::parse(out<ParserV2> p) const
{
	auto& tkns = p.tkns;
	const size_t off = tkns.offset();

	for (auto& rule : rules)
	{
		auto res = rule->parse(p);

		MATCH_EMPTY(res)
		{
			tkns.revertTo(off);
			continue;
		}

		return std::move(res);
	}

	return ParseResult();
}

Status List::test(out<ParserV2> p) const
{
	bool foundDelim = false;

	while (p.tkns.hasCur())
	{
		auto const s = itemRule->test(p);

		if (s != Status::COMPLETE)
		{
			if (foundDelim)
			{
				return Status::INCOMPLETE;
			}

			return s;
		}

		if (p.tkns.cur().str == delim)
		{
			p.tkns.consume();
			foundDelim = true;
		}
		else break;

	}

	return Status::COMPLETE;
}

ParseResult List::parse(out<ParserV2> p) const
{
	auto& tkns = p.tkns;

	Token start = tkns.cur();

	std::vector<ParseResult> results;

	while (tkns.hasCur())
	{
		auto res = itemRule->parse(p);

		MATCH_EMPTY(res)
		{
			break;
		}

		results.push_back(std::move(res));

		if (tkns.cur().str == delim)
		{
			tkns.consume();
		}
		else break;

	}

	Token end = tkns.peekBack(1);

	return results;
}
