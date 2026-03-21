
续接统计已完成：在 **`scripts/`、`tests/`、`Source\RenderPasses`、`Source\Mogwai`** 下对所有 `*.py` 用正则统计 **`createPass('…')` / `createPass("…")`** 的调用次数（单引号与双引号都计入），并排除了全仓库递归导致超时的问题。

---

## 统计口径

| 项目 | 说明 |
|------|------|
| **“使用一次”** | 每出现一次 `createPass('PassName'` 或 `createPass("PassName"` 计 1（同一文件多次调用会多次计数）。 |
| **范围** | `scripts`、`tests`、`Source\RenderPasses`、`Source\Mogwai` 中的 Python，**不含** `external`、`.git`。 |
| **与 CMake 的关系** | `Source/RenderPasses/*/CMakeLists.txt` 里的 **`add_plugin(X)`** 表示“构建进工程的插件名”；Python 里 **`createPass("Y")` 的字符串 Y** 是运行时注册的 Pass 名，**Y 与 X 不一定相同**（例如插件 `NRDPass` / `GBuffer`，图中常写 `NRD`、`GBufferRT` 等）。 |

---

## `createPass` 频率（Top，按次数降序）

| 次数 | Pass 名 |
|------|---------|
| 18 | ToneMapper |
| 18 | ImageLoader |
| 17 | AccumulatePass |
| 12 | VBufferRT |
| 10 | PathTracer |
| 9 | GBufferRT |
| 8 | GBufferRaster |
| 5 | NRD |
| 4 | MinimalPathTracer |
| 3 | SceneDebugger |
| 3 | VBufferRaster |
| 2 | DLSSPass, SDFEditor, ModulateIllumination, Composite, BlitPass, RTXDIPass, SimplePostFX, BSDFViewer |
| 1 | GaussianBlur, FLIPPass, ColorMapPass, WhittedRayTracer, SVGFPass, CrossFade, TestRtProgram, TAA, SplitScreenPass, VBufferMeshletRaster, SideBySidePass |

---

## 如何解读

- 这反映的是 **仓库里示例/测试/工具图脚本** 里各 Pass **被实例化的频繁程度**，不是 GPU 上每帧执行次数。  
- 高频项（**ToneMapper、ImageLoader、AccumulatePass、VBufferRT、PathTracer、GBuffer***）与典型离线/交互渲染管线一致。  
- **`SceneDebugger`** 在部分图中用单引号；若只统计双引号会明显低估，本次已统一计入。


## 结论：在 Python 图里「从未 `createPass`」的 Pass

**口径**：从 `Source/RenderPasses/**/*.h` 里 **`FALCOR_PLUGIN_CLASS(..., "TypeName", ...)`** 的第二个字符串得到所有 **运行时类型名**（与 `createPass("TypeName")` 一致）；在 **`scripts/`、`tests/`、`Source/Mogwai/`、`Source/RenderPasses/`** 下扫描所有 `*.py` 中的 **`createPass('…')` / `createPass("…")`**。若某类型名从未出现，则视为「当前仓库这些 Python 场景里未使用」。

**未使用的 5 个类型名**（已注册、但上述范围内无 `createPass`）：

| 类型名 | 说明 |
|--------|------|
| **ErrorMeasurePass** | 与参考图做误差测量，偏工具/对比流程 |
| **InvalidPixelDetectionPass** | 调试用：标 NaN/Inf 像素 |
| **OverlaySamplePass** | 示例：在 `renderOverlayUI` 里画简单图形 |
| **PixelInspectorPass** | 编辑器向：点选像素看材质/几何 |
| **RenderPassTemplate** | 脚手架模板；`tools/make_new_render_pass.py` 里会提到，用于生成新 Pass，不是业务管线里的节点 |
