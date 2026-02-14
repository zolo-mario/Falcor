from falcor import *

def render_graph_MinimalPathTracer():
    g = RenderGraph("MinimalPathTracer")
    AccumulatePass = createPass("AccumulatePass", {'enabled': True, 'precisionMode': 'Single'})
    g.addPass(AccumulatePass, "AccumulatePass")
    MinimalPathTracer = createPass("MinimalPathTracer", {'maxBounces': 3})
    g.addPass(MinimalPathTracer, "MinimalPathTracer")
    VBufferMeshletRaster = createPass("VBufferMeshletRaster")
    g.addPass(VBufferMeshletRaster, "VBufferMeshletRaster")
    g.addEdge("VBufferMeshletRaster.vbuffer", "MinimalPathTracer.vbuffer")
    # viewW optional - MinimalPathTracer computes internally when not connected
    g.addEdge("MinimalPathTracer.color", "AccumulatePass.input")
    g.markOutput("MinimalPathTracer.color")
    return g

MinimalPathTracer = render_graph_MinimalPathTracer()
try: m.addGraph(MinimalPathTracer)
except NameError: None
