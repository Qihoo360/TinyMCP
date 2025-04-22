
macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

macro(ModuleInclude ModuleName ModulePath)
    message(STATUS "ModuleInclude ${ModuleName} ${ModulePath}")

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/CMakeLists.txt)
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            foreach(subdir ${SUBDIRS})
                ModuleInclude(${subdir} ${ModulePath}/thirdparty/${subdir})
            endforeach()
        endif()

    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/cmake/CMakeLists.txt)
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            foreach(subdir ${SUBDIRS})
                ModuleInclude(${subdir} ${ModulePath}/thirdparty/${subdir})
            endforeach()
        endif()
    else()

    endif()

    if (WIN32)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/src/windows)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/src/${ModuleName}/windows)
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/lib)
    endif(WIN32)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/src)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/src/${ModuleName})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/include/${ModuleName})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/test)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/test/${ModuleName})

endmacro()

macro(InterfaceImport ModuleName ModulePath DependsLib)
    message(STATUS "ModuleImport ${ModuleName} ${ModulePath}")

    set(${ModuleName}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
    mark_as_advanced(${ModuleName}_INCLUDE_DIR)
    set(${ModuleName}_LIBRARIES ${ModuleName})
    mark_as_advanced(${ModuleName}_LIBRARIES)
    
    add_library(${ModuleName} INTERFACE)

    target_include_directories(${ModuleName} INTERFACE ${${ModuleName}_INCLUDE_DIR})

    target_link_libraries(${ModuleName} ${DependsLib})
endmacro()

macro(ModuleImport ModuleName ModulePath)
    message(STATUS "ModuleImport ${ModuleName} ${ModulePath}")

    get_property(DMLIBS GLOBAL PROPERTY DMLIBS)

    list(FIND DMLIBS ${ModuleName} DMLIBS_FOUND)
    IF (DMLIBS_FOUND STREQUAL "-1")
        list(APPEND DMLIBS ${ModuleName})
        set_property(GLOBAL PROPERTY DMLIBS ${DMLIBS})

        message(STATUS "LIST APPEND ${ModuleName} ${DMLIBS}" )

        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/CMakeLists.txt)
            add_subdirectory(${ModulePath})
        elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/cmake/CMakeLists.txt)
            add_subdirectory(${ModulePath}/cmake)
        else()
            message(FATAL_ERROR "ModuleImport ${ModuleName} CMakeLists.txt not exist.")
        endif()

        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            foreach(subdir ${SUBDIRS})
                ModuleInclude(${subdir} ${ModulePath}/thirdparty/${subdir})
            endforeach()
        endif()

        ModuleInclude(${ModuleName} ${ModulePath})
    else()
        if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
            foreach(subdir ${SUBDIRS})
                ModuleInclude(${subdir} ${ModulePath}/thirdparty/${subdir})
            endforeach()
        endif()
        ModuleInclude(${ModuleName} ${ModulePath})
        message(STATUS "LIST REPEAT ${ModuleName} ${DMLIBS}" )
    endif()
endmacro()

macro(ExeImport ModulePath DependsLib)
    message(STATUS "ExeImport ${ModulePath} ${DependsLib}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        foreach(subdir ${SUBDIRS})
            message(STATUS "INCLUDE -> ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}")
            include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir})
            file(GLOB_RECURSE BIN_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.h)

            list(FILTER BIN_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/tpl/*")

            foreach(child ${BIN_SOURCES})
                message(STATUS "SOURCES -> ${child}")
            endforeach()
            
            add_executable(${subdir} ${BIN_SOURCES})
            target_link_libraries(${subdir} ${DependsLib})
        endforeach()
    endif()

endmacro()

macro(ExeImportAndTest ModulePath DependsLib)
    message(STATUS "ExeImport ${ModulePath} ${DependsLib}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        foreach(subdir ${SUBDIRS})
            message(STATUS "INCLUDE -> ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}")
            include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir})
            file(GLOB_RECURSE BIN_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/*.h)

            list(FILTER BIN_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${subdir}/tpl/*")

            foreach(child ${BIN_SOURCES})
                message(STATUS "SOURCES -> ${child}")
            endforeach()
            
            add_executable(${subdir} ${BIN_SOURCES})
            target_link_libraries(${subdir} ${DependsLib})

            add_test(NAME ${subdir} COMMAND $<TARGET_FILE:${subdir}> WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
        endforeach()
    endif()

endmacro()

macro(LibImport ModuleName ModulePath)
    message(STATUS "LibImport ${ModuleName} ${ModulePath}")
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        ModuleInclude(${ModuleName} ${ModulePath})
        file(GLOB_RECURSE LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.h
        )

        list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/tpl/*")

        if (WIN32)
            list(APPEND LIB_SOURCES)
        endif(WIN32)

        add_library(${ModuleName} STATIC ${LIB_SOURCES})
    endif()
endmacro()

macro(LibImportExclude ModuleName ModulePath ExcludeList)
    message(STATUS "LibImport ${ModuleName} ${ModulePath}")
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        ModuleInclude(${ModuleName} ${ModulePath})
        file(GLOB_RECURSE LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.h
        )

        list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/tpl/*")


        foreach(child ${ExcludeList})
            list(FILTER LIB_SOURCES EXCLUDE REGEX "${child}")
            message(STATUS "ExcludeList has ${child}" )
        endforeach()

        if (WIN32)
            list(APPEND LIB_SOURCES)
        endif(WIN32)

        add_library(${ModuleName} STATIC ${LIB_SOURCES})
    endif()
endmacro()

macro(DllImport ModuleName ModulePath)
    message(STATUS "DllImport ${ModuleName} ${ModulePath}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        ModuleInclude(${ModuleName} ${ModulePath})
        file(GLOB_RECURSE LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.h
        )

        list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/tpl/*")

        if (WIN32)
            list(APPEND LIB_SOURCES)
        endif(WIN32)

        if (WIN32)
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${ModuleName}.def)
                add_library(${ModuleName} SHARED ${LIB_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${ModuleName}.def)
            else()
                add_library(${ModuleName} SHARED ${LIB_SOURCES})
            endif()
        else(WIN32)
            add_library(${ModuleName} SHARED ${LIB_SOURCES})
        endif(WIN32)
    endif()
endmacro()

macro(LibImportDepends ModuleName ModulePath DependsLib)
    message(STATUS "LibImportDepends ${ModuleName} ${ModulePath} ${DependsLib}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        ModuleInclude(${ModuleName} ${ModulePath})
        file(GLOB_RECURSE LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.h
        )

        list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/tpl/*")

        if (WIN32)
            list(APPEND LIB_SOURCES)
        endif(WIN32)

        add_library(${ModuleName} STATIC ${LIB_SOURCES})
        target_link_libraries(${ModuleName} ${DependsLib})
    endif()
endmacro()

macro(DllImportDepends ModuleName ModulePath DependsLib)
    message(STATUS "DllImportDepends ${ModuleName} ${ModulePath} ${DependsLib}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        ModuleInclude(${ModuleName} ${ModulePath})
        file(GLOB_RECURSE LIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h

        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/*.h
        )

        list(FILTER LIB_SOURCES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/tpl/*")

        if (WIN32)
            list(APPEND LIB_SOURCES)
        endif(WIN32)

        if (WIN32)
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${ModuleName}.def)
                add_library(${ModuleName} SHARED ${LIB_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/${ModuleName}.def)
            else()
                add_library(${ModuleName} SHARED ${LIB_SOURCES})
            endif()
        else(WIN32)
            add_library(${ModuleName} SHARED ${LIB_SOURCES})
        endif(WIN32)
        target_link_libraries(${ModuleName} ${DependsLib})
    endif()
endmacro()

macro(ModuleInclude2 ModuleName ModulePath)
    message(STATUS "ModuleInclude2 ${ModuleName} ${ModulePath}")

    if (WIN32)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/include/${ModuleName})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/include)

        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/lib)
    else()
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/cmake/Find${ModuleName}.cmake)
            include(${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/cmake/Find${ModuleName}.cmake)
            include_directories(${${ModuleName}_INCLUDE_DIRS})
        else()
            message(FATAL_ERROR "ModuleImport2 ${ModuleName} Find${ModuleName}.cmake not exist.")
        endif()
    endif()

endmacro(ModuleInclude2)

macro(ModuleImport2 ModuleName ModulePath)
    message(STATUS "ModuleImport2 ${ModuleName} ${ModulePath}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
        SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath}/thirdparty)
        foreach(subdir ${SUBDIRS})
            ModuleInclude2(${ModuleName} ${ModulePath}/thirdparty/${subdir})
        endforeach()
    endif()

    ModuleInclude2(${ModuleName} ${ModulePath})
endmacro()

macro(ModuleImportAll ModulePath)
    message(STATUS "ModuleImportAll ${ModulePath}")

    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/${ModulePath})
        foreach(subdir ${SUBDIRS})
            MESSAGE(STATUS "ModuleImportAll ${subdir} ${ModulePath}/${subdir}")

            ModuleImport(${subdir} ${ModulePath}/${subdir})
        endforeach()
    endif()
endmacro()

macro(ModuleConfigure ModuleName ModulePath)
    if (WIN32)
        add_custom_target(
            ${ModuleName}_configure
            COMMAND echo "${ModuleName}_config"
            WORKING_DIRECTORY ${ModulePath}
            )
    elseif (APPLE)
        add_custom_target(
            ${ModuleName}_configure
            COMMAND glibtoolize && aclocal && autoheader && autoconf && automake --add-missing && sh configure
            WORKING_DIRECTORY ${ModulePath}
            )
    elseif (UNIX)
        add_custom_target(
            ${ModuleName}_configure
            COMMAND libtoolize && aclocal && autoheader && autoconf && automake --add-missing && sh configure
            WORKING_DIRECTORY ${ModulePath}
            )
    endif()

    add_dependencies(${ModuleName} ${ModuleName}_configure)
endmacro()

macro(ModuleCommand ModuleName ModulePath CommandLine)
    message(STATUS "ModuleCommand ${ModuleName} ${ModulePath} ${CommandLine}")

    if (WIN32)
        add_custom_target(
            ${ModuleName}_command
            COMMAND ${CommandLine}
            WORKING_DIRECTORY ${ModulePath}
            )
    elseif (APPLE)
        add_custom_target(
            ${ModuleName}_command
            COMMAND ${CommandLine}
            WORKING_DIRECTORY ${ModulePath}
            )
    elseif (UNIX)
        add_custom_target(
            ${ModuleName}_command
            COMMAND ${CommandLine}
            WORKING_DIRECTORY ${ModulePath}
            )
    endif()

    add_dependencies(${ModuleName} ${ModuleName}_command)
endmacro()
