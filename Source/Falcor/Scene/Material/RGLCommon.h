#pragma once
#include "Utils/Math/Vector.h"
#include <memory>

namespace Falcor
{
    /** This class represents a 2D table of 2D distributions.
        The basic model is that given coordinates x, y,
        we cut out a 2D `slice' from the 4D table and then
        sample it to obtain coordinates z, w.

        Within each distribution, the PDFs are linearly interpolated
        with respect to z, w. Also, slices are linearly interpolated
        from the table with respect to x, y.

        To achieve this goal, we first build the marginal/conditional
        distribution for each 2D slice of the 4D table similar to
        Pharr et al., with the only twist is that the PDF is linearly
        interpolated, i.e. the CDFs store the integral of a linearly
        interpolated PDF instead of the straight sum of the PDF.

        The actual interpolation/sampling at runtime happens on the GPU
        (see RGLCommon.slang)
    */
    class SamplableDistribution4D
    {
    public:
        SamplableDistribution4D(const float* pdf, uint4 size);

        const float* getPDF()         { return mPDF.get();         }

        const float* getMarginal()    { return mMarginal.get();    }

        const float* getConditional() { return mConditional.get(); }

    private:
        uint4 mSize;
        std::unique_ptr<float[]> mPDF;
        std::unique_ptr<float[]> mMarginal;
        std::unique_ptr<float[]> mConditional;

        void build2DSlice(int2 size, float* pdf, float* marginalCdf, float* conditionalCdf);
    };
}
