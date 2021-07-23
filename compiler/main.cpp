
#include <iostream>
#include <vector>

int main()
{
    std::vector<uint32_t> test;
    test.push_back(42);
    //test.insert(test.end(), 5);
    
    std::string str = "LOLOL";
    size_t copied = ((str.length() & ~0x3) >> 2) + ((str.length() & 0x3) == 0);
    test.resize(test.size() + copied, 0);
    std::memcpy(test.end()._Ptr - copied, str.c_str(), str.length());

    test.push_back(69);
    
    for (auto i : test)
    {
        std::cout << i << " ";
    }

    std::cout << "Hello World!\n";
}
