# 图像测试 (Image Testing)

本文档介绍如何为RenderPass编写端到端的图像测试。图像测试通过渲染完整的场景并比较输出图像来验证功能。

## 概述

图像测试的工作流程：
1. 使用Python脚本定义RenderGraph
2. 渲染一帧或多帧
3. 捕获输出图像
4. 与参考图像比较
5. 报告差异

测试位置：`tests/image_tests/`

## FilmGrain图像测试示例

### 测试脚本

文件：`tests/image_tests/renderpasses/graphs/FilmGrain.py`

```python
from falcor import *

def render_graph_FilmGrain():
    """创建FilmGrain测试的RenderGraph"""
    g = RenderGraph("FilmGrain")

    # 添加图像加载器作为输入
    imageLoader = createPass("ImageLoader", {
        'filename': 'test_scenes/envmaps/hallstatt4_hd.hdr',
        'mips': False,
        'srgb': False,
        'outputFormat': 'RGBA32Float'
    })
    g.addPass(imageLoader, "ImageLoader")

    # 添加FilmGrain Pass
    filmGrain = createPass("FilmGrain")
    g.addPass(filmGrain, "FilmGrain")

    # 连接passes
    g.addEdge("ImageLoader.dst", "FilmGrain.src")

    # 标记输出
    g.markOutput("FilmGrain.dst")

    return g

# 创建graph实例
FilmGrain = render_graph_FilmGrain()

# 尝试添加到Mogwai（如果在Mogwai环境中）
try:
    m.addGraph(FilmGrain)
except NameError:
    None
```

### 测试配置

创建测试配置文件 `tests/image_tests/renderpasses/test_FilmGrain.py`：

```python
import sys
sys.path.append('..')
from helpers import render_frames
from falcor import *

# 导入graph定义
exec(open('../graphs/FilmGrain.py').read())

# 定义测试
def test_FilmGrain_default():
    """测试FilmGrain的默认参数"""
    render_frames(m, 'FilmGrain_default', frames=[1])

def test_FilmGrain_high_grain():
    """测试高颗粒强度"""
    # 修改参数
    m.activeGraph.updatePass('FilmGrain', {'grainAmount': 0.05})
    render_frames(m, 'FilmGrain_high_grain', frames=[1])

def test_FilmGrain_disabled():
    """测试禁用效果"""
    m.activeGraph.updatePass('FilmGrain', {'enabled': False})
    render_frames(m, 'FilmGrain_disabled', frames=[1])

def test_FilmGrain_animation():
    """测试多帧动画（确保时间变化）"""
    render_frames(m, 'FilmGrain_animation', frames=[1, 10, 20])
```

## 目录结构

```
tests/
├── image_tests/
│   ├── helpers.py                          # 辅助函数
│   ├── renderpasses/
│   │   ├── graphs/
│   │   │   ├── FilmGrain.py               # Graph定义
│   │   │   ├── GaussianBlur.py
│   │   │   └── ...
│   │   └── tests/
│   │       ├── test_FilmGrain.py          # 测试用例
│   │       └── ...
│   ├── ref/                                # 参考图像
│   │   ├── FilmGrain_default.png
│   │   ├── FilmGrain_high_grain.png
│   │   └── ...
│   └── results/                            # 测试输出
│       ├── FilmGrain_default.png
│       └── ...
├── run_image_tests.bat                     # Windows测试运行器
└── run_image_tests.py                      # Python测试运行器
```

## helpers.py详解

`tests/image_tests/helpers.py` 提供了核心的渲染函数：

```python
import falcor

def render_frames(m, name, frames=[1], framerate=60, resolution=[640, 360]):
    """
    渲染指定的帧并保存

    参数:
        m: Mogwai实例
        name: 输出文件名（不含扩展名）
        frames: 要捕获的帧列表，例如 [1, 10, 20]
        framerate: 帧率（用于时间计算）
        resolution: 渲染分辨率 [width, height]
    """
    # 设置分辨率
    m.resizeFrameBuffer(*resolution)

    # 禁用UI（加速渲染）
    m.ui = False

    # 设置帧率和时间
    m.clock.framerate = framerate
    m.clock.time = 0
    m.clock.pause()

    # 设置输出文件名
    m.frameCapture.baseFilename = name

    frame = 0
    for capture_frame in frames:
        # 渲染到目标帧
        while frame < capture_frame:
            frame += 1
            m.clock.frame = frame
            m.renderFrame()

        # 捕获帧（除非是仅运行模式）
        if "IMAGE_TEST_RUN_ONLY" in falcor.__dict__:
            continue
        m.frameCapture.capture()
```

## 创建新的图像测试

### 步骤1：创建Graph定义

文件：`tests/image_tests/renderpasses/graphs/MyPass.py`

```python
from falcor import *

def render_graph_MyPass():
    g = RenderGraph("MyPass")

    # 示例：简单的后处理Pass
    imageLoader = createPass("ImageLoader", {
        'filename': 'test_scenes/envmaps/hallstatt4_hd.hdr',
        'mips': False,
        'srgb': False,
        'outputFormat': 'RGBA32Float'
    })
    g.addPass(imageLoader, "ImageLoader")

    myPass = createPass("MyPass", {
        'param1': 1.0,
        'param2': True
    })
    g.addPass(myPass, "MyPass")

    g.addEdge("ImageLoader.dst", "MyPass.input")
    g.markOutput("MyPass.output")

    return g

MyPass = render_graph_MyPass()
try:
    m.addGraph(MyPass)
except NameError:
    None
```

### 步骤2：创建测试用例

文件：`tests/image_tests/renderpasses/tests/test_MyPass.py`

```python
import sys
sys.path.append('..')
from helpers import render_frames
from falcor import *

exec(open('../graphs/MyPass.py').read())

def test_MyPass_basic():
    """基本功能测试"""
    render_frames(m, 'MyPass_basic', frames=[1])

def test_MyPass_high_quality():
    """高质量设置"""
    m.activeGraph.updatePass('MyPass', {
        'param1': 2.0,
        'param2': True
    })
    render_frames(m, 'MyPass_high_quality', frames=[1])
```

### 步骤3：生成参考图像

首次运行测试以生成参考图像：

```bash
tests\run_image_tests.bat --filter MyPass --gen-refs
```

这会：
1. 运行所有MyPass测试
2. 将输出图像保存到 `tests/image_tests/ref/` 目录
3. 这些图像成为未来比较的"金标准"

### 步骤4：运行测试

后续运行测试并比较：

```bash
tests\run_image_tests.bat --filter MyPass
```

这会：
1. 运行测试并生成新图像到 `tests/image_tests/results/`
2. 与 `ref/` 中的参考图像比较
3. 报告任何差异

## 运行图像测试

### 基本用法

```bash
# 运行所有图像测试
tests\run_image_tests.bat

# 运行特定Pass的测试
tests\run_image_tests.bat --filter FilmGrain

# 生成参考图像
tests\run_image_tests.bat --gen-refs

# 查看帮助
tests\run_image_tests.bat --help
```

### 高级选项

```bash
# 使用特定的构建配置
tests\run_image_tests.bat --config Release

# 使用特定的GPU
tests\run_image_tests.bat --gpu 0

# 详细输出
tests\run_image_tests.bat --verbose

# 只运行测试（不生成图像）
tests\run_image_tests.bat --run-only
```

## 测试结果分析

### 查看测试报告

测试完成后会生成HTML报告：

```
tests/image_tests/results/report.html
```

使用浏览器打开查看：
- 通过/失败的测试列表
- 参考图像 vs 结果图像的对比
- 差异热图
- 统计信息（PSNR、MSE等）

### 使用图像查看器

Falcor提供了专门的工具查看测试结果：

```bash
tests\view_image_tests.bat
```

这会启动一个GUI工具，可以：
- 并排比较参考图像和结果图像
- 放大查看细节
- 查看差异图
- 接受新图像作为参考

## 更新参考图像

当你有意修改了RenderPass行为，需要更新参考图像：

```bash
# 方法1：重新生成所有参考图像
tests\run_image_tests.bat --filter MyPass --gen-refs

# 方法2：只更新特定测试
tests\run_image_tests.bat --filter MyPass_basic --gen-refs

# 方法3：使用查看器手动接受新图像
tests\view_image_tests.bat
# 在GUI中选择测试 -> Accept New Image
```

## 复杂RenderGraph示例

### 多Pass链

```python
def render_graph_ComplexPipeline():
    g = RenderGraph("ComplexPipeline")

    # Pass 1: GBuffer
    gbuffer = createPass("GBufferRaster", {
        'samplePattern': 'Center',
        'sampleCount': 1
    })
    g.addPass(gbuffer, "GBuffer")

    # Pass 2: 自定义光照
    lighting = createPass("MyLighting")
    g.addPass(lighting, "Lighting")

    # Pass 3: 后处理
    postfx = createPass("FilmGrain", {'grainAmount': 0.02})
    g.addPass(postfx, "PostFX")

    # 连接
    g.addEdge("GBuffer.posW", "Lighting.posW")
    g.addEdge("GBuffer.normW", "Lighting.normW")
    g.addEdge("GBuffer.diffuseOpacity", "Lighting.diffuse")
    g.addEdge("Lighting.output", "PostFX.src")

    g.markOutput("PostFX.dst")

    return g
```

### 使用场景

```python
def render_graph_WithScene():
    g = RenderGraph("WithScene")

    # 加载场景的Pass
    gbuffer = createPass("GBufferRaster")
    g.addPass(gbuffer, "GBuffer")

    # 使用GBuffer输出
    myPass = createPass("MyPass")
    g.addPass(myPass, "MyPass")

    g.addEdge("GBuffer.depth", "MyPass.depth")
    g.markOutput("MyPass.output")

    return g

WithScene = render_graph_WithScene()
try:
    m.addGraph(WithScene)
    # 加载场景
    m.loadScene('test_scenes/cornell_box.pyscene')
except NameError:
    None
```

## 测试不同分辨率

```python
def test_MyPass_resolutions():
    """测试不同分辨率"""

    # 720p
    render_frames(m, 'MyPass_720p', frames=[1], resolution=[1280, 720])

    # 1080p
    render_frames(m, 'MyPass_1080p', frames=[1], resolution=[1920, 1080])

    # 4K
    render_frames(m, 'MyPass_4k', frames=[1], resolution=[3840, 2160])
```

## 测试时间相关效果

```python
def test_MyPass_temporal():
    """测试时间相关的效果（如TAA、运动模糊）"""

    # 渲染多帧以建立时间历史
    render_frames(m, 'MyPass_temporal',
                  frames=[1, 5, 10, 15, 20],
                  framerate=60)
```

## 调试测试失败

### 1. 手动运行Graph

```bash
# 直接在Mogwai中运行graph
build\windows-vs2022\bin\Release\Mogwai.exe tests\image_tests\renderpasses\graphs\FilmGrain.py
```

### 2. 比较图像

```bash
# 使用图像查看器
tests\view_image_tests.bat

# 或手动比较
# 参考图像: tests/image_tests/ref/FilmGrain_basic.png
# 结果图像: tests/image_tests/results/FilmGrain_basic.png
```

### 3. 检查差异原因

常见原因：
- **随机性**：使用固定种子
- **浮点精度**：不同GPU可能有微小差异
- **未初始化状态**：确保所有参数都被设置
- **时间依赖**：使用固定的frameTime而不是实时时间

### 4. 添加容差

如果微小差异是可接受的，修改测试配置添加容差（在测试系统配置中）。

## 性能测试

```python
def test_MyPass_performance():
    """性能测试 - 不生成参考图像"""
    import time

    # 预热
    for _ in range(10):
        m.renderFrame()

    # 计时
    start = time.time()
    frames = 100
    for _ in range(frames):
        m.renderFrame()
    elapsed = time.time() - start

    fps = frames / elapsed
    print(f"Performance: {fps:.2f} FPS ({elapsed*1000/frames:.2f} ms/frame)")

    # 可选：断言性能要求
    assert fps > 30, f"Performance too low: {fps:.2f} FPS"
```

## 最佳实践

### 1. 测试命名规范

```python
def test_<PassName>_<feature>():
    """描述测试内容"""
    pass
```

示例：
- `test_FilmGrain_default` - 默认设置
- `test_FilmGrain_high_grain` - 特定参数
- `test_FilmGrain_disabled` - 边界情况

### 2. 使用确定性输入

```python
# 好：使用固定的测试图像
imageLoader = createPass("ImageLoader", {
    'filename': 'test_scenes/envmaps/hallstatt4_hd.hdr'
})

# 避免：使用随机或时间相关的输入
```

### 3. 测试覆盖

确保测试覆盖：
- 默认参数
- 边界值（最小/最大参数）
- 特殊情况（禁用、极端值）
- 不同输入尺寸
- 错误处理

### 4. 保持测试简单

每个测试应该：
- 测试一个特定功能
- 容易理解和维护
- 运行快速（通常只需1帧）

## 常见问题

### Q: 测试在我的机器上通过，但CI上失败？
A:
- 使用确定性输入（固定种子、固定图像）
- 避免依赖系统时间
- 检查GPU差异（使用容差）

### Q: 如何测试随机效果？
A:
```python
# 在shader中使用可控的种子
myPass = createPass("MyPass", {'seed': 12345})
```

### Q: 参考图像太大怎么办？
A:
- 使用较小的测试分辨率（640x360足够）
- 压缩PNG（虽然可能影响比较精度）
- 考虑只测试关键区域

## 下一步

- [07-运行单独测试](./07-running-tests.md): 学习如何运行特定测试
- [08-Mogwai热更新](./08-hot-reload-mogwai.md): 学习交互式调试
- [09-FilmGrain示例](./09-filmgrain-example.md): 完整的FilmGrain实现分析

## 参考

- FilmGrain图像测试: `tests/image_tests/renderpasses/graphs/FilmGrain.py`
- 测试辅助函数: `tests/image_tests/helpers.py`
