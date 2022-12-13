
#pragma once

//#include <llvm/IR/IRBuilder.h>
#include <map>
#include <stdint.h>
#include <vector>

#include "langcore.h"
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
		
		SCOPE,
		SETTER,
		FUNC_CALL,

	};

	enum class ValueType
	{
		LITERAL,
		EXPRESSION,
		VARIABLE,
		FUNCTION_CALL
	};

	enum class ReturnMode : uint64_t
	{
		NONE,
		RETURN,
		CONTINUE,
		BREAK,
		PASS,
		UNREACHABLE
	};

	struct Value : public ParsedObject, public cllr::Emitter
	{
		const ValueType type;

		Value(ValueType vt) : type(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() = 0;

		virtual ConcreteType* getType() = 0;

	private:
		virtual void deduceType() = 0;

	};

	/*
	* A statement is the base class for the abstract syntax tree.
	* 
	* More specifically, a statement also acts as a scope. All scopes contain
	* variables, a return, type aliases, and inner code
	*/
	struct Statement : public ParsedObject, public cllr::Emitter
	{
		const StatementType type;
		const Statement* parent;

		StorageModifiers mods = {};
		std::map<std::string, ConcreteType*> typeAliases;
		std::vector<Statement*> innerCode;
		ReturnMode retMode = ReturnMode::NONE;
		Value* retValue = nullptr;

		Statement(StatementType stmtType, Statement* p) : type(stmtType), parent(p) {}
		virtual ~Statement() {}

		virtual bool isCompileTimeConst() const
		{
			return false;
		}

		/*
		virtual void registerSymbols(CaliburnAssembler* codeAsm, SymbolTable* syms) {}

		virtual void typeEval(CaliburnAssembler* codeAsm, SymbolTable* syms) {}

		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms) = 0;
		*/

	};

}
