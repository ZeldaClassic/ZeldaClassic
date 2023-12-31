# Finds various DirectX components:
#
#  DINPUT
#  D3D9
#  D3DX9
#  DSOUND
#  XINPUT
#
#  For each component, this will fill out the following variables
#
#  ${COMPONENT}_INCLUDE_DIR - Where to find the component header.
#  ${COMPONENT}_LIBRARIES   - List of libraries when using the component.
#  ${COMPONENT}_FOUND       - True if the component is found.

macro(check_winsdk_root_dir key)
  get_filename_component(CANDIDATE ${key} ABSOLUTE)
  if (EXISTS ${CANDIDATE})
    set(WINSDK_ROOT_DIR ${CANDIDATE})
  endif()
endmacro()

check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Microsoft SDKs\\\\Windows\\\\v7.0;InstallationFolder]")
check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Microsoft SDKs\\\\Windows\\\\v7.0A;InstallationFolder]")
check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Microsoft SDKs\\\\Windows\\\\v7.1;InstallationFolder]")
check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Microsoft SDKs\\\\Windows\\\\v7.1A;InstallationFolder]")
check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Windows Kits\\\\Installed Roots;KitsRoot]")
check_winsdk_root_dir("[HKEY_LOCAL_MACHINE\\\\SOFTWARE\\\\Microsoft\\\\Windows Kits\\\\Installed Roots;KitsRoot81]")


# The Win10 SDK doesn't have many DX libraries (such as D3DX9), but it has some.
# Find the Win10 SDK path.
# https://github.com/microsoft/DirectXShaderCompiler/blob/main/cmake/modules/FindD3D12.cmake
if ("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "" )
    get_filename_component(WIN10_SDK_PATH "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    set (WIN10_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
elseif(TRUE)
    set (WIN10_SDK_PATH $ENV{WIN10_SDK_PATH})
    set (WIN10_SDK_VERSION $ENV{WIN10_SDK_VERSION})
endif ("$ENV{WIN10_SDK_PATH}$ENV{WIN10_SDK_VERSION}" STREQUAL "" )

# WIN10_SDK_PATH will be something like C:\Program Files (x86)\Windows Kits\10
# WIN10_SDK_VERSION will be something like 10.0.14393 or 10.0.14393.0; we need the
# one that matches the directory name.

if (IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")
    set(WIN10_SDK_VERSION  "${WIN10_SDK_VERSION}.0")
endif (IS_DIRECTORY "${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}.0")


if(CMAKE_CL_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(MINGW_W64_HINT "x86_64-w64-mingw32")
    set(PROCESSOR_SUFFIX "x64")
else()
    set(MINGW_W64_HINT "i686-w64-mingw32")
    set(PROCESSOR_SUFFIX "x86")
endif()

macro(find_component name header library)
    if(${name}_INCLUDE_DIR)
        # Already in cache, be silent
        set(${name}_FIND_QUIETLY TRUE)
    endif(${name}_INCLUDE_DIR)

    if(MSVC)
        find_path(${name}_INCLUDE_DIR ${header}
            PATH_SUFFIXES
                Include
                Include/um
                Include/shared
                um
                shared
            PATHS
                "$ENV{DXSDK_DIR}"
                ${WINSDK_ROOT_DIR}
                ${WIN10_SDK_PATH}/Include/${WIN10_SDK_VERSION}
            )

        find_library(${name}_LIBRARY
            NAMES lib${library} ${library}
            PATH_SUFFIXES
                Lib
                Lib/${PROCESSOR_SUFFIX}
                Lib/winv6.3/um/${PROCESSOR_SUFFIX}
                Lib/Win8/um/${PROCESSOR_SUFFIX}
                um/${PROCESSOR_SUFFIX}
            PATHS
                "$ENV{DXSDK_DIR}"
                ${WINSDK_ROOT_DIR}
                ${WIN10_SDK_PATH}/Lib/${WIN10_SDK_VERSION}
            )
    else()
        find_path(${name}_INCLUDE_DIR ${header}
            PATH_SUFFIXES
                Include
                ${MINGW_W64_HINT}/include
            PATHS
                "$ENV{DXSDK_DIR}"
            )

        find_library(${name}_LIBRARY
            NAMES lib${library} ${library}
            PATH_SUFFIXES
                ${MINGW_W64_HINT}/lib
                Lib
                Lib/${PROCESSOR_SUFFIX}
            PATHS
                "$ENV{DXSDK_DIR}"
            )
    endif()

    # Handle the QUIETLY and REQUIRED arguments and set ${name}_FOUND to TRUE if
    # all listed variables are TRUE.
    set(FPHSA_NAME_MISMATCHED TRUE)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(${name} DEFAULT_MSG
        ${name}_INCLUDE_DIR ${name}_LIBRARY)

    if(${name}_FOUND)
        set(${name}_LIBRARIES ${${name}_LIBRARY})
    else()
        set(${name}_LIBRARIES)
    endif()

    mark_as_advanced(${name}_INCLUDE_DIR ${name}_LIBRARY)
endmacro()

find_component(DINPUT dinput.h dinput8)
find_component(D3D9 d3d9.h d3d9)
find_component(D3DX9 d3dx9.h d3dx9)
find_component(DSOUND dsound.h dsound)
find_component(XINPUT xinput.h xinput)
