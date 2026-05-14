# Compiler Roadmap (Practical Next Steps)

## 1) Function semantics (highest value)
- Real function call lowering with argument passing
- Proper return flow for non-main functions
- Recursion-safe stack frames
- Distinct local scopes per function/block

## 2) Pass-by modes
- Pass-by-value execution semantics
- Pass-by-reference semantics (address aliasing)
- Pass-by-pointer semantics (`&` and dereference support)

## 3) Type system expansion (int-first)
- Struct field layout and dot access
- Union overlay behavior
- Class member layout with access checks in parser/semantic phase
- Enum constants mapped to integers

## 4) Memory/runtime model tightening
- Explicit code/data/bss/stack segment boundaries
- Register convention document (`ra`, `sp`, `fp`, `a0..a7`, temps)
- Stack overflow checks
- Better VM monitor views (segment + frame dumps)

## 5) Multi-file compiler workflow
- Separate compile stage to object-like IR files
- Link step resolving extern symbols across files
- Better diagnostics for undefined symbols/functions

## 6) Optimizer passes
- Constant folding
- Dead code elimination
- Basic register reuse improvements

## 7) Test plan growth
- Split tests by phase: parser / semantic / codegen / VM
- Add negative tests (invalid syntax, invalid extern, type errors)
- Add regression tests for every fixed bug