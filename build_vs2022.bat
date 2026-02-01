@echo off  
cmake --preset windows-vs2022 -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build/windows-vs2022 --config Debug
