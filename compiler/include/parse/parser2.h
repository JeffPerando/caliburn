
#pragma once

#include <functional>
#include <numeric>
#include <set>
#include <utility>
#include <variant>

#include "basic.h"
#include "buffer.h"
#include "syntax.h"

#include "ast/ast.h"

namespace caliburn
{
	struct Annotation;
	struct Expr;
	
	namespace parse
	{
		//Getting ParseResult to accept a vector of results was PAINFUL
		//thanks to https://stackoverflow.com/a/53504373 for getting it working

		template<typename T>
		using ParseIntermediate = std::variant<
			std::monostate,
			Token,
			sptr<Annotation>,
			sptr<GenericSignature>,
			sptr<Expr>,
			std::vector<T>,
			HashMap<std::string_view, std::vector<T>>
		>;

		template <template<class> class K>
		struct VarFix : K<VarFix<K>>
		{
			using K<VarFix>::K;
		};

		using ParseResult = VarFix<ParseIntermediate>;

		using ParseMap = HashMap<std::string_view, ParseResult>;
		using ParseResultFn = std::function<ParseResult(Token, Token, in<ParseMap>)>;

		enum class Status
		{
			NONE,
			INCOMPLETE,
			COMPLETE
		};

		/*
		MatchKind exists to quantify and create an efficienct order for parsing. If we have the rules laid out randomly in
		the rule vector, it will take a non-deterministic amount of time to parse a given text file. By codifying then sorting
		all rules, we can fail earlier and faster. This also has the added benefit of allowing for ambiguous syntaxes to be
		parsed by attempting to parse longer rules ahead of shorter ones.
		*/
		enum class MatchKind : uint32_t
		{
			STRING, //exact; we know what needs to be fulfilled
			KIND, //less exact; accepts various symbols of a given category.
			RULE, //even less exact; defers parsing to another rule.
			OTHER //I don't even think we need this, honestly
		};

		struct ParserV2;

		typedef Status (*ParseTestFn)(out<ParserV2> p);
		typedef ParseResult (*ParseRuleFn)(out<ParserV2> p);

		struct Subrule
		{
			const MatchKind kind;
			bool optional = false;

			Subrule(MatchKind k) : kind(k) {}
			virtual ~Subrule() = default;

			virtual uint64_t totalRuleCount()
			{
				return 0;
			}

			ptr<Subrule> opt()
			{
				optional = true;

				return this;
			}

			virtual Status test(out<ParserV2> p) const
			{
				return Status::NONE;
			}

			virtual ParseResult parse(out<ParserV2> p) const
			{
				return ParseResult();
			}
			
		};

		struct CmpRules
		{
			bool operator()(in<uptr<Subrule>> lhs, in<uptr<Subrule>> rhs) const
			{
				if (SCAST<uint32_t>(lhs->kind) < SCAST<uint32_t>(rhs->kind))
				{
					return true;
				}

				return lhs->totalRuleCount() > rhs->totalRuleCount();
			}
		};

		using RuleSet = std::set<uptr<Subrule>, CmpRules>;

		struct ParserV2
		{
		private:
			HashMap<std::string, RuleSet> rules;

		public:
			Buffer<Token> tkns;

			ParserV2(in<std::vector<Token>> tkns);
			virtual ~ParserV2() = default;

			void addRule(in<std::string> name, std::initializer_list<std::pair<std::string, uptr<Subrule>>> rules, ParseResultFn outFn);

			Status test(in<std::string> name);

			ParseResult parse(in<std::string> rule);

		};

		class Rule : public Subrule
		{
			//notably not a RuleSet container because these are supposed to be ordered linearly.
			std::vector<std::pair<std::string, uptr<Subrule>>> rules;
			ParseResultFn parseFn;
			uint64_t savedRuleLen = 0;

		public:
			Rule(std::initializer_list<std::pair<std::string, Subrule&&>> rs, in<ParseResultFn> pFn) :
				Subrule(rs.begin()->second.kind), parseFn(pFn)
			{
				for (auto& [name, r] : rs)
				{
					rules.push_back(std::pair(name, new_uptr<Subrule>(std::move(r))));
				}
			}

			uint64_t totalRuleCount() override
			{
				if (savedRuleLen > 0)
				{
					return savedRuleLen;
				}

				for (auto const& [name, r] : rules)
				{
					savedRuleLen += r->totalRuleCount();
				}

				return savedRuleLen;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};

		class Str : public Subrule
		{
			const std::string str;

		public:
			Str(in<std::string> s) :
				Subrule(MatchKind::STRING), str(s) {}

			uint64_t totalRuleCount() override
			{
				return 1;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};

		class Tkn : public Subrule
		{
			const TokenType tknType;

		public:
			Tkn(TokenType tt) :
				Subrule(MatchKind::KIND), tknType(tt) {}

			uint64_t totalRuleCount() override
			{
				return 1;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};

		class Name : public Subrule
		{
			const std::string name;

		public:
			Name(in<std::string> n) :
				Subrule(MatchKind::RULE), name(n) {}

			uint64_t totalRuleCount() override
			{
				return 1;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};

		class Any : public Subrule
		{
			RuleSet rules;
			uint64_t savedRuleLen = 0;

		public:
			Any(out<std::vector<uptr<Subrule>>> rs) :
				//Technically there's a way to calculate how strict and how many rules there are.
				//I'm too lazy to write it out.
				Subrule(MatchKind::OTHER)
			{
				for (auto& r : rs)
				{
					rules.emplace(std::move(r));
				}
			}

			uint64_t totalRuleCount() override
			{
				if (savedRuleLen > 0)
				{
					return savedRuleLen;
				}

				for (auto const& r : rules)
				{
					auto const len = r->totalRuleCount();

					if (len > savedRuleLen)
					{
						savedRuleLen = len;
					}

				}

				return savedRuleLen;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};
		
		class List : public Subrule
		{
			const std::string delim;
			const uptr<Subrule> itemRule;
			
		public:
			List(in<std::string> d, uptr<Subrule> item) :
				Subrule(MatchKind::RULE), delim(d), itemRule(std::move(item)) {}

			uint64_t totalRuleCount() override
			{
				return 2;
			}

			Status test(out<ParserV2> p) const override;

			ParseResult parse(out<ParserV2> p) const override;

		};

	}

}
