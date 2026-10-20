# Snowstorm-Sand: a falling-sand game on Snowstorm Engine. The engine is a submodule, consumed and pinned
# like any other dependency; everything outside it is the game.
cmake_minimum_required(VERSION 3.15)
project(Snowstorm-Sand VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Built from source as a subdirectory: the engine has no install/export package.
add_subdirectory(Snowstorm-Engine)

add_executable(Snowstorm-Sand "${CMAKE_CURRENT_SOURCE_DIR}/SandMain.cpp")

set_target_properties(Snowstorm-Sand PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# WHOLE_ARCHIVE: component registrations and CVars are static initializers in translation units nothing
# references, and a plain link drops them silently rather than failing.
target_link_libraries(Snowstorm-Sand PRIVATE
    $<LINK_LIBRARY:WHOLE_ARCHIVE,Snowstorm-Core>
)

add_custom_command(TARGET Snowstorm-Sand POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:Snowstorm-Sand> $<TARGET_FILE_DIR:Snowstorm-Sand>
    COMMAND_EXPAND_LISTS
)

# The engine owns the `stage` target; a game declares its own contribution to it.
snowstorm_stage_executable(Snowstorm-Sand)
snowstorm_stage_project("${CMAKE_CURRENT_SOURCE_DIR}/Projects/Sand")
