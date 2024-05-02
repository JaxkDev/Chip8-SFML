cmake_minimum_required(VERSION 3.16)
project(
        Chip8-SFML
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/)

option(BUILD_SHARED_LIBS "Build shared libraries" TRUE)
set(SFML_STATIC_LIBRARIES FALSE)

include(FetchContent)
FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x)
if (NOT sfml_POPULATED)
    FetchContent_Populate(sfml)
    add_subdirectory(${sfml_SOURCE_DIR} ${sfml_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

add_executable(Chip8-SFML
        src/main.cpp
        src/Window.cpp
        src/vCPU.cpp
)
target_link_libraries(Chip8-SFML PRIVATE sfml-graphics sfml-system sfml-window sfml-network sfml-audio)
target_compile_features(Chip8-SFML PRIVATE cxx_std_20)

if (WIN32)
    add_custom_command(
            TARGET Chip8-SFML
            COMMENT "Copy OpenAL DLL"
            POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:Chip8-SFML>
            VERBATIM
    )
endif ()

add_custom_command(
        TARGET Chip8-SFML
        COMMENT "Copy assets folder"
        POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets/ $<TARGET_FILE_DIR:Chip8-SFML>/assets/
        VERBATIM
)

install(TARGETS Chip8-SFML)
