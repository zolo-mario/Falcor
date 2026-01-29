#pragma once

#include "ObjectID.h"
#include <pybind11/pybind11.h>

namespace pybind11::detail
{
template<typename TKindEnum, TKindEnum TKind, typename TIntType>
struct type_caster<Falcor::ObjectID<TKindEnum, TKind, TIntType>>
{
    using ObjectID = Falcor::ObjectID<TKindEnum, TKind, TIntType>;

public:
    PYBIND11_TYPE_CASTER(ObjectID, const_name("ObjectID"));

    bool load(handle src, bool)
    {
        PyObject* source = src.ptr();
        PyObject* tmp = PyNumber_Long(source);
        if (!tmp)
            return false;

        typename ObjectID::IntType idValue = PyLong_AsUnsignedLong(tmp);
        Py_DECREF(tmp);

        value = (idValue == ObjectID::kInvalidID) ? ObjectID() : ObjectID(idValue);
        return !PyErr_Occurred();
    }

    static handle cast(const ObjectID& src, return_value_policy /* policy */, handle /* parent */)
    {
        return PyLong_FromUnsignedLong(src.get());
    }
};
} // namespace pybind11::detail
