# - Find allegro
# Find the native ALLEGRO includes and library
#
# ALLEGRO_INCLUDE_DIR - where to find allegro.h, etc.
# ALLEGRO_LIBRARIES - List of libraries when using allegro.
# ALLEGRO_FOUND - True if allegro found.

FIND_PATH(ALLEGRO_INCLUDE_DIR allegro.h
/usr/local/include
/usr/include
$ENV{MINGDIR}/include
)

if(UNIX AND NOT CYGWIN)
    # exec_program(allegro-config ARGS --libs OUTPUT_VARIABLE ALLEGRO_LIBRARY)
else(UNIX AND NOT CYGWIN)
    SET(ALLEGRO_NAMES alleg alleglib alleg41 alleg42 allegdll)
    FIND_LIBRARY(ALLEGRO_LIBRARY
        NAMES ${ALLEGRO_NAMES}
        PATHS /usr/lib /usr/local/lib $ENV{MINGDIR}/lib)
endif(UNIX AND NOT CYGWIN)

IF (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
    SET(ALLEGRO_FOUND TRUE)
    SET( ALLEGRO_LIBRARIES ${ALLEGRO_LIBRARY} )
ELSE (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)
    SET(ALLEGRO_FOUND FALSE)
    SET( ALLEGRO_LIBRARIES )
ENDIF (ALLEGRO_INCLUDE_DIR AND ALLEGRO_LIBRARY)

IF (ALLEGRO_FOUND)
    MESSAGE(STATUS "Found Allegro: ${ALLEGRO_LIBRARY}")
ELSE (ALLEGRO_FOUND)
    MESSAGE(STATUS "Looked for Allegro libraries named ${ALLEGRO_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find Allegro library")
ENDIF (ALLEGRO_FOUND)

MARK_AS_ADVANCED(
ALLEGRO_LIBRARY
ALLEGRO_INCLUDE_DIR
)
