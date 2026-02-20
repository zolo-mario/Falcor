# Falcor .cursor 配置

本目录包含 Cursor AI 的 **Agents** 与 **Skills** 配置，用于 Falcor 项目的开发与迁移工作流。

## 目录结构

```
.cursor/
├── README.md           # 本索引（你正在阅读）
├── agents/             # Agent 定义（任务导向的 AI 角色）
│   ├── dx-samples-migrate.md      # DirectX Samples 迁移主 Agent
│   ├── dx-samples-migrate/        # DX Samples 迁移记录目录
│   │   └── records/               # 各 sample 的迁移记录
│   │       ├── D3D12HelloWorld.md
│   │       ├── D3D12HelloTriangle.md
│   │       ├── D3D12HelloTexture.md
│   │       ├── D3D12HelloBundles.md
│   │       └── D3D12*.md / Meshlet*.md ...
│   ├── migrate-niagara-renderer.md # Niagara 渲染器迁移 Agent
│   ├── migrate-niagara-review.md   # Niagara 迁移审查 Agent
│   ├── qa.md                       # 测试与质量保证 Agent
│   └── render-pass-architect.md    # RenderPass 架构设计 Agent
├── _archived/          # 已归档（不再主动使用）
│   ├── falcor-deepwiki-analysis/
│   ├── merge-falcor-docs/
│   └── slang-user-guide/
└── skills/             # Skill 定义（可复用的能力模块）
    ├── build/          # 构建 Falcor
    └── run/              # 运行 Mogwai、Niagara、测试等
```

## Agents 概览

| Agent | 用途 |
|-------|------|
| **dx-samples-migrate** | 将 DirectX-Graphics-Samples 迁移至 Falcor SampleBase 插件，通过 Karma 运行 |
| **migrate-niagara-renderer** | 将 Niagara mesh-shading 渲染器移植为 Falcor SampleApp |
| **migrate-niagara-review** | 对照 migrate-niagara-renderer 计划审查实现 |
| **qa** | 测试用例管理、图像误差测量、参考图像更新 |
| **render-pass-architect** | 设计 RenderPass 骨架、ShaderBind 风格文档 |

## Skills 概览

| Skill | 用途 |
|-------|------|
| **build** | 构建 Falcor、指定 target、配置 Debug/Release |
| **run** | Karma、Mogwai、Niagara、单元测试、图像测试、Packman Python |

**已归档**（`_archived/`）：falcor-deepwiki-analysis, merge-falcor-docs, slang-user-guide  

**已合并入 render-pass-architect**：create-render-pass, falcor-shader-binding

## 使用说明

- **Agents**：在 Cursor 中通过 `@` 引用或选择对应 Agent 执行任务
- **Skills**：由 Agent 或系统根据用户意图自动加载（见 `agent_skills` 配置）
- **迁移记录**：DX Samples 迁移完成后写入 `agents/dx-samples-migrate/records/<SampleName>.md`
