cmake_minimum_required(VERSION 3.28)
project(chess_weekend)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_CPP_FILES
        src/bitboard.cc
        src/board.cc
        src/position.cc
        src/uciloop.cc
        src/lc0string.cc
        src/christian_utils.cpp
)
#set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -march=native)

include_directories(src)
add_executable(whisperchess_alpha_beta ${COMMON_CPP_FILES} src/agent_alpha_beta.cpp)
add_executable(whisperchess_adaptive_search ${COMMON_CPP_FILES} src/agent_adaptive_search.cpp)
