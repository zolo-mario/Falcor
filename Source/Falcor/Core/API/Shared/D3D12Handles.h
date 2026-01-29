#pragma once

#if FALCOR_HAS_D3D12
#include <d3d12.h>
#include <comdef.h>
#endif

#define FALCOR_MAKE_SMART_COM_PTR(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

namespace Falcor
{
#if FALCOR_HAS_D3D12
FALCOR_MAKE_SMART_COM_PTR(ID3DBlob);
FALCOR_MAKE_SMART_COM_PTR(ID3D12DescriptorHeap);
FALCOR_MAKE_SMART_COM_PTR(ID3D12RootSignature);
#endif

} // namespace Falcor
