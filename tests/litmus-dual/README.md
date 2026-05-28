# Litmus Dual

## TLDR

`tests/litmus-dual` converts selected tests from `litmus-tests-riscv` into a
dual-core nexus-am app.

```sh
make -C tests/litmus-dual
```

The default target fetches the litmus source repository if needed, opens the
interactive category selector, generates `generated/`, and builds it for
`ARCH=riscv64-xs-dual`.

Useful commands:

```sh
make -C tests/litmus-dual update
make -C tests/litmus-dual categories
make -C tests/litmus-dual generate CATEGORY=non-mixed-size/RELAX
make -C tests/litmus-dual generate-tui MAX_TESTS=20
make -C tests/litmus-dual clean
```

## Workflow

Run the default interactive flow:

```sh
make -C tests/litmus-dual
```

This performs:

1. `fetch`: ensure the litmus source repository exists
2. interactive generation: choose categories in the TUI
3. build: compile generated sources into `build/`

If the TUI is cancelled with `q`, generation stops cleanly and the stale
`generated/.litmus-generated.stamp` is removed before the build step.

## Fetching And Updating

`fetch` is conservative:

```sh
make -C tests/litmus-dual fetch
```

It clones the default upstream repository into `.cache/litmus-tests-riscv` only
when the cache does not exist. If the cache already exists and `LITMUS_COMMIT`
is empty, `fetch` leaves the checkout unchanged.

Use `update` when you explicitly want the local cache to follow upstream
`master`:

```sh
make -C tests/litmus-dual update
```

For reproducible generation, pin an upstream commit:

```sh
make -C tests/litmus-dual generate \
  LITMUS_COMMIT=08728edcd99c7e1819d2d4c6789df6d5299ccb69
```

Use an existing external checkout:

```sh
make -C tests/litmus-dual generate LITMUS_ROOT=$HOME/litmus-tests-riscv
```

`make update` does not modify an external `LITMUS_ROOT`.

## Category Selection

List available categories:

```sh
make -C tests/litmus-dual categories
```

Choose categories interactively:

```sh
make -C tests/litmus-dual generate-tui
```

TUI keys:

- `Up`/`Down`, `k`/`j`: move
- `Left`/`Right`, `h`/`l`: collapse or expand a folder
- `Space`: toggle the current category subtree
- `a`: select all
- `n`: clear selection
- `Enter`: generate
- `q` or `Esc`: cancel

Selecting a folder selects all category nodes below that folder. The generated
input set is expanded from every selected node and deduplicated by litmus file
path, so folder selection matches what the TUI shows.

The selector excludes `tests/to-AArch64` because those files contain AArch64
instructions and cannot be built as RISC-V inline assembly.

Generate a category without the selector:

```sh
make -C tests/litmus-dual generate CATEGORY=non-mixed-size/RELAX
```

Pass a comma-separated selection through `CATEGORY` when multiple categories are
needed:

```sh
make -C tests/litmus-dual generate CATEGORY=non-mixed-size,non-mixed-size/RELAX
```

Use a raw input file, directory, or `@all` file:

```sh
make -C tests/litmus-dual generate INPUT=tests/non-mixed-size/@all
```

## Building And Running

Build the generated app:

```sh
make -C tests/litmus-dual build ARCH=riscv64-xs-dual
```

Run the generated app through the litmus-dual Makefile:

```sh
make -C tests/litmus-dual run ARCH=riscv64-xs-dual
```

Pass extra generator arguments with `GEN_ARGS`:

```sh
make -C tests/litmus-dual generate GEN_ARGS=--strict
```

Limit converted tests while debugging:

```sh
make -C tests/litmus-dual generate-tui MAX_TESTS=20
```

The generated app disables randomized perturbation delays by default. Enable
them explicitly when needed:

```sh
make -C tests/litmus-dual build CPPFLAGS=-DLITMUS_PERTURB=1
```

## Cleaning

Remove generated output and the local fetched repository:

```sh
make -C tests/litmus-dual clean
```

`clean` removes this directory's `generated/`, `build/`, and `.cache/`. It does
not remove an external repository passed through `LITMUS_ROOT`.

Use narrower clean targets when needed:

```sh
make -C tests/litmus-dual clean-generated
make -C tests/litmus-dual clean-cache
make -C tests/litmus-dual clean-build
```

## Script Interface

The underlying converter can be run directly:

```sh
python3 tests/litmus-dual/scripts/litmus_to_am.py \
  --repo-root tests/litmus-dual/.cache/litmus-tests-riscv \
  --output-dir tests/litmus-dual/generated \
  --interactive \
  --clean
```

Common options:

- `--repo-root PATH`: root of a `litmus-tests-riscv` checkout
- `--output-dir PATH`: generated source directory
- `--input PATH`: litmus file, directory, or `@all` file
- `--category VALUE`: category name, number, range, or comma-separated selection
- `--interactive`: open the TUI or prompt fallback
- `--list-categories`: print discovered categories and exit
- `--max-tests N`: stop after converting `N` supported tests
- `--clean`: remove generated source files before writing
- `--strict`: return non-zero if any candidate test is skipped

The converter currently targets 2-thread RISC-V litmus tests. Unsupported
tests, missing files referenced by upstream `@all` lists, and non-RISC-V inputs
are reported in `generated/conversion-report.json`.
