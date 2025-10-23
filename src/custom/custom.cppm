module;

#include <iostream>

export module custom;

// export import YOUR_MODULE 1;
// export import YOUR_MODULE 2;
// etc.

// You can use config.cmake variables and #ifdef to make it optional

#ifdef TEMPLATE
export import template1;
#endif

// If you want like python __init__ behavior, use INLINE LAMBDA or INLINE functions here
// Use Check the template.cppm in the submodule
export inline const int init_custom = []
{
    std::cout << "[Custom] modules has been initialized..." << std::endl;
    return 42;
}();