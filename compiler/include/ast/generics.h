
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

	static const std::map<std::string, GenericSymType> GENERIC_SYMBOL_TYPES = {
		{ "type", GenericSymType::TYPE },
		{ "const", GenericSymType::CONST }
	};

	using GenericResult = std::variant<std::monostate, sptr<ParsedType>, sptr<Value>>;

	std::string parseGeneric(in<GenericResult> result);

	struct GenericArguments : ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		std::vector<GenericResult> args;

		GenericArguments() {}
		GenericArguments(in<std::vector<GenericResult>> gArgs) : args(gArgs) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
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
			if (auto t = std::get_if<sptr<ParsedType>>(&args.at(index)))
			{
				return *t;
			}

			return nullptr;
		}

		sptr<Value> getConst(size_t index)
		{
			if (auto v = std::get_if<sptr<Value>>(&args.at(index)))
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

		const sptr<Token> typeTkn;
		const sptr<Token> nameTkn;
		const GenericResult defaultResult;

		GenericName(sptr<Token> t, sptr<Token> n, in<GenericResult> res = GenericResult()) :
			typeTkn(t), nameTkn(n), type(GENERIC_SYMBOL_TYPES.find(t->str)->second), name(n->str), defaultResult(res)
		{}
		GenericName(GenericSymType t, std::string n, in<GenericResult> res = GenericResult()) :
			type(t), name(n), typeTkn(nullptr), nameTkn(nullptr), defaultResult(res)
		{}
		~GenericName() {}

	};

	struct GenericSignature : ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		std::vector<GenericName> names;
		
	private:
		//generated for getDefaultArgs()
		sptr<GenericArguments> defaultArgs = nullptr;

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
		virtual ~GenericSignature() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
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

		sptr<GenericArguments> makeDefaultArgs()
		{
			if (defaultArgs != nullptr)
			{
				return defaultArgs;
			}

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

			defaultArgs = new_sptr<GenericArguments>(defArgs);

			return defaultArgs;
		}

		//void set(uint32_t index, )

		void prettyPrint(out<std::stringstream> ss) const override;

		//TODO consider more explicit rejection reasons (for error handling)
		bool canApply(in<GenericArguments> args) const;

	};

	template<typename T>
	using GenArgMap = HashMap<sptr<GenericArguments>, sptr<T>, GenericArgHash>;

}
