
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

#define CBRN_NO_IMPORT
#include "caliburn.h"

#include "tokenizer.h"
#include "parser.h"

static uint64_t totalTime = 0;

static double bestTime = DBL_MAX;
static double worstTime = DBL_MIN;

std::string read(std::string name)
{
	std::ifstream file((std::stringstream() << "./../../../../" << name).str());

	if (!file.is_open())
	{
		return "";
	}

	std::stringstream buf;
	buf << file.rdbuf();
	file.close();

	return buf.str();
}

int testShaderCompile(bool printTime)
{
	std::string src = read("shader.cbrn");
	
	caliburn::CompilerSettings cs;

	cs.o = caliburn::OptimizeLevel::DEBUG;
	cs.vLvl = caliburn::ValidationLevel::FULL;
	cs.dynTypes.emplace("FP", "fp16");

	caliburn::Compiler cc(cs);

	std::chrono::high_resolution_clock clock{};

	auto startTime = clock.now();
	auto result = cc.compileSrcShaders(src, "TestShader");
	auto time = clock.now() - startTime;

	if (!result.errors.empty())
	{
		std::cout << "Errors found: \n";
		for (auto const& e : result.errors)
			std::cout << e << '\n';

		return -1;
	}

	auto const timeTaken = (time.count() * 0.000001);

	if (printTime)
	{
		std::cout << "Successfully compiled " << result.shaders.size() << " shaders!\n";
		std::cout << "Time taken: " << timeTaken << " ms\n";

	}
	
	bestTime = std::min(timeTaken, bestTime);
	worstTime = std::max(timeTaken, worstTime);

	totalTime += time.count();
	
	for (auto const& s : result.shaders)
	{
		auto name = (std::stringstream() << "./../../../../shader" << ((uint32_t)s->type) << ".spv").str();

		std::ofstream out(name, std::ios::out | std::ios::binary | std::ios::trunc);

		if (!out.is_open())
		{
			continue;
		}

		out.write(RCAST<ptr<const char>>(s->code.data()), s->code.size() * 4);
		out.close();

	}
	
	return 0;
}

int testExprParsing(const std::string& textfile)
{
	std::string src = read(textfile);

	auto doc = new_sptr<caliburn::TextDoc>(src);
	auto cs = new_sptr<caliburn::CompilerSettings>();

	std::chrono::high_resolution_clock clock{};

	auto startTime = clock.now();
	
	auto t = caliburn::Tokenizer(doc);
	auto tkns = t.tokenize();
	auto p = caliburn::Parser(cs, tkns);
	auto v = p.parseExpr();

	auto time = clock.now() - startTime;

	for (auto const& t : tkns)
	{
		std::cout << t.str << ' ';
	}

	std::cout << '\n';

	std::cout << (v == nullptr ? "NULL" : v->prettyStr()) << '\n';
	std::cout << "Time taken: " << (time.count() * 0.000001f) << " ms\n";

	totalTime += time.count();

	std::vector<std::string> errs;

	p.errors->printout(errs, *doc);

	for (auto& e : errs)
	{
		std::cout << e << '\n';
	}

	return 0;
}

void printTokens(const std::string& textfile)
{
	std::string src = read(textfile);
	auto settings = new_sptr<caliburn::CompilerSettings>();
	auto doc = new_sptr<caliburn::TextDoc>(src);
	auto t = caliburn::Tokenizer(doc);
	auto tkns = t.tokenize();

	std::cout << "Token count: " << tkns.size() << '\n';
	std::cout << "Line count: " << doc->lines.size() << '\n';

	for (auto const& t : tkns)
	{
		std::cout << '\"' << t.str << '\"' << ' ' << t.pos.toStr() << '\n';
	}

	std::cout << "Original text:\n";
	
	for (size_t line = 0; line < doc->lines.size(); ++line)
	{
		std::cout << (line + 1) << '|' << doc->lines[line] << '\n';
	}

}

void printLines(const std::string& textfile)
{
	auto txt = read(textfile);
	caliburn::TextDoc doc(txt);

	for (size_t i = 0; i < doc.lines.size(); ++i)
	{
		std::cout << "#" << (i + 1) << ": \"" << doc.lines[i] << "\"\n";
	}

}

void benchmarkShaderCompile()
{
	auto const takes = 2000;

	for (int i = 0; i < takes; ++i)
	{
		if (testShaderCompile(false) != 0)
		{
			break;
		}
	}

	std::cout << "Average time: " << ((totalTime / static_cast<double>(takes)) * 0.000001) << " ms\n";

	std::cout << "Best time: " << bestTime << " ms\n";
	std::cout << "Worst time: " << worstTime << " ms\n";
	std::cout << "Delta: " << (worstTime - bestTime) << " ms\n";

}

int main()
{
	/*
	std::cout << "\033[1;31m";
	std::cout << sizeof(sptr<caliburn::Token>);
	std::cout << "\033[0m\n";
	*/
	return testShaderCompile(true);
	//return testExprParsing("expr.txt");
	//printLines("test.txt");
	//benchmarkShaderCompile();
	//printTokens("expr.txt");
	//printTokens("shader.cbrn");

	return 0;
}
