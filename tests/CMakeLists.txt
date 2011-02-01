
link_libraries(spoo ${SPOO_LIBRARIES})

include_directories(${SPOO_INCLUDE_DIR})

add_executable(corecount corecount.c)
add_executable(sleep sleep.c)

