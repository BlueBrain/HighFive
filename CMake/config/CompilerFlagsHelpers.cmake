# CompilerFlagsHelpers.cmake
#
# set of Convenience functions for portable compiler flags
#
# License: BSD 3
#
# Copyright (c) 2016, Adrien Devresse
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



set(SUPPORTED_COMPILER_LANGUAGE_LIST "C;CXX")

foreach(COMPILER_LANGUAGE ${SUPPORTED_COMPILER_LANGUAGE_LIST})

	# XLC compiler
	if(CMAKE_${COMPILER_LANGUAGE}_COMPILER_ID MATCHES "XL")

		# XLC -qinfo=all is awfully verbose on any platforms that use the GNU STL
		# Enable by default only the relevant one
		set(CMAKE_${COMPILER_LANGUAGE}_WARNING_ALL "-qformat=all -qinfo=lan:trx:ret:zea:cmp:ret")

	## GCC, CLANG, rest of the world
	elseif(CMAKE_${COMPILER_LANGUAGE}_COMPILER_ID MATCHES "Clang"
        OR CMAKE_${COMPILER_LANGUAGE}_COMPILER_ID MATCHES "GNU"
        OR CMAKE_${COMPILER_LANGUAGE}_COMPILER_ID MATCHES "Intel")
		set(CMAKE_${COMPILER_LANGUAGE}_WARNING_ALL " -Wall -Wextra")
		string(CONCAT CMAKE_${COMPILER_LANGUAGE}_WARNING_DEBUG
			" -Werror -Wshadow -Wnon-virtual-dtor -Wunused -Woverloaded-virtual"
			" -Wformat=2 -Wconversion -Wsign-conversion -Wno-error=deprecated-declarations"
		)
		if(NOT CMAKE_${COMPILER_LANGUAGE}_COMPILER_IS_ICC)
			string(CONCAT CMAKE_${COMPILER_LANGUAGE}_WARNING_DEBUG
				${CMAKE_${COMPILER_LANGUAGE}_WARNING_DEBUG}
				" -Wpedantic -Wcast-align -Wdouble-promotion"
			)
		endif()
	endif()

    if(CMAKE_${COMPILER_LANGUAGE}_COMPILER_ID MATCHES "GNU"
            AND (CMAKE_${COMPILER_LANGUAGE}_COMPILER_VERSION VERSION_GREATER "4.7.0"))
        set(CMAKE_${COMPILER_LANGUAGE}_WARNING_ALL
                "${CMAKE_${COMPILER_LANGUAGE}_WARNING_ALL} -Wno-unused-parameter")
    endif()

endforeach()



