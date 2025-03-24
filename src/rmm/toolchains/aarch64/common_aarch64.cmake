#
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright TF-RMM Contributors.
#

include_guard()

include(${CMAKE_CURRENT_LIST_DIR}/../common.cmake)

foreach(language IN ITEMS ASM C)
	string(APPEND CMAKE_${language}_FLAGS_INIT "-ffreestanding ")
	string(APPEND CMAKE_${language}_FLAGS_INIT "-march=armv8.5-a ")
	string(APPEND CMAKE_${language}_FLAGS_INIT "-mgeneral-regs-only ")
	string(APPEND CMAKE_${language}_FLAGS_INIT "-mstrict-align ")
	string(APPEND CMAKE_${language}_FLAGS_INIT "-fpie ")
	string(APPEND CMAKE_${language}_FLAGS_INIT "-gdwarf-4 ")
endforeach()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-pie ")
