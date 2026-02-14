from falcor import *

def render_graph_MinimalPathTracerVBufferRaster():
    g = RenderGraph("MinimalPathTracerVBufferRaster")
    AccumulatePass = createPass("AccumulatePass", {'enabled': True, 'precisionMode': 'Single'})
    g.addPass(AccumulatePass, "AccumulatePass")
    ToneMapper = createPass("ToneMapper", {'autoExposure': False, 'exposureCompensation': 0.0})
    g.addPass(ToneMapper, "ToneMapper")
    MinimalPathTracer = createPass("MinimalPathTracer", {'maxBounces': 3})
    g.addPass(MinimalPathTracer, "MinimalPathTracer")
    VBufferRaster = createPass("VBufferRaster", {'samplePattern': 'Stratified', 'sampleCount': 16})
    g.addPass(VBufferRaster, "VBufferRaster")
    g.addEdge("AccumulatePass.output", "ToneMapper.src")
    g.addEdge("VBufferRaster.vbuffer", "MinimalPathTracer.vbuffer")
    # VBufferRaster does not output viewW; MinimalPathTracer computes it internally when not connected
    g.addEdge("MinimalPathTracer.color", "AccumulatePass.input")
    g.markOutput("ToneMapper.dst")
    return g

MinimalPathTracerVBufferRaster = render_graph_MinimalPathTracerVBufferRaster()
try: m.addGraph(MinimalPathTracerVBufferRaster)
except NameError: None
