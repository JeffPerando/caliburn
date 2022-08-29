
//THIS IS NOT A CONFIG. If you want to change aspects of the compiler,
//see langcore.h and syntax.h
//This is the Context-Free Grammar implementation.

#pragma once

#include <string>

#include "buffer.h"
#include "syntax.h"

namespace caliburn
{
	namespace cfg
	{
		class Rule
		{
		protected:
			bool isOptional = false;
		public:
			Rule() {}
			virtual ~Rule() {}

			virtual bool match(buffer<Token>* tokens) const = 0;

			Rule* optional()
			{
				isOptional = true;
				return this;
			}

		};

		class RuleOptional : Rule
		{
		private:
			const Rule* innerRule;
		public:
			RuleOptional(Rule* rule) : innerRule(rule) {}

			bool match(buffer<Token>* tokens) const override = 0;

		};

		class RuleConst : Rule
		{
			std::string token;
		public:
			RuleConst(std::string tkn) : token(tkn) {}

			bool match(buffer<Token>* tokens) const override = 0;

		};

		class RuleAll : Rule
		{
			std::vector<Rule*> rules;

		public:
			RuleAll() {}
			RuleAll(std::initializer_list<Rule*> ruleList)
			{
				rules.reserve(ruleList.size());
				for (auto rule : ruleList)
				{
					rules.push_back(rule);
				}

			}

			RuleAll* addRule(Rule* rule)
			{
				rules.push_back(rule);
				return this;
			}

			bool match(buffer<Token>* tokens) const override = 0;

		};

		class RuleAny : Rule
		{
			std::vector<Rule*> rules;

		public:
			RuleAny() {}
			RuleAny(std::initializer_list<Rule*> ruleList)
			{
				rules.reserve(ruleList.size());
				for (auto rule : ruleList)
				{
					rules.push_back(rule);
				}

			}

			RuleAny* addRule(Rule* rule)
			{
				rules.push_back(rule);
				return this;
			}

			bool match(buffer<Token>* tokens) const override = 0;

		};

	}

}