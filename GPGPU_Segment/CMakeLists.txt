#================================================
# CMake declarations for GPGPU_Segment
#================================================

find_package(OpenCL REQUIRED 1.1)

# Host source files
set(GPGPU_SRCS
    src/Abstract_Functor.cpp
    src/CL_Electrostatics.cpp
    src/CL_Manager.cpp
    src/Electrostatics.cpp
)


    add_library( GPGPU_Segment STATIC
            ${GPGPU_SRCS})

target_include_directories(GPGPU_Segment PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(GPGPU_Segment ${OpenCL_LIBRARIES})
