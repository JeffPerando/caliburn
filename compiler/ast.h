
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
		TYPEDEF,
		SHADER,
		FUNCTION,
		STRUCT,
		CLASS,
		DESCRIPTOR,
		INPUT,
		
		//Flow control
		IF,
		FOR,
		FORALL,
		WHILE,
		DOWHILE,
		SWITCH,
		CASE,
		
		RETURN,
		CONTINUE,
		BREAK,
		PASS,

		//misc. logic
		VARIABLE,
		SETTER,
		FUNC_CALL,
		SCOPE,

		//Other things
		MISC_VALUE,
		MATH_EXPRESSION,
		LITERAL,
		OP_OVERRIDE

	};

	enum class ValueType
	{
		CONSTANT,
		CALCULATED,
		VARIABLE
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
		Value() {}
		virtual ~Value() {}

		virtual ConcreteType* getType() = 0;

		virtual bool isConstant() = 0;

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

		std::map<std::string, VariableStmt> vars;
		std::vector<Statement*> innerCode;
		ReturnMode mode = ReturnMode::NONE;
		Value* retValue = nullptr;

		Statement(StatementType stmntType) : type(stmntType) {}
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
