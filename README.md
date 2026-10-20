# Snowstorm-Sand

A falling-sand game on [Snowstorm Engine](https://github.com/MegatronJeremy/Snowstorm-Engine). The
engine is a submodule, consumed and not modified.

## Building

Requires Windows and Visual Studio 2022. The engine's dependencies come from its vcpkg tree, so
bootstrap that first, then configure this repository against the same toolchain.

```
git clone --recurse-submodules https://github.com/MegatronJeremy/Snowstorm-Sand
cd Snowstorm-Sand
py Snowstorm-Engine/Scripts/Generate-Solution.py --build-dir Snowstorm-Engine/build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=Snowstorm-Engine/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug --target Snowstorm-Sand
```

## Running

```
build/Debug/Snowstorm-Sand.exe
```

The game boots its own project, `Projects/Sand`.

## Packaging

```
cmake --build build --config Debug --target stage
cd build/stage/Debug
./Snowstorm-Sand.exe
```

## License

Public domain, like the engine. See [`UNLICENSE.txt`](UNLICENSE.txt).
