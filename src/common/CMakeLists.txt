
file(GLOB source_files CONFIGURE_DEPENDS
            "*.h"
            "*.cpp"
    )

message(STATUS "source_files: ${source_files}")


add_library(wuw_common SHARED STATIC ${source_files})