import Rendering.Volumes.IPhaseFunction;

struct VolumeProperties
{
    float3 sigmaA; ///< Absorption coefficient.. or maybe cross section.
    float3 sigmaS; ///< Scattering coefficient.. or maybe cross section.
    IPhaseFunction phaseFunction; ///< Set to NullPhaseFunction when invalid
}
