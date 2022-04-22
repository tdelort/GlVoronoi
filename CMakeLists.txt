cmake_minimum_required(VERSION 3.13)

project(GlVoronoi)

#add_library(${PROJECT_NAME} STATIC)

# adding src, libs, and include dirs
# add_subdirectory(src)
add_subdirectory(libs)
# target_include_directories(${PROJECT_NAME} PUBLIC include)

# creating an exe for the demo of the algorithm
add_executable(${PROJECT_NAME} demo.cpp)

# linking external librairies to the project
target_link_libraries(${PROJECT_NAME} IMGUI GLAD)

# linking the Voronoi lib to the demo
# target_link_libraries(GlVoronoiDemo ${PROJECT_NAME})