# Text Editor

The active C++23/Qt editor lives in [`editor/`](editor/). Earlier implementations are archived in [`historical/`](historical/) and are not part of the current CMake build.

```text
editor/src/core/  UTF-8 piece table, spans/scanner, undo/redo, line index, editor model
editor/src/view/  QAbstractScrollArea renderer using QTextLayout/QTextLine
editor/src/app/   Qt application window and open/save actions
editor/tests/     Catch2 core tests and Qt Test keyboard integration tests
editor/fuzz/      libFuzzer edit-sequence differential harness
historical/python/      original Python/Tkinter behavioral prototype
```

## Historical versions

The Python/Tkinter prototype is retained as a behavioral reference. From the repository root:

```sh
python3 historical/python/testPieceTable.py
python3 historical/python/TextEditor.py
```

## Design attribution

`PieceScanner` and its bounded-range scanning model were informed by Branimir Karadžić’s article, [“Scanner”](https://bkaradzic.github.io/posts/scanner/).

## Build and test

Qt 6 Widgets and Qt Test are required for the GUI targets. `clang-format` is
needed for the formatting targets. On macOS with Homebrew:

```sh
brew install qtbase llvm
cmake -S editor -B editor/build -DCMAKE_BUILD_TYPE=Debug -DEDITOR_ENABLE_SANITIZERS=ON
cmake --build editor/build --parallel
ctest --test-dir editor/build --output-on-failure
cmake --build editor/build --target editor_format_check
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for the development workflow and C++
coding conventions. To apply the repository formatting, run:

```sh
cmake --build editor/build --target editor_format
```

Run the application with:

```sh
./editor/build/editor_app
```

To run the libFuzzer harness with Homebrew LLVM:

```sh
cmake -S editor -B editor/build-fuzz \
  -DCMAKE_CXX_COMPILER=/usr/local/opt/llvm/bin/clang++ \
  -DBUILD_TESTING=OFF -DEDITOR_BUILD_QT=OFF -DEDITOR_BUILD_FUZZERS=ON
cmake --build editor/build-fuzz --target edit_sequence_fuzzer --parallel
./editor/build-fuzz/edit_sequence_fuzzer -runs=20000
```
