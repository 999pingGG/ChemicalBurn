cmake_minimum_required(VERSION 3.5)
project(ChemicalBurn C)

set(CMAKE_C_STANDARD 17)

add_definitions(-DCB_ORGANIZATION="elinge" -DCB_APP="ChemicalBurn")

add_executable(ChemicalBurn
  src/main.c
  src/simulation.c
  include/chemicalburn/simulation.h
  src/node.c
  include/chemicalburn/node.h
  include/chemicalburn/connection.h
  src/connection.c
  include/chemicalburn/package.h
  src/package.c
  include/chemicalburn/util.h
  src/util.c
  include/chemicalburn/settings.h
  include/chemicalburn/node_vec.h
  src/node_vec.c
  include/chemicalburn/connection_vec.h
  src/connection_vec.c
  include/chemicalburn/connections_map.h
  src/connections_map.c
  include/chemicalburn/package_set.h
  src/package_set.c
  include/chemicalburn/package_queue.h
  src/package_queue.c
  include/chemicalburn/cb_math.h
  src/cb_math.c
  src/settings.c
)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
target_link_libraries(ChemicalBurn PRIVATE SDL3::SDL3)

if (UNIX)
  target_link_libraries(ChemicalBurn PRIVATE m)
endif()

target_include_directories(ChemicalBurn PRIVATE include)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()

include(CheckCSourceCompiles)
check_c_source_compiles(
"
__attribute__((unused)) static void a(void);
int main(int a, char**b){return 0;}
"
HAVE___ATTRIBUTE__UNUSED
)

if (HAVE___ATTRIBUTE__UNUSED)
  add_definitions(-DHAVE___ATTRIBUTE__UNUSED)
endif()

check_c_source_compiles(
"
__pragma(warning(suppress:4100)) static void a(void);
int main(int a, char**b){return 0;}
"
HAVE___PRAGMA_WARNING_SUPPRESS
)

if (HAVE___PRAGMA_WARNING_SUPPRESS)
  add_definitions(-DHAVE___PRAGMA_WARNING_SUPPRESS)
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  add_executable(fuzzer src/fuzz_target.c src/settings.c)
  target_include_directories(fuzzer PRIVATE include)
  target_compile_options(fuzzer PRIVATE -g -O1 -fsanitize=fuzzer,address,undefined)
  target_link_options(fuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
  target_link_libraries(fuzzer PRIVATE SDL3::SDL3)
endif()
