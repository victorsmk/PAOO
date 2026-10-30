#include <l1/demos.hpp>

#include <string>

int main(int argc, char* argv[])
{
    const bool crash = (argc > 1 && std::string(argv[1]) == "double-free");

    paoo::demoEncapsulation();
    paoo::demoInitializerList();
    paoo::demoDestructor();
    paoo::demoCopyConstructor(crash);
    paoo::demoMoveConstructor();

    return 0;
}