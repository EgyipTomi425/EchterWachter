# Echter Wächter CPP Modules Discord Bot

Hi! :) The repo allows us to build a highly modular Discord Bot, where we can decide what we want to include and the components are not dependent on each other.

```bash
git clone https://github.com/EgyipTomi425/EchterWachter.git
cd EchterWachter
git -c fetch.recurseSubmodules=false submodule update --init --recursive --remote
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

## Build, CI and service

The production server builds and runs natively on Debian 13. Ubuntu 26.04 is
used only by GitHub Actions; the service does not require a container.

CMake respects `CXX` or `-DCMAKE_CXX_COMPILER`; it does not force a compiler.
D++ and the project submodules follow `master`; build preparation refreshes
them from their remotes. The GitHub Actions pipeline builds all public default modules with GCC and
Clang on Ubuntu 26.04, with `-Wall -Wextra -Wpedantic -Wconversion -Wshadow
-Werror`. CUDA is disabled by default; use `-DECHTERWACHTER_CUDA=ON` only on a
machine with the optional toolkit. Warnings apply to project modules, while
the D++ dependency retains its own build settings.

```sh
CXX=g++ cmake -S . -B build-gcc -G Ninja -DECHTERWACHTER_STRICT=ON
cmake --build build-gcc --parallel 1
ctest --test-dir build-gcc --output-on-failure --no-tests=error

CXX=clang++ cmake -S . -B build-clang -G Ninja -DECHTERWACHTER_STRICT=ON
cmake --build build-clang --parallel 1
ctest --test-dir build-clang --output-on-failure --no-tests=error
```

On the bot server, use `./scripts/build-service.sh` instead. It stops
`echterwachter.service` before configuring/building and starts it only after
the build and tests pass. A failed build leaves the service stopped.
Compiler changes require a separate build directory or a fresh CMake cache.

Successful CI jobs upload a runtime archive for each compiler. Extract it
and run `./run-bot.sh` with `DISCORD_TOKEN` in the environment. Archives target
Ubuntu 26.04 x86_64 and require the system runtime dependencies (libstdc++,
OpenSSL, curl, Opus and zlib). Voice commands also need `ffmpeg` and
`espeak-ng` for the portable speech fallback. Deployment to the running server remains manual;
the server's architecture and distribution may differ from the CI runner.

The gateway watchdog checks heartbeat ACKs every 30 seconds, independently of
command traffic. Missing ACKs for two heartbeat intervals followed by a
90-second grace period trigger D++'s reconnect/resume path. This works around
the seconds/milliseconds mismatch in the D++ version observed during diagnosis. Reconnects do not
register commands again. Logging uses a bounded background queue so a slow
output sink cannot indefinitely grow memory or block gateway logging.

## Command synchronization

The module registry is the source of truth. With `start_bot(true)`, startup uses
Discord's bulk overwrite API once for global commands and once for each joined
guild. Commands removed from the source, disabled modules and obsolete guild
commands are removed too; an empty desired guild list clears old local commands.
The former per-command create calls could only add/update names and therefore
left removed commands behind indefinitely.

Guild and global scopes are separate. A matching guild command takes precedence
in its guild, while DMs and other guilds use the global route. Registering the
same name/type/scope again replaces its callback instead of creating duplicates.
`start_bot(false)` keeps local handlers but leaves the remote command lists alone.
No SDK command group is registered. Gateway resumes do not repeat successful
command synchronization within the same process.

See the [Discord bulk overwrite API](https://docs.discord.com/developers/interactions/application-commands#bulk-overwrite-global-application-commands).
`ctest` checks heartbeat recovery, command scope/routing, removal of registry
entries, and inline module initialization without a live Discord connection.

## Optional private SDK

`src/custom` contains the private `gge_sdk` submodule; the SDK in turn includes
its public `Echter_Websockets` submodule. The SDK uses `update = none` in
`src/custom/.gitmodules`, so ordinary recursive public checkouts skip it
without asking for credentials. CMake never clones it. If either checkout is
missing, the SDK is skipped with a status message. Use `-DGGE_SDK=OFF` to skip
an available SDK explicitly, or `-DCUSTOM=OFF` to disable all custom plugins.

With access to the private repository, initialize it explicitly:

```sh
git -C src/custom submodule update --init --remote --checkout gge_sdk
git -C src/custom/gge_sdk submodule update --init --recursive --remote
```

Install the SDK dependencies documented in its private README before enabling
the available checkout. Embedded SDK tools and tests default to OFF. Linking
and exporting the SDK APIs does not register an SDK Discord command group.
The existing inline module initializers remain part of the plugin architecture.

## Install

Use your distribution's native packages. The Debian 13 deployment is tested
with Clang 19; GitHub Actions tests GCC 15 and Clang 21 on Ubuntu 26.04.
CMake 3.28+ and Ninja 1.11+ are required for named C++ modules.

```sh
sudo apt-get update
sudo apt-get install -y git cmake ninja-build clang clang-tools g++ pkg-config \
    libcurl4-openssl-dev libssl-dev libopus-dev zlib1g-dev ffmpeg espeak-ng
```

On the Debian bot server:

```sh
./scripts/build-service.sh -DECHTERWACHTER_STRICT=ON -DECHTERWACHTER_CUDA=OFF
```

The service helper defaults to `clang++`. Set `CXX` or pass
`-DCMAKE_CXX_COMPILER=...` explicitly to choose another compiler. Use a separate
build directory or remove its CMake cache when changing compilers.
Start with one build job on machines with limited RAM. Building LLVM from source
is unnecessary for the tested Debian deployment.

Piper is optional; voice commands fall back to `espeak-ng` and use `ffmpeg` for
resampling. Neural voices require a separately installed Piper binary; the
voice module downloads its configured model files when missing.

The private SDK needs additional dependencies listed in its own README.
No CUDA toolkit or GPU is required for the default build.
