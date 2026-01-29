__exported import Scene.HitInfo;
__exported import Utils.Math.Ray;

// TODO: Use default arguments when slang supports them:
// bool traceRay(const RayDesc ray, out HitInfo hit, out float hitT, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff);
// bool traceVisibilityRay(const RayDesc ray, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff);

/** Interface for scene ray queries.
*/
interface ISceneRayQuery
{
    /** Trace a ray against the scene and return the closest hit point.
        \param[in] ray Ray.
        \param[out] hitT Hit distance (only valid if function returns true).
        \param[in] rayFlags Optional ray flags OR'ed with queries flags.
        \param[in] instanceInclusionMask Includes/rejects geometry based on instance mask.
        \return Returns Hit info, only valid when there was a hit.
    */
    HitInfo traceRay(const Ray ray, out float hitT, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff);

    /** Trace a visibility ray against the scene.
        \param[in] ray Ray.
        \param[in] rayFlags Optional ray flags OR'ed with queries flags.
        \param[in] instanceInclusionMask Includes/rejects geometry based on instance mask.
        \return Returns true if the ray endpoints are mutually visible (i.e. the ray does NOT intersect the scene).
    */
    bool traceVisibilityRay(const Ray ray, uint rayFlags = RAY_FLAG_NONE, uint instanceInclusionMask = 0xff);
};
