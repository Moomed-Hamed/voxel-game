// Copyright (c) 2021 Mohamed Hamed
// Intermediary version 9.7.21

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
#define force_inline __forceinline // may only work with MSVC
#define Alloc(type, count) (type *)calloc(count, sizeof(type))

#include <proprietary/mathematics.h>

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