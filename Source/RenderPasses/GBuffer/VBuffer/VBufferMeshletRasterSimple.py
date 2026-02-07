from falcor import *


def render_graph_VBufferMeshletRasterSimple():
    g = RenderGraph("VBufferMeshletRasterSimple")

    # Create VBufferMeshletRaster pass
    vbuffer = createPass("VBufferMeshletRaster")
    g.addPass(vbuffer, "VBufferMeshletRaster")

    # Mark outputs directly without connecting to other passes
    g.markOutput("VBufferMeshletRaster.vbuffer")
    g.markOutput("VBufferMeshletRaster.depth")

    return g


VBufferMeshletRasterSimple = render_graph_VBufferMeshletRasterSimple()
try:
    m.addGraph(VBufferMeshletRasterSimple)
except NameError:
    None
