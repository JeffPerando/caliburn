
#include "strhelp.h"

using namespace caliburn;

TextDoc::TextDoc(in<std::string> str) : text(str) 
{
	size_t pos = 0;

	for (size_t off = 0; off < text.size(); ++off)
	{
		if (text[off] != '\n')
		{
			continue;
		}

		size_t len = (off - pos);
		/*
		std::cout << "Pos: " << pos << '\n';
		std::cout << "Off: " << off << '\n';
		std::cout << "Len: " << len << '\n';
		std::cout << "----\n";
		*/
		lines.push_back(text.substr(pos, len));

		pos = off + 1;

	}

	lines.push_back(text.substr(pos));

}

std::string_view TextDoc::getLine(TextPos pos) const
{
	return lines[pos.line];
}
