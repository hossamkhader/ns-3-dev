set(libs "${libs_to_build}")

# Process the visualizer module first if enabled
if(visualizer IN_LIST libs)
  if(${ENABLE_VISUALIZER})
    message(STATUS "Processing src/visualizer")
    add_subdirectory(visualizer)
  endif()
  list(REMOVE_ITEM libs visualizer)
endif()

# Process subdirectories
foreach(libname ${libs})
  if(EXISTS ${PROJECT_SOURCE_DIR}/src/${libname}/CMakeLists.txt)
    message(STATUS "Processing src/${libname}")
    add_subdirectory(${libname})
  else()
    message(${HIGHLIGHTED_STATUS}
            "Skipping src/${libname}: it does not contain a CMakeLists.txt file"
    )
  endif()
endforeach()

# Scan for disabled modules that define settings prefixed with NS3_ or ENABLE_
# so that we can initialize them as turned off (setting as temporary variables
# not to affect the cached variables that are set when normally processed)
subdirlist(modules ${CMAKE_CURRENT_SOURCE_DIR})
foreach(libname ${modules})
  if(NOT (${libname} IN_LIST libs))
    # Skip module directories without CMakeLists.txt files
    if(NOT (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${libname}/CMakeLists.txt))
      continue()
    endif()

    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${libname}/CMakeLists.txt
         lib_cmakelists_contents
    )
    string(REGEX MATCHALL "(NS3_[a-zA-Z0-9_]*)|(ENABLE_[a-zA-Z0-9_]*)" flags
                 "${lib_cmakelists_contents}"
    )
    foreach(flag ${flags})
      # Skip reason flags
      if(${flag} MATCHES "REASON")
        continue()
      endif()
      # Skip flags already defined
      if(DEFINED ${flag})
        continue()
      endif()
      # Set flag to off
      set(${flag} OFF PARENT_SCOPE)
    endforeach()
  endif()
endforeach()

if(NOT ${XCODE})

  # Create object libraries from shared libraries to be reused by static and
  # monolib builds
  set(lib-ns3-static-objs)
  if(${NS3_STATIC} OR ${NS3_MONOLIB})
    foreach(module ${ns3-libs} ${ns3-contrib-libs})
      # Retrieve source files from shared library target
      get_target_property(target_sources ${module} SOURCES)

      # Prepend module path to source files
      set(FOLDER "src")
      if(${module} IN_LIST ns3-contrib-libs)
        set(FOLDER "contrib")
      endif()
      list(TRANSFORM target_sources
           PREPEND ${PROJECT_SOURCE_DIR}/${FOLDER}/${module}/
      )

      # Create object library from shared library
      add_library(${module}-obj OBJECT ${target_sources})

      # Reuse PCH
      if(${PRECOMPILE_HEADERS_ENABLED})
        target_precompile_headers(${module}-obj REUSE_FROM stdlib_pch)
      endif()

      # Retrieve properties from shared library target and apply to object
      foreach(property INCLUDE_DIRECTORIES LINK_DIRECTORIES
                       INTERFACE_COMPILE_DEFINITIONS
      )
        get_target_property(target_definitions ${module} ${property})
        if("${target_definitions}" MATCHES "target_definitions-NOTFOUND")
          continue()
        endif()
        if(${property} STREQUAL "INTERFACE_COMPILE_DEFINITIONS")
          set(property COMPILE_DEFINITIONS)
        endif()

        set_target_properties(
          ${module}-obj PROPERTIES ${property} "${target_definitions}"
        )
      endforeach()

      # Append to the list of object files
      list(APPEND lib-ns3-static-objs $<TARGET_OBJECTS:${module}-obj>)
    endforeach()
  endif()

  # Prevents link errors due to symbol collisions if the same library is linked
  # multiple times
  list(REMOVE_DUPLICATES ns3-external-libs)

  # Build the lib-ns3-static (ns3.x-static-buildtype.a/.lib) with all
  # sublibraries
  if(${NS3_STATIC})
    add_library(
      ${lib-ns3-static} STATIC ${PROJECT_SOURCE_DIR}/build-support/empty.cc
                               "${lib-ns3-static-objs}"
    )

    # Replace shared library suffix and check if static version exists before
    # linking
    set(ns3-external-static-libs)
    foreach(sharedlib ${ns3-external-libs})
      if(NOT (${sharedlib} MATCHES ".so"))
        list(APPEND ns3-external-static-libs ${sharedlib})
        continue()
      endif()

      string(REPLACE ".so" ".a" output ${sharedlib})
      if(EXISTS ${output})
        list(APPEND ns3-external-static-libs ${output})
      else()
        message(
          FATAL_ERROR "Static library version of ${sharedlib} was not found"
        )
      endif()
    endforeach()

    # Required by some static libraries, such as sqlite, for some odd reason
    if(LINUX)
      list(APPEND ns3-external-static-libs -ldl)
    endif()

    target_link_libraries(
      ${lib-ns3-static} ${STATIC_LINK_FLAGS} ${LIB_AS_NEEDED_PRE_STATIC}
      ${ns3-external-static-libs} ${LIB_AS_NEEDED_POST_STATIC}
    )
    if(${NS3_CLANG_TIMETRACE})
      add_dependencies(timeTraceReport ${lib-ns3-static})
    endif()
  endif() # NS3_STATIC

  # Build the lib-ns3 (ns3.x-monolib-buildtype.dll/.dylib/.so) with all
  # sublibraries
  if(${NS3_MONOLIB})
    add_library(
      ${lib-ns3-monolib} SHARED ${PROJECT_SOURCE_DIR}/build-support/empty.cc
                                "${lib-ns3-static-objs}"
    )
    set_target_properties(
      ${lib-ns3-monolib}
      PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE
                 RUNTIME_OUTPUT_DIRECTORY
                 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} # set output directory for
                                                   # DLLs
    )
    target_link_libraries(
      ${lib-ns3-monolib} ${LIB_AS_NEEDED_PRE} ${ns3-external-libs}
      ${LIB_AS_NEEDED_POST}
    )
    if(${NS3_CLANG_TIMETRACE})
      add_dependencies(timeTraceReport ${lib-ns3-monolib})
    endif()
  endif() # NS3_MONOLIB

endif() # NOT XCODE

if(${NS3_FETCH_OPTIONAL_COMPONENTS})
  add_dependency_to_optional_modules_dependencies()
endif()
