# SPDX-License-Identifier: MIT
# Unit tests for the pure core library. No Qt / GUI required, so these run in
# headless CI on any platform.

add_executable(geobiz_core_tests test_core.cpp)
target_link_libraries(geobiz_core_tests PRIVATE geobiz_core)
target_compile_features(geobiz_core_tests PRIVATE cxx_std_23)

add_test(NAME geobiz_core_tests COMMAND geobiz_core_tests)
