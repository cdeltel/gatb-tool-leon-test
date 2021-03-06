project(leon)

cmake_minimum_required(VERSION 2.6)

################################################################################
# Define cmake modules directory
################################################################################
FOREACH (path "cmake" "../../cmake")
    IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
        SET (CMAKE_MODULE_PATH  "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
    ENDIF()
ENDFOREACH(path)

################################################################################
# THIRD PARTIES
################################################################################

# We don't want to install some GATB-CORE artifacts
 SET (GATB_CORE_EXCLUDE_TOOLS     1)
 SET (GATB_CORE_EXCLUDE_TESTS     1)
 SET (GATB_CORE_EXCLUDE_EXAMPLES  1)

# GATB CORE
include (GatbCore)

# LEON
#LOOKUP_PATH ("leon" "../leon;thirdparty/leon" ${PROJECT_SOURCE_DIR} LEON_PATH)
#LIST (APPEND CPACK_SOURCE_INSTALLED_DIRECTORIES  "${LEON_PATH}" "thirdparty/leon") 

################################################################################
# TOOL
################################################################################

# we get compilation definitions from the gatb-core part
add_definitions (${gatb-core-flags})

# we give the headers directories from : 
#       - from project source 
#       - from GATB-CORE source
#       - from dsk source 
include_directories (src  ${gatb-core-includes}  ${LEON_PATH}/src)

# we define the files to be compiled
file (GLOB_RECURSE  ProjectFiles  src/* ${LEON_PATH}/src/RangeCoder.* ${LEON_PATH}/src/HeaderCoder.* ${LEON_PATH}/src/DnaCoder.*)

# we define the artifact to be built: the project binary
add_executable        (${PROJECT_NAME}  ${ProjectFiles})

# we define which libraries to be linked with project binary
target_link_libraries (${PROJECT_NAME}  ${gatb-core-libraries})

################################################################################
#  PACKAGING 
################################################################################

# We chose the kind of archive we want to generate
SET (CPACK_GENERATOR            "TGZ")
SET (CPACK_SOURCE_GENERATOR     "TGZ")

# We copy the project binary to the 'bin' directory
INSTALL (TARGETS ${PROJECT_NAME} DESTINATION bin)

# For the source archive, we exclude some unwanted directories.
SET (CPACK_SOURCE_IGNORE_FILES  ${CPACK_SOURCE_IGNORE_FILES} "/build/" ".gitignore$")

# To be done at the end. 
INCLUDE (Delivery)
