# Echter Wächter CPP Modules Discord Bot

Hi! :) The repo allows us to build a highly modular Discord Bot, where we can decide what we want to include and the components are not dependent on each other.

```bash
git clone git@github.com:EgyipTomi425/EchterWachter.git --recurse-submodules 
```

## Architecture

In the config file, we can disable modules that we don't need. The architecture follows a tree of arbitrary depth, and we can choose which leaves or entire subtrees are needed, these are collected in `plugins.cppm` and defined as CMake libraries.

In the first translation unit, the Discord Bot is compiled, which the components can import and use as an interface. It is important that this does not mean multiple imports, everyone sees the same copy. Then the selected modules are compiled and arranged in a tree.

The suggested documentation for implementing a new feature is here:

https://dpp.dev/

## Using your own plugins + Contribution

For example, if you want to implement your own bot, you can do it by adding a subdirectory, importing the main bot as an interface and including it in the plugins, and you're done. If you want multiple, independent libraries, you can do it in the target link libraries, just pay attention to the compilation order.

If you want to share your creation with us, you can do it on the GitHub forum, and it will even be useful for others. This is not mandatory.

### Considerations

We want to strengthen open source software, where if everyone adds value, they can use everyone else's for free, so everyone benefits.

Many people complain about the C++ language because of pointers, headers, memory safety, macros and complexity. With the code, we want to provide not just a software, but a template for modern C++, maintainable, truly independently developed codes without headers, macros and raw pointers.

## Install

Support for C++20 modules is still very variable even after 5 years, with various features missing between major compilers. GCC does not support certain declarations, so we recommend the following as minimum versions:
- Clang 22+
- CMake 4+
- Ninja Build 1.12+

```bash
sudo apt update
sudo apt install -y git cmake ninja-build build-essential python3     libncurses5-dev libxml2-dev zlib1g-dev libedit-dev swig clang-tools clang gcc g++
sudo apt upgrade -y
```

If you want to build it, you should have minimum 32 GB of RAM. If your versions are fine, this is not reccomend:

```bash
git clone --branch llvmorg-22-init https://github.com/llvm/llvm-project
cd llvm-project
git tag
mkdir build
cd build
cmake -G Ninja ../llvm \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" \
    -DCMAKE_INSTALL_PREFIX=/opt/llvm
ninja
sudo ninja install
du -sh /opt
clang++ --version

```

If you use CLion or QT Creator, link the new compiler.
