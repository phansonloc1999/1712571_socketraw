#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
    if (argc <= 1)
    {
        std::cout << "Usage: ./1712571 NETWORK_IP/SUBNET_BITS" << std::endl;
        return -1;
    }

    std::string argument(argv[1]);

    std::size_t found = argument.find("/");
    std::string network_ip;
    int net_bits;
    if (found != std::string::npos)
    {
        network_ip = argument.substr(0, found);
        net_bits = stoi(argument.substr(found + 1, argument.length()));
    }
    else std::cerr << "[ERROR]: Could not find '/' in " << argument << std::endl;

    return 0;
}
