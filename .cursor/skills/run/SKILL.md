---
name: falcor-run
description: Run Falcor applications, tests, and scripts. Use when the user asks to run Mogwai, execute tests, run image tests, run unit tests, run Python tests, run a headless script, or launch Falcor applications.
---

# Falcor Run

## Quick Reference

| Action | Command |
|--------|---------|
| Unit tests (C++) | `tests/run_unit_tests.bat` |
| Image tests | `tests/run_image_tests.bat` |
| Python tests | `tests/run_python_tests.bat` (requires conda) |
| View image test results | `tests/view_image_tests.bat` |
| Mogwai headless script | `./build/windows-vs2022/bin/Debug/Mogwai.exe --script <SCRIPT> --scene <SCENE> --headless` |
| Mogwai interactive | `./build/windows-vs2022/bin/Debug/Mogwai.exe` |

Run `.bat` scripts from the project root (e.g., `F:/Falcor`).

## Mogwai

**Interactive** (opens GUI):
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe
```

**Headless** (scripted render, no GUI):
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe --script "F:/Falcor/Source/RenderPasses/GBuffer/VBuffer/VBufferMeshletRasterTest.py" --scene "F:/Falcor/media/test_scenes/bunny.pyscene" --headless
```

Swap `Debug` for `Release` if built with that config.

## Test Suites

**Unit tests** (FalcorTest.exe):
```bash
tests/run_unit_tests.bat
```
Options: `--config Debug|Release`, `--environment <json>`, `--list-configs`

**Image tests** (Mogwai-based):
```bash
tests/run_image_tests.bat
```
Options: `--gen-refs`, `--parallel N`, `--config`, `--environment`

**Python tests** (requires conda + CONDA_PYTHON_EXE):
```bash
tests/run_python_tests.bat
```

**View image test results** (web viewer):
```bash
tests/view_image_tests.bat
```

## Build Output Paths

| Config | Path |
|--------|------|
| Debug | `build/windows-vs2022/bin/Debug/` |
| Release | `build/windows-vs2022/bin/Release/` |

Executables: `Mogwai.exe`, `FalcorTest.exe`, `ImageCompare.exe`, `RenderGraphEditor.exe`

## Tips

- Run `.bat` files from repo root; they resolve `tools/.packman/python` and `tests/testing/` automatically.
- If packman Python is missing, run `setup.bat` first.
- Image tests use `tests/environment/default.json` unless overridden.
