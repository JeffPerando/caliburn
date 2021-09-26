
#pragma once

//#include <llvm/IR/IRBuilder.h>
#include <stdint.h>

#include "spirvasm.h"
#include "langcore.h"

namespace caliburn
{
	constexpr auto CALIBURN_RETURN_VAR("__retval__");
	
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

	/*
	* A statement is the base class for the abstract syntax tree.
	*/
	struct Statement
	{
	public:
		StatementType const type;

		Statement(StatementType stmntType) : type(stmntType) {}
		virtual ~Statement() {}

		//virtual bool isCompileTimeConst() = 0;

		//virtual void SPIRVTypeRegister(SpirVAssembler* codeAsm) = 0;

		//virtual void SPIRVTypeEval(SpirVAssembler* codeAsm) = 0;

		/*
		Used for adding the assembly code to the SPIR-V file in progress.

		Returns the SSA that other instructions will use to point to this statement, or 0 if one wasn't made.
		*/
		virtual uint32_t SPIRVEmit(SpirVAssembler* codeAsm) = 0;

	};

	struct ContextStmnt : public Statement
	{
		ContextStmnt* parent = nullptr;
		ContextStmnt(StatementType t) : Statement(t) {}
		virtual ~ContextStmnt() { Statement::~Statement(); }
		/*
		virtual uint32_t addField(std::string name, Visibility vis) = 0;

		virtual uint32_t resolveType(std::string id) = 0;

		virtual uint32_t resolveName(std::string id) = 0;
		*/
	};

}
