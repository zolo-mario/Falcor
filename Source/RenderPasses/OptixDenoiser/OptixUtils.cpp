#include "OptixUtils.h"
#include "Utils/CudaUtils.h"
#include <cuda_runtime.h>

// Apparently: this include may only appear in a single source file:
#include <optix_function_table_definition.h>

// Some debug macros
#define OPTIX_CHECK(call)                                                                                                            \
    {                                                                                                                                \
        OptixResult result = call;                                                                                                   \
        if (result != OPTIX_SUCCESS)                                                                                                 \
        {                                                                                                                            \
            FALCOR_THROW("Optix call {} failed with error {} ({}).", #call, optixGetErrorName(result), optixGetErrorString(result)); \
        }                                                                                                                            \
    }

#define CUDA_CHECK(call)                                                                                                          \
    {                                                                                                                             \
        cudaError_t result = call;                                                                                                \
        if (result != cudaSuccess)                                                                                                \
        {                                                                                                                         \
            FALCOR_THROW("CUDA call {} failed with error {} ({}).", #call, cudaGetErrorName(result), cudaGetErrorString(result)); \
        }                                                                                                                         \
    }

void optixLogCallback(unsigned int level, const char* tag, const char* message, void*)
{
    Falcor::logWarning("[Optix][{:2}][{:12}]: {}", level, tag, message);
}

// This initialization now seems verbose / excessive as CUDA and OptiX initialization
// has evolved.  TODO: Simplify?
OptixDeviceContext initOptix(Falcor::Device* pDevice)
{
    FALCOR_CHECK(pDevice->initCudaDevice(), "Failed to initialize CUDA device.");

    OPTIX_CHECK(optixInit());

    FALCOR_CHECK(g_optixFunctionTable.optixDeviceContextCreate, "OptiX function table not initialized.");

    // Build our OptiX context
    OptixDeviceContext optixContext;
    OPTIX_CHECK(optixDeviceContextCreate(pDevice->getCudaDevice()->getContext(), 0, &optixContext));

    // Tell Optix how to write to our Falcor log.
    OPTIX_CHECK(optixDeviceContextSetLogCallback(optixContext, optixLogCallback, nullptr, 4));

    return optixContext;
}
