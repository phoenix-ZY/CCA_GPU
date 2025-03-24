#
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
#

include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

find_program(CMAKE_C_COMPILER
    NAMES "clang"
    DOC "Path to clang."
    REQUIRED)

set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

foreach(language IN ITEMS ASM C)
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wno-unknown-warning-option ")
    string(APPEND CMAKE_${language}_FLAGS_INIT "-Wno-unused-function ")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--build-id=none ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld ")
