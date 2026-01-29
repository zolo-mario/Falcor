#pragma once
#include "RenderPassReflection.h"
#include "Core/Macros.h"
#include "Core/API/fwd.h"
#include "Core/API/Resource.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Falcor
{
class FALCOR_API ResourceCache
{
public:
    using ResourcesMap = std::unordered_map<std::string, ref<Resource>>;

    /**
     * Properties to use during resource creation when its property has not been fully specified.
     */
    struct DefaultProperties
    {
        uint2 dims;                                      ///< Width, height of the swap chain
        ResourceFormat format = ResourceFormat::Unknown; ///< Format to use for texture creation
    };

    /**
     * Add/Remove reference to a graph input resource not owned by the cache
     * @param[in] name The resource's name
     * @param[in] pResource The resource to register. If this is null, will unregister the resource
     */
    void registerExternalResource(const std::string& name, const ref<Resource>& pResource);

    /**
     * Register a field that requires resources to be allocated.
     * @param[in] name String in the format of PassName.FieldName
     * @param[in] field Reflection data for the field
     * @param[in] timePoint The point in time for when this field is used. Normally this is an index into the execution order.
     * @param[in] alias Optional. Another field name described in the same way as 'name'.
     * If specified, and the field exists in the cache, the resource will be aliased with 'name' and field properties will be merged.
     */
    void registerField(
        const std::string& name,
        const RenderPassReflection::Field& field,
        uint32_t timePoint,
        const std::string& alias = ""
    );

    /**
     * Get a resource by name. Includes external resources known by the cache.
     */
    const ref<Resource>& getResource(const std::string& name) const;

    /**
     * Get the field-reflection of a resource
     */
    const RenderPassReflection::Field& getResourceReflection(const std::string& name) const;

    /**
     * Allocate all resources that need to be created/updated.
     * This includes new resources, resources whose properties have been updated since last allocation call.
     */
    void allocateResources(ref<Device> pDevice, const DefaultProperties& params);

    /**
     * Clears all registered field/resource properties and allocated resources.
     */
    void reset();

private:
    struct ResourceData
    {
        RenderPassReflection::Field field;      // Holds merged properties for aliased resources
        std::pair<uint32_t, uint32_t> lifetime; // Time range where this resource is being used
        ref<Resource> pResource;                // The resource
        bool resolveBindFlags;                  // Whether or not we should resolve the field's bind-flags before creating the resource
        std::string name;                       // Full name of the resource, including the pass name
    };

    // Resources and properties for fields within (and therefore owned by) a render graph
    std::unordered_map<std::string, uint32_t> mNameToIndex;
    std::vector<ResourceData> mResourceData;

    // References to output resources not to be allocated by the render graph
    ResourcesMap mExternalResources;
};

} // namespace Falcor
