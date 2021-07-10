// compiler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

int main()
{
    std::vector<uint32_t> test;
    test.push_back(42);
    //test.insert(test.end(), 5);
    
    std::string str = "LOLOL";
    size_t copied = (((str.length() & ~0x3) >> 2) + ((str.length() & 0x3) > 0 * 1) + 1);
    test.resize(test.size() + copied, 0);
    std::memcpy(test.end()._Ptr - copied, str.c_str(), str.length());

    test.push_back(69);
    
    for (auto i : test)
    {
        std::cout << i << " ";
    }

    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
