---
name: stub-gen
description: Regenerates pybind11 `.pyi` stubs for the Falcor `falcor` Python package (IDE autocomplete). Use when the user asks to run stubgen, regenerate stubs, update FalcorPython typings, or work with `tools/stubgen` / `generate_stubs.py`.
---

# Falcor Python stub generation (stubgen)

**中文**：为 `falcor` 包生成/更新 `.pyi`（供 IDE 补全）。需先编译 `FalcorPython`；脚本在 `tools/stubgen/generate_stubs.py`。手动运行时在环境中设置与构建 POST_BUILD 相同：`PATH` 含 `bin/<Config>`，`PYTHONPATH` 含 `bin/<Config>/python`（Linux 另需 `LD_LIBRARY_PATH` 含 `bin/<Config>`），再传入该 `python` 目录作为唯一参数。

## Prerequisites

- **`FalcorPython` must be built** so `falcor_ext` exists under the CMake output tree (e.g. `build/<preset>/bin/<Config>/python/falcor/`).
- Stub scripts live in **`tools/stubgen/`**: `generate_stubs.py` (entry) and vendored `pybind11_stubgen.py`.

## When stubs are generated automatically

Building **`FalcorPython`** runs a POST_BUILD step that invokes `generate_stubs.py` via **`cmake -E env`** (same effective env as old `setpath.bat` / `setpath.sh`). See `Source/Falcor/CMakeLists.txt` (search for `generate_stubs`).

## Manual run (Windows)

Set **`PATH`** to include `<build>/<preset>/bin/<Config>` first, **`PYTHONPATH`** to include `<build>/<preset>/bin/<Config>/python`, then:

```text
external/packman/python/python.exe tools/stubgen/generate_stubs.py <build>/<preset>/bin/<Config>/python
```

Example: `build/windows-vs2022/bin/Debug/`.

- **`generate_stubs.py` takes exactly one argument**: the directory containing the `falcor` package (the `python` directory under `bin/<Config>`).
- Use the same Python as CMake when not using system Python: **`external/packman/python/python.exe`**.

## Manual run (Linux)

Set **`PATH`**, **`PYTHONPATH`** (and **`LD_LIBRARY_PATH`** with the `bin/<Config>` dir) like the POST_BUILD `cmake -E env` block, then:

```text
python3 tools/stubgen/generate_stubs.py <build>/<preset>/bin/<Config>/python
```

## Implementation notes

- `generate_stubs.py` calls **`pybind11_stubgen.main`** with module name **`falcor`**, output **`-o <package_dir>`**, plus flags `--ignore-invalid=all`, `--skip-signature-downgrade`, `--no-setup-py`, `--root-module-suffix=`.
- It appends submodule aliases to **`falcor/__init__.pyi`** (e.g. `ui`); edit `tools/stubgen/generate_stubs.py` if new submodules need the same treatment.

## Related CMake paths

- `FALCOR_OUTPUT_DIRECTORY` = `bin/<Config>` (multi-config generators) or `bin` (single-config).
- Generated stubs are written under **`${FALCOR_OUTPUT_DIRECTORY}/python/falcor/`** (e.g. `falcor_ext/__init__.pyi`).
