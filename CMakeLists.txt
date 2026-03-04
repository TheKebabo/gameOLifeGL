cmake_minimum_required(VERSION 3.14) # Required for FetchContent
project(gameOLifeGL)

set(CMAKE_CXX_STANDARD 17)

# --- 1. GET GLFW AUTOMATICALLY ---
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.3.8 # Or latest version
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- 2. DEFINE YOUR FILES ---
# Include glad.c so it gets compiled into your project
set(SOURCES 
    main.cpp 
    src/glad.c 
    src/shader_program.cpp
    src/vf_shader_program.cpp
    src/compute_shader_program.cpp
)

# --- 3. CREATE EXECUTABLE ---
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SHADER_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/\"
)

# --- 4. LINK HEADERS AND LIBRARIES ---
# Tell CMake where your local headers (glad, glm) are
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link the OpenGL library (built into Windows) and GLFW
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)