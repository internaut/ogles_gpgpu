include(sugar_include)
sugar_include(.)

add_library(ogles_gpgpu ${OGLES_GPGPU_SRCS} )
if(ANDROID)
  target_link_libraries(ogles_gpgpu PUBLIC EGL GLESv2)
endif()
set_property(TARGET ${library} PROPERTY FOLDER "libs/ogles_gpgpu")

#
# Provide interface lib for clean package config use
#

if(NOT OGLES_GPGPU_INSTALL)
  return()
endif()

### Install ###
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Note: use 'targets_export_name'
configure_file("../cmake/Config.cmake.in" "${project_config}" @ONLY)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${include_install_dir}/${PROJECT_NAME}"
)

install(
    FILES
    ${OGLES_GPGPU_PUBLIC_HDRS}
    DESTINATION "${include_install_dir}/${PROJECT_NAME}"
)

install(
    FILES
    ${OGLES_GPGPU_COMMON_PUBLIC_HDRS}
    DESTINATION "${include_install_dir}/${PROJECT_NAME}/common"
)

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)
