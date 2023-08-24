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
  DAZ Studio SDK include directory.

``DAZStudioSDK_LIBRARY_DIRS``
  Link directories for DAZ Studio SDK libraries.

``DAZStudioSDK_LIBRARIES``
  DAZ Studio SDK library to be linked.

``DAZStudioSDK_VERSION``
  DAZ Studio SDK version number in ``W.X.Y.Z`` format.

``DAZStudioSDK_VERSION_MAJOR``
  DAZ Studio SDK major version number (``W`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_MINOR``
  DAZ Studio SDK major version number (``X`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_PATCH``
  DAZ Studio SDK revision version number (``Y`` in ``W.X.Y.Z``).

``DAZStudioSDK_VERSION_BUILD``
  DAZ Studio SDK build version number (``Z`` in ``W.X.Y.Z``).

``DAZStudioSDK_DAZ_STUDIO_DIR``
  DAZ Studio directory.

``DAZStudioSDK_DAZ_STUDIO_PLUGIN_DIR``
  DAZ Studio plugin directory.

Cache variables
^^^^^^^^^^^^^^^

Search results are saved persistently in CMake cache entries:

``DAZStudioSDK_INCLUDE_DIR``
  Directory containing DAZ Studio SDK headers.

``DAZStudioSDK_LIBRARY``
  DAZ Studio SDK library to be linked.

``DAZStudioSDK_DPC_EXECUTABLE``
  DAZ Studio SDK PreCompiler.

``DAZStudioSDK_EXPORTED_SYMBOLS_FILE``
  DAZ Studio plugin exported symbols file (Apple only)

``DAZStudioSDK_ROOT_DIR``
  DAZ Studio SDK root directory.

``DAZStudioSDK_DAZ_STUDIO_EXECUTABLE``
  DAZ Studio executable.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``DAZStudioSDK::DzCore``
  Target for dependencies.

``DAZStudioSDK::Plugin``
  Target for plugin dependencies.

``DAZStudioSDK::PreCompiler``
  Target for precompiler.

Hints
^^^^^

This module uses the following environment variables as hints if set:

``DAZStudioSDK_ROOT``
  DAZ Studio SDK directory.

``DAZStudio_ROOT``
  DAZ Studio directory.

Examples
^^^^^^^^

Find DAZ Studio SDK libraries and use imported target:

.. code-block:: cmake

  find_package(DAZStudioSDK 4.5 REQUIRED)
  add_executable(foo foo.cc)
  target_link_libraries(foo DAZStudioSDK::DzCore)

.. code-block:: cmake

  find_package(DAZStudioSDK 4.5)
  if(DAZStudioSDK_FOUND)
    include_directories(${DAZStudioSDK_INCLUDE_DIRS})
    add_executable(foo foo.cc)
    target_link_libraries(foo ${DAZStudioSDK_LIBRARIES})
  endif()
#]=======================================================================]
include (FindPackageHandleStandardArgs)

function (get_sdk_version include_dir)
	# Get the SDK version
	file (STRINGS ${include_dir}/dzversion.h _temp REGEX "^#define DZ_SDK_VERSION_(MAJOR|MINOR|REV|BUILD)[ \t]+([0-9]+)$")
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_MAJOR[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_MAJOR ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_MINOR[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_MINOR ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_REV[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_PATCH ${CMAKE_MATCH_1})
	endif()
	if ("${_temp}" MATCHES "#define DZ_SDK_VERSION_BUILD[ \t]+([0-9]+)")
		set (DAZStudioSDK_VERSION_BUILD ${CMAKE_MATCH_1})
	endif()
	set (DAZStudioSDK_VERSION "${DAZStudioSDK_VERSION_MAJOR}.${DAZStudioSDK_VERSION_MINOR}.${DAZStudioSDK_VERSION_PATCH}.${DAZStudioSDK_VERSION_BUILD}" PARENT_SCOPE)
	return (PROPAGATE DAZStudioSDK_VERSION_MAJOR DAZStudioSDK_VERSION_MINOR DAZStudioSDK_VERSION_PATCH DAZStudioSDK_VERSION_BUILD)
endfunction()

function (is_requested_version_sdk validator_result_var item)
	if (DAZStudioSDK_FIND_VERSION_COMPLETE)
		get_sdk_version (${item})
		if (NOT DAZStudioSDK_VERSION VERSION_GREATER_EQUAL DAZStudioSDK_FIND_VERSION_COMPLETE)
			set (${validator_result_var} FALSE PARENT_SCOPE)
		endif()
	endif()
endfunction()

function (search_install_locations result_var)
	# Check Windows registry for DAZ Studio install locations
	if (WIN32)
		# Get architecture bitness
		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			set (_BITNESS 32)
		else()
			set (_BITNESS 64)
		endif()

		execute_process(
			COMMAND powershell
			 (Get-ItemProperty -Path 'HKCU:\\Software\\DAZ\\Studio${DAZStudioSDK_VERSION_MAJOR}*\\' -Name 'InstallPath-*' |
			  Select-Object -ExpandProperty 'InstallPath-${_BITNESS}' |
			  Sort-Object -Unique)
			OUTPUT_VARIABLE _SEARCH_PATHS
		)

		string (REGEX REPLACE "\n" ";" _SEARCH_PATHS "${_SEARCH_PATHS}")

		set (${result_var} ${_SEARCH_PATHS} PARENT_SCOPE)
	endif()
endfunction()

# Check Windows registry for DAZ Studio Content paths to search (DIM installs the SDK here)
if (WIN32)
	execute_process(
		COMMAND powershell
		 (Get-ItemProperty -Path 'HKCU:\\Software\\DAZ\\Studio*\\' -Name 'ContentDir*' |
		  Select-Object 'ContentDir*' |
		  ForEach-Object { $_.PSObject.Properties.Value } |
		  Sort-Object -Unique)
		OUTPUT_VARIABLE _SDK_SEARCH_PATHS
	)

	string (REGEX REPLACE "\n" ";" _SEARCH_PATHS "${_SEARCH_PATHS}")
endif()

# Search for the include directory
find_path (DAZStudioSDK_INCLUDE_DIR
	NAMES
	 dzversion.h
	HINTS
	 ${DAZStudioSDK_ROOT_DIR}
	 ${_SDK_SEARCH_PATHS}
	PATH_SUFFIXES
	 include/
	 DAZStudio4.5+\ SDK/include/
	VALIDATOR
	 is_requested_version_sdk
	DOC
	 "DAZ Studio SDK include directory"
)

if (DAZStudioSDK_INCLUDE_DIR)
	set (DAZStudioSDK_INCLUDE_DIRS ${DAZStudioSDK_INCLUDE_DIR})

	if (NOT DAZStudioSDK_VERSION)
		get_sdk_version (${DAZStudioSDK_INCLUDE_DIR})
	endif()

	# Get the SDK directory root
	cmake_path (GET DAZStudioSDK_INCLUDE_DIR PARENT_PATH DAZStudioSDK_ROOT_DIR)
	set (DAZStudioSDK_ROOT_DIR ${DAZStudioSDK_ROOT_DIR} CACHE PATH "DAZ Studio SDK root directory")

	# Setup library path prefixes and library suffixes
	if (WIN32)
		set (_QT_LIB_SUFFIX 4)

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
	find_library (DAZStudioSDK_LIBRARY
		NAMES
		 dzcore
		HINTS
		 ${DAZStudioSDK_ROOT_DIR}
		PATH_SUFFIXES
		 lib/${_LIB_PATH_SUFFIX}
		DOC
		 "DAZ Studio SDK DzCore library"
		NO_DEFAULT_PATH
	)
	set (DAZStudioSDK_LIBRARIES ${DAZStudioSDK_LIBRARY})

	# Get the library directory
	cmake_path (GET DAZStudioSDK_LIBRARY PARENT_PATH DAZStudioSDK_LIBRARY_DIRS)

	# Find DAZ PreCompiler
	find_program (DAZStudioSDK_DPC_EXECUTABLE
		NAMES
		 dpc
		HINTS
		 ${DAZStudioSDK_ROOT_DIR}
		PATH_SUFFIXES
		 bin/${_LIB_PATH_SUFFIX}
		DOC
		 "DAZ Studio SDK PreCompiler"
		NO_DEFAULT_PATH
	)

	# Get the binary directory
	cmake_path (GET DAZStudioSDK_DPC_EXECUTABLE PARENT_PATH DAZStudioSDK_BINARY_DIR)

	# Set the autogen directory
	set (_autogen_dir ${CMAKE_CURRENT_BINARY_DIR}/DAZStudioSDK_autogen)

	# Setup exported symbols files
	if (WIN32)
		set (DAZStudioSDK_DEF_FILE "${_autogen_dir}/module-definition-file.def")
		if (NOT EXISTS ${DAZStudioSDK_DEF_FILE})
			file (WRITE ${DAZStudioSDK_DEF_FILE}
"EXPORTS
	getSDKVersion	@1
	getPluginDefinition	@2
SECTIONS
	.data READ WRITE"
			)
		endif()
	elseif (APPLE)
		find_file (DAZStudioSDK_EXPORTED_SYMBOLS_FILE
			NAMES
			 exportedPluginSymbols.txt
			PATHS
			 ${DAZStudioSDK_INCLUDE_DIR}
			DOC
			 "DAZ Studio SDK Exported Symbols File"
			NO_DEFAULT_PATH
		)
	endif()

	# Pass thru REQUIRED
	if (DAZStudioSDK_FIND_REQUIRED)
		set (_components_required "REQUIRED")
	else()
		set (_components_required "OPTIONAL_COMPONENTS")
	endif()

	# Pass thru QUIET
	if (DAZStudioSDK_FIND_QUIETLY)
		set (_quiet "QUIET")
	else()
		set (_quiet "")
	endif()

	# Find Qt4 for DAZ Studio SDK 4
	if (DAZStudioSDK_VERSION_MAJOR VERSION_EQUAL 4)
		if (NOT Qt4_FOUND)	# Use SDK included minimal Qt4
			# Find QMake
			find_program (QT_QMAKE_EXECUTABLE
				NAMES
				 qmake
				HINTS
				 ${DAZStudioSDK_BINARY_DIR}
				NO_DEFAULT_PATH
			)

			# Find QtCore library
			find_library (QT_QTCORE_LIBRARY_RELEASE
				NAMES
				 QtCore${_QT_LIB_SUFFIX}
				HINTS
				 ${DAZStudioSDK_LIBRARY_DIRS}
				NO_DEFAULT_PATH
			)

			# Find QtCore include directory
			find_path (QT_QTCORE_INCLUDE_DIR
				NAMES
				 qconfig.h
				HINTS
				 ${DAZStudioSDK_INCLUDE_DIR}
				NO_DEFAULT_PATH
				PATH_SUFFIXES
				 QtCore
			)

			# Pre-populate Qt4 binary directory location
			set (QT_BINARY_DIR ${DAZStudioSDK_BINARY_DIR} CACHE PATH "")
			set (QT_HEADERS_DIR ${DAZStudioSDK_INCLUDE_DIR} CACHE PATH "")

			# Provide fake Qt MKSPECS directory
			set (QT_MKSPECS_DIR "${_autogen_dir}/fake_mkspecs" CACHE PATH "")
			file (MAKE_DIRECTORY "${QT_MKSPECS_DIR}/default")

			message (WARNING "DAZ Studio SDK does not include all Qt4 components. Call find_package (Qt4 ...) before find_package (DAZStudioSDK) if additional components are required.")
		endif()
		find_package (Qt4 4.8.1 ${_quiet} ${_components_required} QtCore QtGui QtNetwork QtOpenGL QtScript QtSql QtXml)
	endif()

	# Find DAZ Studio
	search_install_locations (_EXEC_SEARCH_PATHS)
	find_program (DAZStudioSDK_DAZ_STUDIO_EXECUTABLE
		NAMES
		 DAZStudio
		HINTS
		 ENV DAZStudio_ROOT
		 ${_EXEC_SEARCH_PATHS}
		DOC
		 "DAZ Studio Executable"
	)

	cmake_path (GET DAZStudioSDK_DAZ_STUDIO_EXECUTABLE PARENT_PATH DAZStudioSDK_DAZ_STUDIO_DIR)
	cmake_path (APPEND DAZStudioSDK_DAZ_STUDIO_DIR plugins OUTPUT_VARIABLE DAZStudioSDK_DAZ_STUDIO_PLUGIN_DIR)

	# Export libraries, and import target
	if (EXISTS ${DAZStudioSDK_LIBRARY})
		add_library (DAZStudioSDK::DzCore SHARED IMPORTED)
		target_link_libraries (DAZStudioSDK::DzCore INTERFACE ${DAZStudioSDK_LIBRARY})
		target_include_directories (DAZStudioSDK::DzCore INTERFACE ${DAZStudioSDK_INCLUDE_DIR})

		add_library (DAZStudioSDK::Plugin INTERFACE IMPORTED)
		target_link_libraries (DAZStudioSDK::Plugin INTERFACE DAZStudioSDK::DzCore)

		if (WIN32)
			target_compile_definitions (DAZStudioSDK::DzCore INTERFACE _WIN32_WINNT=$<IF:$<EQUAL:4,${CMAKE_SIZEOF_VOID_P}>,0x0501,0x0502>)
			set_target_properties (DAZStudioSDK::DzCore PROPERTIES IMPORTED_IMPLIB ${DAZStudioSDK_LIBRARY})
			target_sources (DAZStudioSDK::Plugin INTERFACE ${DAZStudioSDK_DEF_FILE})
		elseif (APPLE)
			set_target_properties (DAZStudioSDK::DzCore PROPERTIES IMPORTED_LOCATION ${DAZStudioSDK_LIBRARY})
			set_target_properties (DAZStudioSDK::Plugin PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE ${DAZStudioSDK_EXPORTED_SYMBOLS_FILE})
		endif()

		add_executable (DAZStudioSDK::PreCompiler IMPORTED)
		set_target_properties (DAZStudioSDK::PreCompiler PROPERTIES IMPORTED_LOCATION ${DAZStudioSDK_DPC_EXECUTABLE})
	endif()

	# Check the toolkit
	set (DAZStudioSDK_TOOLKIT_INCOMPATIBLE NO)
	if (DAZStudioSDK_VERSION_MAJOR VERSION_EQUAL 4 AND WIN32)
		if (NOT CMAKE_GENERATOR_TOOLSET STREQUAL "v100")
			if (NOT CMAKE_GENERATOR_TOOLSET STREQUAL "Windows7.1SDK")
				message (WARNING "DAZ Studio 4 SDK requires MSVC toolkit v100 or Windows7.1SDK")
				set (DAZStudioSDK_TOOLKIT_INCOMPATIBLE YES)
			endif()
		endif()
	endif()

	# Cleanup CMake UI
	mark_as_advanced (DAZStudioSDK_LIBRARY DAZStudioSDK_INCLUDE_DIR DAZStudioSDK_DPC_EXECUTABLE DAZStudioSDK_DAZ_STUDIO_EXECUTABLE DAZStudioSDK_EXPORTED_SYMBOLS_FILE)
endif()

# Report problems
find_package_handle_standard_args (DAZStudioSDK REQUIRED_VARS DAZStudioSDK_LIBRARY DAZStudioSDK_INCLUDE_DIR DAZStudioSDK_DPC_EXECUTABLE DAZStudioSDK_ROOT_DIR VERSION_VAR DAZStudioSDK_VERSION)
