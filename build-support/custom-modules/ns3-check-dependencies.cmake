# Copyright (c) 2023 Universidade de Brasília
#
# SPDX-License-Identifier: GPL-2.0-only
#
# Author: Gabriel Ferreira <gabrielcarvfer@gmail.com>

# function used to search for package and program dependencies than store list
# of missing dependencies in the list whose name is stored in missing_deps
function(check_deps missing_deps)
  set(multiValueArgs CMAKE_PACKAGES EXECUTABLES PYTHON_PACKAGES)
  cmake_parse_arguments("DEPS" "" "" "${multiValueArgs}" ${ARGN})

  set(local_missing_deps)
  # Search for package dependencies
  foreach(package ${DEPS_CMAKE_PACKAGES})
    find_package(${package})
    if(NOT ${${package}_FOUND})
      list(APPEND local_missing_deps ${package})
    endif()
  endforeach()

  # And for program dependencies
  foreach(program ${DEPS_EXECUTABLES})
    # CMake likes to cache find_* to speed things up, so we can't reuse names
    # here or it won't check other dependencies
    string(TOUPPER ${program} upper_${program})
    mark_as_advanced(${upper_${program}})
    find_program(
      ${upper_${program}} ${program} HINTS ${3RD_PARTY_FIND_PROGRAM_HINTS}
    )
    if("${${upper_${program}}}" STREQUAL "${upper_${program}}-NOTFOUND")
      list(APPEND local_missing_deps ${program})
    endif()
  endforeach()

  foreach(package ${DEPS_PYTHON_PACKAGES})
    execute_process(
      COMMAND ${Python3_EXECUTABLE} -c "import ${package}"
      RESULT_VARIABLE return_code OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT (${return_code} EQUAL 0))
      list(APPEND local_missing_deps ${package})
    endif()
  endforeach()

  # Store list of missing dependencies in the parent scope
  set(${missing_deps} ${local_missing_deps} PARENT_SCOPE)
endfunction()
