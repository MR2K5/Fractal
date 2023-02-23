
target_sources(Viewer PRIVATE main.cpp input.cpp mandel.cpp newton.cpp math_tools.cpp)

add_library(math-tools STATIC math_tools.cpp)
target_link_libraries(math-tools PRIVATE common)

add_executable(test-polynomial "poly_test.cpp")
target_link_libraries(test-polynomial common GTest::gtest_main math-tools)
add_test(NAME test_polynomial COMMAND test-polynomial)
