
#pragma once

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>

namespace caliburn
{
	enum CFGRuleType : uint8_t
	{
		RAW_STRING, REGEX, RULESET
	};

	struct CFGRule
	{
		std::string name;
		CFGRuleType type;
		bool optional = false;
		uint8_t ruleCount = 0;
		char filler = '\0';
		uint32_t ruleID = 0;
		union
		{
			std::string raw;
			//TODO Find regex library
			//std::regex expr;
			CFGRule* ruleset[16];
		} data;

		bool match(std::vector<std::string>* tokens, uint64_t& current)
		{
			std::string curTok = tokens->at(current);
			if (type == RAW_STRING)
			{
				return data.raw == curTok;
			}
			else if (type == RULESET)
			{
				for (int i = 0; i < ruleCount; ++i)
				{
					if (!data.ruleset[i]->match(tokens, current))
					{
						return false;
					}
				}

				return true;
			}

			return false;
		}

	};

	struct CFGToken
	{
		std::string content;
		uint32_t ruleIdentifier;
	};

	class CFGParser
	{
	private:
		std::vector<void*> rules = {};
		//std::regex whitespace;

	public:
		void addRule(CFGRule* rule)
		{
			rules.push_back(&rule);
		}

		std::vector<CFGToken> tokenize(std::string str)
		{
			std::vector<CFGToken> cfgTokens = {};
			std::vector<std::string> strTokens = {};
			size_t current = 0;

			/*
			1. Clear out any and all whitespace
			2. Split string into tokens
			3. Use CFG rules to determine how these tokens fit
			*/
			
			return cfgTokens;
		}

	};
};
