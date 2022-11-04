cmake_minimum_required(VERSION 3.7.2)
set(CMAKE_CXX_STANDARD 17)
# set the project name
project(run_wuw)

include(CTest)
enable_testing()

#add_library(wuw_common SHARED ${LIBA_SRC})

include_directories(src)
include_directories(src/common)
include_directories(src/backend_end)
include_directories(src/front_end)


file(GLOB source_files CONFIGURE_DEPENDS
            "src/audio_stream.cpp",
            "src/process_in_samples.cpp",
            "src/*.h",
    )

#message(STATUS "source_files: ${source_files}")

#target_include_directories (wuw PUBLIC ${PROJECT_SOURCE_DIR})

add_executable(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)
add_library(main_source ${source_files})

add_subdirectory(src/common)
add_subdirectory(src/front_end)
add_subdirectory(src/back_end)

target_link_libraries(${PROJECT_NAME} main_source wuw_common wuw_front_end wuw_back_end)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

project(run_gen_fef)

add_executable(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)

target_link_libraries(${PROJECT_NAME} main_source wuw_common wuw_front_end wuw_back_end)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

project(train_hmm)

add_executable(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)

target_link_libraries(${PROJECT_NAME} main_source wuw_common wuw_front_end wuw_back_end)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

project(run_gen_hmm)

add_executable(${PROJECT_NAME} src/${PROJECT_NAME}.cpp)

target_link_libraries(${PROJECT_NAME} main_source wuw_common wuw_front_end wuw_back_end)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
