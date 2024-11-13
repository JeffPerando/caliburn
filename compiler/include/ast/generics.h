
#pragma once

#include <functional>
#include <string>
#include <variant>
#include <vector>

#include "basic.h"
#include "syntax.h"

#include "symbols.h"

namespace caliburn
{
	namespace cllr
	{
		struct Assembler;
	}

	struct ParsedType;
	struct Value;

	struct GenericArguments;
	struct GenericSignature;
	
	enum class GenericSymType
	{
		TYPE,
		CONST
	};

	static const std::map<std::string_view, GenericSymType> GENERIC_SYMBOL_TYPES = {
		{ "type", GenericSymType::TYPE },
		{ "const", GenericSymType::CONST }
	};

	static const std::map<GenericSymType, std::string_view> GENERIC_TYPE_NAMES = {
		{ GenericSymType::TYPE, "type" },
		{ GenericSymType::CONST, "const" }
	};

	using GenericResult = std::variant<std::monostate, sptr<ParsedType>, sptr<Value>>;

	std::string parseGeneric(in<GenericResult> result);

	struct GenericArguments : ParsedObject
	{
		Token first;
		Token last;
		std::vector<GenericResult> args;

		GenericArguments() = default;
		GenericArguments(in<std::vector<GenericResult>> gArgs) : args(gArgs) {}

		virtual ~GenericArguments() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		void prettyPrint(out<std::stringstream> ss) const override;

		bool empty() const
		{
			return args.empty();
		}

		sptr<ParsedType> getType(size_t index)
		{
			MATCH(args.at(index), sptr<ParsedType>, t)
			{
				return *t;
			}

			return nullptr;
		}

		sptr<Value> getConst(size_t index)
		{
			MATCH(args.at(index), sptr<Value>, v)
			{
				return *v;
			}

			return nullptr;
		}

		void apply(in<GenericSignature> sig, sptr<SymbolTable> table, out<cllr::Assembler> codeAsm) const;

	};

	struct GenericArgHash
	{
		size_t operator()(sptr<GenericArguments> args) const;
	};

	struct GenericName
	{
		const GenericSymType type;
		const std::string name;

		const GenericResult defaultResult;

		GenericName(in<Token> t, in<Token> n, in<GenericResult> res = GenericResult()) :
			type(GENERIC_SYMBOL_TYPES.find(t.str)->second), name(n.str), defaultResult(res)
		{}

		GenericName(GenericSymType t, std::string n, in<GenericResult> res = GenericResult()) :
			type(t), name(n), defaultResult(res)
		{}

		virtual ~GenericName() = default;

	};

	struct GenericSignature : ParsedObject
	{
		Token first;
		Token last;
		std::vector<GenericName> names;
		
	private:
		uint32_t minArgs = 0;

	public:
		GenericSignature() : GenericSignature(std::vector<GenericName>()) {}
		GenericSignature(std::initializer_list<GenericName> names) : GenericSignature(std::vector<GenericName>(names)) {}
		GenericSignature(in<std::vector<GenericName>> names) : names(names)
		{
			uint32_t min = 0;

			for (auto const& name : names)
			{
				if (std::holds_alternative<std::monostate>(name.defaultResult))
				{
					break;
				}

				++min;

			}

			minArgs = min;

		}

		virtual ~GenericSignature() = default;

		Token firstTkn() const noexcept override
		{
			return first;
		}

		Token lastTkn() const noexcept override
		{
			return last;
		}

		bool empty() const
		{
			return names.empty();
		}

		uint32_t getMinArgs() const
		{
			return minArgs;
		}

		uint32_t getMaxArgs() const
		{
			return (uint32_t)names.size();
		}

		sptr<GenericArguments> makeDefaultArgs() const
		{
			std::vector<GenericResult> defArgs;

			defArgs.reserve(names.size());

			for (auto const& name : names)
			{
				if (std::holds_alternative<std::monostate>(name.defaultResult))
				{
					//WHAT???
					defArgs.shrink_to_fit();
					break;
				}

				defArgs.push_back(name.defaultResult);

			}

			return new_sptr<GenericArguments>(defArgs);
		}

		//void set(uint32_t index, )

		void prettyPrint(out<std::stringstream> ss) const override;

		//TODO consider more explicit rejection reasons (for error handling)
		bool canApply(in<GenericArguments> args) const;

	};

	template<typename T>
	using GenArgMap = HashMap<sptr<GenericArguments>, sptr<T>, GenericArgHash>;

}
