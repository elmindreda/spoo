
include_directories(${spoo_SOURCE_DIR}/src
                    ${spoo_BINARY_DIR}/src
                    ${SPOO_INCLUDE_DIR})

add_library(spoo STATIC ${spoo_SOURCES})
target_link_libraries(spoo ${spoo_LIBRARIES})

