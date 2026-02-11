---
name: merge-falcor-docs
description: Merges Falcor-related Markdown docs by function and load balance. Use when the user asks to 合并 Falcor 文档, Falcor 文档合并, or to consolidate many Falcor MD files into fewer files.
---

# Falcor 文档合并 (Merge Falcor Docs)

将大量 Falcor 相关 MD 文件按功能内聚与负载均衡合并为更少的目标文件，便于阅读与 NotepadLM 等工具使用。

## 触发与输入

- **触发**：用户说「Falcor 文档合并」「合并 Falcor 文档」「把 Falcor 的 MD 合并成更少文件」等。
- **输入**：可选：源目录（默认包含 Falcor 相关 doc 的路径）、目标文件数或每文件约含源文件数。
- **输出**：合并后的 MD 文件（数量显著少于源文件），内部链接按需清理或改写。

## 目标与原则（必须遵守）

- **目标**：将约 235 个 MD 文件合并为约 45 个文件，保证负载均衡和内容内聚。
- **功能内聚**：相关功能的文件合并在一起（如同一 RenderPass、同一模块）。
- **负载均衡**：每个目标文件约包含 **4–6 个**源文件。
- **保持独立性**：组织良好的子模块（如 RenderGraph）可保持独立，不强行合并。
- **内容整合**：Doc 文档按主题合并到对应技术文件中，不单独成堆。
- **清理链接**：合并后移除或改写原内部链接，使内容自洽（可改为锚点或章节引用）。

## 工作流

1. **盘点源文件**
   - 列出待合并目录下所有 Falcor 相关 `.md`（如 `AIStudio/Falcor/Doc/`、`AIStudio/Falcor/ShaderBind/`、`AIStudio/Falcor/RenderPasses/` 等）。
   - 按路径与文件名推断主题（RenderPass 名、模块名、Doc/ShaderBind 等）。

2. **分组**
   - 按**功能/模块**分组（如 ToneMapper 相关、PathTracer 相关、RenderGraph 总览）。
   - 每组目标：约 4–6 个源文件对应 1 个目标文件；过大的组可拆为多个目标文件。

3. **合并与撰写**
   - 对每组：按逻辑顺序拼接内容（如先概述再子模块），统一一级/二级标题层级，避免重复标题。
   - 原内部 `[[链接]]`：若指向本次合并内的笔记，改为同一文件内 `## 标题` 或删除；若指向外部，保留 `[[文件名]]`。
   - 在目标文件顶部可加简短说明（本文件由 XXX、YYY 等合并而成）。

4. **输出与确认**
   - 将目标文件写入约定目录（与用户约定或默认覆盖/新目录），并列出「源文件 → 目标文件」映射，便于回溯。

## 约定

- **链接**：合并后同一文档内部用标题锚点或章节，跨文档用 `[[文件名]]`（符合 `CLAUDE.md`）。
- **不丢内容**：合并时只调整顺序与标题层级，不删除原文段落（可删仅重复的 frontmatter）。
- **命名**：目标文件名可沿用主主题名（如 `ToneMapper.md`、`PathTracer.md`）或合并说明（如 `RenderPasses-Overview.md`）。

## 工具与参考

### 合并脚本
- **`merge_files.py`**：自动化合并脚本，按 MERGE_MAP 批量合并 MD 文件。
  - 用法（在 Vault 根目录）：`python .cursor/skills/merge-falcor-docs/merge_files.py`
  - 可选参数：`--source AIStudio/Falcor`、`--target <输出目录>`
- **`merge_strategy.md`**：合并策略与映射表，含负载均衡统计。

### 参考
- 仓库规范：`CLAUDE.md`（Obsidian 链接）
- 源路径示例：`AIStudio/Falcor/Doc/`、`AIStudio/Falcor/ShaderBind/`、`AIStudio/Falcor/RenderPasses/`
