# Contributing to Text Editor

The active project is the C++23/Qt implementation in `editor/`. Keep a change
small, complete, and covered by the narrowest test that proves its behavior.

## Development loop

1. Read the affected interface and its tests before changing an implementation.
2. Make the invariant or ownership boundary explicit in the code; do not add
   speculative fallback behavior that would hide a broken internal state.
3. Add or update tests with the behavior change. Core edit changes should also
   preserve the `std::string` differential checks and document invariants.
4. Format, build, and run tests. Use sanitizers for changes to the core text
   model, spans, indexes, undo history, or view memory handling. Exercise the
   fuzzer when a change affects edit sequencing or undo/redo semantics.

Commands are listed in the [README](README.md#build-and-test).

## C++ conventions

- Target C++23 and standard-library/Qt APIs already used by this project. Do
  not introduce a second abstraction for an existing project concept without a
  clear, reusable responsibility.
- Editor documents are valid UTF-8. Internal offsets remain byte offsets for
  piece-table efficiency, but selections and cursor movement must always land
  on Unicode code-point boundaries.
- Follow the repository formatter (`.clang-format`): four spaces, Allman
  braces, 120-column limit, and no include reordering. Run
  `cmake --build editor/build --target editor_format_check` before submitting.
- Use `PascalCase` for types, `snake_case` for functions and variables, and a
  trailing underscore for private data members. Header files must not contain
  `using namespace` directives.
- Prefer value semantics and RAII. Express ownership in the type: use a value
  member for required ownership, `std::unique_ptr` for exclusive optional
  ownership, `std::shared_ptr` only for genuine shared lifetime, and raw
  pointers/references for non-owning access.
- Preserve const-correctness. Mark pure query functions `[[nodiscard]]` when
  discarding their result would likely be a mistake, and use `noexcept` only
  when the guarantee is true.
- Keep helpers close to their use and give them a distinct responsibility. Do
  not extract one-use wrappers that merely rename a single expression, but do
  centralize repeated invariants, transformations, and policy decisions.
- Fail fast for violated internal invariants using `EDITOR_CHECK`; fix the
  origin of the invalid state rather than silently repairing it downstream.
  Validate and report errors at external boundaries (files, UI input, or other
  untrusted data) where recovery is part of the product behavior.
- Default to single-threaded code. Add synchronization only for a documented
  shared-state requirement; state what data a lock protects and avoid polling
  or complex lock-free designs unless measurement and design require them.
- Keep portable code platform-neutral. Isolate unavoidable operating-system
  behavior behind the smallest practical interface and test the shared logic.

## Validation expectations

- Unit tests cover each observable behavior and boundary case.
- Sanitizers are enabled for memory-sensitive core changes.
- The edit-sequence fuzzer is run for algorithmic changes to the editor model.
- Public API or invariant changes update the corresponding header, tests, and
  user-facing documentation in the same change.
