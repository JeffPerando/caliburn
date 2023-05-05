
#pragma once

#include <stdint.h>

/*
Caliburn Intermediate Representation (CBIR [pronounced Cyber]):

- High-level bytecode for Caliburn code.
- Format: Instructions with a stated, total number of 'nested' or child instructions.
  - Effectively, variable-width instructions.
  - Having a total length allows for skipping instructions that aren't needed.
  - Example: an ADD function will always have 2 nested instructions, both
    being values to add.
  - Text example: function(return(add(1f, 2f)));
- Represents a source code file.
- Basically desugared source code.
- Distributable.
*/

namespace caliburn
{
	namespace cbir
	{
		enum class Opcode : uint32_t
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

			//Other things
			EXPRESSION,
			LITERAL,
			OP_OVERRIDE
		};

	}

}