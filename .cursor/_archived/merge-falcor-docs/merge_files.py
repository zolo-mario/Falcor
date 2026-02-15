#!/usr/bin/env python3
"""
Falcor 文档合并脚本
将 235 个 MD 文件合并为 48 个文件，保证负载均衡和内容内聚。

用法（在仓库根目录）:
  python .cursor/skills/merge-falcor-docs/merge_files.py
  python .cursor/skills/merge-falcor-docs/merge_files.py --source AIStudio/Falcor
"""

import argparse
import re
from pathlib import Path
from typing import List, Tuple

# 源目录：默认从脚本位置推断 Vault 根目录
_SCRIPT_DIR = Path(__file__).resolve().parent
_VAULT_ROOT = _SCRIPT_DIR.parent.parent.parent  # skill -> skills -> .cursor -> vault
DEFAULT_SOURCE_DIR = _VAULT_ROOT / "AIStudio" / "Falcor"

# 合并映射表
MERGE_MAP = {
    # Core 模块 (63个文件 → 12个文件)
    "Core/API/Core-API-Basics.md": [
        "Core/API/API.md",
        "Core/API/Aftermath.md",
        "Core/API/NativeHandle.md",
        "Core/API/NativeFormats.md",
        "Core/API/Handles.md",
    ],
    "Core/API/Core-API-Resources.md": [
        "Core/API/Device.md",
        "Core/API/Resource.md",
        "Core/API/Buffer.md",
        "Core/API/Texture.md",
        "Core/API/FBO.md",
        "Core/API/ResourceViews.md",
        "Core/API/ParameterBlock.md",
    ],
    "Core/API/Core-API-Contexts.md": [
        "Core/API/RenderContext.md",
        "Core/API/ComputeContext.md",
        "Core/API/CopyContext.md",
        "Core/API/BlitContext.md",
    ],
    "Core/API/Core-API-States.md": [
        "Core/API/GraphicsStateObject.md",
        "Core/API/ComputeStateObject.md",
        "Core/API/BlendState.md",
        "Core/API/DepthStencilState.md",
        "Core/API/RasterizerState.md",
    ],
    "Core/API/Core-API-Raytracing.md": [
        "Core/API/RtStateObject.md",
        "Core/API/RtAccelerationStructure.md",
        "Core/API/ShaderTable.md",
        "Core/API/Raytracing.md",
    ],
    "Core/API/Core-API-Memory.md": [
        "Core/API/GpuMemoryHeap.md",
        "Core/API/GpuTimer.md",
        "Core/API/QueryHeap.md",
        "Core/API/Fence.md",
        "Core/API/FencedPool.md",
        "Core/API/Swapchain.md",
        "Core/API/VAO.md",
    ],
    "Core/API/Core-API-D3D12.md": [
        "Core/API/Shared/Shared.md",
        "Core/API/Shared/MockedD3D12StagingBuffer.md",
        "Core/API/Shared/D3D12RootSignature.md",
        "Core/API/Shared/D3D12Handles.md",
        "Core/API/Shared/D3D12DescriptorSetLayout.md",
        "Core/API/Shared/D3D12DescriptorSet.md",
        "Core/API/Shared/D3D12DescriptorPool.md",
        "Core/API/Shared/D3D12DescriptorHeap.md",
        "Core/API/Shared/D3D12DescriptorData.md",
        "Core/API/Shared/D3D12ConstantBufferView.md",
    ],
    "Core/Pass/Core-Pass-System.md": [
        "Core/Pass/Pass.md",
        "Core/Pass/FullScreenPass.md",
        "Core/Pass/ComputePass.md",
        "Core/Pass/RasterPass.md",
        "Core/Pass/BaseGraphicsPass.md",
    ],
    "Core/Program/Core-Program-System.md": [
        "Core/Program/Program.md",
        "Core/Program/ProgramManager.md",
        "Core/Program/ProgramVars.md",
        "Core/Program/ProgramReflection.md",
        "Core/Program/ShaderVar.md",
        "Core/Program/RtBindingTable.md",
        "Core/Program/ProgramVersion.md",
        "Core/Program/DefineList.md",
    ],
    "Core/State/Core-State-Management.md": [
        "Core/State/State.md",
        "Core/State/GraphicsState.md",
        "Core/State/ComputeState.md",
        "Core/State/StateGraph.md",
    ],
    "Core/Platform/Core-Platform-Abstraction.md": [
        "Core/Platform/Platform.md",
        "Core/Platform/OS.md",
        "Core/Platform/MonitorInfo.md",
        "Core/Platform/ProgressBar.md",
        "Core/Platform/MemoryMappedFile.md",
        "Core/Platform/LockFile.md",
        "Core/Platform/PlatformHandles.md",
    ],
    "Core/Core-Overview.md": [
        "Core/Core.md",
    ],

    # Utils 模块 (34个文件 → 7个文件)
    "Utils/Utils-String-IO.md": [
        "Utils/StringUtils.md",
        "Utils/StringFormatters.md",
        "Utils/PathResolving.md",
        "Utils/BinaryFileStream.md",
        "Utils/Properties.md",
    ],
    "Utils/Utils-Sampling-Algorithms.md": [
        "Utils/Sampling.md",
        "Utils/Algorithm.md",
    ],
    "Utils/Utils-Math-Color.md": [
        "Utils/Math.md",
        "Utils/Color.md",
        "Utils/NumericRange.md",
        "Utils/HostDeviceShared.md",
    ],
    "Utils/Utils-Image-Processing.md": [
        "Utils/Image.md",
        "Utils/TermColor.md",
    ],
    "Utils/Utils-Threading-Debug.md": [
        "Utils/Threading.md",
        "Utils/TaskManager.md",
        "Utils/Logger.md",
        "Utils/Debug.md",
        "Utils/SplitBuffer.md",
        "Utils/fast_vector.md",
        "Utils/ObjectID.md",
        "Utils/ObjectIDPython.md",
    ],
    "Utils/Utils-Core-Utilities.md": [
        "Utils/Utils.md",
        "Utils/UI.md",
        "Utils/CoreUtilities.md",
        "Utils/AlignedAllocator.md",
        "Utils/BufferAllocator.md",
        "Utils/Dictionary.md",
    ],
    "Utils/Utils-Special-Features.md": [
        "Utils/IndexedVector.md",
        "Utils/SharedCache.md",
        "Utils/Attributes.md",
        "Utils/NVAPI.md",
        "Utils/CudaUtils.md",
        "Utils/CudaRuntime.md",
        "Utils/CryptoUtils.md",
        "Utils/SlangUtils.md",
    ],

    # Scene 模块 (47个文件 → 9个文件)
    "Scene/Scene-Core-System.md": [
        "Scene/Scene.md",
        "Scene/SceneCore/SceneCore.md",
        "Scene/SceneCore/Scene.md",
        "Scene/SceneCore/IScene.md",
        "Scene/SceneCore/SceneCache.md",
        "Scene/SceneCache.md",
        "Scene/SceneCore/SceneBuilder.md",
        "Scene/SceneCore/SceneBuilderDump.md",
    ],
    "Scene/Scene-Geometry-Mesh.md": [
        "Scene/SceneCore/VertexData.md",
        "Scene/SceneCore/VertexAttrib.md",
        "Scene/SceneCore/TriangleMesh.md",
        "Scene/SceneCore/Transform.md",
        "Scene/VertexAttrib.md",
        "Scene/TriangleMesh.md",
        "Scene/Transform.md",
    ],
    "Scene/Scene-Geometry-Shading.md": [
        "Scene/SceneCore/ShadingData.md",
        "Scene/SceneCore/Shading.md",
        "Scene/SceneCore/HitInfo.md",
    ],
    "Scene/Scene-Raytracing.md": [
        "Scene/SceneCore/Raytracing.md",
        "Scene/SceneCore/RaytracingInline.md",
        "Scene/SceneCore/SceneRayQueryInterface.md",
        "Scene/SceneCore/Intersection.md",
        "Scene/SceneCore/NullTrace.md",
    ],
    "Scene/Scene-Raster-Import.md": [
        "Scene/SceneCore/Raster.md",
        "Scene/SceneCore/Importer.md",
        "Scene/SceneCore/MeshIO.md",
    ],
    "Scene/Scene-Material-Light.md": [
        "Scene/Material.md",
        "Scene/Lights.md",
        "Scene/SceneCore/SceneDefines.md",
    ],
    "Scene/Scene-Components.md": [
        "Scene/Camera.md",
        "Scene/Animation.md",
        "Scene/Curves.md",
    ],
    "Scene/Scene-Displacement.md": [
        "Scene/Displacement/Displacement.md",
        "Scene/Displacement/DisplacementData.md",
        "Scene/Displacement/DisplacementMapping.md",
        "Scene/Displacement/DisplacementUpdate.md",
        "Scene/Displacement/DisplacementUpdateTask.md",
    ],
    "Scene/Scene-Volume-SDF.md": [
        "Scene/Volume/Volume.md",
        "Scene/Volume/GridVolume.md",
        "Scene/Volume/GridConverter.md",
        "Scene/Volume/Grid.md",
        "Scene/Volume/BrickedGrid.md",
        "Scene/Volume/BC4Encode.md",
        "Scene/SDFs/SparseVoxelSet.md",
        "Scene/SDFs/SparseVoxelOctree.md",
        "Scene/SDFs/SparseBrickSet.md",
        "Scene/SDFs/SDFs.md",
        "Scene/SDFs/NormalizedDenseSDFGrid.md",
    ],

    # RenderPasses 模块 (28个文件 → 8个文件)
    "RenderPasses/Path-Tracing.md": [
        "RenderPasses/PathTracer.md",
        "RenderPasses/MinimalPathTracer.md",
        "RenderPasses/WARDiffPathTracer.md",
        "RenderPasses/WhittedRayTracer.md",
        "Doc/renderpasses/PathTracer.md",
        "Doc/renderpasses/MinimalPathTracer.md",
        "Doc/usage/path-tracer.md",
    ],
    "RenderPasses/GBuffer-Accumulation.md": [
        "RenderPasses/GBuffer.md",
        "RenderPasses/AccumulatePass.md",
        "Doc/renderpasses/AccumulatePass.md",
        "Doc/usage/render-passes.md",
    ],
    "RenderPasses/Anti-Aliasing.md": [
        "RenderPasses/TAA.md",
        "Doc/renderpasses/TAA.md",
    ],
    "RenderPasses/Post-Processing.md": [
        "RenderPasses/ToneMapper.md",
        "RenderPasses/SVGFPass.md",
        "RenderPasses/SimplePostFX.md",
        "Doc/renderpasses/ToneMapper.md",
    ],
    "RenderPasses/Denoising-Upscaling.md": [
        "RenderPasses/DLSSPass.md",
        "RenderPasses/NRDPass.md",
        "RenderPasses/OptixDenoiser.md",
        "RenderPasses/RTXDIPass.md",
    ],
    "RenderPasses/Debug-Utils.md": [
        "RenderPasses/DebugPasses.md",
        "RenderPasses/SceneDebugger.md",
        "RenderPasses/PixelInspectorPass.md",
        "RenderPasses/Utils.md",
        "RenderPasses/SDFEditor.md",
        "RenderPasses/ErrorMeasurePass.md",
        "RenderPasses/FLIPPass.md",
    ],
    "RenderPasses/Special-Passes.md": [
        "RenderPasses/BSDFViewer.md",
        "RenderPasses/BSDFOptimizer.md",
        "RenderPasses/BlitPass.md",
        "RenderPasses/ImageLoader.md",
        "RenderPasses/ModulateIllumination.md",
        "RenderPasses/OverlaySamplePass.md",
    ],
    "RenderPasses/RenderPasses-Overview.md": [
        "RenderPasses/RenderPasses.md",
    ],

    # Rendering 模块 (5个文件 → 3个文件)
    "Rendering/Rendering-System.md": [
        "Rendering/Rendering.md",
    ],
    "Rendering/Lighting-Material.md": [
        "Rendering/Lights/Lights.md",
        "Rendering/Materials.md",
        "Rendering/Lights/EmissiveLightSampler.md",
        "Rendering/Utils/PixelStats.md",
        "Doc/usage/materials.md",
    ],
    "Rendering/Advanced-Rendering.md": [
        "Rendering/RTXDI.md",
        "Rendering/Volumes.md",
    ],

    # Doc 模块 (22个文件 → 3个文件)
    "Doc/Doc-Shaders.md": [
        "Doc/shaders/README.md",
        "Doc/shaders/01-quick-start.md",
        "Doc/shaders/02-creating-renderpass.md",
        "Doc/shaders/03-writing-shaders.md",
        "Doc/shaders/04-cpp-shader-integration.md",
        "Doc/shaders/05-gpu-unit-testing.md",
        "Doc/shaders/06-image-testing.md",
    ],
    "Doc/Doc-Usage.md": [
        "Doc/usage/scene-creation.md",
        "Doc/usage/scenes.md",
        "Doc/usage/scene-formats.md",
        "Doc/usage/custom-primitives.md",
        "Doc/usage/scripting.md",
    ],
    "Doc/Doc-Tools.md": [
        "Doc/usage/sdf-editor.md",
        "Doc/usage/environment-variables.md",
    ],

    # 其他模块（保持独立）
    "RenderGraph/RenderGraph.md": [
        "RenderGraph/RenderGraph.md",
    ],
    "Testing/Testing.md": [
        "Testing/Testing.md",
    ],
    "INDEX.md": [
        "INDEX.md",
    ],
    "Summary.md": [
        "Summary.md",
    ],
    "Falcor.md": [
        "Falcor.md",
    ],
}


def clean_obsidian_links(content: str) -> str:
    """移除 Obsidian 链接 [[...]]"""
    # 移除内部链接格式 [[文件名|显示文本]] 或 [[文件名]]
    content = re.sub(r'\[\[([^\]|]+)(?:\|[^\]]+)?\]\]', r'\1', content)
    return content


def merge_files(
    source_dir: Path,
    target_dir: Path,
    target_file: str,
    source_files: List[str],
) -> Tuple[int, int]:
    """合并多个文件到一个目标文件"""
    target_path = target_dir / target_file
    target_path.parent.mkdir(parents=True, exist_ok=True)

    merged_content = []
    total_chars = 0
    success_count = 0

    for source_file in source_files:
        source_path = source_dir / source_file

        if not source_path.exists():
            print(f"  [WARNING] 源文件不存在: {source_file}")
            continue

        try:
            with open(source_path, 'r', encoding='utf-8') as f:
                content = f.read()

            # 清理 Obsidian 链接
            content = clean_obsidian_links(content)

            # 添加分隔符
            file_title = source_file.replace('/', ' > ')
            separator = f"\n\n---\n\n## 源文件: {file_title}\n\n"
            merged_content.append(separator)
            merged_content.append(content)

            total_chars += len(content)
            success_count += 1

        except Exception as e:
            print(f"  [ERROR] 读取文件失败 {source_file}: {e}")

    # 写入目标文件
    if merged_content:
        # 添加标题
        target_title = target_file.replace('/', ' > ').replace('.md', '')
        header = f"# {target_title}\n\n"

        try:
            with open(target_path, 'w', encoding='utf-8') as f:
                f.write(header)
                f.write(''.join(merged_content))

            print(f"  [OK] 成功合并 {success_count} 个文件 ({total_chars} 字符) -> {target_file}")
            return success_count, total_chars
        except Exception as e:
            print(f"  [ERROR] 写入文件失败 {target_file}: {e}")
            return success_count, 0

    return 0, 0


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="Falcor 文档合并工具")
    parser.add_argument(
        "--source",
        type=str,
        default=None,
        help="源目录（相对于 Vault 根目录，如 AIStudio/Falcor）",
    )
    parser.add_argument(
        "--target",
        type=str,
        default=None,
        help="目标目录（默认在源目录下的 merged/）",
    )
    args = parser.parse_args()

    source_dir = Path(args.source) if args.source else DEFAULT_SOURCE_DIR
    if not source_dir.is_absolute():
        source_dir = _VAULT_ROOT / source_dir
    target_dir = source_dir / "merged"
    if args.target:
        td = Path(args.target)
        target_dir = td if td.is_absolute() else _VAULT_ROOT / td

    print("=" * 60)
    print("Falcor 文档合并工具")
    print("=" * 60)
    print(f"源目录: {source_dir}")
    print(f"目标目录: {target_dir}")
    print()

    # 统计信息
    total_target_files = len(MERGE_MAP)
    total_source_files = sum(len(files) for files in MERGE_MAP.values())
    total_chars_merged = 0
    total_files_merged = 0

    # 执行合并
    for target_file, source_files in MERGE_MAP.items():
        print(f"[MERGE] 合并: {target_file} ({len(source_files)} 个源文件)")
        merged_count, chars = merge_files(
            source_dir, target_dir, target_file, source_files
        )
        total_files_merged += merged_count
        total_chars_merged += chars

    # 输出统计
    print()
    print("=" * 60)
    print("合并完成!")
    print("=" * 60)
    print(f"目标文件数: {total_target_files}")
    print(f"成功合并源文件: {total_files_merged}/{total_source_files}")
    print(f"总字符数: {total_chars_merged:,}")
    if total_files_merged > 0:
        print(f"平均每个目标文件: {total_files_merged/total_target_files:.2f} 个源文件")
        print(f"平均每个文件: {total_chars_merged/total_files_merged:.0f} 字符")
    print()
    print(f"合并后的文件位于: {target_dir}")


if __name__ == "__main__":
    main()
