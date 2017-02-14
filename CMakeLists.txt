
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

# Global vars

SET(CMAKE_PROJECT_NAME "LockscreenHotKeyDaemon")

SET(EXECUTABLE_NAME "lhkd")

SET(EXECUTABLE_VERSION_MAJOR "0")
SET(EXECUTABLE_VERSION_MINOR "1")
SET(EXECUTABLE_VERSION_PATCH "0")

SET(EXECUTABLE_VERSION "${EXECUTABLE_VERSION_MAJOR}.${EXECUTABLE_VERSION_MINOR}.${EXECUTABLE_VERSION_PATCH}")

# Search all .cpp files within src - recursive!
# You can add all source files by hand here too
file(GLOB_RECURSE SRCS "src/*.cpp")


# Add compiler flags
add_definitions(-std=c++11 -Wall) # -g O3 etc are added according to Release / Debug compilation

ADD_EXECUTABLE(
    ${EXECUTABLE_NAME}
    ${SRCS}
)

TARGET_LINK_LIBRARIES(
    ${EXECUTABLE_NAME} X11 Xi
)
