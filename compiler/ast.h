
#pragma once

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include "basic.h"
#include "langcore.h"
#include "module.h"
#include "symbols.h"
#include "syntax.h"
#include "type.h"

namespace caliburn
{
	struct Statement;

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

	enum class ReturnMode : uint64_t
	{
		NONE,
		RETURN,
		CONTINUE,
		BREAK,
		PASS,
		UNREACHABLE,
		DISCARD
	};

	enum class ValidationStatus
	{
		VALID,
		INVALID_STATEMENT,
		INVALID_RET_MODE
	};

	struct ValidationData
	{
		ValidationStatus status;
		//ptr<const Statement> stmt;
		ReturnMode retMode;
		/*
		static ValidationData valid()
		{
			return ValidationData{ ValidationStatus::VALID, nullptr, ReturnMode::NONE };
		}

		static ValidationData badStmt(ptr<const Statement> stmt)
		{
			return ValidationData{ ValidationStatus::INVALID_STATEMENT, stmt, ReturnMode::NONE };
		}

		static ValidationData badRetMode(ptr<const Statement> stmt, ReturnMode mode)
		{
			return ValidationData{ ValidationStatus::INVALID_RET_MODE, stmt, mode };
		}
		*/
	};

	constexpr auto TOP_STMT_TYPES = {
		StatementType::VARIABLE,
		StatementType::IF, //conditional compilation
		StatementType::MODULE, StatementType::IMPORT,
		StatementType::FUNCTION,
		StatementType::SHADER,
		StatementType::STRUCT, StatementType::RECORD, StatementType::CLASS,
		StatementType::ENUM,
	};

	constexpr auto LOGIC_STMT_TYPES = {
		StatementType::VARIABLE,
		StatementType::IF,
		StatementType::FOR, StatementType::FORALL,
		StatementType::WHILE, StatementType::DOWHILE,
		//Switch is not supported yet in any way; must revisit the concept
		StatementType::SETTER, StatementType::FUNC_CALL,
	};

	constexpr auto CLASS_STMT_TYPES = {
		StatementType::VARIABLE,
		StatementType::IF, //conditional compilation
		StatementType::METHOD,
		StatementType::CONSTRUCTOR, StatementType::DESTRUCTOR,
	};

	constexpr auto STRUCT_STMT_TYPES = {
		StatementType::VARIABLE,
		StatementType::IF, //conditional compilation
		StatementType::CONSTRUCTOR, StatementType::DESTRUCTOR,
	};

	struct Statement : public Module, public ParsedObject
	{
		const StatementType type;

		StorageModifiers mods = {};
		std::map<std::string, uptr<ParsedType>> typeAliases;

		Statement(StatementType stmtType) : type(stmtType) {}
		virtual ~Statement() {}

		virtual bool validateModule() const override
		{
			return false;
		}

		virtual sptr<Token> firstTkn() const override = 0;

		virtual sptr<Token> lastTkn() const override = 0;

		//Only used by top-level statements which declare symbols. The rest, like variables, should use declareSymbols() instead
		virtual void declareHeader(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) {}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) = 0;

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override = 0;

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override = 0;

	};

	struct ScopeStatement : public Statement
	{
		sptr<Token> first = nullptr;
		sptr<Token> last = nullptr;

		std::vector<uptr<Statement>> stmts;
		
		sptr<SymbolTable> scopeTable = nullptr;

		ReturnMode retMode = ReturnMode::NONE;
		uptr<Value> retValue = nullptr;

		ScopeStatement(StatementType stmtType = StatementType::SCOPE) : Statement(stmtType) {}
		virtual ~ScopeStatement() {}

		virtual sptr<Token> firstTkn() const override
		{
			return first;
		}

		virtual sptr<Token> lastTkn() const override
		{
			return last;
		}

		virtual void declareSymbols(sptr<SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			if (scopeTable != nullptr)
			{
				return;
			}

			scopeTable = std::make_shared<SymbolTable>(table);

			for (auto const &stmt : stmts)
			{
				stmt->declareSymbols(scopeTable, codeAsm);

			}

		}

		virtual void resolveSymbols(sptr<const SymbolTable> table, ref<cllr::Assembler> codeAsm) override
		{
			for (auto const& stmt : stmts)
			{
				stmt->resolveSymbols(scopeTable, codeAsm);

			}

		}

		virtual void emitDeclCLLR(ref<cllr::Assembler> codeAsm) override
		{
			for (auto const& inner : stmts)
			{
				inner->emitDeclCLLR(codeAsm);
			}

			switch (retMode)
			{
			case ReturnMode::NONE:
			case ReturnMode::UNREACHABLE: break;
			case ReturnMode::RETURN: {
				if (retValue != nullptr)
				{
					auto retID = retValue->emitValueCLLR(codeAsm);

					codeAsm.push(0, cllr::Opcode::RETURN_VALUE, {}, { retID });

				}
				else
				{
					codeAsm.push(0, cllr::Opcode::RETURN, {}, {});

				}
				break;
			};
			case ReturnMode::CONTINUE:
				codeAsm.push(0, cllr::Opcode::JUMP, {}, { codeAsm.getLoopStart() }); break;
			case ReturnMode::BREAK:
				codeAsm.push(0, cllr::Opcode::JUMP, {}, { codeAsm.getLoopEnd() }); break;
			case ReturnMode::PASS:
				//TODO implement
				break;
			case ReturnMode::DISCARD:
				codeAsm.push(0, cllr::Opcode::DISCARD, {}, {}); break;
			}

		}

	};

	struct GenericStatement : public Statement
	{
		std::map<std::string, sptr<Type>> cNames;
		std::map<std::string, sptr<Value>> tNames;

		GenericStatement(StatementType stmtType) : Statement(stmtType) {}
		virtual ~GenericStatement() {}

	};

}
