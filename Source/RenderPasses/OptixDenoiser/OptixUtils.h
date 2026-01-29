#pragma once

#include "Falcor.h"
#include "Utils/CudaRuntime.h"
#include "Utils/CudaUtils.h"

#include <optix.h>
#include <optix_stubs.h>

/**
 * Utility class for a GPU/device buffer for use with CUDA.
 * Adapted from Ingo Wald's SIGGRAPH 2019 tutorial code for OptiX 7.
 */
class CudaBuffer
{
public:
    CudaBuffer() {}

    CUdeviceptr getDevicePtr() { return (CUdeviceptr)mpDevicePtr; }
    size_t getSize() { return mSizeBytes; }

    void allocate(size_t size)
    {
        if (mpDevicePtr)
            free();
        mSizeBytes = size;
        FALCOR_CUDA_CHECK(cudaMalloc((void**)&mpDevicePtr, mSizeBytes));
    }

    void resize(size_t size) { allocate(size); }

    void free()
    {
        FALCOR_CUDA_CHECK(cudaFree(mpDevicePtr));
        mpDevicePtr = nullptr;
        mSizeBytes = 0;
    }

private:
    size_t mSizeBytes = 0;
    void* mpDevicePtr = nullptr;
};

// Initializes OptiX context. Throws an exception if initialization fails.
OptixDeviceContext initOptix(Falcor::Device* pDevice);
