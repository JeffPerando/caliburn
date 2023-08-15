
#pragma once

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include "basic.h"
#include "error.h"
#include "langcore.h"
#include "syntax.h"

#include "ast/module.h"
#include "ast/symbols.h"

#include "cllr/cllrasm.h"

#include "types/type.h"

namespace caliburn
{
	enum class StatementType
	{
		UNKNOWN,

		VARIABLE,

		//Flow control
		IF,
		FOR,
		FORALL,
		WHILE,
		DOWHILE,
		SWITCH,
		CASE,

		MODULE,
		IMPORT,

		TYPEDEF,

		SHADER,
		SHADER_STAGE,

		FUNCTION,
		
		STRUCT,
		RECORD,

		CLASS,
		METHOD,
		CONSTRUCTOR,
		DESTRUCTOR,
		
		ENUM,
		ENUM_WRAPPED,

		SCOPE,
		SETTER,
		FUNC_CALL,

	};

	constexpr auto TOP_STMT_TYPES = {
		StatementType::VARIABLE,
		//StatementType::IF, //conditional compilation
		StatementType::MODULE, StatementType::IMPORT,
		StatementType::FUNCTION,
		StatementType::SHADER,
		StatementType::STRUCT, StatementType::RECORD, StatementType::CLASS,
		//StatementType::ENUM,
	};

	struct Annotation : ParsedObject
	{
		sptr<Token> first = nullptr;
		sptr<Token> name = nullptr;
		sptr<Token> last = nullptr;
		std::vector<sptr<Token>> contents;

		Annotation() = default;
		~Annotation() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << '@' << name->str << '(';

			for (auto const& tkn : contents)
			{
				ss << tkn->str;

				if (tkn != contents.back())
				{
					ss << ' ';
				}

			}

			ss << ')';

		}

	};

	struct Statement : Module, ParsedObject, cllr::Emitter
	{
		const StatementType type;

		StmtModifiers mods = {};
		std::map<std::string, uptr<Annotation>> annotations;

		Statement(StatementType stmtType) : type(stmtType) {}
		virtual ~Statement() {}
		
		ptr<Annotation> getAnnotation(std::string name) const
		{
			auto found = annotations.find(name);

			if (found == annotations.end())
			{
				return nullptr;
			}

			return found->second.get();
		}
		
		virtual bool validate(sptr<ErrorHandler> errors) const
		{
			return false;
		}

		sptr<Token> firstTkn() const override = 0;

		sptr<Token> lastTkn() const override = 0;

		void prettyPrint(ref<std::stringstream> ss) const override {}

		//Only used by top-level statements which declare symbols. The rest, like variables, should use declareSymbols() instead
		virtual void declareHeader(sptr<SymbolTable> table) {}

		void declareSymbols(sptr<SymbolTable> table) override = 0;

	};

	struct ScopeStatement : Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<Statement>> stmts;
		
		sptr<SymbolTable> scopeTable = nullptr;

		ReturnMode retMode = ReturnMode::NONE;
		sptr<Value> retValue = nullptr;

		ScopeStatement(StatementType stmtType = StatementType::SCOPE) : Statement(stmtType) {}
		virtual ~ScopeStatement() {}

		sptr<Token> firstTkn() const override
		{
			return first;
		}

		sptr<Token> lastTkn() const override
		{
			return last;
		}

		void prettyPrint(ref<std::stringstream> ss) const override
		{
			ss << "{\n";

			for (auto const& stmt : stmts)
			{
				stmt->prettyPrint(ss);
				ss << ';\n';
			}

			ss << "}";

		}
		
		void declareSymbols(sptr<SymbolTable> table) override
		{
			if (scopeTable != nullptr)
			{
				return;
			}

			scopeTable = new_sptr<SymbolTable>(table);

			for (auto const& stmt : stmts)
			{
				stmt->declareSymbols(scopeTable);

			}

		}

		cllr::SSA emitDeclCLLR(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			scopeTable->reparent(table);

			for (auto const& inner : stmts)
			{
				inner->emitDeclCLLR(scopeTable, codeAsm);
			}

			switch (retMode)
			{
				case ReturnMode::NONE:
				case ReturnMode::UNREACHABLE: break;
				case ReturnMode::RETURN: {
					if (retValue != nullptr)
					{
						auto ret = retValue->emitValueCLLR(scopeTable, codeAsm);

						codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN_VALUE, {}, { ret.value }));

					}
					else
					{
						codeAsm.push(cllr::Instruction(cllr::Opcode::RETURN));

					}
					break;
				};
				case ReturnMode::CONTINUE:
					codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopStart() })); break;
				case ReturnMode::BREAK:
					codeAsm.push(cllr::Instruction(cllr::Opcode::JUMP, {}, { codeAsm.getLoopEnd() })); break;
				case ReturnMode::PASS:
					//TODO implement
					break;
				case ReturnMode::DISCARD:
					codeAsm.push(cllr::Instruction(cllr::Opcode::DISCARD)); break;
			}

			return 0;
		}

	};

}
