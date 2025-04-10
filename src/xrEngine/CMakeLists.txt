project(xrAbstractions)
set(ENGINE_SRC_FILES)

# Files import
file(GLOB_RECURSE ENGINE_SOURCE_INTERFACE_FILES
    "I*.cpp"
    "I*.h"
)
file(GLOB_RECURSE ENGINE_SOURCE_ALL_FILES
    "ai*.cpp"
    "ai*.h"
)
file(GLOB_RECURSE ENGINE_SOURCE_DEVICE_FILES
    "device*.*"
    "Device*.*"
)
file(GLOB_RECURSE ENGINE_SOURCE_ENV_FILES
    "Environment*.*"
)
file(GLOB_RECURSE ENGINE_SOURCE_WEDIT_FILES
    "editor*.*"
)
file(GLOB_RECURSE ENGINE_SOURCE_KERNEL_FILES
    "x_ray.*"
    "EngineAPI.*"
    "EngineExternal*.*"
)
file(GLOB_RECURSE ENGINE_SOURCE_ALL_FILES CONFIGURE_DEPENDS
    "*.cpp"
    "*.c"
    "*.h"
)

# File groups
source_group("core" FILES ${ENGINE_SOURCE_ALL_FILES})
source_group("kernel" FILES ${ENGINE_SOURCE_KERNEL_FILES})
source_group("interfaces" FILES ${ENGINE_SOURCE_INTERFACE_FILES})
source_group("lua" FILES ${ENGINE_SOURCE_LUA_FILES})
source_group("device" FILES ${ENGINE_SOURCE_DEVICE_FILES})
source_group("weather_editor" FILES ${ENGINE_SOURCE_WEDIT_FILES})
source_group("environment" FILES ${ENGINE_SOURCE_ENV_FILES})

# Apply list
list(APPEND ENGINE_SRC_FILES ${ENGINE_SOURCE_ALL_FILES})

# Remove files
#list(FILTER ENGINE_SRC_FILES EXCLUDE REGEX "")

# xrAbstractions project
add_library(xrAbstractions SHARED ${ENGINE_SRC_FILES})
set_property(TARGET xrAbstractions PROPERTY ENABLE_EXPORTS ON)

# Include directories
target_include_directories(xrAbstractions PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(xrAbstractions PUBLIC "${IXRAY_SDK_INC}")

# Move precompiled header
set_target_properties(xrAbstractions PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
set_target_properties(xrAbstractions PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
target_precompile_headers(xrAbstractions PUBLIC "stdafx.h")

file(GLOB_RECURSE ENGINE_SOURCE_PCH_FILES "stdafx.*")
file(GLOB_RECURSE ENGINE_SOURCE_ALL_C_FILES "*.c")

set_source_files_properties(${ENGINE_SOURCE_ALL_C_FILES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
source_group("pch" FILES ${ENGINE_SOURCE_PCH_FILES})

# Project defines
target_compile_definitions(xrAbstractions PRIVATE ENGINE_BUILD)

add_compile_options(/fp:fast)
target_compile_definitions(xrAbstractions PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

# Linker list
target_link_libraries(xrAbstractions PUBLIC xrCore)
target_link_libraries(xrAbstractions PUBLIC xrCDB)
target_link_libraries(xrAbstractions PUBLIC xrAPI)
target_link_libraries(xrAbstractions PUBLIC xrSound)
target_link_libraries(xrAbstractions PUBLIC luabind)
target_link_libraries(xrAbstractions PUBLIC xrNetServer)
target_link_libraries(xrAbstractions PUBLIC "Winmm.lib")
target_link_libraries(xrAbstractions PUBLIC "Vfw32.lib")

# Nuget
set(ENGINE_THRA ${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.LibTheora.1.1.1.3/)
set(FREEIMAGE ${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.FreeImage.WinMerge.3.18.0.20230819)

add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${NUGET_COMMAND} restore ${CMAKE_CURRENT_SOURCE_DIR}/Packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ENGINE_THRA}native/bin/${CMAKE_VS_PLATFORM_NAME}/Release/libtheora.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/
)

## Theora
target_link_libraries(xrAbstractions PUBLIC ${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.LibTheora.1.1.1.3/native/lib/${CMAKE_VS_PLATFORM_NAME}/Release/libtheora.lib)
target_include_directories(xrAbstractions PUBLIC "${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.LibTheora.1.1.1.3/native/include/")

## OGG
target_link_libraries(xrAbstractions PUBLIC ${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.LibOgg.1.3.5.4/native/lib/${CMAKE_VS_PLATFORM_NAME}/Release/libogg.lib)
target_include_directories(xrAbstractions PUBLIC "${CMAKE_BINARY_DIR}/packages/ImeSense.Packages.LibOgg.1.3.5.4/native/include/")

## FreeImage
if(NOT "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(x64)")
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREEIMAGE}/native/bin/x86/Release/FreeImage.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/
    )
    target_link_libraries(xrAbstractions PUBLIC ${FREEIMAGE}/native/lib/x86/Release/FreeImage.lib)
else()
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREEIMAGE}/native/bin/x64/Release/FreeImage.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/
    )
    target_link_libraries(xrAbstractions PUBLIC ${FREEIMAGE}/native/lib/x64/Release/FreeImage.lib)
endif()
