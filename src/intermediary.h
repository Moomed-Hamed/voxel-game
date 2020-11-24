//  CORE PRINCIPLES :
// - understand limits, no program can process infinite data
// - know exact program memory usage

#pragma comment(lib, "winmm")
#pragma comment(lib, "opengl32")
#pragma comment(lib, "external/GLEW/glew32s")
#pragma comment(lib, "external/GLFW/glfw3")

#define _CRT_SECURE_NO_WARNINGS // because printf is "too dangerous"

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define GLEW_STATIC
#include <external/GLEW\glew.h> // OpenGL functions
#include <external/GLFW\glfw3.h>// window & input

#include <Windows.h>
#include <fileapi.h>
#include <iostream>

#define out(val) std::cout << ' ' << val << '\n'
#define stop std::cin.get()
#define print printf
#define printvec(vec) printf("%f %f %f\n", vec.x, vec.y, vec.z)

#define PI	  3.14159265359f
#define TWOPI 6.28318530718f

#define ToRadians(value) ( ((value) * PI) / 180.0f )
#define ToDegrees(value) ( ((value) * 180.0f) / PI )

#define force_inline __forceinline // may only work with MSVC

#define Alloc(type, count) (type *)calloc(count, sizeof(type))

#define GLM_ENABLE_EXPERIMENTAL
#include <external/GLM/glm.hpp> //for math
#include <external/GLM/gtc/matrix_transform.hpp>
#include <external/GLM/gtc/quaternion.hpp> //for quaternions
#include <external/GLM/gtx/quaternion.hpp>
using glm::vec2;  using glm::vec3; using glm::vec4;
using glm::mat3;  using glm::mat4;
using glm::quat;
using glm::ivec2; using glm::ivec3;
using glm::uvec2; using glm::uvec3;

#define NONE 0
#define INVALID 65535

typedef signed char  int8;
typedef signed short int16;
typedef signed int   int32;
typedef signed long long int64;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long long uint64;

typedef uint8  u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef int8  s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;

typedef s32 sint;
typedef u32 uint;
typedef u32 bool32;

typedef float  f32;
typedef double f64;

typedef uint8 byte;

byte* read_text_file_into_memory(const char* path, uint* file_size)
{
	DWORD BytesRead;
	HANDLE os_file = CreateFile(path, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	LARGE_INTEGER size;
	GetFileSizeEx(os_file, &size);

	byte* memory = (byte*)calloc(size.QuadPart + 1, sizeof(byte));
	ReadFile(os_file, memory, size.QuadPart, &BytesRead, NULL);

	CloseHandle(os_file);

	*file_size = size.QuadPart;
	return memory;
}

mat3 point_at(vec3 dir, vec3 up)
{
	//Assumed to be normalized
	vec3 f = dir * -1.f; //front
	vec3 r = cross(up, f); //right
	vec3 u = up;

	glm::mat3 result(1);
	result[0][0] = r.x;
	result[1][0] = r.y;
	result[2][0] = r.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = f.x;
	result[1][2] = f.y;
	result[2][2] = f.z;

	return glm::inverse(result);
}

//linearly interpolate between a0 and a1, Weight w should be in the range [0.0, 1.0]
float interpolate(float f1, float f2, float w)
{
	if (0.0 > w) return f1;
	if (1.0 < w) return f2;

	//return (f2 - f1) * w + f1;
	// Use this cubic interpolation (smoothstep) instead, for a smooth appearance:
	 return (f2 - f1) * (3.0 - w * 2.0) * w * w + f1;
	// Use (smoothstep) for an even smoother result with a second derivative equal to zero on boundaries:
	//return (f2 - f1) * (x * (w * 6.0 - 15.0) * w * w *w + 10.0) + f1;
}

// create random direction vector
vec2 random_gradient(int ix, int iy)
{
	// random float. no precomputed gradients = this works for any number of grid coordinates
	float random = 2920.f * sin(ix * 21942.f + iy * 171324.f + 8912.f) * cos(ix * 23157.f * iy * 217832.f + 9758.f);
	return vec2(cos(random), sin(random));
}

// Computes the dot product of the distance and gradient vectors.
float dot_grid_gradient(int ix, int iy, float x, float y)
{
	vec2 gradient = random_gradient(ix, iy);
	vec2 distance = { x - (float)ix, y - (float)iy };

	return glm::dot(distance, gradient);
}

// compute perlin noise at coordinates x, y
float perlin(float x, float y)
{
	// grid coordinates
	int x0 = (int)x, y0 = (int)y;
	int x1 = x0 + 1, y1 = y0 + 1;

	// interpolation weights (could also use higher order polynomial/s-curve here)
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// interpolate between grid point gradients
	float n0, n1, ix0, ix1, value;

	n0 = dot_grid_gradient(x0, y0, x, y);
	n1 = dot_grid_gradient(x1, y0, x, y);
	ix0 = interpolate(n0, n1, sx);

	n0 = dot_grid_gradient(x0, y1, x, y);
	n1 = dot_grid_gradient(x1, y1, x, y);
	ix1 = interpolate(n0, n1, sx);

	value = interpolate(ix0, ix1, sy);
	return (value + 1) / 2;
}

// -- Timers -- //

// while the raw timestamp can be used for relative performence measurements,
// it does not necessarily correspond to any external notion of time
typedef uint64 Timestamp;

Timestamp get_timestamp()
{
	LARGE_INTEGER win32_timestamp;
	QueryPerformanceCounter(&win32_timestamp);

	return win32_timestamp.QuadPart;
}

int64 calculate_milliseconds_elapsed(Timestamp start, Timestamp end)
{
	//Get CPU clock frequency for Timing
	LARGE_INTEGER win32_performance_frequency;
	QueryPerformanceFrequency(&win32_performance_frequency);

	return (1000 * (end - start)) / win32_performance_frequency.QuadPart;
}
int64 calculate_microseconds_elapsed(Timestamp start, Timestamp end)
{
	//Get CPU clock frequency for Timing
	LARGE_INTEGER win32_performance_frequency;
	QueryPerformanceFrequency(&win32_performance_frequency);

	// i think (end - start) corresponds directly to cpu clock cycles but i'm not sure
	return (1000000 * end - start) / win32_performance_frequency.QuadPart;
}

void os_sleep(uint milliseconds)
{
#define DESIRED_SCHEDULER_GRANULARITY 1 // milliseconds
	HRESULT SchedulerResult = timeBeginPeriod(DESIRED_SCHEDULER_GRANULARITY);
#undef DESIRED_SCHEDULER_GRANULARITY

	Sleep(milliseconds);
}