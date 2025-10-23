module;

// You can use this global modul fragment in every file if you need to include classic HPPs
#include <iostream>

module template1.test;

namespace templ
{
    std::string add()
    {
        return "[Template] module has been initialized...";;
    }
}