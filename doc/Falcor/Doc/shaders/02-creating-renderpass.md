# 创建RenderPass (Creating a RenderPass)

本文档详细介绍如何在Falcor中创建新的RenderPass。

## 使用脚本创建RenderPass

Falcor提供了自动化工具来创建RenderPass模板。

### 基本用法

```bash
cd F:\Falcor
tools\make_new_render_pass.bat <RenderPassName>
```

### 示例：创建FilmGrain RenderPass

```bash
tools\make_new_render_pass.bat FilmGrain
```

### 脚本执行流程

`make_new_render_pass.bat` 内部调用Python脚本，执行以下步骤：

1. **验证名称**
   - 检查是否已存在同名RenderPass
   - 位置：`Source/RenderPasses/<RenderPassName>/`

2. **复制模板**
   - 从 `Source/RenderPasses/RenderPassTemplate/` 复制所有文件
   - 将模板中的 `RenderPassTemplate` 替换为你的RenderPass名称

3. **创建的文件结构**
   ```
   Source/RenderPasses/<RenderPassName>/
   ├── CMakeLists.txt
   ├── <RenderPassName>.h
   └── <RenderPassName>.cpp
   ```

4. **更新CMake配置**
   - 自动在 `Source/RenderPasses/CMakeLists.txt` 中添加：
     ```cmake
     add_subdirectory(<RenderPassName>)
     ```
   - 按字母顺序排序

## 脚本源码分析

`tools/make_new_render_pass.py` 的核心逻辑：

```python
def create_project(name):
    # 源和目标目录
    src_dir = RENDER_PASSES_DIR / TEMPLATE_NAME  # RenderPassTemplate
    dst_dir = RENDER_PASSES_DIR / name

    # 检查目标是否已存在
    if dst_dir.exists():
        print(f'"{name}" already exists!')
        return False

    # 创建目标文件夹
    dst_dir.mkdir()

    # 复制并替换模板文件
    for src_file in filter(lambda f: not f.suffix in EXCLUDE_EXT, src_dir.iterdir()):
        dst_file = dst_dir / (src_file.name.replace(TEMPLATE_NAME, name))

        # 替换所有 'RenderPassTemplate' 为新名称
        content = src_file.read_text()
        content = content.replace(TEMPLATE_NAME, name)
        dst_file.write_text(content)

    # 更新CMakeLists.txt
    cmake_file = RENDER_PASSES_DIR / "CMakeLists.txt"
    lines = [line for line in open(cmake_file, "r").readlines() if line.strip()]
    lines.append(f"add_subdirectory({name})\n")
    lines.sort(key=str.lower)
    open(cmake_file, "w").writelines(lines)

    return True
```

## 手动创建RenderPass（不推荐）

如果需要手动创建，请按照以下步骤：

### 1. 创建目录结构

```bash
mkdir Source\RenderPasses\YourPass
```

### 2. 创建CMakeLists.txt

```cmake
add_plugin(YourPass)
target_source_group(YourPass "Source/RenderPasses")
```

### 3. 创建头文件

参考 `Source/RenderPasses/RenderPassTemplate/RenderPassTemplate.h`

### 4. 创建实现文件

参考 `Source/RenderPasses/RenderPassTemplate/RenderPassTemplate.cpp`

关键点：
- 实现 `registerPlugin` 函数导出插件
- 继承 `RenderPass` 基类
- 实现必要的虚函数

### 5. 更新父CMakeLists.txt

在 `Source/RenderPasses/CMakeLists.txt` 中添加：
```cmake
add_subdirectory(YourPass)
```

## RenderPass模板结构分析

### CMakeLists.txt

```cmake
add_plugin(RenderPassTemplate)
target_source_group(RenderPassTemplate "Source/RenderPasses")
```

- `add_plugin`: Falcor的CMake函数，用于创建插件库
- `target_source_group`: 组织Visual Studio项目文件夹结构

### 头文件结构 (*.h)

```cpp
#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class RenderPassTemplate : public RenderPass
{
public:
    // 插件类宏 - 注册到插件系统
    FALCOR_PLUGIN_CLASS(RenderPassTemplate, "RenderPassTemplate", "Description");

    // 工厂方法
    static ref<RenderPassTemplate> create(ref<Device> pDevice, const Properties& props)
    {
        return make_ref<RenderPassTemplate>(pDevice, props);
    }

    // 构造函数
    RenderPassTemplate(ref<Device> pDevice, const Properties& props);

    // 必须实现的虚函数
    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

private:
    // 私有成员变量
};
```

### 实现文件结构 (*.cpp)

```cpp
#include "RenderPassTemplate.h"

// 插件注册函数 - 必须导出
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, RenderPassTemplate>();
}

// 构造函数
RenderPassTemplate::RenderPassTemplate(ref<Device> pDevice, const Properties& props)
    : RenderPass(pDevice)
{
    // 初始化代码
}

// 获取属性 - 用于序列化
Properties RenderPassTemplate::getProperties() const
{
    return {};
}

// 反射 - 定义输入输出资源
RenderPassReflection RenderPassTemplate::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    // reflector.addOutput("dst");
    // reflector.addInput("src");
    return reflector;
}

// 执行 - 主要逻辑
void RenderPassTemplate::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    // 实际的渲染代码
}

// UI渲染 - 可选的参数调整界面
void RenderPassTemplate::renderUI(Gui::Widgets& widget)
{
    // UI控件代码
}
```

## 关键概念

### 1. 插件系统

Falcor使用插件系统动态加载RenderPass：

```cpp
extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, YourPass>();
}
```

- `extern "C"`: C链接，避免名称修饰
- `FALCOR_API_EXPORT`: 导出符号供DLL使用
- `registerClass`: 注册到类型系统

### 2. 资源反射 (Reflection)

定义Pass的输入输出资源：

```cpp
RenderPassReflection RenderPassTemplate::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;

    // 添加输入
    reflector.addInput("src", "Source texture")
        .bindFlags(ResourceBindFlags::ShaderResource)
        .format(ResourceFormat::RGBA32Float);

    // 添加输出
    reflector.addOutput("dst", "Destination texture")
        .bindFlags(ResourceBindFlags::UnorderedAccess | ResourceBindFlags::RenderTarget)
        .format(ResourceFormat::RGBA32Float);

    return reflector;
}
```

### 3. 属性系统 (Properties)

用于序列化和参数传递：

```cpp
// 构造时读取属性
for (const auto& [key, value] : props)
{
    if (key == "myParam")
        mMyParam = value;
}

// 保存属性
Properties getProperties() const override
{
    Properties props;
    props["myParam"] = mMyParam;
    return props;
}
```

## 重新生成Visual Studio项目

创建RenderPass后，需要重新生成项目文件：

```bash
# 方法1：重新运行setup
setup_vs2022.bat

# 方法2：使用CMake命令
cmake --preset windows-vs2022

# 方法3：在Visual Studio中
# 右键点击CMakeLists.txt -> Configure Cache -> Reload
```

## FilmGrain示例分析

FilmGrain的创建过程（已完成）：

```bash
tools\make_new_render_pass.bat FilmGrain
```

生成的文件：
- `Source/RenderPasses/FilmGrain/CMakeLists.txt`
- `Source/RenderPasses/FilmGrain/FilmGrain.h`
- `Source/RenderPasses/FilmGrain/FilmGrain.cpp`

之后手动添加：
- `Source/RenderPasses/FilmGrain/FilmGrain.cs.slang` (Compute Shader)

## 下一步

- [03-编写Shader](./03-writing-shaders.md): 学习如何编写Shader文件
- [04-C++与Shader集成](./04-cpp-shader-integration.md): 学习如何连接C++和Shader
- [09-FilmGrain示例](./09-filmgrain-example.md): 查看完整的FilmGrain实现

## 常见问题

### Q: 创建后Visual Studio看不到新项目？
A: 运行 `setup_vs2022.bat` 重新生成解决方案。

### Q: 编译错误 "unresolved external symbol registerPlugin"
A: 确保在.cpp文件中实现了 `registerPlugin` 函数，并使用了 `FALCOR_API_EXPORT` 宏。

### Q: 如何删除RenderPass？
A:
1. 删除目录 `Source/RenderPasses/YourPass/`
2. 从 `Source/RenderPasses/CMakeLists.txt` 中移除 `add_subdirectory(YourPass)`
3. 重新运行 `setup_vs2022.bat`
