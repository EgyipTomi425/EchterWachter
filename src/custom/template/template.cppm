module;

#include <iostream>

export module template1;

// It does not have syntactic meaning to name them with "."
// Modul names just have to be unique
export import template1.test;

// If you want like python __init__ behavior, use INLINE LAMBDA or INLINE functions here
// Check the YouTube module if you want a command register example
export inline const int init_template = []
{
    std::cout << templ::add() << std::endl;
    return 42;
}();