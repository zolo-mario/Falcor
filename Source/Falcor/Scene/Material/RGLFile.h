#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Falcor
{
    /** Class representing a measured material file from the RGL BRDF database.
    */
    class RGLFile
    {
    public:
        /** There are many more field types, but none that we need. Ignore all other types.
        */
        enum FieldType
        {
            UInt8 = 1,
            UInt32 = 5,
            Float32 = 10,
        };

        struct Field
        {
            std::string name;
            FieldType type;
            uint32_t dim;
            int64_t numElems;
            std::unique_ptr<uint64_t[]> shape;
            std::unique_ptr<uint8_t[]> data;
        };

        /** Collected set of fields necessary to render the BRDF.
        */
        struct MeasurementData
        {
            const Field* thetaI;
            const Field* phiI;
            const Field* sigma;
            const Field* ndf;
            const Field* vndf;
            const Field* rgb;
            const Field* luminance;
            bool isotropic;
            std::string description;
        };

        RGLFile() = default;

        /** Loads RGL measured BRDF file and validates contents. Throws Falcor::Exception on failure.
        */
        RGLFile(std::ifstream& in);

        void saveFile(std::ofstream& out) const;

        const MeasurementData& data() const
        {
            return mMeasurement;
        }

        void addField(const std::string& name, FieldType type, const std::vector<uint32_t>& shape, const void* data);

    private:
        std::unordered_map<std::string, int> mFieldMap;
        std::vector<Field> mFields;
        MeasurementData mMeasurement;

        /** Make sure all required fields are present and have correct shape and dimensions,
            then populates mMeasurement field if all fields are correct.
            Throws Falcor::Exception on validation error.
        */
        void validate();

        const Field* getField(const std::string& name) const;

        static size_t fieldSize(FieldType type);
    };
}
