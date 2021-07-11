
#pragma once

#include "statement.h"

namespace caliburn
{
	struct FieldStatement : public Statement
	{
		Visibility vis = Visibility::PUBLIC;
		std::string name = "";
		std::string type = "NONE";
		uint32_t reads = 0;
		uint32_t writes = 0;
		FieldStatement() : Statement(0) {}

	};

}
