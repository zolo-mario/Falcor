---
name: falcor-run
description: Run Falcor applications, tests, and scripts. Use when the user asks to run Mogwai, execute tests, run image tests, run unit tests (including SlangUserGuide), run Python tests, run a headless script, run balls.py or other Falcor Python scripts with packman Python, or launch Falcor applications.
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
| Packman Python (balls.py etc.) | `tools/.packman/python/python.exe` with `PYTHONPATH` and `PATH` set |

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
Options: `--config Debug|Release`, `--environment <json>`, `--list-configs`, `--test-suite <regex>`, `--test-case <regex>`, `--tags <tags>`

**Run specific test suites** (e.g., SlangUserGuide):
```bash
tests/run_unit_tests.bat --test-suite HelloWorldTests
```

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

## Packman Python (Falcor Python scripts)

Scripts like `scripts/python/balls/balls.py` use the `falcor` module from the build output. Run with packman Python:

```bash
# From project root; requires Falcor built (build/windows-vs2022/bin/Debug/python/falcor exists)
set PYTHONPATH=build\windows-vs2022\bin\Debug\python;%PYTHONPATH%
set PATH=build\windows-vs2022\bin\Debug;build\windows-vs2022\bin\Debug\plugins;%PATH%
tools\.packman\python\python.exe scripts\python\balls\balls.py
```

PowerShell:
```powershell
$env:PYTHONPATH = "F:\Falcor\build\windows-vs2022\bin\Debug\python;$env:PYTHONPATH"
$env:PATH = "F:\Falcor\build\windows-vs2022\bin\Debug;F:\Falcor\build\windows-vs2022\bin\Debug\plugins;$env:PATH"
.\tools\.packman\python\python.exe scripts\python\balls\balls.py
```

Alternative: `scripts/python/balls/run_balls.bat` (uses `uv run` if available).

## Tips

- Run `.bat` files from repo root; they resolve `tools/.packman/python` and `tests/testing/` automatically.
- If packman Python is missing, run `setup.bat` first.
- Image tests use `tests/environment/default.json` unless overridden.
- **SlangUserGuide** tests live in `Source/Tools/FalcorTest/SlangUserGuide/`; run with `--test-suite HelloWorldTests` (suite name = source filename without extension).
