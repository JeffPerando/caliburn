
#include <iostream>

#include "tokenizer.h"

int main()
{
    std::string test = "5";
    auto tokens = std::vector<caliburn::Token>();

    caliburn::tokenize(test, tokens);

    for (auto t : tokens)
    {
        std::cout << (int64_t)t.type << std::endl;

    }
    
}
