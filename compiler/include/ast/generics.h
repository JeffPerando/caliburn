
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

	std::string parseGeneric(ref<const GenericResult> result);

	struct GenericArguments : public ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		std::vector<GenericResult> args;

		GenericArguments() {}
		GenericArguments(ref<std::vector<GenericResult>> gArgs) : args(gArgs) {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(ref<std::stringstream> ss) const override;

		bool empty() const
		{
			return args.empty();
		}

		void apply(sptr<GenericSignature> sig, sptr<SymbolTable> symbols) const;

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

		GenericName(sptr<Token> t, sptr<Token> n, ref<GenericResult> res = GenericResult()) :
			typeTkn(t), nameTkn(n), type(GENERIC_SYMBOL_TYPES.find(t->str)->second), name(n->str), defaultResult(res)
		{}
		GenericName(GenericSymType t, std::string n, ref<GenericResult> res = GenericResult()) :
			type(t), name(n), typeTkn(nullptr), nameTkn(nullptr), defaultResult(res)
		{}
		~GenericName() {}

	};

	struct GenericSignature : public ParsedObject
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
		GenericSignature(ref<std::vector<GenericName>> names) : names(names)
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
					defArgs.shrink_to_fit();
					break;
				}

				defArgs.push_back(name.defaultResult);

			}

			defaultArgs = new_sptr<GenericArguments>(defArgs);

			return defaultArgs;
		}

		//void set(uint32_t index, )

		void prettyPrint(ref<std::stringstream> ss) const override;

		//TODO consider more explicit rejection reasons (for error handling)
		bool canApply(ref<GenericArguments> args) const;

	};

	template<typename T>
	using GenFactoryFn = T(sptr<GenericArguments>);

	template<typename T>
	using GenArgMap = HashMap<sptr<GenericArguments>, sptr<T>, GenericArgHash>;

	template<typename T>
	class Generic
	{
	protected:
		const sptr<GenericSignature> sig;
		
		const uptr<GenArgMap<T>> variants = new_uptr<GenArgMap<T>>();

		Generic(sptr<GenericSignature> sig) : sig(sig) {}
		virtual ~Generic() {}

	public:
		virtual sptr<T> makeVariant(sptr<GenericArguments> args);

	};

}
