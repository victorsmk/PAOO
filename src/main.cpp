#include <l1/demos.hpp>

#include <string>

int main(int argc, char *argv[])
{
    const std::string arg = (argc > 1) ? argv[1] : "";

    paoo::demoEncapsulation();
    paoo::demoInitializerList();
    paoo::demoDestructor();
    paoo::demoCopyConstructor(arg == "double-free");
    paoo::demoMoveConstructor();
    paoo::demoCompilerGeneratedFunctions();
    paoo::demoAssignment(arg == "self-assign");

    return 0;
}