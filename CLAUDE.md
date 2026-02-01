# Falcor

## Build Command
```bash
cmake --build build/windows-vs2022 --config Debug
```

Or to build just the MeshletCulling plugin:
```bash
cmake --build build/windows-vs2022 --config Debug --target MeshletCulling
```

## Run Command
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe --script "F:/Falcor/Source/RenderPasses/MeshletCulling/MeshCulling.py" --scene "F:/Falcor/media/test_scenes/bunny.pyscene" --headless
```
