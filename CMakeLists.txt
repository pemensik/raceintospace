cmake_minimum_required (VERSION 3.0)
project(raceintospace
	VERSION 1.1.0
	LANGUAGES C CXX)

enable_testing()

set (raceintospace_VERSION_RELEASE ${raceintospace_VERSION_PATCH})

# Platform detection
if (WIN32 AND NOT WINDOWS)
  set (WINDOWS TRUE)
endif (WIN32 AND NOT WINDOWS)

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
endif (WINDOWS)

add_subdirectory(lib)
add_subdirectory(src)
install(PROGRAMS icons/raceintospace.desktop DESTINATION /usr/share/applications)
install(FILES icons/raceintospace.xpm DESTINATION /usr/share/pixmaps)

set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
include(CPack)

list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\.git/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/CMakeFiles/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/build/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\.tar\\.gz$/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/\\.tar\\.bz2$/")

execute_process(COMMAND git rev-list -1 --abbrev-commit HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE)
set(ARCHIVE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}-git${COMMIT}")
add_custom_target(dist
    COMMAND git archive --prefix="${ARCHIVE_NAME}/" HEAD | bzip2 > "${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
