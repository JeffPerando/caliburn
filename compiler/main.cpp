
#include <iostream>

//#include "tokenizer.h"
#include "buffer.h"

int main()
{
    std::vector<int*> xs;
    caliburn::buffer<int*> buf(&xs);
    auto i = buf.next();
    /*
    std::string test = "2DTexture";
    auto tokens = std::vector<caliburn::Token>();

    caliburn::tokenize(test, tokens);

    for (auto t : tokens)
    {
        std::cout << (int64_t)t.type << std::endl;

    }
    */
}
