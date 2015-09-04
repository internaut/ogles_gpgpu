if(is_ios)
  add_subdirectory(ios)
elseif(ANDROID)
  # TODO: Need cmake-fu for APK packaging
  #add_subdirectory(android)
else()
  # TODO: Possibly add working (glfw ?) sample
  #add_subdirectory(opengl)
endif()