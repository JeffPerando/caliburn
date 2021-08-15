
#include "assembler.h"

using namespace caliburn;

Visibility caliburn::strToVis(std::string str)
{
	if (str == "public")
		return Visibility::PUBLIC;
	if (str == "protected")
		return Visibility::PROTECTED;
	if (str == "private")
		return Visibility::PRIVATE;
	if (str == "shared")
		return Visibility::SHARED;
	if (str == "static")
		return Visibility::STATIC;

	return Visibility::NONE;
}

void SpirVAssembler::push(uint32_t op)
{
	ops.push_back(op);

}

void SpirVAssembler::pushAll(std::initializer_list<SpvOp> args)
{
	ops.insert(ops.end(), args);

}

void SpirVAssembler::pushStr(std::string str)
{
	size_t copied = (str.length() >> 2) + ((str.length() & 0x3) != 0);
	ops.resize(ops.size() + copied, 0);
	std::memcpy(ops.end()._Ptr - copied, str.c_str(), str.length());

}

void SpirVAssembler::pushVarSetter(std::string name, uint32_t value)
{
	if (!currentStack)
	{
		//TODO complain
		return;
	}

	SpirVStack* stack = currentStack;
	SpirVStack* foundStack = nullptr;
	uint32_t bottomLabel = stack->label;

	while (stack)
	{
		uint32_t fieldSSA = stack->fields[name].value;

		if (fieldSSA)
		{
			foundStack = stack;
			break;
		}
		else
		{
			stack = stack->parent;

		}

	}

	if (!foundStack)
	{
		foundStack = currentStack;
	}

	foundStack->fields[name] = SSA(value);
	SpirVPhi* phi = foundStack->fieldPhis[name];

	if (!phi)
	{
		phi = new SpirVPhi();
		foundStack->fieldPhis[name] = phi;

	}

	phi->options.push_back(SpirVPhiOption(bottomLabel, value));

}

//TODO is a label really necessary?
void SpirVAssembler::startScope(uint32_t label)
{
	SpirVStack* scope = new SpirVStack();

	scope->label = label;

	if (currentStack)
	{
		currentStack->child = scope;
		scope->parent = currentStack;
		currentStack = scope;

	}
	else
	{
		currentStack = scope;

	}

}

void SpirVAssembler::endScope()
{
	SpirVStack* stack = currentStack;

	if (!stack)
	{
		//TODO complain
		return;
	}

	if (stack->child)
	{
		//TODO complain
		return;
	}

	stack->parent->child = nullptr;
	currentStack = stack->parent;

	delete stack;

	if (!currentStack)
	{
		return;
	}

	for (auto field : currentStack->fieldPhis)
	{
		SpirVPhi* phi = field.second;

		if (phi->options.size() < 2)
		{
			continue;
		}

		auto options = phi->options;

		auto fieldValue = newAssign();

		pushAll({ spirv::OpPhi(uint32_t(options.size()) * 2), phi->type, fieldValue });

		for (auto option : phi->options)
		{
			pushAll({ option.label, option.ssa });

		}

		pushVarSetter(field.first, fieldValue);

	}

}

uint32_t SpirVAssembler::getVar(std::string name)
{
	if (!currentStack)
	{
		//TODO complain
	}

	SpirVStack* stack = currentStack;

	while (stack)
	{
		uint32_t fieldSSA = stack->fields[name].value;

		if (fieldSSA)
		{
			return fieldSSA;
		}

		stack = stack->parent;

	}

	return 0;
}

CompiledType* SpirVAssembler::resolveType(ParsedType* type)
{
	auto it = defaultTypes.find(type->getBasicName());
	CompiledType* resolved;

	if (it == defaultTypes.end())
	{
		//TODO find custom type
		resolved = nullptr;
		
	}
	else
	{
		resolved = it->second;

	}

	if (!resolved)
	{
		return nullptr;
	}

	if (resolved->hasA(TypeAttrib::GENERIC))
	{
		if (type->generics.size() < ((SpecializedType*)resolved)->getMandatoryGenericCount())
		{
			//TODO complain
			return nullptr;
		}

		if (type->getFullName() == resolved->getFullName())
		{
			return resolved;
		}

		//gonna make a new generic type
		resolved = resolved->clone();

		for (auto i = 0; i < type->generics.size(); ++i)
		{
			CompiledType* compiledGeneric = resolveType(type->generics[i]);
			resolved->setGeneric(i, compiledGeneric);

		}

	}
	else if (type->generics.size() > 0)
	{
		//TODO complain about inappropriate generic
		return nullptr;
	}
	
	return resolved;
}
/*
uint32_t* SpirVAssembler::writeFile()
{
	ops.push_back(spirv::MagicNumber());
	ops.push_back(spirv::Version(1, 0));
	ops.push_back(CALIBURN_MAGIC_NUMBER);
	ops.push_back(0);//TODO REMEMBER TO SET THIS TO THE BOUNDS
	ops.push_back(0);

	return nullptr;
}
*/