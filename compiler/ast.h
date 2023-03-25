
#pragma once

//#include <llvm/IR/IRBuilder.h>
#include <map>
#include <stdint.h>
#include <vector>

#include "langcore.h"
#include "symbols.h"
#include "type.h"

namespace caliburn
{
	enum class StatementType
	{
		UNKNOWN,

		ROOT,
		IMPORT,
		FUNCTION,
		SHADER,
		DESCRIPTOR,
		STRUCT,
		CLASS,
		CONSTRUCTOR,
		DESTRUCTOR,
		
		//Flow control
		IF,
		ELSE,
		FOR,
		FORALL,
		WHILE,
		DOWHILE,
		SWITCH,
		CASE,

		VARIABLE,
		
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

	/*
	* A statement is the base class for the abstract syntax tree.
	* 
	* More specifically, a statement also acts as a scope. All scopes contain
	* variables, a return, type aliases, and inner code
	*/
	struct Statement : public cllr::Emitter, public ParsedObject
	{
		StatementType const type;
		Statement* const parent;

		StorageModifiers mods = {};
		std::map<std::string, ConcreteType*> typeAliases;

		Statement(StatementType stmtType, Statement* p) : type(stmtType), parent(p) {}
		Statement(StatementType stmtType) : type(stmtType), parent(nullptr) {}
		virtual ~Statement() {}

		virtual bool isCompileTimeConst() const
		{
			return false;
		}

		virtual void declSymbols(SymbolTable& table) = 0;

		virtual void resolveSymbols(const SymbolTable& table) = 0;

	};

	struct ScopeStatement : public Statement
	{
		Token* first = nullptr;
		Token* last = nullptr;

		std::vector<Statement*> stmts;
		std::vector<Variable*> vars;

		ReturnMode retMode = ReturnMode::NONE;
		Value* retValue = nullptr;

		ScopeStatement(StatementType stmtType, Statement* p) : Statement(stmtType, p) {}
		ScopeStatement(StatementType stmtType) : Statement(stmtType, nullptr) {}
		ScopeStatement(Statement* parent) : Statement(StatementType::SCOPE, parent) {}
		virtual ~ScopeStatement() {}

		virtual Token* firstTkn() const override
		{
			return first;
		}

		virtual Token* lastTkn() const override
		{
			return last;
		}

		virtual void getSSAs(cllr::Assembler& codeAsm) override
		{
			for (auto stmt : stmts)
			{
				stmt->getSSAs(codeAsm);

			}

		}

		virtual void declSymbols(SymbolTable& table) override
		{
			for (auto stmt : stmts)
			{
				stmt->declSymbols(table);

			}

		}

		virtual void resolveSymbols(const SymbolTable& table) override
		{
			for (auto stmt : stmts)
			{
				stmt->resolveSymbols(table);

			}

		}

		virtual void emitDeclCLLR(cllr::Assembler& codeAsm) override
		{
			for (auto inner : stmts)
			{
				inner->emitDeclCLLR(codeAsm);
			}

		}

	};

}
