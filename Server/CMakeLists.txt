cmake_minimum_required(VERSION 3.20)
project(Cypress LANGUAGES C CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(FetchContent)

# Dependencies
### MinHook
FetchContent_Declare(
  minhook
  GIT_REPOSITORY https://github.com/TsudaKageyu/minhook.git
  GIT_TAG 1e9ad1eb42db11bfcb65461f687c656612d1b555
)

### EASTL
FetchContent_Declare(
  EASTL
  GIT_REPOSITORY https://github.com/kitsilanosoftware/EASTL.git
  GIT_TAG 4e576b099d2ef475262459aa8a888ebed5788807
)

###EABase
FetchContent_Declare(
  EABase
  GIT_REPOSITORY https://github.com/kitsilanosoftware/EABase.git
  GIT_TAG 53a97991c69c2f6394a8ff95057743daea6a1f3e
)

### json
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG 8167d2f64122c3ef7e5a0d9cab239fdba37aa54c
)

### Monocypher
FetchContent_Declare(
  monocypher
  GIT_REPOSITORY https://github.com/LoupVaillant/Monocypher.git
  GIT_TAG c0e0efd25b614ef22bb72aa71203a747904c370d
)

FetchContent_MakeAvailable(
  minhook
  EASTL
  EABase
  nlohmann_json
  monocypher
)

# Options for variants
option(CYPRESS_GW1 "Build GW1 version" OFF)
option(CYPRESS_GW2 "Build GW2 version" OFF)
option(CYPRESS_BFN "Build BFN version" OFF)

# Default fallback
if(NOT CYPRESS_GW1 AND NOT CYPRESS_GW2 AND NOT CYPRESS_BFN)
  set(CYPRESS_GW2 ON)
endif()

# Sources
file(GLOB_RECURSE SOURCES
    dllmain.cpp
    Source/*.cpp
    include/*.cpp
)

add_library(cypress SHARED ${SOURCES})

# Explicitly compile monocypher
target_sources(cypress PRIVATE
    ${monocypher_SOURCE_DIR}/src/monocypher.c
    ${EASTL_SOURCE_DIR}/source/assert.cpp
    ${EASTL_SOURCE_DIR}/source/string.cpp
)

set_source_files_properties(
    ${EASTL_SOURCE_DIR}/source/assert.cpp
    ${EASTL_SOURCE_DIR}/source/string.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

# Includes
target_include_directories(cypress PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Source
    ${monocypher_SOURCE_DIR}/src
    ${EASTL_SOURCE_DIR}/include
    ${EABase_SOURCE_DIR}/include/Common
)

# Precompiled headers
target_precompile_headers(cypress PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/pch.h>"
)

# Defines
target_compile_definitions(cypress PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    GW2SERVERPROJECT_EXPORTS
    _WINDOWS
    _USRDLL
    CHAR8_T_DEFINED
)

# Variant-specific defines + output name
if(CYPRESS_GW1)
  target_compile_definitions(cypress PRIVATE CYPRESS_GW1)
  set_target_properties(cypress PROPERTIES OUTPUT_NAME "cypress_GW1")
elseif(CYPRESS_GW2)
  target_compile_definitions(cypress PRIVATE CYPRESS_GW2)
  set_target_properties(cypress PROPERTIES OUTPUT_NAME "cypress_GW2")
elseif(CYPRESS_BFN)
  target_compile_definitions(cypress PRIVATE CYPRESS_BFN)
  set_target_properties(cypress PROPERTIES OUTPUT_NAME "cypress_BFN")
endif()

# MSVC specifics
if(MSVC)
  target_compile_options(cypress PRIVATE /W3 /MP)
endif()

# Link libraries
target_link_libraries(cypress PRIVATE
    minhook
    nlohmann_json
    ws2_32.lib
    winhttp.lib
    bcrypt.lib
    iphlpapi.lib
)

# Optimization flags
target_compile_options(cypress PRIVATE
    $<$<CONFIG:Release>:/O2>
)

# Custom step (TouchVersionInfo)
add_custom_command(
    TARGET cypress
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch
        ${CMAKE_SOURCE_DIR}/Source/Core/VersionInfo.cpp
)
