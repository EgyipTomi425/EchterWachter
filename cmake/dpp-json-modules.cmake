# Backport nlohmann/json's C++20 module fix to D++'s bundled header.
# https://github.com/nlohmann/json/pull/4764
# Keep the fetched checkout intact; all D++ consumers use the same overlay.
set(json_header "${dpp_SOURCE_DIR}/include/dpp/nlohmann/json.hpp")
file(READ "${json_header}" json_original)
string(REPLACE "static void unescape(StringType& s)"
               "inline void unescape(StringType& s)" json_patched "${json_original}")
string(REPLACE "static inline bool little_endianness(int num = 1) noexcept"
               "inline bool little_endianness(int num = 1) noexcept"
               json_patched "${json_patched}")
set(json_patched "#include <string_view>\n${json_patched}")
set(json_overlay "${CMAKE_CURRENT_BINARY_DIR}/compat/dpp/nlohmann/json.hpp")
set(json_existing "")
if(EXISTS "${json_overlay}")
    file(READ "${json_overlay}" json_existing)
endif()
if(NOT json_existing STREQUAL json_patched)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/compat/dpp/nlohmann")
    file(WRITE "${json_overlay}" "${json_patched}")
endif()
target_include_directories(dpp SYSTEM BEFORE PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/compat>")
