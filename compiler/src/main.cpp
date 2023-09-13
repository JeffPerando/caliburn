
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#define CBRN_NO_IMPORT
#include "caliburn.h"

#include "tokenizer.h"
#include "parser.h"

static uint64_t totalTime = 0;

int testShaderCompile()
{
	caliburn::CompilerSettings cs;

	cs.o = caliburn::OptimizeLevel::DEBUG;
	cs.vLvl = caliburn::ValidationLevel::BASIC;
	cs.coloredErrors = true;
	cs.dynTypes.emplace("FP", "float16");

	std::ifstream file("./../../../../shader.cbrn");

	if (!file.is_open())
	{
		return -1;
	}

	std::stringstream buf;
	buf << file.rdbuf();
	std::string src = buf.str();
	file.close();

	caliburn::Compiler cc(cs);

	std::chrono::high_resolution_clock clock{};

	auto startTime = clock.now();
	auto shaders = cc.compileSrcShaders(src, "TestShader");
	auto time = clock.now() - startTime;

	auto errors = cc.getErrors();

	if (!errors.empty())
	{
		std::cout << "Errors found: \n";
		for (auto const& e : errors)
			std::cout << e << '\n';

		return -1;
	}

	std::cout << "Successfully compiled " << shaders.size() << " shaders!\n";
	std::cout << "Time taken: " << (time.count() * 0.000001f) << " ms\n";

	totalTime += time.count();

	for (auto const& s : shaders)
	{
		auto name = (std::stringstream() << "./../../../../shader" << ((uint32_t)s->type) << ".spv").str();

		std::ofstream out(name, std::ios::out | std::ios::binary | std::ios::trunc);

		if (!out.is_open())
		{
			continue;
		}

		out.write((char*)&s->code->at(0), s->code->size() * 4);
		out.close();

	}

	return 0;
}

int testExprParsing()
{
	std::ifstream file("./../../../../expr.txt");

	if (!file.is_open())
	{
		return -1;
	}

	std::stringstream buf;
	buf << file.rdbuf();
	std::string src = buf.str();
	file.close();

	auto doc = new_sptr<caliburn::TextDoc>(src);
	auto cs = new_sptr<caliburn::CompilerSettings>();

	auto t = caliburn::Tokenizer(doc);
	auto tkns = t.tokenize();

	for (auto const& t : tkns)
	{
		std::cout << t->str << ' ';
	}

	std::cout << '\n';

	auto p = caliburn::Parser(cs, tkns);
	std::chrono::high_resolution_clock clock{};

	auto startTime = clock.now();
	auto v = p.parseExpr();
	auto time = clock.now() - startTime;

	std::cout << (v == nullptr ? "NULL" : v->prettyStr()) << '\n';
	std::cout << "Time taken: " << (time.count() * 0.000001f) << " ms\n";

	totalTime += time.count();

	return 0;
}

int main()
{
	/*
	std::cout << "\033[1;31m";
	std::cout << sizeof(sptr<caliburn::Token>);
	std::cout << "\033[0m\n";
	*/
	//testShaderCompile();
	
	auto const takes = 20;

	for (int i = 0; i < takes; ++i)
	{
		testShaderCompile();
	}

	std::cout << "Average time: " << ((totalTime / static_cast<double>(takes)) * 0.000001) << " ms\n";
	
	return 0;
}
