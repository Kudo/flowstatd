set(FLOWD_SRCS
    command.c
    netflow.c
    netflow_handler_v5.c
    netflow_handler_v9.c
    flowd.c
    fttime.c
    multiplex.c
    socket.c
    global.c
)

find_package(ZLIB REQUIRED)
link_libraries(${ZLIB_LIBRARIES})
add_executable(flowd ${FLOWD_SRCS})
install(TARGETS flowd RUNTIME DESTINATION bin)
