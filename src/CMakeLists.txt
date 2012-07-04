set(FLOWSTATD_SRCS
    command.c
    netflow.c
    netflow_handler_v5.c
    netflow_handler_v9.c
    flowstatd.c
    fttime.c
    multiplex.c
    socket.c
    global.c
)

find_package(ZLIB REQUIRED)
link_libraries(${ZLIB_LIBRARIES} jansson)
add_executable(flowstatd ${FLOWSTATD_SRCS})
install(TARGETS flowstatd RUNTIME DESTINATION bin)
