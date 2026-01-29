#include <map>
#include <string>

namespace Falcor
{

class SceneBuilder;

/// Used to dump content of SceneBuilder for debugging purposes.
class SceneBuilderDump
{
public:
    /// Returns pairs of geometry and its serialization to text used for debugging.
    /// The interface as well as output are unstable and depend on the latest debugging needs
    static std::map<std::string, std::string> getDebugContent(const SceneBuilder& sceneBuilder);
};

} // namespace Falcor
