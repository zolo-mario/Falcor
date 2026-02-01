from falcor import *


def render_graph_MeshletCulling():
    g = RenderGraph("MeshletCulling")
    MeshletCulling = createPass(
        "MeshletCulling",
        {"enableCulling": True, "outputVBuffer": True, "visualizeMode": 0},
    )
    g.addPass(MeshletCulling, "MeshletCulling")

    g.markOutput("MeshletCulling.color")
    g.markOutput("MeshletCulling.color", TextureChannelFlags.Alpha)
    g.markOutput("MeshletCulling.vbuffer")
    g.markOutput("MeshletCulling.depth")
    return g


MeshletCulling = render_graph_MeshletCulling()
try:
    m.addGraph(MeshletCulling)
except NameError:
    None
