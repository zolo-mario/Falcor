#pragma once
#include "Core/Macros.h"
#include "Core/API/Formats.h"
#include "Utils/Properties.h"
#include <memory>
#include <string>

namespace Falcor
{
class Scene;

class FALCOR_API RenderGraphIR
{
public:
    RenderGraphIR(const std::string& name, bool newGraph = true);

    void createPass(const std::string& passClass, const std::string& passName, const Properties& props = Properties());
    void updatePass(const std::string& passName, const Properties& props);
    void removePass(const std::string& passName);
    void addEdge(const std::string& src, const std::string& dst);
    void removeEdge(const std::string& src, const std::string& dst);
    void markOutput(const std::string& name, const TextureChannelFlags mask = TextureChannelFlags::RGB);
    void unmarkOutput(const std::string& name);

    std::string getIR() { return mIR + mIndentation + (mIndentation.size() ? "return g\n" : "\n"); }

    static std::string getFuncName(const std::string& graphName);

private:
    std::string mName;
    std::string mIR;
    std::string mIndentation;
    std::string mGraphPrefix;
};
} // namespace Falcor
