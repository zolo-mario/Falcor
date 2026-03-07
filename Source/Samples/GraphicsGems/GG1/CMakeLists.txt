add_plugin(GraphicsGems1)

target_sources(GraphicsGems1 PRIVATE
    GraphicsGems1.h
    GraphicsGems1.cpp
    src/GraphicsGems.h
    src/HSLtoRGB.c
)

# The .c files are #included directly via extern "C" in the .cpp;
# exclude them from the build system's own compile step to avoid duplicate symbols.
set_source_files_properties(src/HSLtoRGB.c PROPERTIES HEADER_FILE_ONLY TRUE)

target_source_group(GraphicsGems1 "Samples/GraphicsGems")
