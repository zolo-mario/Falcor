#pragma once

// Core
#include "Core/Macros.h"
#include "Core/Version.h"
#include "Core/Error.h"

// Core/Platform
#include "Core/Platform/OS.h"

// Core/API
#include "Core/API/Types.h"
#include "Core/API/BlendState.h"
#include "Core/API/Buffer.h"
#include "Core/API/ComputeContext.h"
#include "Core/API/ComputeStateObject.h"
#include "Core/API/CopyContext.h"
#include "Core/API/DepthStencilState.h"
#include "Core/API/Device.h"
#include "Core/API/FBO.h"
#include "Core/API/FencedPool.h"
#include "Core/API/Formats.h"
#include "Core/API/Fence.h"
#include "Core/API/GpuTimer.h"
#include "Core/API/GraphicsStateObject.h"
#include "Core/API/IndirectCommands.h"
#include "Core/API/LowLevelContextData.h"
#include "Core/API/ParameterBlock.h"
#include "Core/API/QueryHeap.h"
#include "Core/API/RasterizerState.h"
#include "Core/API/Raytracing.h"
#include "Core/API/RenderContext.h"
#include "Core/API/Resource.h"
#include "Core/API/GpuMemoryHeap.h"
#include "Core/API/ResourceViews.h"
#include "Core/API/RtStateObject.h"
#include "Core/API/Sampler.h"
#include "Core/API/Texture.h"
#include "Core/API/VAO.h"
#include "Core/API/VertexLayout.h"

// Core/Platform
#include "Core/Platform/OS.h"
#include "Core/Platform/ProgressBar.h"

// Core/Program
#include "Core/Program/Program.h"
#include "Core/Program/ProgramReflection.h"
#include "Core/Program/ProgramVars.h"
#include "Core/Program/ProgramVersion.h"

// Core/State
#include "Core/State/ComputeState.h"
#include "Core/State/GraphicsState.h"

// Scene
#include "Scene/Scene.h"
#include "Scene/Camera/Camera.h"
#include "Scene/Camera/CameraController.h"
#include "Scene/Lights/Light.h"
#include "Scene/Material/MaterialSystem.h"
#include "Scene/Material/StandardMaterial.h"
#include "Scene/Material/HairMaterial.h"
#include "Scene/Material/ClothMaterial.h"
#include "Scene/Animation/Animation.h"
#include "Scene/Animation/AnimationController.h"

// Utils
#include "Utils/StringFormatters.h"
#include "Utils/Math/Common.h"
#include "Utils/Math/Vector.h"
#include "Utils/Logger.h"
#include "Utils/UI/InputTypes.h"
#include "Utils/Timing/Profiler.h"

#include <fmt/format.h> // TODO C++20: Replace with <format>
#include <fstd/span.h>  // TODO C++20: Replace with <span>

#include <memory>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <variant>

#include <cstdint>
#include <cmath>
