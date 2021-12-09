# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindDAZStudioSDK
----------------

Find DAZ Studio SDK include dirs and libraries

Use this module by invoking :command:`find_package` with the form:

.. code-block:: cmake

  find_package(DAZStudioSDK
    [version] [EXACT]      # Minimum or EXACT version e.g. 4.5
    [REQUIRED]             # Fail with error if DAZ Studio SDK is not found
    )

This module finds headers and libraries.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``DAZStudioSDK_FOUND``
  True if headers and requested libraries were found.

``DAZStudioSDK_TOOLKIT_INCOMPATIBLE``
  True if the current compiler toolkit will produce a shared library incompatible with DAZ Studio

``DAZStudioSDK_INCLUDE_DIRS``
  DAZ Studio SDK include directories.

``DAZStudioSDK_LIBRARY_DIRS``
  Link directories for DAZ Studio SDK libraries.

``DAZStudioSDK_ROOT``
  DAZ Studio SDK root directory.

``DAZStudioSDK_LIBRARIES``
  DAZ Studio SDK libraries to be linked.

``DAZStudioSDK_VERSION``
  DAZ Studio SDK version number in ``W.X.Y.Z`` format.

``DAZStudioSDK_VERSION_MAJOR``
  DAZ Studio SDK major version number (``W`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_MINOR``
  DAZ Studio SDK major version number (``X`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_REV``
  DAZ Studio SDK revision version number (``Y`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_BUILD``
  DAZ Studio SDK build version number (``Z`` in ``W.X.Y.Z``).

Cache variables
^^^^^^^^^^^^^^^

Search results are saved persistently in CMake cache entries:

``DAZStudioSDK_INCLUDE_DIR``
  Directory containing DAZ Studio SDK headers.

``DAZStudioSDK_LIBRARY_DIR``
  Directory containing DAZ Studio SDK libraries.

Hints
^^^^^

This module reads hints about search locations from a variable:

``DAZStudioSDK_ROOT_DIR``
  Preferred installation prefix.

Users may set this hint or results as ``CACHE`` entries.  Projects
should not read these entries directly but instead use the above
result variables.  One may specify this hint as an environment
variable if it is not specified as a CMake variable or cache entry.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``DAZStudioSDK::DAZStudioSDK``
  Target for dependencies.

Examples
^^^^^^^^

Find DAZ Studio SDK libraries and use imported target:

.. code-block:: cmake

  find_package(DAZStudioSDK 4.5 REQUIRED)
  add_executable(foo foo.cc)
  target_link_libraries(foo DAZStudioSDK::DAZStudioSDK)

.. code-block:: cmake

  find_package(DAZStudioSDK 4.5)
  if(DAZStudioSDK_FOUND)
    include_directories(${DAZStudioSDK_INCLUDE_DIRS})
    add_executable(foo foo.cc)
    target_link_libraries(foo ${DAZStudioSDK_LIBRARIES})
  endif()
#]=======================================================================]

include (FindPackageHandleStandardArgs)

# Get user specified directory to search
set (DAZStudioSDK_ROOT_DIR "${DAZStudioSDK_ROOT_DIR}" CACHE PATH "Directory to search")

# Check Windows registry for DAZ Studio Content paths to search (DIM installs the SDK here)
if (WIN32)
	execute_process(
		COMMAND powershell
		 (Get-ItemProperty -Path 'HKCU:\\Software\\DAZ\\Studio*\\' -Name 'ContentDir*' |
		  Select ContentDir* |
		  ForEach-Object { $_.PSObject.Properties | ForEach-Object { $_.Value} } |
		  Sort-Object -Unique
		 ) -replace ' ', '\\ '
		OUTPUT_VARIABLE _SEARCH_PATHS
	)
	string (REGEX REPLACE "\n" ";" _SEARCH_PATHS "${_SEARCH_PATHS}")
endif()

# Search for the include directory
find_path (DAZStudioSDK_INCLUDE_DIR
	NAMES
	 dzversion.h
	HINTS
	 ${DAZStudioSDK_ROOT_DIR}
	 $ENV{DAZStudioSDK_ROOT_DIR}
	 ${_SEARCH_PATHS}
	PATH_SUFFIXES
	 include/
	 DAZStudio4.5+\ SDK/include/
)

# Export the include directories
set (DAZStudioSDK_INCLUDE_DIRS ${DAZStudioSDK_INCLUDE_DIR})

# Find the included Qt include directories
foreach (inc ActiveQt phonon Qt3Support QtCore QtDBus QtDeclarative QtDesigner QtGui QtHelp QtMultimedia QtNetwork QtOpenGl QtOpenVG QtScript QtScriptTools QtSql QtSvg QtTest QtUiTools QtWebKit QtXml QtXmlPatterns)
	find_path (DAZStudioSDK_INC_${inc}
	 NAMES
	  ${inc}
	 HINTS
	  ${DAZStudioSDK_INCLUDE_DIR}/${inc}
	 NO_DEFAULT_PATH
	)
	if (DAZStudioSDK_INC_${inc})
		list (APPEND DAZStudioSDK_INCLUDE_DIRS ${DAZStudioSDK_INC_${inc}})
	endif()
endforeach()

# Get the SDK directory root
cmake_path (GET DAZStudioSDK_INCLUDE_DIR PARENT_PATH DAZStudioSDK_ROOT)
set (DAZStudioSDK_ROOT "${DAZStudioSDK_ROOT}" CACHE PATH "SDK root directory")

# Get the SDK version
if (DAZStudioSDK_INCLUDE_DIR)
	file (STRINGS ${DAZStudioSDK_INCLUDE_DIR}/dzversion.h _temp REGEX "^#define DZ_SDK_VERSION_(MAJOR|MINOR|REV|BUILD)[ \t]+([0-9]+)$")
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_MAJOR[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_MAJOR ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_MINOR[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_MINOR ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_REV[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_REV ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_BUILD[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_BUILD ${CMAKE_MATCH_1})
	endif()
	set (DAZStudioSDK_VERSION "${DAZStudioSDK_VERSION_MAJOR}.${DAZStudioSDK_VERSION_MINOR}.${DAZStudioSDK_VERSION_REV}.${DAZStudioSDK_VERSION_BUILD}")
endif()

# Setup library path prefixes and library suffixes
if (WIN32)
	set (_QT_LIB_SUFFIX ${DAZStudioSDK_VERSION_MAJOR})

	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (_LIB_PATH_SUFFIX Win32)
	else()
		set (_LIB_PATH_SUFFIX x64)
	endif()
elseif (APPLE)
	set (_QT_LIB_SUFFIX "")

	if (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (_LIB_PATH_SUFFIX Mac32)
	else()
		set (_LIB_PATH_SUFFIX Mac64)
	endif()
endif()

# Find dzcore library
find_library (DAZStudioSDK_LIB_DzCore
 NAMES
  dzcore
 HINTS
  ${DAZStudioSDK_ROOT}
  ${DAZStudioSDK_ROOT_DIR}
  $ENV{DAZStudioSDK_ROOT_DIR}
 PATH_SUFFIXES
  lib/${_LIB_PATH_SUFFIX}
)

# Get the library directory
cmake_path (GET DAZStudioSDK_LIB_DzCore PARENT_PATH DAZStudioSDK_LIBRARY_DIRS)

# Find the Qt libraries dzcore uses
foreach (lib Qt3Support QtCore QtGui QtNetwork QtOpenGl QtScript QtSql QtXml aa)
	find_library (DAZStudioSDK_LIB_${lib}
	 NAMES
	  ${lib}${_QT_LIB_SUFFIX}
	 HINTS
	  ${DAZStudioSDK_LIBRARY_DIRS}
	 NO_DEFAULT_PATH
	)
	if (DAZStudioSDK_LIB_${lib})
		list (APPEND _QT_LIBRARIES ${DAZStudioSDK_LIB_${lib}})
	endif()
endforeach()

# Export the library directory, libraries, and import target
if (EXISTS ${DAZStudioSDK_LIB_DzCore})
	set (DAZStudioSDK_LIBRARIES ${DAZStudioSDK_LIB_DzCore})
	foreach (lib ${_QT_LIBRARIES})
		list (APPEND DAZStudioSDK_LIBRARIES ${lib})
	endforeach()

	add_library (DAZStudioSDK::DAZStudioSDK SHARED IMPORTED)
	set_target_properties (DAZStudioSDK::DAZStudioSDK
		PROPERTIES
		INTERFACE_LINK_LIBRARIES
		 "${_QT_LIBRARIES}"
		INTERFACE_INCLUDE_DIRECTORIES
		 "${DAZStudioSDK_INCLUDE_DIRS}"
	)
	
	if (WIN32)
		set_property (TARGET DAZStudioSDK::DAZStudioSDK PROPERTY IMPORTED_IMPLIB ${DAZStudioSDK_LIB_DzCore})

		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			add_compile_definitions (_WIN32_WINNT=0x0501)
		else()
			add_compile_definitions (_WIN32_WINNT=0x0502)
		endif()
	else()
		set_property (TARGET DAZStudioSDK::DAZStudioSDK PROPERTY IMPORTED_LOCATION ${DAZStudioSDK_LIB_DzCore})
	endif()
endif()

# Check the toolkit
set (DAZStudioSDK_TOOLKIT_INCOMPATIBLE NO)
if (WIN32)
	if (DAZStudioSDK_VERSION_MAJOR EQUAL 4)
		if (NOT CMAKE_GENERATOR_TOOLSET STREQUAL "v100")
			if (NOT CMAKE_GENERATOR_TOOLSET STREQUAL "Windows7.1SDK")
				message (WARNING "DAZ Studio ${DAZStudioSDK_VERSION_MAJOR} SDK requires MSVC toolkit v100 or Windows7.1SDK")
				set (DAZStudioSDK_TOOLKIT_INCOMPATIBLE YES)
			endif()
		endif()
	endif()
endif()

# Cleanup CMake UI
mark_as_advanced (DAZStudioSDK_INCLUDE_DIR DAZStudioSDK_LIB_DzCore DAZStudioSDK_LIB_Qt3Support DAZStudioSDK_LIB_QtCore DAZStudioSDK_LIB_QtGui DAZStudioSDK_LIB_QtNetwork DAZStudioSDK_LIB_QtOpenGl DAZStudioSDK_LIB_QtScript DAZStudioSDK_LIB_QtSql DAZStudioSDK_LIB_QtXml)

# Report problems
find_package_handle_standard_args (DAZStudioSDK REQUIRED_VARS DAZStudioSDK_LIBRARY_DIRS DAZStudioSDK_INCLUDE_DIRS DAZStudioSDK_ROOT DAZStudioSDK_LIBRARIES VERSION_VAR DAZStudioSDK_VERSION)