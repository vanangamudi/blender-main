
set(INC
	.
	../kernel
	../kernel/svm
	../kernel/osl
	../util
	../render
	${OPENGL_INCLUDE_DIR}
	${GLEW_INCLUDE_PATH}
)

set(SRC
	device.cpp
	device_cpu.cpp
	device_cuda.cpp
	device_multi.cpp
	device_network.cpp
	device_opencl.cpp
)

set(SRC_HEADERS
	device.h
	device_memory.h
	device_intern.h
	device_network.h
)

add_definitions(-DGLEW_STATIC)

include_directories(${INC})

add_library(cycles_device ${SRC} ${SRC_HEADERS})
