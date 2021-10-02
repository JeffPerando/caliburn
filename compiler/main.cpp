
//#include <chrono>
//#include <fstream>
#include <iostream>

//#include "tokenizer.h"
/*
void read(std::string fileName, std::string& data)
{
    std::ifstream file;
    
    file.open(fileName);

    if (file.is_open())
    {
        std::ostringstream buf;
        buf << file.rdbuf();
        data = buf.str();
        file.close();

    }
    

}
*/
int main()
{
    /*
    std::string test;
    auto tokens = std::vector<caliburn::Token>();

    read("../shader.glsl", test);
    
    auto tests = 10;
    long time = 0;

    for (int i = 0; i < tests; ++i)
    {
        tokens.clear();

        auto a = std::chrono::steady_clock::now();

        caliburn::tokenize(test, tokens);

        auto b = std::chrono::steady_clock::now();
        
        time += std::chrono::duration_cast<std::chrono::microseconds>(b - a).count();

    }

    std::cout << "Token count: " << tokens.size() << std::endl;
    std::cout << "Time in microseconds:" << (time / tests) << std::endl;

    return 0;
    */
    /*
    
    for (auto t : tokens)
    {
        std::cout << (int64_t)t.type << std::endl;

    }
    */
}
