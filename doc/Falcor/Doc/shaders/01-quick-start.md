# 快速开始 (Quick Start)

本文档提供快速开始指南，帮助你在30分钟内创建第一个Shader。

This document provides a quick start guide to create your first shader in 30 minutes.

## 前置条件 (Prerequisites)

1. **已构建的Falcor项目**
   - 确保已运行 `setup_vs2022.bat`
   - 使用Visual Studio 2022成功编译项目
   - 构建目录：`build/windows-vs2022/`

2. **Python环境**
   - Falcor自带的Python环境（通过setup.bat自动安装）
   - 位置：`tools/.packman/python/python.exe`

## 五步创建第一个Shader

### 步骤1：创建RenderPass

使用批处理脚本创建新的RenderPass：

```bash
cd F:\Falcor
tools\make_new_render_pass.bat MyFirstShader
```

这将创建：
```
Source/RenderPasses/MyFirstShader/
├── CMakeLists.txt
├── MyFirstShader.h
├── MyFirstShader.cpp
└── (需要手动创建shader文件)
```

### 步骤2：编写简单的Compute Shader

创建文件 `Source/RenderPasses/MyFirstShader/MyFirstShader.cs.slang`：

```hlsl
// MyFirstShader.cs.slang
cbuffer CB
{
    float intensity;
};

Texture2D gSrc;
RWTexture2D<float4> gDst;

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    uint2 dimensions;
    gDst.GetDimensions(dimensions.x, dimensions.y);

    if (any(pixel >= dimensions))
        return;

    // 简单的颜色增强效果
    float4 color = gSrc[pixel];
    color.rgb *= intensity;
    gDst[pixel] = color;
}
```

### 步骤3：修改C++代码集成Shader

修改 `MyFirstShader.cpp`：

```cpp
#include "MyFirstShader.h"

namespace
{
const std::string kShaderFile("RenderPasses/MyFirstShader/MyFirstShader.cs.slang");
const char kSrc[] = "src";
const char kDst[] = "dst";
const char kIntensity[] = "intensity";
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, MyFirstShader>();
}

MyFirstShader::MyFirstShader(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    // 创建Compute Pass
    mpComputePass = ComputePass::create(mpDevice, kShaderFile, "main");

    // 从属性中读取参数
    for (const auto& [key, value] : props)
    {
        if (key == kIntensity)
            mIntensity = value;
        else
            logWarning("Unknown property '{}' in MyFirstShader properties.", key);
    }
}

Properties MyFirstShader::getProperties() const
{
    Properties props;
    props[kIntensity] = mIntensity;
    return props;
}

RenderPassReflection MyFirstShader::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kSrc, "Source texture");
    reflector.addOutput(kDst, "Output texture")
        .bindFlags(ResourceBindFlags::UnorderedAccess)
        .format(ResourceFormat::RGBA32Float);
    return reflector;
}

void MyFirstShader::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pSrcTex = renderData.getTexture(kSrc);
    const auto& pDstTex = renderData.getTexture(kDst);

    if (!pSrcTex || !pDstTex)
    {
        logWarning("MyFirstShader::execute() - missing an input or output resource");
        return;
    }

    // 设置shader参数
    auto var = mpComputePass->getRootVar();
    var["gSrc"] = pSrcTex;
    var["gDst"] = pDstTex;
    var["CB"]["intensity"] = mIntensity;

    // 执行compute shader
    mpComputePass->execute(pRenderContext, pDstTex->getWidth(), pDstTex->getHeight());
}

void MyFirstShader::renderUI(Gui::Widgets& widget)
{
    widget.slider("Intensity", mIntensity, 0.0f, 2.0f);
}
```

修改 `MyFirstShader.h`：

```cpp
#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class MyFirstShader : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(MyFirstShader, "MyFirstShader", "My first shader example.");

    static ref<MyFirstShader> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<MyFirstShader>(pDevice, props);
    }

    MyFirstShader(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

private:
    ref<ComputePass> mpComputePass;
    float mIntensity = 1.0f;
};
```

### 步骤4：构建项目

在Visual Studio 2022中：
1. 打开解决方案：`build/windows-vs2022/Falcor.sln`
2. 在Solution Explorer中找到 `MyFirstShader` 项目
3. 右键点击 → Build
4. 或按 `Ctrl+Shift+B` 构建整个解决方案

或使用命令行：
```bash
cmake --build build/windows-vs2022 --config Release --target MyFirstShader
```

### 步骤5：在Mogwai中测试

创建测试脚本 `test_my_first_shader.py`：

```python
from falcor import *

def render_graph_MyFirstShader():
    g = RenderGraph("MyFirstShader")

    # 加载一张图片
    imageLoader = createPass("ImageLoader", {
        'filename': 'test_scenes/envmaps/hallstatt4_hd.hdr',
        'mips': False,
        'srgb': False,
        'outputFormat': 'RGBA32Float'
    })
    g.addPass(imageLoader, "ImageLoader")

    # 添加我们的shader
    myShader = createPass("MyFirstShader", {'intensity': 1.5})
    g.addPass(myShader, "MyFirstShader")

    # 连接pass
    g.addEdge("ImageLoader.dst", "MyFirstShader.src")
    g.markOutput("MyFirstShader.dst")

    return g

# 创建并添加到Mogwai
MyFirstShader = render_graph_MyFirstShader()
try:
    m.addGraph(MyFirstShader)
except NameError:
    None
```

运行Mogwai：
```bash
build\windows-vs2022\bin\Release\Mogwai.exe test_my_first_shader.py
```

## 验证结果

在Mogwai中你应该能看到：
1. 加载的图像经过intensity调整
2. UI界面中有一个滑块可以调整intensity参数
3. 实时预览效果

## 热更新测试

1. 修改 `MyFirstShader.cs.slang` 文件中的shader代码
2. 在Mogwai中按 `F5` 键
3. Shader会自动重新编译并应用新的效果

例如，修改shader为：
```hlsl
// 添加颜色反转效果
color.rgb = color.rgb * intensity;
color.rgb = 1.0 - color.rgb;  // 反转颜色
gDst[pixel] = color;
```

保存后按F5，立即看到反转效果！

## 下一步

- 查看 [02-创建RenderPass](./02-creating-renderpass.md) 了解更多细节
- 查看 [03-编写Shader](./03-writing-shaders.md) 学习Shader编程
- 查看 [09-FilmGrain示例](./09-filmgrain-example.md) 学习完整的生产级示例

## 常见问题

### Q: 编译错误 "cannot find shader file"
A: 确保shader文件路径与C++代码中的kShaderFile字符串匹配。

### Q: Mogwai无法加载我的pass
A: 检查：
1. DLL是否成功编译到 `build/windows-vs2022/bin/Release/` 目录
2. `registerPlugin` 函数是否正确导出
3. 在Mogwai的日志中查看是否有加载错误

### Q: F5热更新不工作
A:
1. 确保只修改了shader文件（.slang），而不是C++文件
2. 检查shader文件是否有语法错误
3. 查看Mogwai控制台的错误信息
