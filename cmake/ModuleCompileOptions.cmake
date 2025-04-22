
# Copyright (c) 2018 brinkqiang (brink.qiang@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

macro(ShowEnvironment)
  message(STATUS "================================================================================")
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
      message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()

  execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "environment")
  message(STATUS "================================================================================")
endmacro(ShowEnvironment)

function(print_package_vars PREFIX)
    get_cmake_property(_vars VARIABLES)
    foreach(_var IN LISTS _vars)
        if(_var MATCHES "^${PREFIX}")
            message(STATUS "package ${PREFIX} -> ${_var} = ${${_var}}")
        endif()
    endforeach()
endfunction()

macro(ModuleSetCompileOptions)
  cmake_policy(SET CMP0022 NEW)
  include(CheckCXXCompilerFlag)
  if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
  endif()
  
  set(CMAKE_C_STANDARD 99)
  
  if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "debug")
  endif()

  if (WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/windows)
  endif()

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/test)

  if (WIN32 AND NOT MINGW)
      link_directories(${CMAKE_SOURCE_DIR}/bin)
      set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
      set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
  else()
      link_directories(${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
      set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
      set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
  endif()

  if (WIN32)
    message(STATUS "Now is windows")

    set(DMOS_NAME "win")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
      
    link_libraries(Ws2_32)
    if(MSVC)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:NOICF /STACK:16777216")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:NOICF /STACK:16777216")
      set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")

      add_definitions(/bigobj)
      add_definitions(/DNOMINMAX /DWIN32_LEAN_AND_MEAN=1 /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS)
      #add_definitions(/utf-8)
      add_compile_options(/W3 /wd4005 /wd4068 /wd4244 /wd4267 /wd4800 /wd4996)

      check_cxx_compiler_flag("/std:c++17" COMPILER_SUPPORTS_CXX17)
      check_cxx_compiler_flag("/std:c++14" COMPILER_SUPPORTS_CXX14)
      check_cxx_compiler_flag("/std:c++11" COMPILER_SUPPORTS_CXX11)
      
      if(COMPILER_SUPPORTS_CXX17)
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
          message(STATUS "The compiler has /std:c++17 support.")
      elseif(COMPILER_SUPPORTS_CXX14)
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")
          message(STATUS "The compiler has /std:c++14 support.")
      elseif(COMPILER_SUPPORTS_CXX11)
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
          message(STATUS "The compiler has /std:c++11 support.")
      else()
          message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 or newer support. Please use a different C++ compiler.")
      endif()
    endif()
  elseif (APPLE)
    message(STATUS "Now is Apple OS")

    set(DMOS_NAME "mac") 

    check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
    check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

    if(COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        message(STATUS "The compiler has -std=c++17 support.")
    elseif(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
        message(STATUS "The compiler has -std=c++14 support.")
    elseif(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        message(STATUS "The compiler has -std=c++11 support.")
    else(COMPILER_SUPPORTS_CXX17)
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif(COMPILER_SUPPORTS_CXX17)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -g")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")

    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)

    set(CMAKE_THREAD_LINK "")
    if(THREADS_FOUND)
      set(CMAKE_THREAD_LINK "Threads::Threads")
    endif()

    link_libraries(${CMAKE_THREAD_LINK})
  elseif (UNIX)
    message(STATUS "Now is UNIX-like OS")

    set(DMOS_NAME "lin")

    check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
    check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    
    if(COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
        message(STATUS "The compiler has -std=c++17 support.")
    elseif(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
        message(STATUS "The compiler has -std=c++14 support.")
    elseif(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        message(STATUS "The compiler has -std=c++11 support.")
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 or newer support. Please use a different C++ compiler.")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -Wl,--rpath=./ -Wl,-rpath-link=./lib")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -g")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,--rpath=./ -Wl,-rpath-link=./lib")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -g")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")

    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    find_package(Threads REQUIRED)

    set(CMAKE_THREAD_LINK "")
    if(THREADS_FOUND)
      set(CMAKE_THREAD_LINK "Threads::Threads")
    endif()

    link_libraries(m ${CMAKE_THREAD_LINK})
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
      set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
  endif ()
endmacro(ModuleSetCompileOptions)

macro(ModuleSetWinCompilerFlags)
  if (WIN32)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_MINSIZEREL
            )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif (WIN32)
endmacro()

macro(AddInstall ModuleList)
    message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
    message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
    message(STATUS "Install Path: ${CMAKE_INSTALL_PREFIX}/bin")

    message(STATUS "AddInstall ${ModuleList} ...")
    if (WIN32)
        install(TARGETS ${ModuleList}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
    else(WIN32)
        include(GNUInstallDirs)
        install(TARGETS ${ModuleList}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif(WIN32)

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endmacro()
