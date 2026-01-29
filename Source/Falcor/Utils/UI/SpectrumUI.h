#pragma once
#include "Core/Macros.h"
#include "Utils/Math/Vector.h"
#include "Utils/Color/SampledSpectrum.h"
#include "Utils/UI/Gui.h"
#include <imgui.h>
#include <string>
#include <vector>

namespace Falcor
{
namespace
{
Gui::DropdownList kInterpolationDropdownList = {
    {(uint32_t)SpectrumInterpolation::Linear, "Linear"},
};
}

/**
 * User interface for SampledSpectrum<T>. Implemented using ImGui and Gui::Widgets. Can be called as widget.spectrum(...) or
 * renderSpectrumUI(...).
 */
template<typename T>
class FALCOR_API SpectrumUI
{
public:
    SpectrumUI();
    SpectrumUI(const float2& wavelengthRange, const float2& spectralIntensityRange);
    void setWavelengthRange(const float2& range) { mWavelengthRange = range; }
    void setSpectralIntensityRange(const float2& range) { mSpectralIntensityRange = range; }
    bool render(Gui::Widgets& w, const std::string name, std::vector<SampledSpectrum<T>*> spectra, const bool renderOnlySpectrum = false);

protected:
    std::string makeUnique(const std::string& str) const;
    float getSpectralIntensity(const float wavelength, const SampledSpectrum<T>* spectrum, const uint32_t curveIndex) const;
    float getSpectralIntensity(const uint32_t pointIndex, const SampledSpectrum<T>* spectrum, const uint32_t curveIndex) const;
    uint32_t getNumComponents() const;
    void drawLine(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& point0,
        const float2& point1,
        const float4& color,
        const float lineWidth = 2.0f
    );
    void drawCircle(ImDrawList* drawList, const float2& canvasPos, const float2& center, const float radius, const float4& color);
    void textHorizontallyCentered(const std::string& text, const float2& pos, const float4& color);
    void textVerticallyCenteredLeft(const std::string& text, const float2& pos, const float4& color);
    float toXCoord(const float wavelength, const float2& xAxisRange) const;
    float toYCoord(const float spectralIntensity, const float2& yAxisRange) const;
    float2 toCoords(const float wavelength, const float spectralIntensity, const float2& xAxisRange, const float2& yAxisRange) const;
    float2 toCoords(
        const SampledSpectrum<T>* spectrum,
        const int index,
        const float2& xAxisRange,
        const float2& yAxisRange,
        const uint32_t float3Index = 0
    ) const;
    void drawColorMatchingFunctions(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& canvasSize,
        const float2& xAxisRange,
        const float2& yAxisRange
    );
    void drawTextWavelengthsAndTicks(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& xAxisRange,
        const float2& yAxisRange,
        const float4& textColor,
        const float4& tickColor,
        const float4& gridColor
    );
    void drawTextSpectralIntensityAndTicks(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& xAxisRange,
        const float2& yAxisRange,
        const float4& textColor,
        const float4& tickColor,
        const float4& gridColor
    );
    void drawSpectrumBar(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& xAxisRange,
        const float2& yAxisRange,
        SampledSpectrum<T>* spectrum,
        const bool multiplyBySpectrum
    );
    void drawSpectrumCurve(
        ImDrawList* drawList,
        const float2& canvasPos,
        const float2& canvasSize,
        const float2& xAxisRange,
        const float2& yAxisRange,
        SampledSpectrum<T>* spectrum,
        const uint32_t spectrumIndex
    );
    bool handleMouse(
        const float2& canvasPos,
        const float2& canvasSize,
        const float2& xAxisRange,
        const float2& yAxisRange,
        ImDrawList* drawList,
        SampledSpectrum<T>* spectrum,
        const uint32_t float3Index = 0
    );
    float generateNiceNumber(const float x) const;

protected:
    // UI parameters.
    float2 mWavelengthRange = float2(350.0f, 750.0f);
    float2 mSpectralIntensityRange = float2(0.0f, 1.0f);
    uint32_t mEditSpectrumIndex = 0;
    bool mDrawSpectrumBar = true;
    bool mMultiplyWithSpectralIntensity = true;
    bool mDrawGridX = true;
    bool mDrawGridY = true;
    uint32_t mDrawAreaHeight = 300u;
    bool mDrawColorMatchingFunctions = false;

    bool mMovePoint = false;
    uint32_t mPointIndexToBeEdited = 0u;
    SpectrumInterpolation mInterpolationType = SpectrumInterpolation::Linear;
};

template<typename T>
bool renderSpectrumUI(Gui::Widgets& w, SampledSpectrum<T>& spectrum, const char label[] = "Spectrum UI")
{
    // Use default parameters. Those will not be saved from frame to frame.
    SpectrumUI<T> spectrumUI;
    // True in the last parameter means that the UI for changing parameters will not be shown.
    return spectrumUI.render(w, label, {&spectrum}, true);
}

template<typename T>
bool renderSpectrumUI(Gui::Widgets& w, SampledSpectrum<T>& spectrum, SpectrumUI<T>& spectrumUI, const char label[] = "Spectrum UI")
{
    // False in the last parameter means that the UI for changing parameters will be shown.
    return spectrumUI.render(w, label, {&spectrum}, false);
}

template<typename T>
bool renderSpectrumUI(
    Gui::Widgets& w,
    std::vector<SampledSpectrum<T>*>& spectra,
    SpectrumUI<T>& spectrumUI,
    const char label[] = "Spectrum UI"
)
{
    // False in the last parameter means that the UI for changing parameters will be shown.
    return spectrumUI.render(w, label, spectra, false);
}
} // namespace Falcor
