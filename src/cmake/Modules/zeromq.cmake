# CMake include file for zeromq
include(ZMQSourceRunChecks)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckFunctionExists)

check_include_files(ifaddrs.h ZMQ_HAVE_IFADDRS)
check_include_files(windows.h ZMQ_HAVE_WINDOWS)
check_include_files(sys/uio.h ZMQ_HAVE_UIO)
check_include_files(sys/eventfd.h ZMQ_HAVE_EVENTFD)

check_library_exists(ws2_32 printf "" HAVE_WS2_32) # TODO: Why doesn't something logical like WSAStartup work?
check_library_exists(ws2 printf "" HAVE_WS2)
check_library_exists(rpcrt4 printf "" HAVE_RPCRT4) # UuidCreateSequential
check_library_exists(iphlpapi printf "" HAVE_IPHLAPI) # GetAdaptersAddresses

find_library(RT_LIBRARY rt)

find_package(Threads)
set(CMAKE_REQUIRED_LIBRARIES rt)
check_function_exists(clock_gettime HAVE_CLOCK_GETTIME)
set(CMAKE_REQUIRED_LIBRARIES )

set(CMAKE_REQUIRED_INCLUDES sys/time.h)
check_function_exists(gethrtime HAVE_GETHRTIME)
set(CMAKE_REQUIRED_INCLUDES )

zmq_check_sock_cloexec()
zmq_check_so_keepalive()
zmq_check_tcp_keepcnt()
zmq_check_tcp_keepidle()
zmq_check_tcp_keepintvl()
zmq_check_tcp_keepalive()

set(ZMQ-BASE-SOURCES
    address.cpp
    clock.cpp
    ctx.cpp
    curve_client.cpp
    curve_server.cpp
    dealer.cpp
    devpoll.cpp
    dist.cpp
    epoll.cpp
    err.cpp
    fq.cpp
    io_object.cpp
    io_thread.cpp
    ip.cpp
    ipc_address.cpp
    ipc_connecter.cpp
    ipc_listener.cpp
    kqueue.cpp
    lb.cpp
    mailbox.cpp
    mechanism.cpp
    msg.cpp
    mtrie.cpp
    object.cpp
    options.cpp
    own.cpp
    null_mechanism.cpp
    pair.cpp
    pgm_receiver.cpp
    pgm_sender.cpp
    pgm_socket.cpp
    pipe.cpp
    plain_mechanism.cpp
    poll.cpp
    poller_base.cpp
    precompiled.cpp
    proxy.cpp
    pub.cpp
    pull.cpp
    push.cpp
    random.cpp
    raw_encoder.cpp
    raw_decoder.cpp
    reaper.cpp
    rep.cpp
    req.cpp
    router.cpp
    select.cpp
    session_base.cpp
    signaler.cpp
    socket_base.cpp
    stream.cpp
    stream_engine.cpp
    sub.cpp
    tcp.cpp
    tcp_address.cpp
    tcp_connecter.cpp
    tcp_listener.cpp
    thread.cpp
    trie.cpp
    v1_decoder.cpp
    v1_encoder.cpp
    v2_decoder.cpp
    v2_encoder.cpp
    xpub.cpp
    xsub.cpp
    zmq.cpp
    zmq_utils.cpp)

SET(ZMQ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libs/zeromq4)
foreach(source ${ZMQ-BASE-SOURCES})
  list(APPEND ZMQ_SOURCES ${ZMQ_DIR}/src/${source})
endforeach()

configure_file(${ZMQ_DIR}/builds/cmake/platform.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/platform.hpp)
list(APPEND ZMQ_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/platform.hpp)

