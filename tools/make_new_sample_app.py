#!/usr/bin/env python3
"""
Create a new Falcor sample application from the SampleAppTemplate.

Usage:
    python make_new_sample_app.py <Name> [--path PATH]
    python make_new_sample_app.py <Name>  # Creates at Source/<Name>/

Example:
    python make_new_sample_app.py MySample
    python make_new_sample_app.py MySample --path Source/Samples/MySample
"""

import argparse
import re
import sys
from pathlib import Path


def get_project_root() -> Path:
    """Get Falcor project root (parent of tools/)."""
    return Path(__file__).resolve().parent.parent


def to_pascal_case(name: str) -> str:
    """Convert name to PascalCase (e.g. my_sample -> MySample). Preserves existing PascalCase."""
    parts = re.split(r"[-_\s]+", name)
    if not parts:
        return name
    # Capitalize first letter of each part, preserve rest (e.g. SampleAppTemplate stays)
    result = "".join(part[0].upper() + part[1:] if len(part) > 1 else part.capitalize() for part in parts if part)
    return result if result else name


def get_template_content(name: str, karma_path: str) -> dict:
    """Return template content with placeholders replaced by name.
    karma_path: hierarchical path for Karma registration (e.g. Samples/MySample, Samples/Desktop/MySample).
    """
    return {
        "CMakeLists.txt": f"""add_plugin({name})

target_sources({name} PRIVATE
    {name}.cpp
    {name}.h
)

target_copy_shaders({name} {karma_path})

target_source_group({name} "Samples")
""",
        f"{name}.h": f"""#pragma once
#include "Falcor.h"
#include "Core/SampleBase.h"

using namespace Falcor;

class {name} : public SampleBase
{{
public:
    FALCOR_PLUGIN_CLASS({name}, "{name}", SampleBase::PluginInfo{{"{karma_path}"}});

    explicit {name}(SampleApp* pHost);
    ~{name}();

    static SampleBase* create(SampleApp* pHost);

    void onLoad(RenderContext* pRenderContext) override;
    void onShutdown() override;
    void onResize(uint32_t width, uint32_t height) override;
    void onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo) override;
    void onGuiRender(Gui* pGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onHotReload(HotReloadFlags reloaded) override;

private:
}};
""",
        f"{name}.cpp": f"""#include "{name}.h"

FALCOR_EXPORT_D3D12_AGILITY_SDK

{name}::{name}(SampleApp* pHost) : SampleBase(pHost)
{{
    //
}}

{name}::~{name}()
{{
    //
}}

void {name}::onLoad(RenderContext* pRenderContext)
{{
    //
}}

void {name}::onShutdown()
{{
    //
}}

void {name}::onResize(uint32_t width, uint32_t height)
{{
    //
}}

void {name}::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{{
    const float4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
}}

void {name}::onGuiRender(Gui* pGui)
{{
    Gui::Window w(pGui, "Falcor", {{250, 200}});
    renderGlobalUI(pGui);
    w.text("Hello from {name}");
    if (w.button("Click Here"))
    {{
        msgBox("Info", "Now why would you do that?");
    }}
}}

bool {name}::onKeyEvent(const KeyboardEvent& keyEvent)
{{
    return false;
}}

bool {name}::onMouseEvent(const MouseEvent& mouseEvent)
{{
    return false;
}}

void {name}::onHotReload(HotReloadFlags reloaded)
{{
    //
}}

SampleBase* {name}::create(SampleApp* pHost)
{{
    return new {name}(pHost);
}}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{{
    registry.registerClass<SampleBase, {name}>();
}}
""",
    }


def add_subdirectory_to_cmake(root_cmake: Path, subdir: str) -> bool:
    """Add add_subdirectory(subdir) to root CMakeLists.txt if not present."""
    content = root_cmake.read_text(encoding="utf-8")
    subdir_line = f"add_subdirectory({subdir})"

    if subdir_line in content:
        return False  # Already present

    # Find the project sources section and add after add_subdirectory(Source/Tools)
    pattern = r"(add_subdirectory\(Source/Tools\)\s*\n)"
    if re.search(pattern, content):
        content = re.sub(pattern, rf"\1add_subdirectory({subdir})\n", content)
    else:
        # Fallback: append before the add_dependencies line
        content += f"\n{subdir_line}\n"

    root_cmake.write_text(content, encoding="utf-8")
    return True


def create_sample(name: str, output_path: Path) -> bool:
    """Create a new sample at the given path."""
    root = get_project_root()
    dst_dir = root / output_path

    print(f'Creating sample application "{name}":')

    if dst_dir.exists():
        print(f'  Error: "{dst_dir}" already exists!')
        return False

    dst_dir.mkdir(parents=True)

    # Karma path for tree (e.g. Source/Samples/Desktop/MySample -> Samples/Desktop/MySample)
    output_str = str(output_path).replace("\\", "/")
    if output_str.startswith("Source/Samples/"):
        karma_path = output_str[len("Source/"):]
    elif output_str.startswith("Source/"):
        karma_path = f"Samples/{output_str[len('Source/'):]}"
    else:
        karma_path = f"Samples/{name}"

    templates = get_template_content(name, karma_path)
    for filename, content in templates.items():
        filepath = dst_dir / filename
        print(f"  Writing {filepath.relative_to(root)}")
        filepath.write_text(content, encoding="utf-8")

    # Add subdirectory to root CMakeLists.txt
    root_cmake = root / "CMakeLists.txt"
    subdir = str(output_path).replace("\\", "/")
    if add_subdirectory_to_cmake(root_cmake, subdir):
        print(f"  Added add_subdirectory({subdir}) to CMakeLists.txt")
    else:
        print(f"  Note: add_subdirectory({subdir}) may already exist in CMakeLists.txt")

    return True


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create a new Falcor sample application from the SampleAppTemplate."
    )
    parser.add_argument("name", help="Sample application name (e.g. MySample)")
    parser.add_argument(
        "--path",
        default=None,
        help="Output path relative to project root (default: Source/<Name>)",
    )
    args = parser.parse_args()

    name = to_pascal_case(args.name)
    if not name:
        print("Error: Invalid sample name.")
        return 1

    if args.path is None:
        output_path = Path(f"Source/{name}")
    else:
        output_path = Path(args.path)
        if output_path.name != name:
            output_path = output_path / name

    success = create_sample(name, output_path)
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
