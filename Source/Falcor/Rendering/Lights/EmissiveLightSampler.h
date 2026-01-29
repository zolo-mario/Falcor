#pragma once
#include "EmissiveLightSamplerType.slangh"
#include "Core/Macros.h"
#include "Core/Program/DefineList.h"
#include "Scene/Lights/LightCollection.h"

namespace Falcor
{
    class RenderContext;
    struct ShaderVar;

    /** Base class for emissive light sampler implementations.

        All light samplers follows the same interface to make them interchangeable.
        If an unrecoverable error occurs, these functions may throw exceptions.
    */
    class FALCOR_API EmissiveLightSampler
    {
    public:
        virtual ~EmissiveLightSampler() = default;

        /** Updates the sampler to the current frame.
            \param[in] pRenderContext The render context.
            \param[in] pLightCollection Updated LightCollection
            \return True if the sampler was updated.
        */
        virtual bool update(RenderContext* pRenderContext, ref<ILightCollection> pLightCollection) { return false; }

        /** Return a list of shader defines to use this light sampler.
        *   \return Returns a list of shader defines.
        */
        virtual DefineList getDefines() const;

        /** Bind the light sampler data to a given shader var
        */
        virtual void bindShaderData(const ShaderVar& var) const {}

        /** Render the GUI.
            \return True if settings that affect the rendering have changed.
        */
        virtual bool renderUI(Gui::Widgets& widget) { return false; }

        /** Returns the type of emissive light sampler.
            \return The type of the derived class.
        */
        EmissiveLightSamplerType getType() const { return mType; }

    protected:
        EmissiveLightSampler(EmissiveLightSamplerType type, ref<ILightCollection> pLightCollection);
        void setLightCollection(ref<ILightCollection> pLightCollection);

        // Internal state
        const EmissiveLightSamplerType mType;       ///< Type of emissive sampler. See EmissiveLightSamplerType.slangh.
        ref<Device> mpDevice;
        ref<ILightCollection> mpLightCollection;
        sigs::Connection mUpdateFlagsConnection;
        ILightCollection::UpdateFlags mLightCollectionUpdateFlags = ILightCollection::UpdateFlags::None;
    };
}
