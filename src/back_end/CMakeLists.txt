
file(GLOB source_files CONFIGURE_DEPENDS
            "*.h"
            "*.cpp"
    )

message(STATUS "source_files: ${source_files}")

add_library(wuw_back_end ${source_files})

include_directories(../)
include_directories(./svm)
include_directories(./hmm)

target_link_libraries(wuw_back_end wuw_common)