
#include "caliburn.h"
#include "parser.h"
#include "tokenizer.h"

using namespace caliburn;

Compiler* Compiler::o(uint32_t opLvl)
{
	optimizeLvl = opLvl;
	return this;
}

Compiler* Compiler::setDynamicType(std::string inner, std::string concrete)
{
	dynTypes.insert(inner, concrete);
	return this;
}

std::vector<uint32_t>* Compiler::parseSrc(std::string text)
{
	auto tokens = std::vector<Token>();
	auto ast = std::vector<Statement*>();

	tokenize(text, tokens);

	auto p = Parser();

	p.parse(&tokens, &ast);

	//TODO check errors, convert to CBIR

	return nullptr;
}
