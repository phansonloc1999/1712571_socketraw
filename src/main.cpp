#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: ./1712571 IP_NETWORK/NET_BITS" << std::endl;
        return -1;
    }

    std::string arg(argv[1]);

    std::cout << arg << std::endl;

    return 0;
}
