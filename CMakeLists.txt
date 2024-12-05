cmake_minimum_required(VERSION 3.28)
project(DragonUtils)
set(CMAKE_CXX_STANDARD 26)

include(FetchContent)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
)

FetchContent_MakeAvailable(nlohmann_json)
include_directories(${nlohmann_json_SOURCE_DIR}/single_include)

add_executable(dragon main.cpp)

set(COPY_DESTINATION "$ENV{APPDATA}/Microsoft/Windows/Start Menu/Programs/dragon")

add_custom_command(
        TARGET dragon
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COPY_DESTINATION}"
        COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:dragon>
            ${COPY_DESTINATION}
)