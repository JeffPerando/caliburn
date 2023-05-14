
#pragma once

#include <string>
#include <variant>
#include <vector>

#include "basic.h"
#include "syntax.h"

namespace caliburn
{
	struct ParsedType;
	struct Value;
	
	using GenericResult = std::variant<std::monostate, uptr<ParsedType>, uptr<Value>>;
	
	struct GenericArguments : public ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		std::vector<GenericResult> args;

		virtual sptr<Token> firstTkn() const override
		{
			return first;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return last;
		}
		
	};

	struct GenericName
	{
		sptr<Token> type = nullptr;
		sptr<Token> name = nullptr;
		GenericResult defaultResult;

	};

	struct GenericSignature : public ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;
		size_t minArgs = 0;
		std::vector<GenericName> names;
		bool isValid = false;

		virtual sptr<Token> firstTkn() const override
		{
			return first;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return last;
		}

		//TODO consider more explicit rejection reasons (for error handling)
		bool canApply(ref<GenericArguments> args);

	};

	std::string parseGeneric(ref<const GenericResult> result);

}
