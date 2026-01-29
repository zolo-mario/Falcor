#include "PythonImporter.h"
#include "Scene/Importer.h"
#include "GlobalState.h"
#include "Utils/Scripting/Scripting.h"
#include <filesystem>
#include <regex>

namespace Falcor
{

namespace
{
/**
 * Parse the legacy header on the first line of the script with the syntax:
 * # filename.extension
 */
static std::optional<std::string> parseLegacyHeader(const std::string& script)
{
    if (size_t endOfFirstLine = script.find_first_of("\n\r"); endOfFirstLine != std::string::npos)
    {
        const std::regex headerRegex(R"""(#\s+([\w-]+\.[\w]{1,10}))""");

        std::smatch match;
        if (std::regex_match(script.begin(), script.begin() + endOfFirstLine, match, headerRegex))
        {
            if (match.size() > 1)
                return match[1].str();
        }
    }

    return {};
}

/// Set of currently imported paths, used to avoid recursion. TODO: REMOVEGLOBAL
static std::set<std::filesystem::path> sImportPaths;
/// Keeps track of how many recursive importers are in process. TODO: REMOVEGLOBAL
static size_t sImportDepth;

/**
 * This class is used to handle nested imports through RAII.
 * It keeps a set of import paths in sImportPaths to detect recursive imports.
 * It keeps a stack of import directories in sImportdirectories and updates the global data search directories.
 * It also keeps track of Settings, keeping them scoped to the individual scenes
 */
class ScopedImport
{
public:
    ScopedImport(SceneBuilder& builder, const std::filesystem::path& path) : mBuilder(builder), mPath(path)
    {
        if (!path.empty())
        {
            FALCOR_ASSERT(path.is_absolute());
            sImportPaths.emplace(mPath);

            // Add base directory to search paths.
            mBuilder.pushAssetResolver();
            mBuilder.getAssetResolver().addSearchPath(path.parent_path(), SearchPathPriority::First);
        }

        // Set global scene builder as workaround to support old Python API.
        setActivePythonSceneBuilder(&mBuilder);
        sImportDepth++;
    }
    ~ScopedImport()
    {
        if (!mPath.empty())
        {
            auto erased = sImportPaths.erase(mPath);
            FALCOR_ASSERT(erased == 1);

            mBuilder.popAssetResolver();
        }

        // Unset global scene builder.
        FALCOR_ASSERT(sImportDepth > 0);
        if (--sImportDepth == 0)
            setActivePythonSceneBuilder(nullptr);
    }

private:
    SceneBuilder& mBuilder;
    std::filesystem::path mPath;
};

static bool isRecursiveImport(const std::filesystem::path& path)
{
    return sImportPaths.find(path) != sImportPaths.end();
}

} // namespace

std::unique_ptr<Importer> PythonImporter::create()
{
    return std::make_unique<PythonImporter>();
}

void PythonImporter::importScene(
    const std::filesystem::path& path,
    SceneBuilder& builder,
    const std::map<std::string, std::string>& materialToShortName
)
{
    if (!path.is_absolute())
        throw ImporterError(path, "Expected absolute path.");

    if (isRecursiveImport(path))
        throw ImporterError(path, "Scene is imported recursively.");

    // Load the script file
    const std::string script = readFile(path);

    importInternal(script, path, builder);
}

void PythonImporter::importSceneFromMemory(
    const void* buffer,
    size_t byteSize,
    std::string_view extension,
    SceneBuilder& builder,
    const std::map<std::string, std::string>& materialToShortName
)
{
    FALCOR_CHECK(extension == "pyscene", "Unexpected format.");
    FALCOR_CHECK(buffer != nullptr, "Missing buffer.");
    FALCOR_CHECK(byteSize > 0, "Empty buffer.");

    const std::string script(static_cast<const char*>(buffer), byteSize);

    importInternal(script, {}, builder);
}

void PythonImporter::importInternal(const std::string& script, const std::filesystem::path& path, SceneBuilder& builder)
{
    // Check for legacy .pyscene file format.
    if (auto sceneFile = parseLegacyHeader(script))
        throw ImporterError(path, "Python scene is using old header comment syntax. Use the new 'sceneBuilder' object instead.");

    // Keep track of this import and add script directory to data search directories.
    // We use RAII here to make sure the scope is properly removed when throwing an exception.
    ScopedImport scopedImport(builder, path);

    // Execute script.
    try
    {
        Scripting::Context context;
        context.setObject("sceneBuilder", &builder);
        Scripting::runScript("from falcor import *", context);
        if (path.empty())
            Scripting::runScript(script, context);
        else
            Scripting::runScriptFromFile(path, context);
    }
    catch (const std::exception& e)
    {
        throw ImporterError(path, fmt::format("Failed to run python scene script: {}", e.what()));
    }
}

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<Importer, PythonImporter>();
}

} // namespace Falcor
