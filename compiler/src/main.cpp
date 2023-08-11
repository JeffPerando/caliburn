
#include <fstream>
#include <iostream>
#include <sstream>

#include "caliburn.h"

#include "spirv/spirv.h"

int main()
{
	caliburn::Compiler cc;
	std::ifstream file("./../../../../shader.cbrn");

	if (!file.is_open())
	{
		return -1;
	}

	std::stringstream buf;
	buf << file.rdbuf();
	std::string src = buf.str();
	file.close();

	std::cout << "Compiling:\n";
	std::cout << src << '\n';

	auto shaders = cc.compileSrcShaders(src, "TestShader");
	auto errors = cc.getErrors();

	if (!errors.empty())
	{
		std::cout << "Errors found: \n";
		for (auto const& e : errors)
			std::cout << e << '\n';

		return -1;
	}

	std::cout << "Successfully compiled " << shaders.size() << " shaders!\n";

	for (auto const& s : shaders)
	{
		auto name = (std::stringstream() << "./../../../../shader" << ((uint32_t)s->type) << ".spv").str();

		std::ofstream out(name, std::ios::out | std::ios::binary | std::ios::trunc);

		if (!out.is_open())
		{
			continue;
		}

		out.write((char*)&s->spirv->at(0), s->spirv->size() * 4);
		out.close();

	}
	
	return 0;
}
