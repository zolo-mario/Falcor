---
name: install-serena
description: Installs Serena for Cursor and configures project-level MCP settings. Use when the user asks to install Serena, set up Cursor MCP for Serena, or initialize Serena with uvx.
---

# Install Serena for Cursor

## Scope

Use this skill only for initial Serena setup in Cursor:
- Ensure `uv` is available.
- Validate Serena startup command.
- Create or update project MCP config at `.cursor/mcp.json`.

Do not create personal/global skill files during this workflow.

## Minimal workflow

1. Confirm workspace root and existing `.cursor` directory.
2. Check `uv`:
   - Run `uv --version`.
   - If missing, stop and tell the user to install `uv` first.
3. Verify Serena command:
   - Run `uvx --from git+https://github.com/oraios/serena serena start-mcp-server --help`.
   - Treat exit code `0` as success.
4. Write project MCP config to `.cursor/mcp.json`:

```json
{
  "mcpServers": {
    "serena": {
      "command": "uvx",
      "args": [
        "--from",
        "git+https://github.com/oraios/serena",
        "serena",
        "start-mcp-server",
        "--project-from-cwd"
      ]
    }
  }
}
```

5. Re-read `.cursor/mcp.json` and confirm exact keys exist: `mcpServers`, `serena`, `command`, `args`.
6. Tell the user to reload/restart Cursor so MCP reconnects.

## Response format

When done, report:
- `uv` check result
- Serena command validation result
- file written path (`.cursor/mcp.json`)
- next step: restart/reload Cursor
