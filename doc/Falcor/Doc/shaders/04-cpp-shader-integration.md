# C++与Shader集成 (C++ and Shader Integration)

本文档详细介绍如何在C++代码中加载、配置和执行Shader。

## 概述

在Falcor中，C++代码负责：
1. 加载和编译Shader
2. 设置Shader参数（Constant Buffer、纹理、缓冲区）
3. 执行Shader
4. 管理资源生命周期

Shader负责：
1. 实际的GPU计算
2. 定义数据结构和接口

## Compute Shader集成（以FilmGrain为例）

### 步骤1：声明Shader路径和变量

```cpp
// FilmGrain.cpp

namespace
{
// Shader文件路径（相对于项目根目录）
const std::string kShaderFile("RenderPasses/FilmGrain/FilmGrain.cs.slang");

// 资源名称常量
const char kSrc[] = "src";
const char kDst[] = "dst";

// 参数名称常量
const char kGrainAmount[] = "grainAmount";
const char kEnabled[] = "enabled";
}
```

**路径规则**：
- 路径相对于 `Source/` 目录
- 不需要 `Source/` 前缀
- 使用正斜杠 `/`

### 步骤2：创建ComputePass

在构造函数中创建ComputePass：

```cpp
FilmGrain::FilmGrain(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    // 创建Compute Pass
    // 参数1: 设备
    // 参数2: shader文件路径
    // 参数3: 入口点函数名（通常是"main"）
    mpFilmGrainPass = ComputePass::create(mpDevice, kShaderFile, "main");

    // 从属性中初始化参数
    for (const auto& [key, value] : props)
    {
        if (key == kGrainAmount)
            mGrainAmount = value;
        else if (key == kEnabled)
            mEnabled = value;
        else
            logWarning("Unknown property '{}' in FilmGrain properties.", key);
    }
}
```

**ComputePass类型**：
- `ComputePass`: 计算着色器
- `FullScreenPass`: 全屏后处理
- `Program`: 通用着色器程序

### 步骤3：定义资源反射

告诉系统这个Pass需要什么输入和输出：

```cpp
RenderPassReflection FilmGrain::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;

    // 添加输入
    reflector.addInput(kSrc, "Source texture");

    // 添加输出
    reflector.addOutput(kDst, "Output texture with film grain effect")
        .bindFlags(ResourceBindFlags::UnorderedAccess)  // UAV访问
        .format(ResourceFormat::RGBA32Float);            // 格式

    return reflector;
}
```

**常用绑定标志**：
```cpp
// 只读纹理
.bindFlags(ResourceBindFlags::ShaderResource)

// UAV纹理（compute shader输出）
.bindFlags(ResourceBindFlags::UnorderedAccess)

// 渲染目标（pixel shader输出）
.bindFlags(ResourceBindFlags::RenderTarget)

// 组合
.bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::ShaderResource)
```

**常用格式**：
```cpp
ResourceFormat::RGBA32Float   // 32位浮点，4通道
ResourceFormat::RGBA16Float   // 16位浮点，4通道
ResourceFormat::RGBA8Unorm    // 8位标准化，4通道
ResourceFormat::R32Float      // 32位浮点，单通道
ResourceFormat::D32Float      // 32位深度
```

### 步骤4：执行Shader

在execute方法中设置参数并执行：

```cpp
void FilmGrain::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // 4.1 获取输入输出纹理
    const auto& pSrcTex = renderData.getTexture(kSrc);
    const auto& pDstTex = renderData.getTexture(kDst);

    // 4.2 验证资源
    if (!pSrcTex || !pDstTex)
    {
        logWarning("FilmGrain::execute() - missing an input or output resource");
        return;
    }

    // 4.3 获取根变量（用于设置shader参数）
    auto var = mpFilmGrainPass->getRootVar();

    // 4.4 绑定纹理资源
    var["gSrc"] = pSrcTex;
    var["gDst"] = pDstTex;

    // 4.5 设置Constant Buffer参数
    var["CB"]["frameTime"] = mFrameTime;
    var["CB"]["grainAmount"] = mGrainAmount;
    var["CB"]["enabled"] = mEnabled ? 1u : 0u;

    // 4.6 执行compute shader
    // 参数: RenderContext, 宽度, 高度
    // 系统会自动计算dispatch大小 based on numthreads
    mpFilmGrainPass->execute(
        pRenderContext,
        pDstTex->getWidth(),
        pDstTex->getHeight()
    );

    // 4.7 更新动态参数
    mFrameTime += 0.016f;  // ~60 FPS
}
```

## 参数绑定详解

### 1. 纹理绑定

**Shader侧**：
```hlsl
Texture2D gSrc;
RWTexture2D<float4> gDst;
```

**C++侧**：
```cpp
auto var = mpPass->getRootVar();
var["gSrc"] = pTexture;
var["gDst"] = pOutputTexture;
```

### 2. Constant Buffer绑定

**Shader侧**：
```hlsl
cbuffer CB
{
    float frameTime;
    float grainAmount;
    uint enabled;
}
```

**C++侧**：
```cpp
var["CB"]["frameTime"] = 0.5f;
var["CB"]["grainAmount"] = 0.01f;
var["CB"]["enabled"] = 1u;

// 或使用结构体（如果定义了）
struct CBData
{
    float frameTime;
    float grainAmount;
    uint32_t enabled;
    uint32_t padding;  // 确保16字节对齐
};

CBData data;
data.frameTime = 0.5f;
data.grainAmount = 0.01f;
data.enabled = 1;
var["CB"].setBlob(&data, sizeof(data));
```

### 3. 结构化缓冲区绑定

**Shader侧**：
```hlsl
struct Particle
{
    float3 position;
    float3 velocity;
};

StructuredBuffer<Particle> gInputParticles;
RWStructuredBuffer<Particle> gOutputParticles;
```

**C++侧**：
```cpp
// 创建结构化缓冲区
auto pBuffer = mpDevice->createStructuredBuffer(
    sizeof(Particle),
    particleCount,
    ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
);

// 绑定
var["gInputParticles"] = pBuffer;
var["gOutputParticles"] = pBuffer;
```

### 4. 采样器绑定

**Shader侧**：
```hlsl
SamplerState gSampler;
```

**C++侧**：
```cpp
// 创建采样器
Sampler::Desc desc;
desc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Linear);
desc.setAddressingMode(TextureAddressingMode::Clamp, TextureAddressingMode::Clamp, TextureAddressingMode::Clamp);
auto pSampler = mpDevice->createSampler(desc);

// 绑定
var["gSampler"] = pSampler;
```

## 不同类型的Pass

### 1. Compute Pass（像FilmGrain）

```cpp
// 创建
mpComputePass = ComputePass::create(mpDevice, shaderFile, "main");

// 执行
mpComputePass->execute(pRenderContext, width, height);
// 或指定dispatch维度
mpComputePass->execute(pRenderContext, width, height, depth);
```

### 2. Full Screen Pass

用于全屏后处理效果：

```cpp
// 创建
mpFullScreenPass = FullScreenPass::create(mpDevice, shaderFile);

// 执行
mpFullScreenPass->execute(pRenderContext, pFbo);
```

**Shader示例**（Vertex + Pixel）：
```hlsl
// FullScreen.vs.slang
struct VSOut
{
    float4 posH : SV_POSITION;
    float2 texC : TEXCOORD;
};

VSOut main(uint vertexId : SV_VertexID)
{
    VSOut vOut;
    vOut.texC = float2((vertexId << 1) & 2, vertexId & 2);
    vOut.posH = float4(vOut.texC * float2(2, -2) + float2(-1, 1), 0, 1);
    return vOut;
}

// FullScreen.ps.slang
Texture2D gTexture;
SamplerState gSampler;

float4 main(float2 texC : TEXCOORD) : SV_TARGET
{
    return gTexture.Sample(gSampler, texC);
}
```

### 3. Raster Pass

用于传统的光栅化渲染：

```cpp
// 创建Program
Program::Desc desc;
desc.addShaderLibrary("MyPass.vs.slang").vsEntry("vsMain");
desc.addShaderLibrary("MyPass.ps.slang").psEntry("psMain");
auto pProgram = Program::create(mpDevice, desc);

// 创建GraphicsState
mpGraphicsState = GraphicsState::create(mpDevice);
mpGraphicsState->setProgram(pProgram);

// 创建Vars
mpVars = ProgramVars::create(mpDevice, pProgram.get());

// 执行
mpGraphicsState->setFbo(pFbo);
pRenderContext->draw(mpGraphicsState.get(), mpVars.get(), vertexCount, startVertex);
```

## Dispatch大小计算

### 自动计算（推荐）

```cpp
// Falcor自动计算dispatch组数
mpComputePass->execute(pRenderContext, width, height);
```

对于 `[numthreads(16, 16, 1)]`，如果width=1920, height=1080：
- 组数X = ceil(1920 / 16) = 120
- 组数Y = ceil(1080 / 16) = 68
- 组数Z = 1

### 手动Dispatch

```cpp
uint3 dispatchSize;
dispatchSize.x = (width + 15) / 16;   // ceil(width / 16)
dispatchSize.y = (height + 15) / 16;  // ceil(height / 16)
dispatchSize.z = 1;

pRenderContext->dispatch(mpComputePass->getState().get(), mpVars.get(), dispatchSize);
```

## UI集成

```cpp
void FilmGrain::renderUI(Gui::Widgets& widget)
{
    // 复选框
    widget.checkbox("Enabled", mEnabled);

    // 滑块
    widget.slider("Grain Amount", mGrainAmount, 0.0f, 0.1f);

    // 拖拽
    widget.var("Frame Time", mFrameTime, 0.0f, 10.0f);

    // 按钮
    if (widget.button("Reset"))
    {
        mFrameTime = 0.0f;
    }

    // 折叠组
    if (auto group = widget.group("Advanced"))
    {
        group.slider("Detail 1", mDetail1, 0.0f, 1.0f);
        group.slider("Detail 2", mDetail2, 0.0f, 1.0f);
    }
}
```

## 属性系统（序列化）

### 保存属性

```cpp
Properties FilmGrain::getProperties() const
{
    Properties props;
    props[kGrainAmount] = mGrainAmount;
    props[kEnabled] = mEnabled;
    return props;
}
```

### 加载属性

```cpp
FilmGrain::FilmGrain(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    for (const auto& [key, value] : props)
    {
        if (key == kGrainAmount)
            mGrainAmount = value;
        else if (key == kEnabled)
            mEnabled = value;
    }
}
```

## 资源管理最佳实践

### 1. 延迟创建资源

```cpp
void MyPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // 第一次使用时创建
    if (!mpTempTexture)
    {
        mpTempTexture = mpDevice->createTexture2D(
            width, height,
            ResourceFormat::RGBA16Float,
            1, 1,
            nullptr,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
        );
    }

    // 使用资源...
}
```

### 2. 检测尺寸变化

```cpp
void MyPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pOutput = renderData.getTexture("output");

    // 检查是否需要重新创建资源
    if (!mpTempTexture ||
        mpTempTexture->getWidth() != pOutput->getWidth() ||
        mpTempTexture->getHeight() != pOutput->getHeight())
    {
        mpTempTexture = mpDevice->createTexture2D(
            pOutput->getWidth(),
            pOutput->getHeight(),
            ResourceFormat::RGBA16Float,
            1, 1, nullptr,
            ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
        );
    }
}
```

### 3. 使用智能指针

Falcor使用 `ref<>` 智能指针自动管理资源：

```cpp
ref<Texture> mpTexture;       // 自动释放
ref<ComputePass> mpPass;      // 自动释放
ref<Buffer> mpBuffer;         // 自动释放
```

## 常见模式

### 模式1：双Buffer交换

```cpp
class MyPass : public RenderPass
{
private:
    ref<Texture> mpPingTexture;
    ref<Texture> mpPongTexture;
    bool mUsePing = true;
};

void MyPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pInput = mUsePing ? mpPingTexture : mpPongTexture;
    auto pOutput = mUsePing ? mpPongTexture : mpPingTexture;

    var["gInput"] = pInput;
    var["gOutput"] = pOutput;

    mpComputePass->execute(pRenderContext, width, height);

    mUsePing = !mUsePing;  // 交换
}
```

### 模式2：多Pass链

```cpp
void MyPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // Pass 1: 预处理
    {
        auto var = mpPreprocessPass->getRootVar();
        var["gInput"] = pInputTexture;
        var["gOutput"] = mpTempTexture1;
        mpPreprocessPass->execute(pRenderContext, width, height);
    }

    // Pass 2: 主要处理
    {
        auto var = mpMainPass->getRootVar();
        var["gInput"] = mpTempTexture1;
        var["gOutput"] = mpTempTexture2;
        mpMainPass->execute(pRenderContext, width, height);
    }

    // Pass 3: 后处理
    {
        auto var = mpPostprocessPass->getRootVar();
        var["gInput"] = mpTempTexture2;
        var["gOutput"] = pOutputTexture;
        mpPostprocessPass->execute(pRenderContext, width, height);
    }
}
```

## 性能提示

### 1. 减少状态切换

```cpp
// 好：一次性设置所有参数
auto var = mpPass->getRootVar();
var["gTexture1"] = pTex1;
var["gTexture2"] = pTex2;
var["CB"]["param1"] = value1;
var["CB"]["param2"] = value2;
mpPass->execute(pRenderContext, width, height);

// 避免：多次调用execute
```

### 2. 重用资源

```cpp
// 好：创建一次，多次使用
if (!mpTempTexture)
{
    mpTempTexture = createTexture(...);
}

// 避免：每帧创建
// mpTempTexture = createTexture(...);  // 不要这样做！
```

### 3. 适当的资源格式

```cpp
// LDR内容使用8位
ResourceFormat::RGBA8Unorm

// HDR内容使用16位
ResourceFormat::RGBA16Float

// 只在需要时使用32位
ResourceFormat::RGBA32Float
```

## 下一步

- [05-GPU单元测试](./05-gpu-unit-testing.md): 学习如何测试你的Shader
- [06-图像测试](./06-image-testing.md): 学习如何进行图像测试
- [08-Mogwai热更新](./08-hot-reload-mogwai.md): 学习如何实时调试Shader

## FilmGrain完整代码参考

参见：
- `Source/RenderPasses/FilmGrain/FilmGrain.h` - FilmGrain.h:28
- `Source/RenderPasses/FilmGrain/FilmGrain.cpp` - FilmGrain.cpp:30
- `Source/RenderPasses/FilmGrain/FilmGrain.cs.slang` - FilmGrain.cs.slang:1
