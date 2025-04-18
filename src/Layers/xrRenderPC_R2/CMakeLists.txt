project(xrRender_R2)
set(XRR2_SRC_FILES)

# Files import
file(GLOB_RECURSE XRR2_SOURCE_ALL_FILES
    "*.cpp"
    "*.h"
)
file(GLOB_RECURSE XRR2_SOURCE_COMP_FILES
    "../xrRender/*.cpp"
    "../xrRender/*.h"
)
file(GLOB_RECURSE XRR2_SOURCE_COMP_BLEND_FILES
    "../xrRender/Blender_*.cpp"
    "../xrRender/Blender_*.h"
)
file(GLOB_RECURSE XRR2_SOURCE_BLEND_FILES
    "Blender_*.cpp"
    "Blender_*.h"
)
file(GLOB_RECURSE XRR2_SOURCE_DX9_FILES
    "../xrRenderDX9/*.cpp"
    "../xrRenderDX9/*.h"
)
file(GLOB_RECURSE XRR2_SOURCE_LUA_FILES
    "../../xrEngine/ai_script_lua*.cpp"
    "../../xrEngine/ai_script_lua*.h"
)

# Source groups
source_group("render" FILES ${XRR2_SOURCE_ALL_FILES})
source_group("layers" FILES ${XRR2_SOURCE_COMP_FILES})
source_group("general_dx9" FILES ${XRR2_SOURCE_DX9_FILES})
source_group("lua_ext" FILES ${XRR2_SOURCE_LUA_FILES})
source_group("blend" FILES ${XRR2_SOURCE_COMP_BLEND_FILES})
source_group("blend" FILES ${XRR2_SOURCE_BLEND_FILES})

# Apply list
list(APPEND XRR2_SRC_FILES ${XRR2_SOURCE_ALL_FILES})
list(APPEND XRR2_SRC_FILES ${XRR2_SOURCE_COMP_FILES})
list(APPEND XRR2_SRC_FILES ${XRR2_SOURCE_DX9_FILES})
list(APPEND XRR2_SRC_FILES ${XRR2_SOURCE_LUA_FILES})

# Remove unused files
#list(FILTER XRR2_SRC_FILES EXCLUDE REGEX "../xrRender/r__pixel_calculator.cpp")

# xrRender_R2 project
add_library(xrRender_R2 SHARED ${XRR2_SRC_FILES})
target_precompile_headers(xrRender_R2 PRIVATE "stdafx.h")

# Include directories
target_include_directories(xrRender_R2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(xrRender_R2 PUBLIC "${IXRAY_SDK_INC}")

# Project defines
target_compile_definitions(xrRender_R2 PRIVATE _WINDOWS)
target_compile_definitions(xrRender_R2 PRIVATE XRRENDER_R2_EXPORTS)
target_compile_definitions(xrRender_R2 PRIVATE _USRDLL)

add_compile_options(/fp:fast)
target_compile_definitions(xrRender_R2 PRIVATE "$<$<CONFIG:Debug>:DEBUG>")

# Linker list
target_link_libraries(xrRender_R2 PUBLIC xrAbstractions)
target_link_libraries(xrRender_R2 PUBLIC xrParticles)
target_link_libraries(xrRender_R2 PUBLIC "d3dcompiler.lib")

# Nuget
add_custom_command(TARGET ${PROJECT_NAME}
    PRE_BUILD
    COMMAND ${NUGET_COMMAND} restore ${CMAKE_CURRENT_SOURCE_DIR}/Packages.config -SolutionDirectory ${CMAKE_BINARY_DIR}
)

## DirectX SDK
if(NOT "${CMAKE_VS_PLATFORM_NAME}" MATCHES "(x64)")
    target_link_libraries(xrRender_R2 PUBLIC ${CMAKE_BINARY_DIR}/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/release/lib/x86/d3dx9.lib)
else()
    target_link_libraries(xrRender_R2 PUBLIC ${CMAKE_BINARY_DIR}/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/release/lib/x64/d3dx9.lib)
endif()

target_include_directories(xrRender_R2 PUBLIC "${CMAKE_BINARY_DIR}/packages/Microsoft.DXSDK.D3DX.9.29.952.8/build/native/include/")
target_include_directories(xrRender_R2 PUBLIC "${CMAKE_BINARY_DIR}/packages/directxmesh_desktop_win10.2023.4.28.1/include/")

## RenderDoc
target_include_directories(xrRender_R2 PUBLIC "${RENDERDOC_API}")
