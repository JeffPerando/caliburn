
#include <iostream>

#include "tokenizer.h"

int main()
{
    std::string test = "x | y";
    auto tokens = std::vector<caliburn::Token>();

    caliburn::tokenize(test, tokens);

    for (auto t : tokens)
    {
        std::cout << t.type << std::endl;

    }

}
