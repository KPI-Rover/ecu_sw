set(CFLAGS -Og -g3 -Wall -Wpedantic -Wno-pointer-to-int-cast)
set(CPPFLAGS -std=c++20 -Og -g3 -Wall -Wpedantic)

cmake_minimum_required(VERSION 3.25.1)

project(buzzer_testing)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

add_library(scheduler STATIC src/scheduler.c)
add_library(timers STATIC src/timers.c)
add_library(stm32f4xx_hal_gpio STATIC src/stm32f4xx_hal_gpio.c)
add_library(stm32f4xx_hal_systick STATIC src/stm32f4xx_hal_systick.c)
add_library(cmsis_os2 STATIC src/cmsis_os2.c)
add_library(driver STATIC ../driver.c)
add_library(generic STATIC ../generic.c)

add_executable(buzzer-driver test/buzzer-driver.cpp)

target_include_directories(scheduler PRIVATE . inc)
target_include_directories(timers PRIVATE . inc)
target_include_directories(stm32f4xx_hal_gpio PRIVATE inc)
target_include_directories(stm32f4xx_hal_systick PRIVATE inc)
target_include_directories(driver PRIVATE .. . inc)
target_include_directories(cmsis_os2 PRIVATE . inc)
target_include_directories(generic PRIVATE . inc)
target_include_directories(buzzer-driver PRIVATE .. . inc ${CMAKE_BINARY_DIR}/googletest)

target_link_libraries(scheduler timers)
target_link_libraries(driver timers stm32f4xx_hal_gpio stm32f4xx_hal_systick)
target_link_libraries(cmsis_os2 timers)
target_link_libraries(buzzer-driver gtest gtest_main scheduler timers stm32f4xx_hal_gpio driver cmsis_os2 generic)

target_compile_options(scheduler PRIVATE ${CFLAGS})
target_compile_options(timers PRIVATE ${CFLAGS})
target_compile_options(stm32f4xx_hal_gpio PRIVATE ${CFLAGS})
target_compile_options(driver PRIVATE ${CFLAGS})
target_compile_options(cmsis_os2 PRIVATE ${CFLAGS})
target_compile_options(buzzer-driver PRIVATE ${CPPFLAGS})
