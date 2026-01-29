#pragma once
#include "Fence.h"
#include "Core/Macros.h"
#include "Core/Error.h"
#include "Core/Object.h"
#include <queue>

namespace Falcor
{
template<typename ObjectType>
class FALCOR_API FencedPool : public Object
{
    FALCOR_OBJECT(FencedPool)
public:
    using NewObjectFuncType = ObjectType (*)(void*);

    /**
     * Create a new fenced pool.
     * @param[in] pFence GPU fence to use for synchronization.
     * @param[in] newFunc Ptr to function called to create new objects.
     * @param[in] pUserData Optional ptr to user data passed to the object creation function.
     * @return A new object, or throws an exception if creation failed.
     */
    static ref<FencedPool> create(ref<Fence> pFence, NewObjectFuncType newFunc, void* pUserData = nullptr)
    {
        return new FencedPool(pFence, newFunc, pUserData);
    }

    /**
     * Return an object.
     * @return An object, or throws an exception on failure.
     */
    ObjectType newObject()
    {
        // Retire the active object
        Data data;
        data.alloc = mActiveObject;
        data.timestamp = mpFence->getSignaledValue();
        mQueue.push(data);

        // The queue is sorted based on time. Check if the first object is free
        data = mQueue.front();
        if (data.timestamp < mpFence->getCurrentValue())
        {
            mQueue.pop();
        }
        else
        {
            data.alloc = createObject();
        }

        mActiveObject = data.alloc;
        return mActiveObject;
    }

private:
    FencedPool(ref<Fence> pFence, NewObjectFuncType newFunc, void* pUserData) : mNewObjFunc(newFunc), mpFence(pFence), mpUserData(pUserData)
    {
        FALCOR_ASSERT(pFence && newFunc);
        mActiveObject = createObject();
    }

    ObjectType createObject()
    {
        ObjectType pObj = mNewObjFunc(mpUserData);
        if (pObj == nullptr)
            FALCOR_THROW("Failed to create new object in fenced pool");
        return pObj;
    }

    struct Data
    {
        ObjectType alloc;
        uint64_t timestamp;
    };

    ObjectType mActiveObject;
    NewObjectFuncType mNewObjFunc = nullptr;
    std::queue<Data> mQueue;
    ref<Fence> mpFence;
    void* mpUserData;
};
} // namespace Falcor
