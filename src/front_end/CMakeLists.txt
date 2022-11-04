
file(GLOB source_files CONFIGURE_DEPENDS
            "*/*.cpp"
            "*/*.h"
            "*.h"
            "*.cpp"
    )

message(STATUS "source_files: ${source_files}")

add_library(wuw_front_end ${source_files})

include_directories(../)
include_directories(./spectrum)
include_directories(./buffer)
include_directories(./enh)
include_directories(./mfcc)
include_directories(./vad)

target_link_libraries(wuw_front_end wuw_common wuw_back_end)