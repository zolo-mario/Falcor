/** Data and resources needed for displacement mapping.
*/
struct DisplacementData
{
    static const float2 kShellMinMaxMargin = float2(-0.0001f, 0.0001f);
    static const float2 kSurfaceSafetyScaleBias = float2(1.0001, 0.0001);

    Texture2D texture;              ///< Displacement texture.
    SamplerState samplerState;      ///< Displacement texture sampler.
    SamplerState samplerStateMin;   ///< Displacement texture sampler Min.
    SamplerState samplerStateMax;   ///< Displacement texture sampler Max.
    float2 size;                    ///< Texture size in texels.
    float scale;                    ///< Displacement scale.
    float bias;                     ///< Displacement bias.
};
