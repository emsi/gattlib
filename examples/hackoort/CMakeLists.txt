cmake_minimum_required(VERSION 2.6)

set(hackoort_LIBS gattlib)

set(hackoort_SRCS hackoort.c)

add_executable(hackoort ${hackoort_SRCS})
target_link_libraries(hackoort ${hackoort_LIBS})
