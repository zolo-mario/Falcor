#include "RenderGraphIR.h"
#include "RenderGraph.h"
#include "Utils/StringUtils.h"
#include "Utils/Scripting/Scripting.h"
#include "Utils/Scripting/ScriptWriter.h"

namespace Falcor
{

RenderGraphIR::RenderGraphIR(const std::string& name, bool newGraph) : mName(name)
{
    if (newGraph)
    {
        mIR += "from pathlib import WindowsPath, PosixPath\n";
        mIR += "from falcor import *\n";
        mIR += "\n";
        mIR += "def " + getFuncName(mName) + "():\n";
        mIndentation = "    ";
        mGraphPrefix += mIndentation;
        mIR += mIndentation + "g" + " = " + ScriptWriter::makeFunc("RenderGraph", mName);
    }
    mGraphPrefix += "g.";
}

void RenderGraphIR::createPass(const std::string& passClass, const std::string& passName, const Properties& props)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("create_pass", passName, passClass, props.toPython());
}

void RenderGraphIR::updatePass(const std::string& passName, const Properties& props)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("update_pass", passName, props.toPython());
}

void RenderGraphIR::removePass(const std::string& passName)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("remove_pass", passName);
}

void RenderGraphIR::addEdge(const std::string& src, const std::string& dst)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("add_edge", src, dst);
}

void RenderGraphIR::removeEdge(const std::string& src, const std::string& dst)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("remove_edge", src, dst);
}

void RenderGraphIR::markOutput(const std::string& name, const TextureChannelFlags mask)
{
    if (mask == TextureChannelFlags::RGB)
    {
        // Leave out mask parameter for the default case (RGB).
        mIR += mGraphPrefix + ScriptWriter::makeFunc("mark_output", name);
    }
    else
    {
        mIR += mGraphPrefix + ScriptWriter::makeFunc("mark_output", name, mask);
    }
}

void RenderGraphIR::unmarkOutput(const std::string& name)
{
    mIR += mGraphPrefix + ScriptWriter::makeFunc("unmark_output", name);
}

std::string RenderGraphIR::getFuncName(const std::string& graphName)
{
    std::string name = "render_graph_" + graphName;
    name = replaceCharacters(name, " /\\", '_');
    return name;
}

} // namespace Falcor
