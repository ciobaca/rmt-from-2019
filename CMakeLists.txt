cmake_minimum_required(VERSION 2.8.9)
project(rmt)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(examples)

include(CTest)

add_test(TestSmt1 "./rmt" -v 0 ../tests/smt/smt1.rmt)
set_tests_properties (TestSmt1 PROPERTIES PASS_REGULAR_EXPRESSION "is SAT")

add_test(TestSmt2 "./rmt" -v 0 ../tests/smt/smt2.rmt)
set_tests_properties (TestSmt2 PROPERTIES PASS_REGULAR_EXPRESSION "is UNSAT")

add_test(TestSmt3 "./rmt" -v 0 ../tests/smt/smt3.rmt)
set_tests_properties (TestSmt3 PROPERTIES PASS_REGULAR_EXPRESSION "is false")

add_test(TestSmt4 "./rmt" -v 0 ../tests/smt/smt4.rmt)
set_tests_properties (TestSmt4 PROPERTIES PASS_REGULAR_EXPRESSION "is B")

add_test(TestSmt5 "./rmt" -v 0 ../tests/smt/smt5.rmt)
set_tests_properties (TestSmt5 PROPERTIES PASS_REGULAR_EXPRESSION "result is")

add_test(TestReachExample1 "./rmt" -v 0 ../examples/z-all-examples/example1.rmt)
set_tests_properties (TestReachExample1 PROPERTIES PASS_REGULAR_EXPRESSION "Success: 2 solutions.")

add_test(TestReachExample2 "./rmt" -v 0 ../examples/z-all-examples/example2.rmt)
set_tests_properties (TestReachExample2 PROPERTIES PASS_REGULAR_EXPRESSION "Success: 1 solutions.")

add_test(TestReachExample3 "./rmt" -v 0 ../examples/z-all-examples/example3.rmt)
set_tests_properties (TestReachExample3 PROPERTIES PASS_REGULAR_EXPRESSION "Success: 4 solutions.")

add_test(TestReachExample4 "./rmt" -v 0 ../examples/z-all-examples/example4.rmt)
set_tests_properties (TestReachExample4 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")
set_tests_properties (TestReachExample4 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")

add_test(TestReachExample5 "./rmt" -v 0 ../examples/z-all-examples/example5.rmt)
set_tests_properties (TestReachExample5 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample5 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 not proved.")

add_test(TestReachExample6Slow "./rmt" -v 0 ../examples/z-all-examples/example6.rmt)
set_tests_properties (TestReachExample6Slow PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample6Slow PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")

add_test(TestReachExample8Slow "./rmt" -v 0 ../examples/z-all-examples/example8.rmt)
set_tests_properties (TestReachExample8Slow PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample8Slow PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")

add_test(TestReachExample10 "./rmt" -v 0 ../examples/z-all-examples/example10.rmt)
set_tests_properties (TestReachExample10 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample10 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")

add_test(TestReachExample11 "./rmt" -v 0 ../examples/z-all-examples/example11.rmt)
set_tests_properties (TestReachExample11 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample11 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")

add_test(TestReachExample12 "./rmt" -v 0 ../examples/z-all-examples/example12.rmt)
set_tests_properties (TestReachExample12 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample12 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")
set_tests_properties (TestReachExample12 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #3 proved.")

add_test(TestReachExample13 "./rmt" -v 0 ../examples/z-all-examples/example13.rmt)
set_tests_properties (TestReachExample13 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #1 proved.")
set_tests_properties (TestReachExample13 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #2 proved.")
set_tests_properties (TestReachExample13 PROPERTIES PASS_REGULAR_EXPRESSION "Circularity #3 proved.")

add_test(TestReachExample14 "./rmt" -v 0 ../examples/z-all-examples/example14.rmt)
set_tests_properties (TestReachExample14 PROPERTIES PASS_REGULAR_EXPRESSION "Success: 2 solutions.")
