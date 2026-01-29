#pragma once

#include "Core/Macros.h"
#include "Core/AssetResolver.h"
#include "Core/API/Device.h"
#include "Scene/SceneBuilder.h"

namespace Falcor
{

/// This file is a temporary workaround to give access to global state in deprecated python bindings.
/// Some of the Python API was originally designed to allow creation of objects "out of thin air".
/// Two places are affected:
/// - Loading `.pyscene` files: Here many of the scene objects can just be created without a factory.
/// - Creating/loading render graphs and passes
/// The C++ side is currently being refactored to get rid of all that global state (for example, the GPU device).
/// In order to not break the existing Python API, we use global state in very specific contexts only.
/// All of the affected python bindings are marked with PYTHONDEPRECATED. Once these bindings are removed,
/// this file can also be removed as well.

FALCOR_API void setActivePythonSceneBuilder(SceneBuilder* pSceneBuilder);
FALCOR_API SceneBuilder& accessActivePythonSceneBuilder();
FALCOR_API AssetResolver& getActiveAssetResolver();

FALCOR_API void setActivePythonRenderGraphDevice(ref<Device> pDevice);
FALCOR_API ref<Device> getActivePythonRenderGraphDevice();
FALCOR_API ref<Device> accessActivePythonRenderGraphDevice();

} // namespace Falcor
