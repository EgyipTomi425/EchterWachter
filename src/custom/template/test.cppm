module;

#include <string>

export module template1.test;

// Here is the declarations of the functions
// You can implement them here too, but in larger project is not recommend.
// Using namespaces are not necessary

// If you make the functions inline, it will be the same value in every import
// Do not use it if that is not a recommendation
// If you use inline functions, most of the time have to implement here and pay attention the order
// If the function is inline nad can get value in the compile stage, use constexpr
export namespace templ
{
    std::string add();
}