add_falcor_executable(Karma)

target_sources(Karma PRIVATE
    Karma.cpp
    Karma.h
)

target_link_libraries(Karma PRIVATE Falcor args)

target_source_group(Karma "/")
