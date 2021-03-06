# Editline library : http://thrysoee.dk/editline/

find_path(EDITLINE_INCLUDE_DIR histedit.h
    HINTS
        /opt/local/include
        /usr/local/include
    )

find_library(EDITLINE_LIBRARY edit
    HINTS
        /opt/local/lib
        /usr/local/lib
    )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Editline DEFAULT_MSG
    EDITLINE_INCLUDE_DIR 
    EDITLINE_LIBRARY
    )

set(EDITLINE_LIBRARIES ${EDITLINE_LIBRARY})
