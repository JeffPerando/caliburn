
#pragma once

#include <map>
#include <set>
#include <stdint.h>
#include <vector>

#include "basic.h"
#include "langcore.h"
#include "module.h"
#include "symbols.h"
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
		std::map<std::string, ParsedType*> typeAliases;

		Statement(StatementType stmtType) : type(stmtType) {}
		virtual ~Statement() {}

		virtual bool validateModule() const override
		{
			return false;
		}

		virtual Token* firstTkn() const override = 0;

		virtual Token* lastTkn() const override = 0;

		//Only used by top-level statements which declare symbols. The rest, like local variables, should use declareSymbols() instead
		virtual void declareHeader(ref<SymbolTable> table, cllr::Assembler& codeAsm) {}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override = 0;

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override = 0;

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) = 0;

	};

	struct ScopeStatement : public Statement
	{
		Token* first = nullptr;
		Token* last = nullptr;

		cllr::SSA id = 0;

		std::vector<Statement*> stmts;
		
		ptr<SymbolTable> scopeTable = nullptr;

		ReturnMode retMode = ReturnMode::NONE;
		Value* retValue = nullptr;

		ScopeStatement(StatementType stmtType = StatementType::SCOPE) : Statement(stmtType) {}
		virtual ~ScopeStatement()
		{
			delete scopeTable;
		}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return last;
		}

		virtual void declareSymbols(ref<SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			if (scopeTable != nullptr)
			{
				return;
			}

			scopeTable = new SymbolTable(table);

			for (auto stmt : stmts)
			{
				stmt->declareSymbols(*scopeTable, codeAsm);

			}

		}

		virtual void resolveSymbols(ref<const SymbolTable> table, cllr::Assembler& codeAsm) override
		{
			for (auto stmt : stmts)
			{
				stmt->resolveSymbols(*scopeTable, codeAsm);

			}

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			codeAsm.push(id, cllr::Opcode::LABEL, {}, {});

			for (auto inner : stmts)
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
		//std::vector<std::pair<std::string, ParsedType*>> 
		std::map<std::string, Type*> tNames;
		std::map<std::string, Value*> cNames;

		GenericStatement(StatementType stmtType) : Statement(stmtType) {}
		virtual ~GenericStatement() {}

	};

}
