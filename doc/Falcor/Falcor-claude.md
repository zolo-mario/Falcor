---
modified: 2026-02-11T10:09:55+08:00
created: 2026-02-10T22:26:29+08:00
tags:
  - 2026/02/10
---

## Source/Core/API 架构文档

### 概述

`Source/Core/API` 目录是 Falcor 的图形 API 抽象层核心，提供了统一的接口来支持 D3D12 和 Vulkan。该目录包含 **95 个文件**，分为主 API 目录和 Shared 子目录（D3D12 特定实现）。

### 目录结构

**主 API 目录** (`F:/Falcor/Source/Falcor/Core/API/`)：
- 核心抽象层文件
- 上下文管理（RenderContext, ComputeContext, CopyContext）
- 资源管理（Buffer, Texture, Resource）
- 状态对象（GraphicsStateObject, ComputeStateObject, RasterizerState 等）
- 视图和采样器（ResourceViews, Sampler）
- 同步机制（Fence, QueryHeap）
- 光线追踪支持（RtAccelerationStructure, RtStateObject）
- 设备和格式管理

**Shared 子目录** (`F:/Falcor/Source/Falcor/Core/API/Shared/`)：
- D3D12 特定实现
- 描述符管理（D3D12DescriptorHeap, D3D12DescriptorPool, D3D12DescriptorSet）
- 根签名处理（D3D12RootSignature）
- 常量缓冲区视图（D3D12ConstantBufferView）
- 测试用模拟暂存缓冲区

---

### 核心抽象接口

#### 1. 资源层次结构

资源系统基于 `Resource` 抽象基类构建：

```
Resource (抽象基类)
├── Buffer (缓冲区)
├── Texture (纹理)
│   ├── Texture1D
│   ├── Texture2D
│   ├── Texture3D
│   ├── TextureCube
│   └── Texture2DMultisample
└── RtAccelerationStructure (光线追踪加速结构)
```

**关键概念**：
- **Resource::Type**: 定义资源类型的枚举
- **Resource::State**: 跟踪资源状态（Undefined, Common, VertexBuffer, ConstantBuffer, RenderTarget, UnorderedAccess, DepthStencil, ShaderResource 等）
- **ResourceBindFlags**: 定义资源绑定方式（ShaderResource, UnorderedAccess, RenderTarget, DepthStencil 等）
- **Resource::getGfxResource()**: 返回底层 gfx::IResource 用于 API 特定操作

#### 2. 上下文层次结构

命令执行通过上下文层次结构实现：

```
CopyContext (基类)
├── ComputeContext (计算上下文)
│   └── RenderContext (渲染上下文)
```

**职责划分**：
- **CopyContext**: 缓冲区/纹理复制、围栏信号、命令提交
- **ComputeContext**: 计算着色器调度、UAV 操作、间接调度
- **RenderContext**: 图形渲染、FBO 操作、绘制调用、光线追踪

---

### 资源管理系统

#### Buffer 管理 (Buffer.h)

**内存类型**：
- `DeviceLocal`: GPU 本地内存（高性能）
- `Upload`: CPU 可写，GPU 可读（上传数据）
- `ReadBack`: GPU 可写，CPU 可读（回读结果）

**Buffer 创建**：
- 支持类型化和非类型化缓冲区
- 格式支持（用于类型化 UAV 加载）

**支持的 UAV 格式**：
- 保证支持：R32Float, R32Uint, R32Int
- 可选集合：RGBA32Float/Uint/Int, RG32Float/Uint/Int
- 单独可选：R16Uint/Int, R8Uint/Int, float2, uint2, int2, float3

#### Texture 管理 (Texture.h)

**SubresourceLayout**: 描述纹理内存布局
- `rowSize`: 行大小（字节）
- `rowSizeAligned`: 对齐后的行大小
- `rowCount`: 行数
- `depth`: 深度

**纹理属性**：
- 宽度、高度、深度
- 数组大小、Mip 级别
- 采样数（多重采样）

**纹理类型**：1D, 2D, 3D, Cube, 2D Multisample

#### 资源视图 (ResourceViews.h)

**视图类型**：
- **ShaderResourceView (SRV)**: 着色器只读访问
- **UnorderedAccessView (UAV)**: 着色器读写访问
- **RenderTargetView (RTV)**: 渲染目标
- **DepthStencilView (DSV)**: 深度模板

**ResourceViewInfo**: 存储视图参数（Mip 级别、数组切片、缓冲区偏移）

---

### 渲染管线组件

#### 1. 状态对象

**GraphicsStateObject** (GraphicsStateObject.h)：
封装完整的图形管线状态，包含：
- FBO 描述符（渲染目标配置）
- 顶点布局
- 程序内核（着色器）
- 光栅化状态
- 深度模板状态
- 混合状态
- 采样掩码
- 图元类型（Point, Line, Triangle, Patch）

**RasterizerState** (RasterizerState.h)：
- 剔除模式：None, Front, Back
- 填充模式：Wireframe, Solid
- 深度偏移、斜率缩放深度偏移
- 裁剪启用、多重采样启用

**DepthStencilState** (DepthStencilState.h)：
- 深度测试启用/函数
- 深度写入启用
- 模板操作（正面/背面）
- 模板参考值

**BlendState** (BlendState.h)：
- 每个渲染目标的混合配置
- 混合因子、操作
- Alpha-to-Coverage 支持

#### 2. 采样器管理 (Sampler.h)

**过滤模式**：Point, Linear
**寻址模式**：Wrap, Mirror, Clamp, Border, MirrorOnce
**采样器特性**：
- 各向异性过滤
- 比较函数
- 边界颜色指定
- LOD 偏移和钳制

---

### 命令执行和同步机制

#### 1. Fence 同步 (Fence.h)

**主机-设备同步模式**：
```cpp
ref<Fence> fence = device->createFence();
// 调度设备工作
renderContext->signal(fence);  // 从设备发出信号
fence->wait();                 // 在主机上等待
```

**Fence 特性**：
- 64 位围栏值
- 自动递增能力（Fence::kAuto）
- 共享围栏支持
- 无限超时支持

#### 2. Query Heap (QueryHeap.h)

支持的查询类型：
- GPU 性能查询
- 时间戳查询
- 遮挡查询
- 管线统计

#### 3. Framebuffer Objects (FBO.h)

**功能**：
- 渲染目标附件管理
- 深度模板附件
- 多个颜色目标支持
- 附件类型标志（Color, Depth, Stencil, All）

---

### 设备和上下文管理

#### Device 类 (Device.h)

**设备类型**：
- Default（优先选择 D3D12 而非 Vulkan）
- D3D12
- Vulkan

**设备描述符 (Device::Desc)**：
- 设备类型选择
- GPU 索引
- 调试层启用
- Aftermath GPU 崩溃转储支持
- 着色器缓存配置
- 光线追踪验证

**设备能力 (Device::SupportedFeatures)**：
- 可编程采样位置（Partial/Full）
- 重心坐标
- 光线追踪（Tier 1.1 支持）
- 保守光栅化（Tier 1/2/3）
- 光栅化顺序视图（ROVs）
- Wave 操作
- 着色器执行重排序（SER）

**设备限制 (Device::Limits)**：
- 最大计算调度线程组
- 最大着色器可见采样器数量

**设备信息 (Device::Info)**：
- 适配器名称
- 适配器 LUID
- API 名称

---

### 图形 API 抽象层

#### GFXAPI (GFXAPI.h/GFXAPI.cpp)

**核心功能**：
- 基于 Slang 的 gfx 库构建抽象层
- 为 D3D12 和 Vulkan 提供统一接口

**使用的关键 gfx 接口**：
- `gfx::IDevice`: 设备接口
- `gfx::ICommandQueue`: 命令队列
- `gfx::IResource`: 资源接口
- `gfx::ITextureResource`: 纹理资源
- `gfx::IPipelineState`: 管线状态
- `gfx::IResourceView`: 资源视图
- `gfx::IShaderObject`: 着色器对象

#### Native Handle 支持

**NativeHandle.h / NativeHandleTraits.h**：
- 封装原生 API 句柄（D3D12 COM 对象、Vulkan 句柄）
- 类型安全的句柄管理
- 自动引用计数

---

### D3D12 特定实现 (Shared 子目录)

#### 描述符管理

**D3D12DescriptorHeap** (D3D12DescriptorHeap.h/cpp)：
- 管理 GPU 描述符堆
- 为资源分配描述符
- 处理描述符失效

**D3D12DescriptorPool** (D3D12DescriptorPool.h/cpp)：
- 描述符堆池
- 高效的描述符分配
- 跨帧重用堆

**D3D12DescriptorSet** (D3D12DescriptorSet.h/cpp)：
- 着色器的描述符集合
- 管理描述符表布局
- 处理描述符更新

#### 根签名和常量缓冲区

**D3D12RootSignature** (D3D12RootSignature.h/cpp)：
- 定义着色器参数布局
- 根参数、描述符表
- 静态采样器

**D3D12ConstantBufferView** (D3D12ConstantBufferView.h/cpp)：
- 封装 D3D12 常量缓冲区视图
- 管理 CBV 描述符

---

### 光线追踪支持

#### RtAccelerationStructure (RtAccelerationStructure.h/cpp)

**功能**：
- 底层和顶层加速结构（BLAS/TLAS）
- 构建标志和几何描述符
- 压缩支持

#### RtStateObject (RtStateObject.h/cpp)

**功能**：
- 光线追踪管线状态
- 着色器关联
- Hit Group 定义

#### RtAccelerationStructurePostBuildInfoPool

**功能**：
- 管理构建后信息查询
- 压缩大小查询
- 序列化支持

#### ShaderTable.h

**功能**：
- 光线追踪的着色器表管理
- Hit Group 记录
- Miss 着色器记录

---

### 参数和着色器绑定

#### ParameterBlock (ParameterBlock.h/cpp)

**功能**：
- 保存着色器参数数据
- 支持嵌套类型
- 变量设置和获取
- 资源绑定（缓冲区、纹理、采样器）
- 着色器对象封装（gfx::IShaderObject）

#### ShaderResourceType.h

**功能**：
- 枚举着色器资源类型
- 反射的类型信息

#### Handles.h

**功能**：
- 强引用和弱引用句柄
- 可断开引用（用于打破循环依赖）

---

### 工具和辅助组件

#### Formats (Formats.h/Formats.cpp)

**功能**：
- ResourceFormat 枚举（全面的格式支持）
- 格式属性和转换
- 格式验证

#### Types (Types.h/Types.cpp)

**功能**：
- 核心类型定义
- 向量类型（uint2, uint3, uint4, float2, float3, float4）
- 矩阵类型

#### GpuTimer (GpuTimer.h/cpp)

**功能**：
- GPU 性能计时
- 时间戳查询
- 帧时间分析

#### GpuMemoryHeap (GpuMemoryHeap.h/cpp)

**功能**：
- GPU 内存分配
- 堆管理
- 内存池化

#### FencedPool.h

**功能**：
- 基于帧的资源池化模板
- 基于围栏值的自动清理

#### Aftermath (Aftermath.h/Aftermath.cpp)

**功能**：
- NVIDIA Aftermath GPU 崩溃诊断
- 崩溃转储捕获和分析
- 执行跟踪的标记支持

---

### 设计模式和架构关系

#### 1. 抽象模式

**特点**：
- 所有图形 API 调用通过 gfx:: 接口
- 平台特定代码隔离在 Shared/ 子目录
- Device 作为所有资源的工厂

#### 2. 引用计数

**特点**：
- 使用 `ref<T>` 智能指针（基于 Slang ComPtr）
- 自动资源清理
- 通过 BreakableReference 处理循环依赖

#### 3. 状态管理

**特点**：
- 不可变状态对象（RasterizerState, BlendState 等）
- 用于创建的状态描述符
- 用于缓存的相等运算符

#### 4. 资源绑定

**特点**：
- ParameterBlock 用于着色器参数
- ResourceViews 用于资源访问
- 描述符管理抽象

#### 5. 命令记录

**特点**：
- 不同命令类型的上下文层次结构
- 待处理命令跟踪
- 基于围栏的同步

#### 6. 内存管理

**特点**：
- MemoryType 枚举（DeviceLocal, Upload, ReadBack）
- 自动暂存缓冲区管理
- GPU 内存堆抽象

---

### 关键文件关系和依赖

#### 核心依赖关系

```
Device.h
├── Buffer.h, Texture.h (资源创建)
├── RenderContext.h, ComputeContext.h (命令执行)
├── Sampler.h, Fence.h (资源管理)
├── GraphicsStateObject.h (管线状态)
└── GpuMemoryHeap.h (内存管理)

RenderContext.h
├── ComputeContext.h (继承)
├── GraphicsStateObject.h (管线绑定)
├── FBO.h (渲染目标管理)
├── Texture.h, Buffer.h (资源绑定)
└── RtAccelerationStructure.h (光线追踪)

ParameterBlock.h
├── Buffer.h, Texture.h, Sampler.h (资源绑定)
├── RtAccelerationStructure.h (光线追踪资源)
└── ProgramReflection.h (类型信息)
```

---

### 架构优势总结

1. **清晰的抽象**: API 无关代码与平台特定实现完全分离
2. **层次化上下文设计**: 从复制 → 计算 → 渲染操作的逻辑递进
3. **灵活的资源管理**: 支持多种内存类型和资源视图
4. **全面的状态管理**: 不可变状态对象，支持相等比较以便缓存
5. **现代 C++ 模式**: 智能指针、移动语义、模板元编程
6. **可扩展性**: 易于添加新的状态类型、资源视图或上下文操作
7. **GPU 同步**: 基于围栏的健壮同步机制，支持自动递增
8. **光线追踪集成**: 对 DXR/Vulkan 光线追踪的一流支持
9. **性能分析**: 内置 GPU 计时器和查询支持
10. **调试支持**: Aftermath GPU 崩溃诊断、调试层支持

---

### 关键文件索引

**核心抽象**：
- `Device.h/cpp`: 设备管理和资源工厂
- `Resource.h/cpp`: 资源基类
- `Buffer.h/cpp`: 缓冲区管理
- `Texture.h/cpp`: 纹理管理

**上下文和命令**：
- `CopyContext.h/cpp`: 复制操作
- `ComputeContext.h/cpp`: 计算操作
- `RenderContext.h/cpp`: 渲染操作

**状态对象**：
- `GraphicsStateObject.h/cpp`: 图形管线状态
- `RasterizerState.h/cpp`: 光栅化状态
- `DepthStencilState.h/cpp`: 深度模板状态
- `BlendState.h/cpp`: 混合状态

**资源视图和采样器**：
- `ResourceViews.h/cpp`: SRV/UAV/RTV/DSV
- `Sampler.h/cpp`: 纹理采样器

**同步和查询**：
- `Fence.h/cpp`: 围栏同步
- `QueryHeap.h/cpp`: GPU 查询

**光线追踪**：
- `RtAccelerationStructure.h/cpp`: 加速结构
- `RtStateObject.h/cpp`: 光线追踪管线状态
- `ShaderTable.h`: 着色器表

**D3D12 特定**：
- `Shared/D3D12DescriptorHeap.h/cpp`: 描述符堆
- `Shared/D3D12DescriptorPool.h/cpp`: 描述符池
- `Shared/D3D12DescriptorSet.h/cpp`: 描述符集
- `Shared/D3D12RootSignature.h/cpp`: 根签名

**工具和辅助**：
- `Formats.h/cpp`: 格式定义和转换
- `GpuTimer.h/cpp`: GPU 计时
- `GpuMemoryHeap.h/cpp`: 内存堆管理
- `Aftermath.h/cpp`: GPU 崩溃诊断

---

## 同步机制详细分析

### 1. Fence 实现详解

#### 1.1 核心架构

**Fence 结构** (Fence.h/cpp)：
```cpp
struct FenceDesc {
    bool initialValue{0};
    bool shared{false};
};

class Fence : public Object {
    static constexpr uint64_t kAuto = std::numeric_limits<uint64_t>::max();
    static constexpr uint64_t kTimeoutInfinite = std::numeric_limits<uint64_t>::max();

    // 内部状态
    BreakableReference<Device> mpDevice;
    FenceDesc mDesc;
    Slang::ComPtr<gfx::IFence> mGfxFence;  // GFX 抽象层
    uint64_t mSignaledValue{0};             // 主机端最后信号值副本
};
```

**设计原则**：
- 64 位无符号整数围栏值用于 GPU 同步
- 主机维护最后信号值的副本（`mSignaledValue`）
- 默认单调递增（自动递增模式）
- 支持共享和非共享围栏模式
- 封装 GFX 抽象层（`gfx::IFence`）

#### 1.2 信号和等待机制

**信号操作（主机端）**：
```cpp
uint64_t Fence::signal(uint64_t value) {
    uint64_t signalValue = updateSignaledValue(value);
    FALCOR_GFX_CALL(mGfxFence->setCurrentValue(signalValue));
    return signalValue;
}

uint64_t Fence::updateSignaledValue(uint64_t value) {
    mSignaledValue = value == kAuto ? mSignaledValue + 1 : value;
    return mSignaledValue;
}
```

**等待操作（主机端）**：
```cpp
void Fence::wait(uint64_t value, uint64_t timeoutNs) {
    uint64_t waitValue = value == kAuto ? mSignaledValue : value;
    uint64_t currentValue = getCurrentValue();
    if (currentValue >= waitValue)
        return;  // 已经信号，无需等待

    gfx::IFence* fences[] = {mGfxFence};
    uint64_t waitValues[] = {waitValue};
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->waitForFences(
        1, fences, waitValues, true, timeoutNs));
}
```

**自动递增功能**：
- 当 `value == kAuto` 时，围栏值自动递增 1
- 简化顺序 GPU 工作的常见用例
- 主机维护 `mSignaledValue` 以跟踪预期的 GPU 值
- 防止手动值管理错误

#### 1.3 共享围栏支持

**共享句柄获取**：
```cpp
SharedResourceApiHandle Fence::getSharedApiHandle() const {
    gfx::InteropHandle sharedHandle;
    FALCOR_GFX_CALL(mGfxFence->getSharedHandle(&sharedHandle));
    return (SharedResourceApiHandle)sharedHandle.handleValue;
}

NativeHandle Fence::getNativeHandle() const {
    gfx::InteropHandle gfxNativeHandle = {};
    FALCOR_GFX_CALL(mGfxFence->getNativeHandle(&gfxNativeHandle));
#if FALCOR_HAS_D3D12
    if (mpDevice->getType() == Device::Type::D3D12)
        return NativeHandle(reinterpret_cast<ID3D12Fence*>(gfxNativeHandle.handleValue));
#endif
    return {};
}
```

**共享围栏特性**：
- 通过 `getSharedApiHandle()` 支持进程间同步
- D3D12 原生句柄访问用于高级场景
- Vulkan 支持当前未实现
- 支持跨进程的 GPU-to-GPU 同步

#### 1.4 CPU-GPU 同步模式

**设备端等待（异步）**：
```cpp
// 在 CopyContext::wait() 中
void CopyContext::wait(Fence* pFence, uint64_t value) {
    uint64_t waitValue = value == Fence::kAuto ? pFence->getSignaledValue() : value;
    gfx::IFence* fences[] = {pFence->getGfxFence()};
    uint64_t waitValues[] = {waitValue};
    FALCOR_GFX_CALL(mpLowLevelData->getGfxCommandQueue()->waitForFenceValuesOnDevice(
        1, fences, waitValues));
}
```

**主机端等待（阻塞）**：
```cpp
// 在 Fence::wait() 中
void Fence::wait(uint64_t value, uint64_t timeoutNs) {
    // 阻塞 CPU 线程直到 GPU 达到围栏值
    FALCOR_GFX_CALL(mpDevice->getGfxDevice()->waitForFences(
        1, fences, waitValues, true, timeoutNs));
}
```

**同步模式**：
- **设备端**：GPU 等待围栏（CPU 非阻塞）
- **主机端**：CPU 等待围栏（CPU 线程阻塞）
- **超时支持**：可配置的纳秒级超时
- **无限等待**：默认行为使用 `kTimeoutInfinite`

---

### 2. QueryHeap 实现

#### 2.1 查询类型和结构

```cpp
class QueryHeap : public Object {
    enum class Type {
        Timestamp,      // GPU 时间戳查询
        Occlusion,      // 遮挡查询（未完全实现）
        PipelineStats   // 管线统计（未完全实现）
    };

    static constexpr uint32_t kInvalidIndex = 0xffffffff;

    // 内部状态
    BreakableReference<Device> mpDevice;
    Slang::ComPtr<gfx::IQueryPool> mGfxQueryPool;
    uint32_t mCount = 0;
    uint32_t mCurrentObject = 0;
    std::deque<uint32_t> mFreeQueries;  // 已释放查询的池
    Type mType;
};
```

**查询类型支持**：
- **Timestamp**：主要用例 - GPU 时间戳查询用于性能测量
- **Occlusion**：可见性测试（基础设施存在但使用有限）
- **PipelineStats**：GPU 管线统计（基础设施存在但使用有限）

#### 2.2 查询分配和管理

```cpp
uint32_t QueryHeap::allocate() {
    if (mFreeQueries.size()) {
        uint32_t entry = mFreeQueries.front();
        mFreeQueries.pop_front();
        return entry;
    }
    if (mCurrentObject < mCount)
        return mCurrentObject++;
    else
        return kInvalidIndex;  // 查询耗尽
}

void QueryHeap::release(uint32_t entry) {
    FALCOR_ASSERT(entry != kInvalidIndex);
    mFreeQueries.push_back(entry);  // 返回到空闲池
}
```

**分配策略**：
- 从空闲池进行首次适配分配
- 从未使用的索引进行顺序分配
- 耗尽时返回 `kInvalidIndex`
- 已释放查询的 FIFO 重用模式

#### 2.3 时间戳查询实现

**GpuTimer 使用示例**：
```cpp
// 在 GpuTimer::begin() 中
mpDevice->getRenderContext()->getLowLevelData()->getResourceCommandEncoder()->writeTimestamp(
    mpDevice->getTimestampQueryHeap()->getGfxQueryPool(), mStart
);

// 在 GpuTimer::resolve() 中
encoder->resolveQuery(
    mpDevice->getTimestampQueryHeap()->getGfxQueryPool(),
    mStart, 2,  // 起始索引和数量
    mpResolveBuffer->getGfxBufferResource(),
    0           // 目标偏移
);
```

#### 2.4 查询结果获取

**时间戳解析流程**：
```cpp
void GpuTimer::resolve() {
    // 1. 将时间戳解析到 GPU 缓冲区
    auto encoder = mpDevice->getRenderContext()->getLowLevelData()->getResourceCommandEncoder();
    encoder->resolveQuery(
        mpDevice->getTimestampQueryHeap()->getGfxQueryPool(),
        mStart, 2,
        mpResolveBuffer->getGfxBufferResource(),
        0
    );

    // 2. 复制到暂存缓冲区以供 CPU 回读
    mpDevice->getRenderContext()->copyResource(mpResolveStagingBuffer.get(), mpResolveBuffer.get());

    mDataPending = true;
}

double GpuTimer::getElapsedTime() {
    if (mDataPending) {
        uint64_t result[2];
        uint64_t* pRes = (uint64_t*)mpResolveStagingBuffer->map();
        result[0] = pRes[0];
        result[1] = pRes[1];
        mpResolveStagingBuffer->unmap();

        double start = (double)result[0];
        double end = (double)result[1];
        double range = end - start;
        mElapsedTime = range * mpDevice->getGpuTimestampFrequency();
        mDataPending = false;
    }
    return mElapsedTime;
}
```

**查询结果管线**：
1. 在 GPU 执行期间将时间戳写入查询堆
2. 将查询解析到 GPU 缓冲区
3. 将 GPU 缓冲区复制到 CPU 可读的暂存缓冲区
4. 映射暂存缓冲区并读取结果
5. 使用 GPU 时间戳频率计算经过时间

---

### 3. 其他同步原语

#### 3.1 FencedPool - 基于围栏的资源池

```cpp
template<typename ObjectType>
class FencedPool : public Object {
    using NewObjectFuncType = ObjectType (*)(void*);

    struct Data {
        ObjectType alloc;
        uint64_t timestamp;  // 对象退役时的围栏值
    };

    ObjectType newObject() {
        // 退役活动对象
        Data data;
        data.alloc = mActiveObject;
        data.timestamp = mpFence->getSignaledValue();
        mQueue.push(data);

        // 检查队列中的第一个对象是否空闲
        data = mQueue.front();
        if (data.timestamp < mpFence->getCurrentValue()) {
            // 对象空闲，重用它
            mQueue.pop();
        } else {
            // 对象仍在使用中，创建新对象
            data.alloc = createObject();
        }

        mActiveObject = data.alloc;
        return mActiveObject;
    }
};
```

**用途**：
- 管理 GPU 资源池（命令缓冲区、描述符堆等）
- 通过仅在 GPU 完成使用后重用资源来防止 GPU 停顿
- 跟踪围栏值以确定何时可以安全重用资源
- 需要时自动创建新资源

**使用场景**：
- 命令缓冲区池化
- 描述符堆管理
- 瞬态资源分配

#### 3.2 资源状态转换和屏障

**资源屏障同步**：
```cpp
bool CopyContext::resourceBarrier(const Resource* pResource, Resource::State newState,
    const ResourceViewInfo* pViewInfo) {
    const Texture* pTexture = dynamic_cast<const Texture*>(pResource);
    if (pTexture) {
        bool globalBarrier = pTexture->isStateGlobal();
        if (globalBarrier) {
            return textureBarrier(pTexture, newState);
        } else {
            return subresourceBarriers(pTexture, newState, pViewInfo);
        }
    } else {
        const Buffer* pBuffer = dynamic_cast<const Buffer*>(pResource);
        return bufferBarrier(pBuffer, newState);
    }
}

bool CopyContext::textureBarrier(const Texture* pTexture, Resource::State newState) {
    auto resourceEncoder = getLowLevelData()->getResourceCommandEncoder();
    bool recorded = false;
    if (pTexture->getGlobalState() != newState) {
        gfx::ITextureResource* textureResource = pTexture->getGfxTextureResource();
        resourceEncoder->textureBarrier(
            1, &textureResource,
            getGFXResourceState(pTexture->getGlobalState()),
            getGFXResourceState(newState)
        );
        mCommandsPending = true;
        recorded = true;
    }
    pTexture->setGlobalState(newState);
    return recorded;
}
```

**UAV 屏障（无序访问视图）**：
```cpp
void CopyContext::uavBarrier(const Resource* pResource) {
    auto resourceEncoder = getLowLevelData()->getResourceCommandEncoder();

    if (pResource->getType() == Resource::Type::Buffer) {
        gfx::IBufferResource* bufferResource =
            static_cast<gfx::IBufferResource*>(pResource->getGfxResource());
        resourceEncoder->bufferBarrier(1, &bufferResource,
            gfx::ResourceState::UnorderedAccess,
            gfx::ResourceState::UnorderedAccess);
    } else {
        gfx::ITextureResource* textureResource =
            static_cast<gfx::ITextureResource*>(pResource->getGfxResource());
        resourceEncoder->textureBarrier(1, &textureResource,
            gfx::ResourceState::UnorderedAccess,
            gfx::ResourceState::UnorderedAccess);
    }
    mCommandsPending = true;
}
```

**屏障类型**：
- **纹理屏障**：全局和每个子资源的转换
- **缓冲区屏障**：缓冲区的状态转换
- **UAV 屏障**：UAV 访问之间的同步
- **子资源屏障**：细粒度的每个 Mip/数组切片控制

#### 3.3 帧同步机制

**提交和等待模式**：
```cpp
void CopyContext::submit(bool wait) {
    if (mCommandsPending) {
        mpLowLevelData->submitCommandBuffer();
        mCommandsPending = false;
    } else {
        // 即使没有命令也发出信号以确保资源清理
        signal(mpLowLevelData->getFence().get());
    }

    bindDescriptorHeaps();

    if (wait) {
        mpLowLevelData->getFence()->wait();  // 阻塞直到 GPU 完成
    }
}
```

**命令缓冲区提交**：
```cpp
void LowLevelContextData::submitCommandBuffer() {
    closeCommandBuffer();
    mpGfxCommandQueue->executeCommandBuffers(
        1,
        mGfxCommandBuffer.readRef(),
        mpFence->getGfxFence(),
        mpFence->updateSignaledValue()  // 自动递增围栏
    );
    openCommandBuffer();
}
```

---

### 4. 同步使用模式

#### 4.1 在 RenderContext/ComputeContext 中的使用

**上下文层次结构**：
```
CopyContext (基类)
    ├─ ComputeContext
    │   └─ RenderContext
```

**CopyContext 中的同步**：
```cpp
class CopyContext {
    // 设备端围栏等待
    void wait(Fence* pFence, uint64_t value = Fence::kAuto);

    // 设备端围栏信号
    uint64_t signal(Fence* pFence, uint64_t value = Fence::kAuto);

    // 提交命令，可选主机等待
    virtual void submit(bool wait = false);
};
```

**信号操作（设备端）**：
```cpp
uint64_t CopyContext::signal(Fence* pFence, uint64_t value) {
    uint64_t signalValue = pFence->updateSignaledValue(value);
    mpLowLevelData->getGfxCommandQueue()->executeCommandBuffers(
        0, nullptr,  // 无命令缓冲区
        pFence->getGfxFence(),
        signalValue
    );
    return signalValue;
}
```

#### 4.2 命令提交和同步流程

**典型工作流程**：
```cpp
// 1. 调度 GPU 工作
renderContext->draw(...);
renderContext->dispatch(...);

// 2. 工作后发出围栏信号
ref<Fence> fence = device->createFence();
renderContext->signal(fence);

// 3. 调度更多工作
renderContext->draw(...);

// 4. 在设备上等待围栏（GPU 等待）
renderContext->wait(fence);

// 5. 提交所有命令
renderContext->submit(false);  // 不阻塞 CPU

// 6. 稍后，如果需要在 CPU 上等待
fence->wait();
```

**异步纹理读取示例**：
```cpp
CopyContext::ReadTextureTask::SharedPtr CopyContext::ReadTextureTask::create(
    CopyContext* pCtx, const Texture* pTexture, uint32_t subresourceIndex) {

    // ... 设置代码 ...

    // 创建围栏用于同步
    pThis->mpFence = pCtx->getDevice()->createFence();
    pThis->mpFence->breakStrongReferenceToDevice();

    // 提交复制命令
    pCtx->submit(false);

    // 复制后发出围栏信号
    pCtx->signal(pThis->mpFence.get());

    return pThis;
}

void CopyContext::ReadTextureTask::getData(void* pData, size_t size) const {
    // 等待 GPU 完成复制
    mpFence->wait();

    // 现在可以安全地从暂存缓冲区读取
    uint8_t* pSrc = reinterpret_cast<const uint8_t*>(mpBuffer->map());
    // ... 复制数据 ...
    mpBuffer->unmap();
}
```

#### 4.3 多帧并行渲染

**帧同步策略**：
```cpp
// 多帧渲染的伪代码
class FrameManager {
    static const int NUM_FRAMES = 3;
    ref<Fence> frameFences[NUM_FRAMES];
    int currentFrame = 0;

    void beginFrame() {
        // 等待 GPU 完成此帧的资源
        frameFences[currentFrame]->wait();

        // 现在可以安全地更新帧数据
        updateFrameData();
    }

    void endFrame() {
        // 提交帧命令
        renderContext->submit(false);

        // 为此帧发出围栏信号
        renderContext->signal(frameFences[currentFrame]);

        // 移动到下一帧
        currentFrame = (currentFrame + 1) % NUM_FRAMES;
    }
};
```

**优势**：
- 防止 CPU-GPU 停顿
- 允许 2-3 帧的缓冲
- 支持 CPU 和 GPU 工作的流水线化
- 减少延迟

---

### 5. 性能考虑

#### 5.1 同步点优化

**最小化同步点**：
```cpp
// 不好：每次操作后同步
renderContext->draw(...);
renderContext->submit(true);  // 等待 GPU
renderContext->draw(...);
renderContext->submit(true);  // 等待 GPU

// 好：批处理操作并同步一次
renderContext->draw(...);
renderContext->draw(...);
renderContext->draw(...);
renderContext->submit(true);  // 一次等待所有
```

**避免不必要的等待**：
```cpp
// 在 CopyContext::submit() 中
if (mCommandsPending) {
    mpLowLevelData->submitCommandBuffer();
} else {
    // 即使没有命令也发出围栏信号
    // 这确保资源清理发生
    signal(mpLowLevelData->getFence().get());
}
```

#### 5.2 异步操作

**异步纹理回读**：
```cpp
// 非阻塞回读
auto task = renderContext->asyncReadTextureSubresource(texture, 0);

// 做其他工作...

// 稍后，获取结果（如果未准备好则阻塞）
auto data = task->getData();
```

**优势**：
- CPU 不会阻塞等待 GPU
- 允许 CPU 和 GPU 工作重叠
- 减少帧延迟

#### 5.3 资源重用和池化

**FencedPool 模式**：
```cpp
// 通过跟踪围栏值防止 GPU 停顿
auto pool = FencedPool::create(fence, createCommandBuffer);

// 获取资源（如果 GPU 完成则重用旧资源）
auto cmdBuffer = pool->newObject();

// 使用 cmdBuffer...

// 下次调用将在 GPU 完成时重用，或创建新资源
auto nextCmdBuffer = pool->newObject();
```

**效率**：
- 避免资源耗尽导致的 GPU 停顿
- 安全时自动重用
- 最小的分配开销

---

### 6. 详细代码示例

#### 6.1 完整的 Fence 使用示例

```cpp
// 创建围栏
ref<Fence> fence = device->createFence();

// 调度 GPU 工作 1
renderContext->draw(state1, vars1, ...);

// 工作 1 后发出围栏信号
renderContext->signal(fence);

// 调度 GPU 工作 2
renderContext->draw(state2, vars2, ...);

// 设备在工作 2 之前等待围栏
renderContext->wait(fence);

// 提交所有命令
renderContext->submit(false);

// CPU 可以在这里做其他工作...

// 稍后，等待 GPU 完成
fence->wait();

// 现在可以安全地读取结果
auto data = readBuffer(resultBuffer);
```

#### 6.2 GPU Timer 使用

```cpp
ref<GpuTimer> timer = GpuTimer::create(device);

// 开始测量
timer->begin();

// 要测量的 GPU 工作
renderContext->draw(...);
renderContext->dispatch(...);

// 结束测量
timer->end();

// 提交并等待 GPU
renderContext->submit(true);

// 解析时间戳
timer->resolve();

// 获取经过时间
double elapsedMs = timer->getElapsedTime();
```

#### 6.3 渲染通道中的同步

```cpp
class MyRenderPass {
    void execute(RenderContext* pCtx, const RenderData& renderData) {
        // 确保输入资源处于正确状态
        pCtx->resourceBarrier(inputTexture, Resource::State::ShaderResource);
        pCtx->resourceBarrier(outputTexture, Resource::State::RenderTarget);

        // 绘制
        pCtx->draw(state, vars, vertexCount, 0);

        // 为下一个通道转换输出
        pCtx->resourceBarrier(outputTexture, Resource::State::ShaderResource);
    }
};
```

---

### 7. 同步机制总结

| 组件 | 用途 | 关键特性 |
|------|------|----------|
| **Fence** | CPU-GPU 同步 | 64 位值、自动递增、共享支持 |
| **QueryHeap** | GPU 性能查询 | 时间戳、遮挡、管线统计 |
| **FencedPool** | 资源池化 | 自动重用、基于围栏的跟踪 |
| **Barriers** | 资源状态同步 | 纹理、缓冲区、UAV、子资源 |
| **Submit/Wait** | 命令提交 | 异步设备端、阻塞主机端 |
| **GpuTimer** | 性能测量 | 时间戳查询与 CPU 回读 |

**关键要点**：
- **Fence** 是主要的同步原语，支持 CPU-GPU 和 GPU-GPU 同步
- **自动递增模式**简化了顺序工作的围栏管理
- **设备端等待**（GPU 等待）不阻塞 CPU，实现异步操作
- **主机端等待**（CPU 等待）阻塞 CPU 线程，用于结果回读
- **资源屏障**确保资源状态转换的正确性
- **FencedPool** 通过跟踪围栏值防止资源重用导致的 GPU 停顿
- **多帧缓冲**（2-3 帧）通过流水线化 CPU 和 GPU 工作减少延迟
- **最小化同步点**是性能优化的关键策略

---

## Shader 代码详细分析

### 1. VBufferRaster 实现

#### 1.1 VBufferRaster.3d.slang - 光栅化管线

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/VBufferRaster.3d.slang`

**架构概述**：
- **顶点着色器 (vsMain)**：将几何体转换到裁剪空间并准备每顶点数据
- **像素着色器 (psMain)**：使用重心坐标存储可见性缓冲区信息
- **关键特性**：使用 SV_Barycentrics 获取精确的三角形命中信息

**顶点着色器实现**：
```hlsl
VBufferVSOut vsMain(VSIn vsIn)
{
    VBufferVSOut vsOut;
    const GeometryInstanceID instanceID = { vsIn.instanceID };

    // 世界空间变换
    float4x4 worldMat = gScene.getWorldMatrix(instanceID);
    float3 posW = mul(worldMat, float4(vsIn.pos, 1.f)).xyz;
    vsOut.posH = mul(gScene.camera.getViewProj(), float4(posW, 1.f));

    vsOut.texC = vsIn.texC;
    vsOut.instanceID = instanceID;
    vsOut.materialID = gScene.getMaterialID(instanceID);

    // 运动矢量计算用于时间一致性
    #if is_valid(gMotionVector)
    float3 prevPos = vsIn.pos;
    GeometryInstanceData instance = gScene.getGeometryInstance(instanceID);
    if (instance.isDynamic())
    {
        uint prevVertexIndex = gScene.meshes[instance.geometryID].prevVbOffset + vsIn.vertexID;
        prevPos = gScene.prevVertices[prevVertexIndex].position;
    }
    float3 prevPosW = mul(gScene.getPrevWorldMatrix(instanceID), float4(prevPos, 1.f)).xyz;
    vsOut.prevPosH = mul(gScene.camera.data.prevViewProjMatNoJitter, float4(prevPosW, 1.f));
    #endif

    return vsOut;
}
```

**关键技术**：
- **每实例数据**：存储 instanceID 和 materialID 作为非插值值
- **运动矢量**：计算前一帧位置用于时间效果
- **动态对象**：通过前一帧顶点缓冲区处理动画几何体

**像素着色器实现**：
```hlsl
VBufferPSOut psMain(VBufferVSOut vsOut, uint triangleIndex: SV_PrimitiveID,
                     float3 barycentrics: SV_Barycentrics)
{
    VBufferPSOut psOut;
    int2 ipos = int2(vsOut.posH.xy);

    // Alpha 测试用于透明度
    #if USE_ALPHA_TEST
    VertexData v = {};
    v.texC = vsOut.texC;
    float lod = 0.f;
    if (gScene.materials.alphaTest(v, vsOut.materialID, lod))
        discard;
    #endif

    // 可选的遮罩输出
    if (is_valid(gMask))
        gMask[ipos] = 1.f;

    // 使用重心坐标存储命中信息
    TriangleHit triangleHit;
    triangleHit.instanceID = vsOut.instanceID;
    triangleHit.primitiveIndex = triangleIndex;
    triangleHit.barycentrics = barycentrics.yz;  // 存储 B,C (A = 1-B-C)
    psOut.packedHitInfo = triangleHit.pack();

    // 运动矢量计算
    #if is_valid(gMotionVector)
    {
        const float2 pixelPos = ipos + float2(0.5, 0.5);
        const float4 prevPosH = vsOut.prevPosH;
        const float2 mv = calcMotionVector(pixelPos, prevPosH, gFrameDim) +
                          float2(gScene.camera.data.jitterX, -gScene.camera.data.jitterY);
        gMotionVector[ipos] = mv;
    }
    #endif

    return psOut;
}
```

**深度测试优化**：
- 通过 DepthStencilState 使用硬件深度测试（LessEqual 比较）
- 光栅化顺序视图（ROVs）用于原子操作
- 启用深度写入掩码以进行正确的 Z-buffer 管理

**数据打包策略**：
- **TriangleHit 结构**：编码 instanceID、primitiveIndex 和重心坐标
- **重心坐标存储**：仅存储 B、C 坐标（A 计算为 1-B-C）
- **紧凑格式**：适合 64 位 PackedHitInfo

---

### 2. MeshletRaster 实现

#### 2.1 MeshletRaster.cs.slang - 计算着色器光栅化

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/MeshletRaster.cs.slang`

**架构**：使用计算着色器进行软件光栅化，用于基于 meshlet 的渲染

**GPU Meshlet 结构**：
```hlsl
struct GpuMeshlet
{
    uint vertexOffset;      // Meshlet 顶点缓冲区偏移
    uint triangleOffset;    // Meshlet 三角形缓冲区偏移
    uint vertexCount;       // Meshlet 中的顶点数
    uint triangleCount;     // Meshlet 中的三角形数

    float3 boundCenter;     // 包围球中心（世界空间）
    float boundRadius;      // 包围球半径

    uint instanceID;        // 几何实例 ID
    uint primitiveOffset;   // 图元 ID 偏移
    uint meshID;            // 网格标识符
    uint _pad0;
};
```

**计算着色器入口点**：
```hlsl
[numthreads(32, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 groupThreadID : SV_GroupThreadID)
{
    uint meshletID = groupID.x;
    uint threadID = groupThreadID.x;

    if (meshletID >= gMeshletCount)
        return;

    GpuMeshlet meshlet = gMeshlets[meshletID];

    // 获取变换矩阵
    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = meshlet.instanceID;
    float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);
    float4x4 viewProj = gScene.camera.getViewProj();
    float4x4 worldViewProj = mul(viewProj, worldMat);

    // 处理 meshlet 中的所有三角形
    for (uint triIdx = 0; triIdx < meshlet.triangleCount; triIdx++)
    {
        // 加载三角形索引（打包为 uint8）
        uint triOffset = meshlet.triangleOffset + triIdx * 3;
        uint idx0 = gMeshletTriangles[triOffset + 0];
        uint idx1 = gMeshletTriangles[triOffset + 1];
        uint idx2 = gMeshletTriangles[triOffset + 2];

        // 获取顶点索引
        uint v0Idx = gMeshletVertices[meshlet.vertexOffset + idx0];
        uint v1Idx = gMeshletVertices[meshlet.vertexOffset + idx1];
        uint v2Idx = gMeshletVertices[meshlet.vertexOffset + idx2];

        // 加载顶点位置
        float3 pos0 = gScene.getVertex(v0Idx).position;
        float3 pos1 = gScene.getVertex(v1Idx).position;
        float3 pos2 = gScene.getVertex(v2Idx).position;

        // 转换到裁剪空间
        float4 clipPos0 = mul(worldViewProj, float4(pos0, 1.0));
        float4 clipPos1 = mul(worldViewProj, float4(pos1, 1.0));
        float4 clipPos2 = mul(worldViewProj, float4(pos2, 1.0));

        // ... 继续光栅化处理
    }
}
```

**关键算法**：

**1. 边缘函数覆盖测试**：
```hlsl
bool isInsideTriangle(float2 p, float2 v0, float2 v1, float2 v2)
{
    float2 e0 = v1 - v0;
    float2 e1 = v2 - v1;
    float2 e2 = v0 - v2;

    float c0 = cross2D(e0, p - v0);
    float c1 = cross2D(e1, p - v1);
    float c2 = cross2D(e2, p - v2);

    // 所有符号相同 = 在内部
    return (c0 >= 0 && c1 >= 0 && c2 >= 0) || (c0 <= 0 && c1 <= 0 && c2 <= 0);
}
```

**2. 重心坐标插值**：
```hlsl
float interpolateDepth(float2 p, float2 v0, float2 v1, float2 v2,
                       float d0, float d1, float d2)
{
    // 使用点积计算重心坐标
    float2 v0v1 = v1 - v0;
    float2 v0v2 = v2 - v0;
    float2 v0p = p - v0;

    float d00 = dot(v0v1, v0v1);
    float d01 = dot(v0v1, v0v2);
    float d11 = dot(v0v2, v0v2);
    float d20 = dot(v0p, v0v1);
    float d21 = dot(v0p, v0v2);

    float denom = d00 * d11 - d01 * d01;
    if (abs(denom) < 1e-8)
        return d0;

    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    return u * d0 + v * d1 + w * d2;
}
```

**3. 原子深度测试**：
```hlsl
// 将深度转换为 uint 用于原子操作
uint depthUint = asuint(depth);

// 原子深度测试 - 保留最近的深度
InterlockedMin(gVisBufferDepth[uint2(x, y)], depthUint);

// 写入负载
gVisBufferPayload[uint2(x, y)] = payload;
```

**性能优化**：
- **线程分布**：每组 32 个线程，每个处理像素子集
- **Meshlet 粒度**：每组一个 meshlet 以实现缓存一致性
- **包围盒剔除**：减少像素处理开销
- **原子操作**：最小化同步开销

---

### 3. MeshletResolve 实现

#### 3.1 MeshletResolve.cs.slang - 可见性缓冲区解码

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/MeshletResolve.cs.slang`

**目的**：将可见性缓冲区（深度 + 负载）转换为 PackedHitInfo 格式

**计算着色器实现**：
```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadID.xy;

    if (any(pixel >= gFrameDim))
        return;

    // 读取可见性缓冲区
    uint depthUint = gVisBufferDepth[pixel];
    uint payload = gVisBufferPayload[pixel];

    // 检查是否为背景
    if (payload == 0 && depthUint == 0)
    {
        // 背景像素
        gVBuffer[pixel] = 0;
        if (is_valid(gViewW))
        {
            Ray ray = gScene.camera.computeRayPinhole(pixel, gFrameDim);
            gViewW[pixel] = float4(ray.dir, 0.0);
        }
        if (is_valid(gDepth))
            gDepth[pixel] = 1.0f;
        return;
    }

    // 解码负载
    uint instanceID = payload >> 20;
    uint primitiveID = payload & 0xFFFFF;
    float depth = asfloat(depthUint);

    // 获取几何实例
    GeometryInstanceID geomInstanceID;
    geomInstanceID.index = instanceID;

    // 从场景加载三角形顶点
    uint3 vtxIndices = gScene.getIndices(geomInstanceID, primitiveID);
    StaticVertexData vtx0 = gScene.getVertex(vtxIndices[0]);
    StaticVertexData vtx1 = gScene.getVertex(vtxIndices[1]);
    StaticVertexData vtx2 = gScene.getVertex(vtxIndices[2]);

    // 转换到世界空间
    float4x4 worldMat = gScene.getWorldMatrix(geomInstanceID);
    float3 positions[3];
    positions[0] = mul(worldMat, float4(vtx0.position, 1.0)).xyz;
    positions[1] = mul(worldMat, float4(vtx1.position, 1.0)).xyz;
    positions[2] = mul(worldMat, float4(vtx2.position, 1.0)).xyz;

    // 从像素重建光线
    Ray ray = gScene.camera.computeRayPinhole(pixel, gFrameDim);

    // 通过光线-三角形相交计算重心坐标（Möller-Trumbore）
    float2 barycentrics = rayTriangleIntersection(ray.origin, ray.dir,
                                                  positions[0], positions[1], positions[2]);

    // 打包并写入输出
    TriangleHit triangleHit;
    triangleHit.instanceID = geomInstanceID;
    triangleHit.primitiveIndex = primitiveID;
    triangleHit.barycentrics = barycentrics;

    gVBuffer[pixel] = triangleHit.pack();
}
```

**Möller-Trumbore 光线-三角形相交算法**：
```hlsl
float2 rayTriangleIntersection(float3 rayOrigin, float3 rayDir,
                               float3 v0, float3 v1, float3 v2)
{
    float3 edge1 = v1 - v0;
    float3 edge2 = v2 - v0;
    float3 h = cross(rayDir, edge2);
    float a = dot(edge1, h);

    if (abs(a) < 1e-8)
        return float2(0, 0);

    float f = 1.0 / a;
    float3 s = rayOrigin - v0;
    float u = f * dot(s, h);

    float3 q = cross(s, edge1);
    float v = f * dot(rayDir, q);

    return float2(u, v);
}
```

**关键特性**：
- **负载解码**：提取 instanceID（高 12 位）和 primitiveID（低 20 位）
- **光线重建**：从像素坐标计算相机光线
- **重心坐标恢复**：使用光线-三角形相交恢复精确的重心坐标
- **运动矢量**：从前一帧位置计算时间一致性数据

---

### 4. VBufferRT 实现

#### 4.1 VBufferRT.slang - 共享光线追踪逻辑

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/VBufferRT.slang`

**VBufferRT 类结构**：
```hlsl
struct VBufferRT
{
    static const bool kComputeDepthOfField = COMPUTE_DEPTH_OF_FIELD;
    static const bool kUseAlphaTest = USE_ALPHA_TEST;
    static const uint kRayFlags = RAY_FLAGS;

    uint2 frameDim;
    uint frameCount;

    Ray generateRay(uint2 pixel)
    {
        if (kComputeDepthOfField)
        {
            SampleGenerator sg = SampleGenerator(pixel, frameCount);
            return gScene.camera.computeRayThinlens(pixel, frameDim, sampleNext2D(sg));
        }
        else
        {
            return gScene.camera.computeRayPinhole(pixel, frameDim);
        }
    }

    void writeHit(uint2 pixel, float3 rayOrigin, float3 rayDir, const HitInfo hit)
    {
        gVBuffer[pixel] = hit.pack();

        VertexData v;
        float depth = 1.f;
        float3 prevPosW = float3(0.f);

        if (hit.getType() == HitType::Triangle)
        {
            const TriangleHit triangleHit = hit.getTriangleHit();
            v = gScene.getVertexData(triangleHit);
            prevPosW = gScene.getPrevPosW(triangleHit);
        }
        // ... 其他几何类型处理
    }
};
```

**支持的几何类型**：
- 三角形网格（标准）
- 位移三角形网格（带位移贴图）
- 曲线（毛发/皮毛渲染）
- SDF 网格（有符号距离场）

#### 4.2 VBufferRT.rt.slang - DXR 光线追踪

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/VBufferRT.rt.slang`

**光线生成着色器**：
```hlsl
[shader("raygeneration")]
void rayGen()
{
    uint2 launchIndex = DispatchRaysIndex().xy;

    GpuTimer timer;
    gVBufferRT.beginTime(timer);

    // 追踪光线
    const Ray ray = gVBufferRT.generateRay(launchIndex);
    RayData rayData;
    TraceRay(
        gScene.rtAccel,
        VBufferRT::kRayFlags,
        0xff /* instanceInclusionMask */,
        0 /* hitIdx */,
        getRayTypeCount(),
        0 /* missIdx */,
        ray.toRayDesc(),
        rayData
    );

    gVBufferRT.writeAux(launchIndex, ray);
    gVBufferRT.endTime(launchIndex, timer);
}
```

**命中着色器**：
```hlsl
[shader("closesthit")]
void closestHit(inout RayData rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    TriangleHit triangleHit;
    triangleHit.instanceID = getGeometryInstanceID();
    triangleHit.primitiveIndex = PrimitiveIndex();
    triangleHit.barycentrics = attribs.barycentrics;

    uint2 launchIndex = DispatchRaysIndex().xy;
    gVBufferRT.writeHit(launchIndex, WorldRayOrigin(), WorldRayDirection(), HitInfo(triangleHit));
}

[shader("miss")]
void miss(inout RayData rayData)
{
    uint2 launchIndex = DispatchRaysIndex().xy;
    gVBufferRT.writeMiss(launchIndex, WorldRayOrigin(), WorldRayDirection());
}
```

**Alpha 测试**：
```hlsl
[shader("anyhit")]
void anyHit(inout RayData rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    if (VBufferRT::kUseAlphaTest)
    {
        GeometryInstanceID instanceID = getGeometryInstanceID();
        VertexData v = getVertexData(instanceID, PrimitiveIndex(), attribs);
        uint materialID = gScene.getMaterialID(instanceID);
        if (gScene.materials.alphaTest(v, materialID, 0.f))
            IgnoreHit();
    }
}
```

#### 4.3 VBufferRT.cs.slang - 内联光线追踪

**文件位置**：`Source/RenderPasses/GBuffer/VBuffer/VBufferRT.cs.slang`

**计算着色器实现**：
```hlsl
[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadId.xy;
    if (any(pixel >= gVBufferRT.frameDim))
        return;

    GpuTimer timer;
    gVBufferRT.beginTime(timer);

    const Ray ray = gVBufferRT.generateRay(pixel);

    SceneRayQuery<VBufferRT::kUseAlphaTest> sceneRayQuery;
    float hitT;
    const HitInfo hit = sceneRayQuery.traceRay(ray, hitT, VBufferRT::kRayFlags);
    if (hit.isValid())
    {
        gVBufferRT.writeHit(pixel, ray.origin, ray.dir, hit);
    }
    else
    {
        gVBufferRT.writeMiss(pixel, ray.origin, ray.dir);
    }

    gVBufferRT.writeAux(pixel, ray);
    gVBufferRT.endTime(pixel, timer);
}
```

**内联光线追踪的优势**：
- 无需单独的光线生成着色器
- 更好的缓存局部性
- 更简单的调试
- 减少着色器编译开销

---

### 5. MinimalPathTracer 实现

#### 5.1 MinimalPathTracer.rt.slang - 完整路径追踪

**文件位置**：`Source/RenderPasses/MinimalPathTracer/MinimalPathTracer.rt.slang`

**架构概述**：
- 从 V-buffer 加载主光线
- 递归路径追踪与散射光线
- 多种光源类型（解析光源、发光表面、环境光）
- 可配置的反弹深度和采样策略

**光线负载结构**：

**阴影光线负载**：
```hlsl
struct ShadowRayData
{
    bool visible;  // 如果光线未被终止则由 miss 着色器设置为 true
};
```

**散射光线负载（最多 72 字节）**：
```hlsl
struct ScatterRayData
{
    float3 radiance;      // 从路径累积的出射辐射度
    bool terminated;      // 路径终止时设置为 true
    float3 thp;           // 当前路径吞吐量（在每个顶点更新）
    uint pathLength;      // 路径长度（段数）
    float3 origin;        // 下一个路径段起点
    float3 direction;     // 下一个路径段方向
    SampleGenerator sg;   // 采样生成器的每光线状态（16B）

    __init(SampleGenerator sg)
    {
        this.terminated = false;
        this.pathLength = 0;
        this.radiance = float3(0, 0, 0);
        this.thp = float3(1, 1, 1);
        this.origin = float3(0, 0, 0);
        this.direction = float3(0, 0, 0);
        this.sg = sg;
    }
};
```

**着色数据加载**：
```hlsl
ShadingData loadShadingData(const HitInfo hit, const float3 rayOrigin, const float3 rayDir)
{
    VertexData v = {};
    uint materialID = {};

    // 根据几何类型加载顶点数据
    if (hit.getType() == HitType::Triangle)
    {
        const TriangleHit triangleHit = hit.getTriangleHit();
        v = gScene.getVertexData(triangleHit);
        materialID = gScene.getMaterialID(triangleHit.instanceID);
    }
    // ... 其他几何类型处理

    // 准备着色数据
    ShadingData sd = gScene.materials.prepareShadingData(v, materialID, -rayDir);
    return sd;
}
```

**光线生成着色器**：
```hlsl
[shader("raygeneration")]
void rayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint2 frameDim = DispatchRaysDimensions().xy;

    if (any(pixel >= frameDim))
        return;

    // 初始化采样生成器
    SampleGenerator sg = SampleGenerator(pixel, gFrameCount);

    // 从 V-buffer 加载主光线命中信息
    const HitInfo hit = HitInfo(gVBuffer[pixel]);

    // 初始化光线数据
    ScatterRayData rayData = ScatterRayData(sg);

    if (hit.isValid())
    {
        // 加载着色数据
        let lod = ExplicitLodTextureSampler(0.f);
        ShadingData sd = loadShadingData(hit, float3(0), float3(0), lod);

        // 创建 BSDF 实例
        let bsdf = gScene.materials.getBSDF(sd, lod);

        // 计算直接光照
        float3 Lr = evalDirect(sd, bsdf, rayData.sg);
        rayData.radiance += rayData.thp * Lr;

        // 采样 BSDF 用于间接光照
        BSDFSample bsdfSample;
        if (bsdf.sample(sd, rayData.sg, bsdfSample))
        {
            rayData.origin = sd.computeRayOrigin();
            rayData.direction = bsdfSample.wo;
            rayData.thp *= bsdfSample.weight;
            rayData.pathLength = 1;
        }
        else
        {
            rayData.terminated = true;
        }
    }
    else
    {
        // 背景命中 - 评估环境贴图
        rayData.radiance = evalEnvironmentMap(gScene.camera.computeRayPinhole(pixel, frameDim).dir);
        rayData.terminated = true;
    }

    // 追踪散射光线
    for (uint depth = 1; depth <= kMaxBounces && !rayData.terminated; depth++)
    {
        traceScatterRay(rayData);
    }

    // 写入最终颜色
    gOutputColor[pixel] = float4(rayData.radiance, 1.f);
}
```

**散射光线追踪**：
```hlsl
void traceScatterRay(inout ScatterRayData rayData)
{
    RayDesc ray;
    ray.Origin = rayData.origin;
    ray.Direction = rayData.direction;
    ray.TMin = 0.f;
    ray.TMax = kRayTMax;

    TraceRay(
        gScene.rtAccel,
        RAY_FLAG_NONE,
        0xff,
        kRayTypeScatter,
        getRayTypeCount(),
        kRayTypeScatter,
        ray,
        rayData
    );
}
```

**散射光线命中着色器**：
```hlsl
[shader("closesthit")]
void scatterClosestHit(inout ScatterRayData rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    // 构建命中信息
    TriangleHit triangleHit;
    triangleHit.instanceID = getGeometryInstanceID();
    triangleHit.primitiveIndex = PrimitiveIndex();
    triangleHit.barycentrics = attribs.barycentrics;
    HitInfo hit = HitInfo(triangleHit);

    // 加载着色数据
    let lod = ExplicitLodTextureSampler(0.f);
    ShadingData sd = loadShadingData(hit, WorldRayOrigin(), WorldRayDirection(), lod);

    // 创建 BSDF
    let bsdf = gScene.materials.getBSDF(sd, lod);

    // 累积发射
    rayData.radiance += rayData.thp * sd.emissive;

    // 评估直接光照
    float3 Lr = evalDirect(sd, bsdf, rayData.sg);
    rayData.radiance += rayData.thp * Lr;

    // 采样 BSDF 用于下一次反弹
    BSDFSample bsdfSample;
    if (bsdf.sample(sd, rayData.sg, bsdfSample))
    {
        rayData.origin = sd.computeRayOrigin();
        rayData.direction = bsdfSample.wo;
        rayData.thp *= bsdfSample.weight;
        rayData.pathLength++;
    }
    else
    {
        rayData.terminated = true;
    }

    // 俄罗斯轮盘赌终止
    if (rayData.pathLength >= kMinBounces)
    {
        float p = max(rayData.thp.x, max(rayData.thp.y, rayData.thp.z));
        if (sampleNext1D(rayData.sg) > p)
        {
            rayData.terminated = true;
        }
        else
        {
            rayData.thp /= p;
        }
    }
}

[shader("miss")]
void scatterMiss(inout ScatterRayData rayData)
{
    // 评估环境贴图
    rayData.radiance += rayData.thp * evalEnvironmentMap(WorldRayDirection());
    rayData.terminated = true;
}
```

**直接光照评估**：
```hlsl
float3 evalDirect(const ShadingData sd, const IBSDF bsdf, inout SampleGenerator sg)
{
    float3 Lr = float3(0);

    // 采样解析光源
    for (uint lightIndex = 0; lightIndex < gScene.getLightCount(); lightIndex++)
    {
        AnalyticLightSample ls;
        if (sampleLight(sd.posW, gScene.getLight(lightIndex), sg, ls))
        {
            // 评估 BSDF
            float3 f = bsdf.eval(sd, ls.dir);

            // 追踪阴影光线
            if (any(f > 0.f))
            {
                float visibility = traceShadowRay(sd.posW, ls.dir, ls.distance);
                Lr += f * ls.Li * visibility;
            }
        }
    }

    // 采样发光三角形
    if (kUseEmissiveSampler)
    {
        TriangleLightSample tls;
        if (gScene.lightCollection.sampleTriangle(sd.posW, sg, tls))
        {
            float3 f = bsdf.eval(sd, tls.dir);

            if (any(f > 0.f))
            {
                float visibility = traceShadowRay(sd.posW, tls.dir, tls.distance);
                Lr += f * tls.Le * visibility / tls.pdf;
            }
        }
    }

    return Lr;
}
```

**阴影光线追踪**：
```hlsl
float traceShadowRay(float3 origin, float3 dir, float distance)
{
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = dir;
    ray.TMin = 0.001f;
    ray.TMax = distance - 0.001f;

    ShadowRayData rayData;
    rayData.visible = false;

    TraceRay(
        gScene.rtAccel,
        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
        0xff,
        kRayTypeShadow,
        getRayTypeCount(),
        kRayTypeShadow,
        ray,
        rayData
    );

    return rayData.visible ? 1.f : 0.f;
}

[shader("miss")]
void shadowMiss(inout ShadowRayData rayData)
{
    rayData.visible = true;
}

[shader("anyhit")]
void shadowAnyHit(inout ShadowRayData rayData, BuiltInTriangleIntersectionAttributes attribs)
{
    // Alpha 测试
    GeometryInstanceID instanceID = getGeometryInstanceID();
    VertexData v = getVertexData(instanceID, PrimitiveIndex(), attribs);
    uint materialID = gScene.getMaterialID(instanceID);

    if (gScene.materials.alphaTest(v, materialID, 0.f))
        IgnoreHit();
}
```

**环境贴图评估**：
```hlsl
float3 evalEnvironmentMap(float3 dir)
{
    if (kUseEnvMap && gScene.envMap != null)
    {
        return gScene.envMap.eval(dir);
    }
    return gScene.envMapIntensity * gScene.envMapColor;
}
```

---

### 6. 关键技术总结

#### 6.1 可见性缓冲区技术

**核心概念**：
- 存储三角形命中信息（instanceID + primitiveID + 重心坐标）而非完整的 G-Buffer
- 延迟着色数据重建到后续通道
- 减少内存带宽和存储需求

**优势**：
- **内存效率**：64 位 vs 传统 G-Buffer 的 128+ 位
- **带宽优化**：减少光栅化阶段的写入
- **灵活性**：可以按需重建任意着色数据

#### 6.2 Meshlet 渲染管线

**关键特性**：
- **软件光栅化**：使用计算着色器进行三角形光栅化
- **细粒度剔除**：在 meshlet 级别进行剔除
- **原子深度测试**：使用 InterlockedMin 进行线程安全的深度比较
- **负载打包**：将 instanceID 和 primitiveID 打包到单个 uint

**性能考虑**：
- 每组 32 个线程，每个 meshlet 一组
- 包围盒剔除减少像素处理
- 适合高度并行的 GPU 架构

#### 6.3 路径追踪优化

**采样策略**：
- **重要性采样**：BSDF 采样和光源采样
- **多重重要性采样（MIS）**：结合 BSDF 和光源采样
- **俄罗斯轮盘赌**：概率性路径终止以减少计算

**性能优化**：
- **V-Buffer 集成**：重用主光线命中信息
- **阴影光线优化**：使用 RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
- **负载大小**：ScatterRayData 限制在 72 字节以提高性能
- **早期终止**：Alpha 测试和俄罗斯轮盘赌

**光照技术**：
- **解析光源**：点光源、方向光、聚光灯
- **发光三角形**：基于三角形的区域光
- **环境贴图**：基于图像的照明
- **直接 + 间接光照**：完整的全局光照

---

### 7. Shader 文件索引

**VBuffer 相关**：
- `VBufferRaster.3d.slang`: 硬件光栅化实现
- `MeshletRaster.cs.slang`: 软件光栅化（计算着色器）
- `MeshletResolve.cs.slang`: 可见性缓冲区解码
- `VBufferRT.slang`: 光线追踪共享逻辑
- `VBufferRT.rt.slang`: DXR 光线追踪实现
- `VBufferRT.cs.slang`: 内联光线追踪实现

**路径追踪相关**：
- `MinimalPathTracer.rt.slang`: 完整路径追踪实现
- 支持多种几何类型（三角形、曲线、SDF）
- 集成 BSDF 采样和光源采样
- 俄罗斯轮盘赌和重要性采样优化
