
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
		PASS
	};

	struct VariableStmt
	{
		ParsedType* type = nullptr;
		uint32_t offset = 0;

	};

	struct Value
	{
		const ValueType type;

		Value(ValueType vt) : type(vt) {}
		virtual ~Value() {}

		virtual bool isLValue() = 0;

		virtual ConcreteType* getType() = 0;

	};

	/*
	* A statement is the base class for the abstract syntax tree.
	* 
	* More specifically, a statement also acts as a scope. All scopes contain
	* variables, a return, type aliases, and inner code
	*/
	struct Statement
	{
		const StatementType type;
		const Statement* parent;

		std::map<std::string, VariableStmt> vars;
		std::vector<Statement*> innerCode;
		ReturnMode mode = ReturnMode::NONE;
		Value* retValue = nullptr;

		Statement(StatementType stmtType) : type(stmtType), parent(nullptr) {}
		Statement(StatementType stmtType, Statement* p) : type(stmtType), parent(p) {}
		virtual ~Statement() {}

		/*
		virtual bool isCompileTimeConst() = 0;

		virtual void registerSymbols(CaliburnAssembler* codeAsm, SymbolTable* syms) {}

		virtual void typeEval(CaliburnAssembler* codeAsm, SymbolTable* syms) {}

		virtual void addStorageMod(StorageModifier mod) {}

		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm, SymbolTable* syms) = 0;
		*/

	};

}
