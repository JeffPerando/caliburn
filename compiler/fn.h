
#pragma once

#include <map>

#include "basic.h"
#include "cllrasm.h"
#include "langcore.h"

namespace caliburn
{
	struct Function
	{
		StorageModifiers mods = {};

		Function() {}
		virtual ~Function() {}




	};

	struct FunctionImpl : public cllr::Emitter
	{

	};

}
