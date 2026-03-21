---
name: obsidian-kanban
description: Reads and edits Obsidian Kanban-style Markdown (frontmatter kanban-plugin, ## columns, checklist cards, wiki links, tags, dates). Use when the user works with Kanban boards, Obsidian kanban files, moving tasks between columns, archiving done items, or mentions kanban-plugin / Project_* Kanban markdown.
---

# Obsidian Kanban Markdown

## 文件格式要点

1. **YAML Frontmatter（必须）**  
   文件开头需包含 `kanban-plugin: basic`（或插件要求的值）。无此行则视为普通 Markdown，Obsidian 不会出现「Open as Kanban」。

2. **列 = 二级标题 `##`**  
   每个 `## 列名` 下是一块列表；移动任务 = 把某条 `- [ ]` / `- [x]` 行从某一 `##` 块剪切到另一 `##` 块。

3. **卡片行**  
   标准任务行：`- [ ]` 未完成，`- [x]` 已完成。保持一行一条任务。

4. **可选语法（整行保留，勿随意改写）**  
   - 双链：`[[Note_Name]]`  
   - 标签：`#bug` `#feature` 等  
   - 日期：`@2026-03-25`（`@YYYY-MM-DD`）  
   - 块 ID：`^task-101`（供引用）  
   - 归档占位：`{Archive}` 等插件约定文本勿删除非用户要求  

5. **设置块（必须原样保留）**  
   文件末尾常见：
   `%% kanban:settings` … JSON … `%%`  
   读写时整体保留，不要丢 JSON 或改插件字段除非用户明确要求。

## Agent 操作原则

- **先读后改**：读全文件，定位各 `##` 区块与 `%% kanban:settings %%`。  
- **按块编辑**：在同一 `##` 内可调整顺序；跨列只移动对应行，不合并列标题。  
- **完成一项**：从原列删除该行；在「已完成 / Done」列**按插件习惯**插入（常见：prepend = 新完成项在最上），并把 `- [ ]` 改为 `- [x]`；可按用户规则追加 `@YYYY-MM-DD` 为完成日。  
- **新建卡片**：在目标列的 `##` 下追加 `- [ ]`，遵守 `new-card-insertion-method`（prepend 则插在该列任务列表顶部）。  
- **编码与换行**：保持原文件换行风格；不无关重排整个文件。

## 解析与写回（实现思路）

**Read**

1. 分离 frontmatter（`---` … `---`）与正文。  
2. 正文按 `^## ` 分行切分，得到列名 → 该列下列表文本。  
3. 从每列文本中提取以 `- [` 开头的行作为卡片。  
4. 单独保存尾部 `%% kanban:settings` … `%%` 块。

**Modify**

- 移动：从源列的卡片列表删除目标行，插入目标列列表（注意 prepend/append）。  
- 完成：改 `- [ ]` → `- [x]`，可选追加或更新 `@date`。  
- 阻塞/取消阻塞：整行在「阻塞」与「进行中」等列之间移动即可。

**Write**

1. 写回 frontmatter（确保含 `kanban-plugin`）。  
2. 按原列顺序输出每个 `## 标题` 与下列表行。  
3. 末尾接 `%% kanban:settings` 块（若存在）。  

## 示例结构（节选）

```markdown
---
kanban-plugin: basic
---

## 待处理 (Todo)

- [ ] 示例任务 #feature ^task-101

## 已完成 (Done)

- [x] 已完成项 @2026-03-20

%% kanban:settings
{"kanban-plugin":"basic","new-card-insertion-method":"prepend"}
%%
```

## 与用户协作

- 若列名语言为中文，新增列仍用用户一致的命名风格。  
- 用户用 `#bug` / `#feature` 等分类时，移动或归档不要删除这些标签除非用户说明。  
- 双链 `[[...]]` 指向详细笔记：需要上下文时可按路径读取被链笔记。  
- 从 git 汇总「已完成」时，默认使用 `git log <基线>..HEAD`（不按 `--author` 过滤），除非用户明确要求只统计某作者。
