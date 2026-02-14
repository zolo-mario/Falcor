# Slang Mesh Shader 支持概览

### 目标平台支持

| 平台 | 支持情况 |
|------|----------|
| **D3D12** | ✅ 支持 |
| **Vulkan** | ✅ 支持 |
| **Metal** | ⚠️ 开发中 |
| **D3D11** | ❌ 不支持 |
| **CUDA** | ❌ 不支持 |

### 支持的 Shader 阶段

- **Mesh Shader** (`mesh`)：负责生成顶点和索引数据
- **Amplification Shader** (`amplification` / `task`)：可选，决定 mesh shader 的调度数量

### 所需 Profile

- **DirectX**：`ms_6_5`、`ms_6_6`、`ms_6_7`（Mesh）；`as_6_5`、`as_6_6`、`as_6_7`（Amplification）
- **Vulkan**：需启用 `SPV_EXT_mesh_shader` 扩展
