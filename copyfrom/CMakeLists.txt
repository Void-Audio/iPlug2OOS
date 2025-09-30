cmake_minimum_required(VERSION 3.14)

project(Rend VERSION 1.0.0.0 DESCRIPTION "Rend VST 3 Plug-in")

# === Options ===
option(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES "Enable VST 3 Plug-in Examples" OFF)
option(SMTG_ENABLE_VST3_HOSTING_EXAMPLES "Enable VST 3 Hosting Examples" OFF)

# === SDK Setup ===
set(vst3sdk_SOURCE_DIR "C:/Users/Derek Wingard/Desktop/Work/Plugins/VST_SDK/vst3sdk")
if(NOT EXISTS "${vst3sdk_SOURCE_DIR}")
    message(FATAL_ERROR "VST3 SDK not found at: ${vst3sdk_SOURCE_DIR}")
endif()

set(REND_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/VST3/$<CONFIG>/Rend.vst3/Contents/x86_64-win")

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

# === Plugin Source ===
smtg_add_vst3plugin(Rend
    source/version.h
    source/cids.h
    source/validity.h
    source/validity.cpp
    source/licenser.h
    source/editor.h
    source/editordelegate.h
    source/compression.h
    source/compression.cpp
    source/processor.h
    source/processor.cpp
    source/controller.h
    source/controller.cpp
    source/factory.cpp
)


# === Optional GUI Support ===
target_sources(Rend PRIVATE resource/editor.uidesc)
target_link_libraries(Rend PRIVATE vstgui_support)

smtg_target_add_plugin_resources(Rend
    RESOURCES 
    "resource/editor.uidesc"
)

smtg_target_add_plugin_snapshots(Rend
    RESOURCES
        resource/8E9537EDF9915C74B33300A75FBCE049_snapshot.png
        resource/8E9537EDF9915C74B33300A75FBCE049_snapshot_2.0x.png
)

# === External DSPFilters ===
target_include_directories(Rend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../DSPFilters/shared/DSPFilters/include
)
file(GLOB DSPFILTERS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../DSPFilters/shared/DSPFilters/source/*.cpp
)
target_sources(Rend PRIVATE ${DSPFILTERS_SOURCES})


# === STATIC ===

# === LicenseSpring Integration ===
set(LICENSESPRING_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/LicenseSpring/include")
set(LICENSESPRING_LIB_PATH "${CMAKE_SOURCE_DIR}/LicenseSpring/bin/x64/staticMD")

target_include_directories(Rend PRIVATE ${LICENSESPRING_INCLUDE_PATH})

# Define LS_STATIC to disable __declspec(dllimport) for static linking
target_compile_definitions(Rend PRIVATE LS_STATIC CURL_STATICLIB)


# Link against static LicenseSpring lib (adjust lib names & configs as needed)
target_link_libraries(Rend PRIVATE
    $<$<CONFIG:Debug>:${LICENSESPRING_LIB_PATH}/LicenseSpringD.lib>
    $<$<CONFIG:Release>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
    $<$<CONFIG:RelWithDebInfo>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
    $<$<CONFIG:MinSizeRel>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
    
    # Static OpenSSL and libcurl libraries
    ${LICENSESPRING_LIB_PATH}/libcrypto.lib
    ${LICENSESPRING_LIB_PATH}/libssl.lib
    ${LICENSESPRING_LIB_PATH}/libcurl.lib
    
    # Required Windows system libraries
    ws2_32.lib
    crypt32.lib
    winhttp.lib
    wldap32.lib
    normaliz.lib
)

# === DYNAMIC ===

# # === LicenseSpring Integration (Dynamic Linking) ===
# set(LICENSESPRING_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/LicenseSpring/include")
# set(LICENSESPRING_LIB_PATH "${CMAKE_SOURCE_DIR}/LicenseSpring/bin/x64/dynamic")  
# set(LICENSESPRING_DLL_PATH "${LICENSESPRING_LIB_PATH}/LicenseSpring.dll")        

# target_include_directories(Rend PRIVATE ${LICENSESPRING_INCLUDE_PATH})

# # Remove LS_STATIC to enable __declspec(dllimport) for dynamic linking
# # (This macro must NOT be defined when using dynamic linking)

# # Link against LicenseSpring import library (.lib for the DLL)
# target_link_libraries(Rend PRIVATE
#     $<$<CONFIG:Debug>:${LICENSESPRING_LIB_PATH}/LicenseSpringD.lib>
#     $<$<CONFIG:Release>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
#     $<$<CONFIG:RelWithDebInfo>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
#     $<$<CONFIG:MinSizeRel>:${LICENSESPRING_LIB_PATH}/LicenseSpring.lib>
# )

# # Copy LicenseSpring.dll to plugin output directory after build
# add_custom_command(TARGET Rend POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#         ${LICENSESPRING_DLL_PATH}
#         ${SWELL_OUTPUT_DIR}
#     COMMENT "Copying LicenseSpring.dll to output directory..."
# )


# === Steinberg VST SDK Dependencies ===
target_link_libraries(Rend PRIVATE
    sdk
    base
    pluginterfaces
)

# === Version Info ===
smtg_target_configure_version_file(Rend)

# === Platform-specific ===
if(SMTG_MAC)
    smtg_target_set_bundle(Rend
        BUNDLE_IDENTIFIER fromtheVOID
        COMPANY_NAME "Void Audio"
    )
    smtg_target_set_debug_executable(Rend
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(Rend PRIVATE resource/win32resource.rc)

    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Rend)
        smtg_target_set_debug_executable(Rend
            "$ENV{ProgramW6432}/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )

        # Force MSVC to use static runtime (/MT) instead of /MD
        foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                         CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
                         CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                         CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endforeach()
    endif()
endif()
target_compile_definitions(Rend PRIVATE VSTGUI_DISABLE_DEPRECATED_EDITORS=1)

# === Copy Resource Folder to VST3 Resources Folder After Build ===
add_custom_command(TARGET Rend POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resource"
        "$<TARGET_FILE_DIR:Rend>/../Resources"
    COMMENT "Copying resource files to VST3 bundle Resources directory"
)