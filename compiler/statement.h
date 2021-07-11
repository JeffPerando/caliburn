
#pragma once

//#include <llvm/IR/IRBuilder.h>
#include <stdint.h>

#include "assembler.h"
#include "deftypes.h"
#include "spirv.h"

namespace caliburn
{
	/*
	* A statement is the base class for the abstract syntax tree.
	*/
	struct Statement
	{
	protected:
		bool registered = false;
		uint32_t ssa = 0;
	public:
		int32_t const type;
		Statement(int32_t stmntType) : type(stmntType) {}
		~Statement() {}

		/*
		Used for adding the assembly code to the SPIR-V file in progress.

		Returns the assignment that other instructions will use to point to this statement. 0 is not valid.
		*/
		virtual uint32_t toSPIRV(SpirVAssembler* codeAsm) = 0;

	};

	struct ContextStmnt : public Statement
	{
		ContextStmnt* parent = nullptr;
		ContextStmnt(int32_t id) : Statement(id) {}
		~ContextStmnt() { Statement::~Statement(); }

		virtual uint32_t addField(std::string name, Visibility vis) = 0;

		virtual uint32_t resolveType(std::string id) = 0;

		virtual uint32_t resolveName(std::string id) = 0;

	};

}
