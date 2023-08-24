
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#define CBRN_NO_IMPORT
#include "caliburn.h"

#include "tokenizer.h"
#include "parser.h"

int main()
{
	/*
	caliburn::CompilerSettings cs;

	cs.o = caliburn::OptimizeLevel::DEBUG;
	cs.vLvl = caliburn::ValidationLevel::FULL;

	caliburn::Compiler cc(cs);
	*/
	std::ifstream file("./../../../../expr.txt");

	if (!file.is_open())
	{
		return -1;
	}

	std::stringstream buf;
	buf << file.rdbuf();
	std::string src = buf.str();
	file.close();

	auto t = caliburn::Tokenizer(src);
	auto tkns = t.tokenize();

	for (auto const& t : tkns)
	{
		std::cout << t->str << ' ';
	}

	std::cout << '\n';

	auto p = caliburn::Parser(tkns);
	std::chrono::high_resolution_clock clock{};

	auto startTime = clock.now();
	auto v = p.parseExpr();
	auto time = clock.now() - startTime;

	std::cout << (v == nullptr ? "NULL" : v->prettyStr()) << '\n';
	std::cout << "Time taken: " << (time.count() * 0.000001f) << " ms\n";

	/*
	std::cout << "Compiling:\n";
	std::cout << src << '\n';

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
	*/
	return 0;
}
