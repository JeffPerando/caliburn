
#include "caliburn.h"
#include "parser.h"
#include "tokenizer.h"

using namespace caliburn;

Compiler* Compiler::o(OptimizeLevel lvl)
{
	optimizeLvl = lvl;
	return this;
}

Compiler* Compiler::setDynamicType(std::string inner, std::string concrete)
{
	dynTypes[inner] = concrete;
	return this;
}

std::vector<uint32_t>* Compiler::parseSrc(std::string text)
{
	auto tokens = std::vector<Token>();
	auto ast = std::vector<Statement*>();
	
	auto t = Tokenizer(text);
	t.tokenize(tokens);

	auto p = Parser();
	p.parse(&tokens, &ast);

	//TODO check errors, convert to CBIR

	return nullptr;
}

bool Compiler::compile(std::vector<uint32_t>* cbir, std::string shaderName, std::vector<Shader>& shaderDest)
{
	return false;
}