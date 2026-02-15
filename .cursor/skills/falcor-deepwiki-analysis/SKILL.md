---
name: falcor-deepwiki-analysis
description: Performs depth-first recursive code analysis of Falcor C++ source, creating a DeepWiki knowledge base in AIStudio/Falcor/ with folder notes and technical specs. Use when analyzing Falcor codebase, creating DeepWiki, building architecture documentation, or when asked to 深度分析 Falcor 源码、生成 Falcor 技术文档.
---

# Falcor DeepWiki 深度分析

对 Falcor 游戏引擎 C++ 源码执行**深度优先递归分析**，在 `AIStudio/Falcor/` 下创建与代码架构一一对应的结构化知识库。

## 核心要求

### 分析原则

- **来源**：仅分析提供的源码文件，不使用网络搜索或外部工具
- **顺序**：深度优先遍历，完整处理一个模块后再处理兄弟模块
- **粒度**：递归分解大模块直至原子单位

### 输出结构

```
AIStudio/Falcor/
├── ModuleA/
│   ├── ModuleA.md          # 文件夹说明（状态机）
│   ├── ClassX.md           # 技术规格
│   ├── SubModuleB/
│   │   ├── SubModuleB.md
│   │   └── ClassY.md
│   └── ...
└── Falcor.md
```

### 文件夹说明格式（Folder Note）

每个文件夹必须包含自引用说明，结构如下：

```markdown
# Module: [模块名]

## Status: [In Progress | Complete]

## Module Overview
[模块目的与范围简述]

## Dependency Graph
- **Upstream Dependencies**: [本模块依赖]
- **Downstream Dependents**: [依赖本模块者]

## Sub-Module Tracking
| Sub-Module | Status | Last Updated |
|------------|--------|--------------|
| SubA | ✅ Complete | YYYY-MM-DD |
| SubB | 🔄 Pending | - |

## Files Analyzed
- [x] file1.cpp
- [ ] file2.cpp

## Next Action
[下一步执行项]
```

### 技术规格格式（Technical Spec）

每个 C++ 类/结构体/系统：

```markdown
# [类/系统名]

## File Location
`path/to/file.cpp` (Lines: XXX-YYY)

## Purpose & Responsibility
[组件职责]

## Dependencies
- **Includes**: [头文件依赖]
- **Interacts With**: [交互类/系统]

## Implementation Notes
[关键算法、设计模式、优化手段]

## Potential Improvements
[技术债、优化机会]
```

## 执行协议

### Step 1: 初始化/恢复

1. 检查 `AIStudio/Falcor/` 是否存在
2. 若存在，找到最近状态为 "In Progress" 的 Folder Note
3. 读取 "Next Action" 从中断点恢复
4. 若不存在，创建根结构并从仓库根目录开始

### Step 2: 模块分析循环

```
WHILE 未达迭代上限:
    1. 确定当前待分析模块/文件夹
    2. 创建/更新 Folder Note，状态 "In Progress"
    3. 列出当前范围内所有源文件
    4. FOR EACH 源文件:
        a. 解析识别所有 classes/structs/systems
        b. 生成技术规格文档
        c. 更新 "Files Analyzed" 清单
    5. 分析子目录:
        - 若存在子目录，在 tracking 表标记 "Pending"
        - 递归创建子目录 Folder Note
    6. 更新当前 Folder Note:
        - 标记已完成项为 "Complete"
        - 设置 "Next Action" 为下一待处理项
    7. CHECKPOINT: 保存所有变更
    8. 移动到下一个深度优先节点
END WHILE
```

### Step 3: 状态保持

- **每分析一个文件后**：更新父级 Folder Note
- **即将达迭代上限时**：写入明确的 "Next Action" 指示
- **完成时**：将模块状态设为 "✅ Complete"

## 分析要求（每类 C++ 实体）

提取以下维度：

1. **内存语义**：对齐、大小、布局、padding 分析
2. **并发**：线程安全、锁机制
3. **性能**：复杂度、缓存行为、热路径
4. **依赖**：include 图、耦合分析
5. **设计模式**：识别的架构模式

## 约束

❌ **禁止**：网络搜索、无源码支撑的假设、跳过 Folder Note 状态更新  
✅ **必须**：仅由静态代码分析得出结论、任意中断点可恢复、操作幂等

## 完成标准

- 所有 Folder Note 中模块均为 "✅ Complete"
- 每个 C++ 类均有技术规格文档
- 完整依赖图已记录
- 无 "🔄 Pending" 项残留

## 输出语言

所有笔记内容使用 **中文（简体）** 撰写。
