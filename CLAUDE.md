# Falcor

## Commands

Build everything
```bash
cmake --build build/windows-vs2022 --config Debug -DCMAKE_MESSAGE_LOG_LEVEL=ERROR
```

Build specific plugins
```bash
cmake --build build/windows-vs2022 --config Debug --target GBuffer
```

Run VBufferMeshletRaster Tests
```bash
./build/windows-vs2022/bin/Debug/Mogwai.exe --script "F:/Falcor/Source/RenderPasses/GBuffer/VBuffer/VBufferMeshletRasterTest.py" --scene "F:/Falcor/media/test_scenes/bunny.pyscene" --headless
```
