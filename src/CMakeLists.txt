set(SRC_LIST example.c png_decoder.c)
add_executable(pngdecoder ${SRC_LIST})
include_directories(/usr/local/include/SDL2)
target_link_libraries(pngdecoder SDL2)
