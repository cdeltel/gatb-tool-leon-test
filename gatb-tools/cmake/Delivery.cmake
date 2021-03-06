################################################################################
# The version number.
################################################################################
IF (DEFINED MAJOR)
    SET (CPACK_PACKAGE_VERSION_MAJOR ${MAJOR})
ENDIF()
IF (DEFINED MINOR)
    SET (CPACK_PACKAGE_VERSION_MINOR ${MINOR})
ENDIF()
IF (DEFINED PATCH)
    SET (CPACK_PACKAGE_VERSION_PATCH ${PATCH})
ENDIF()

################################################################################
# DELIVERY
################################################################################

# We get the 'package' and 'package_source' targets
include (CPack)

# We get the user name
IF (DEFINED GFORGE_USER)
    SET (CPACK_USER_NAME  ${GFORGE_USER})
ELSE()
    SET (CPACK_USER_NAME  $ENV{USER})
ENDIF()

# We get the date
GetCurrentDateShort (CPACK_DATE) 

# We set the name of the versions file.
SET (CPACK_VERSIONS_FILENAME  "versions.txt")

SET (PROJECT_GLOBAL_NAME "gatb-tools")

# We set the server URI
SET (CPACK_SERVER_ADDRESS   "${CPACK_USER_NAME}@scm.gforge.inria.fr")
SET (CPACK_SERVER_DIR       "/home/groups/${PROJECT_GLOBAL_NAME}/htdocs/versions/")
SET (CPACK_SERVER_DIR_BIN   "${CPACK_SERVER_DIR}/bin/")
SET (CPACK_SERVER_DIR_SRC   "${CPACK_SERVER_DIR}/src/")
SET (CPACK_SERVER_VERSIONS  "${CPACK_SERVER_DIR}/${CPACK_VERSIONS_FILENAME}")

# We define the name of the bin and src targets
SET (CPACK_URI_BIN "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}.tar.gz")
SET (CPACK_URI_SRC "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Source.tar.gz")

# We define the location where the bin and src targets have to be uploaded
SET (CPACK_UPLOAD_URI_BIN  "${CPACK_SERVER_ADDRESS}:${CPACK_SERVER_DIR_BIN}")
SET (CPACK_UPLOAD_URI_SRC  "${CPACK_SERVER_ADDRESS}:${CPACK_SERVER_DIR_SRC}")
SET (CPACK_UPLOAD_VERSIONS "${CPACK_SERVER_ADDRESS}:${CPACK_SERVER_VERSIONS}")

# We set the text holding all the information about the delivery.
SET (CPACK_INFO_BIN ${PROJECT_GLOBAL_NAME} bin ${PROJECT_NAME} ${CPACK_PACKAGE_VERSION} ${CPACK_DATE} ${CPACK_SYSTEM_NAME} ${CPACK_USER_NAME} ${CPACK_URI_BIN})
SET (CPACK_INFO_SRC ${PROJECT_GLOBAL_NAME} src ${PROJECT_NAME} ${CPACK_PACKAGE_VERSION} ${CPACK_DATE} all ${CPACK_USER_NAME} ${CPACK_URI_SRC})

# We define the delivery script URL
SET (DELIVERY_SCRIPT_URL "${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/delivery.sh")

################################################################################
# MAIN TARGET 
################################################################################

# We add a custom target for delivery
add_custom_target (delivery     

    DEPENDS delivery_bin  delivery_src 
    
    COMMAND echo "-----------------------------------------------------------"
    COMMAND echo "DELIVERY FOR ${PROJECT_NAME}, VERSION ${CPACK_PACKAGE_VERSION}"
    COMMAND echo "-----------------------------------------------------------"

    # We dump the known versions
    COMMAND make delivery_dump
)

################################################################################
# TARGETS 'bin'
################################################################################

# We add a custom target for delivery binaries
add_custom_target (delivery_bin 

    # We get the versions.txt file from the server
    COMMAND ${DELIVERY_SCRIPT_URL}  "BIN" ${PROJECT_NAME} ${CPACK_PACKAGE_VERSION} ${CPACK_UPLOAD_VERSIONS} ${CPACK_VERSIONS_FILENAME}  \"${CPACK_INFO_BIN}\"  ${CPACK_URI_BIN}   ${CPACK_UPLOAD_URI_BIN}
)

################################################################################
# TARGETS 'src'
################################################################################

# We add a custom target for delivery sources
add_custom_target (delivery_src 

    # We get the versions.txt file from the server
    COMMAND ${DELIVERY_SCRIPT_URL}  "SRC" ${PROJECT_NAME} ${CPACK_PACKAGE_VERSION} ${CPACK_UPLOAD_VERSIONS} ${CPACK_VERSIONS_FILENAME}  \"${CPACK_INFO_SRC}\"  ${CPACK_URI_SRC}   ${CPACK_UPLOAD_URI_SRC}
)

################################################################################
# TARGET 'help'
################################################################################

# We add a custom target for delivery sources
add_custom_target (delivery_help
    COMMAND echo "-----------------------------------------------------------"
    COMMAND echo "DELIVERY TARGETS"
    COMMAND echo "-----------------------------------------------------------"
    COMMAND echo "delivery:      build a targz for binaries and sources and upload them to GForge"
    COMMAND echo "delivery_bin:  build a targz for binaries and upload it to GForge"
    COMMAND echo "delivery_src:  build a targz for sources and upload it to GForge"
    COMMAND echo "delivery_dump: dump existing releases on GForge"
    COMMAND echo ""
)

################################################################################
# TARGET 'dump'
################################################################################

# We add a custom target for dumping existing deliveries
add_custom_target (delivery_dump

    # We get the versions.txt file from the server
    COMMAND scp ${CPACK_UPLOAD_VERSIONS} ${CPACK_VERSIONS_FILENAME}

    # We dump the versions file.
    COMMAND echo ""
    COMMAND echo "-------------------------------------------------------------------------------------------------"
    COMMAND echo "LIST OF DELIVERIES FOR " ${CMAKE_PROJECT_NAME}
    COMMAND echo "-------------------------------------------------------------------------------------------------"
    COMMAND cat ${CPACK_VERSIONS_FILENAME}
    COMMAND echo "-------------------------------------------------------------------------------------------------"
    COMMAND echo ""
)
