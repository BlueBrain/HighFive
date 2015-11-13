## debug / release autoManagement


## default configuration
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
endif()

set(STACK_PROTECTION_FLAGS "")

if(CMAKE_COMPILER_IS_GNUCC)
set(STACK_PROTECTION_FLAGS "-fstack-protector-all")
endif()

set(CMAKE_C_FLAGS_RELEASE  "-O2 -Wall")
set(CMAKE_C_FLAGS_DEBUG  "-g -Wall -Wextra -pedantic ${STACK_PROTECTION_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO " -Wall -g -O2 -Wall -Wextra ")


set(CMAKE_CXX_FLAGS_RELEASE  "-O2 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG  "-g -Wall -Wextra  ${STACK_PROTECTION_FLAGS} ")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -Wall -g -O2 -Wall -Wextra ")
