set(CFLAGS -Og -g3 -Wall -Wpedantic -Wno-pointer-to-int-cast)

cmake_minimum_required(VERSION 3.25.1)

project(buzzer_testing C)

add_library(scheduler STATIC src/scheduler.c)
add_library(timers STATIC src/timers.c)
add_library(stm32f4xx_hal_gpio STATIC src/stm32f4xx_hal_gpio.c)
add_library(driver STATIC ../driver.c)

add_executable(buzzer-driver test/buzzer-driver.c)

target_include_directories(scheduler PRIVATE . inc)
target_include_directories(timers PRIVATE . inc)
target_include_directories(stm32f4xx_hal_gpio PRIVATE inc)
target_include_directories(driver PRIVATE .. . inc)
target_include_directories(buzzer-driver PRIVATE .. . inc)

target_link_libraries(scheduler timers)
target_link_libraries(driver timers stm32f4xx_hal_gpio)
target_link_libraries(buzzer-driver scheduler timers stm32f4xx_hal_gpio driver)

target_compile_options(scheduler PRIVATE ${CFLAGS})
target_compile_options(timers PRIVATE ${CFLAGS})
target_compile_options(stm32f4xx_hal_gpio PRIVATE ${CFLAGS})
target_compile_options(driver PRIVATE ${CFLAGS})
target_compile_options(buzzer-driver PRIVATE ${CFLAGS})

# include actual driver dir to compile

# then try to compile whole project
