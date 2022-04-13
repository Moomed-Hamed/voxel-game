#define GLM_ENABLE_EXPERIMENTAL
#include <external/GLM/glm.hpp> // for math
#include <external/GLM/gtc/matrix_transform.hpp>
#include <external/GLM/gtc/quaternion.hpp> // for quaternions
#include <external/GLM/gtx/quaternion.hpp>
#include <external/GLM/gtx/transform.hpp>

using glm::vec2;  using glm::vec3; using glm::vec4;
using glm::mat3;  using glm::mat4;
using glm::quat;
using glm::ivec2; using glm::ivec3;
using glm::uvec2; using glm::uvec3;
using glm::lookAt; using glm::perspective; using glm::fract;

#define PI	  3.14159265359f
#define TWOPI 6.28318530718f
#define INVALID 65535

#define ToRadians(value) ( ((value) * PI) / 180.0f )
#define ToDegrees(value) ( ((value) * 180.0f) / PI )

typedef signed   char      int8 , i8;
typedef signed   short     int16, i16;
typedef signed   int       int32, i32;
typedef signed   long long int64, i64;

typedef unsigned char      uint8 , u8 , byte;
typedef unsigned short     uint16, u16;
typedef unsigned int       uint32, u32, uint;
typedef unsigned long long uint64, u64;

// random numbers

#define BIT_NOISE_1 0xB5297A4D
#define BIT_NOISE_2 0x68E31DA4
#define BIT_NOISE_3 0x1B56C4E9

uint random_uint()
{
	uint seed = __rdtsc();
	seed *= BIT_NOISE_1;
	seed *= seed; // helps avoid linearity
	seed ^= (seed >> 8);
	seed += BIT_NOISE_2;
	seed ^= (seed >> 8);
	seed *= BIT_NOISE_3;
	seed ^= (seed >> 8);
	return seed;
}
int random_int()
{
	union {
		uint seed;
		int ret;
	} u;

	u.seed = random_uint();

	return u.ret;
}
float random_normalized_float() // random float between 0 and 1
{
	uint seed = random_uint();
	return (float)seed / (float)UINT_MAX; // is there a better way to do this?
}
float random_normalized_float_signed() // random float between -1 and 1
{
	return (random_normalized_float() * 2.f) - 1.f;
}
bool random_boolean(float probability_of_returning_true = .5f)
{
	if (random_normalized_float() < probability_of_returning_true) return true;

	return false;
}

// noise

uint random_uint(uint n, uint seed = 0)
{
	n *= BIT_NOISE_1;
	n *= seed; // helps avoid linearity
	n ^= (n >> 8);
	n += BIT_NOISE_2;
	n ^= (n >> 8);
	n *= BIT_NOISE_3;
	n ^= (n >> 8);
	return n;
}
int random_int(uint n, uint seed = 0)
{
	union {
		uint seed;
		int ret;
	} u;

	u.seed = random_uint(n, seed);

	return u.ret;
}
float random_normalized_float(uint n, uint seed = 0) // random float between 0 and 1
{
	seed = random_uint(n, seed);
	return (float)seed / (float)UINT_MAX; // is there a better way to do this?
}
float random_normalized_float_signed(uint n, uint seed = 0) // random float between -1 and 1
{
	return (random_normalized_float(n, seed) * 2.f) - 1.f;
}

// random & noise utilities

float randfn(uint n, uint seed = 0) { return random_normalized_float(n, seed); }
float randfns(uint n, uint seed = 0) { return (random_normalized_float(n, seed) * 2) - 1; }
float randfn() { return random_normalized_float(); }
float randfns() { return random_normalized_float_signed(); }
vec3  randf3n() { return vec3(randfn(), randfn(), randfn()); }
vec3  randf3ns() { return vec3(randfns(), randfns(), randfns()); }
vec3  randf3n(uint a, uint b, uint c) { return vec3(randfn(a), randfn(b), randfn(c)); }
vec3  randf3ns(uint a, uint b, uint c) { return vec3(randfns(a), randfns(b), randfns(c)); }

// interpolation

float lerp_spring(float amount, float stiffness = 1, float period = 1) // probably broken
{
	return 1.f - abs(sin(TWOPI * amount * period) * exp(-stiffness * amount));
}
float lerp(float a, float b, float amount) { return (a + amount * (b - a)); }
vec3  lerp(vec3  a, vec3  b, float amount) { return (a + amount * (b - a)); }
quat  lerp(quat  a, quat  b, float amount) { return (a + amount * (b - a)); }
mat4  lerp(mat4 a, mat4 b, float amount)
{
	vec3 pos_1 = vec3(a[0][3], a[1][3], a[2][3]);
	vec3 pos_2 = vec3(b[0][3], b[1][3], b[2][3]);

	quat rot_1 = quat(a);
	quat rot_2 = quat(b);

	vec3 pos = lerp(pos_1, pos_2, amount);
	quat rot = lerp(rot_1, rot_2, amount);

	mat4 ret = mat4(rot);
	ret[0][3] = pos.x;
	ret[1][3] = pos.y;
	ret[2][3] = pos.z;

	return ret;
}
mat4  nlerp(mat4 a, mat4 b, float amount)
{
	vec3 pos_1 = vec3(a[0][3], a[1][3], a[2][3]);
	vec3 pos_2 = vec3(b[0][3], b[1][3], b[2][3]);

	quat rot_1 = quat(a);
	quat rot_2 = quat(b);

	vec3 pos = lerp(pos_1, pos_2, amount);
	quat rot = normalize(lerp(rot_1, rot_2, amount));

	mat4 ret = mat4(rot);
	ret[0][3] = pos.x;
	ret[1][3] = pos.y;
	ret[2][3] = pos.z;

	return ret;
}

float bezier3(float b, float c, float t) // a = 0 and d = 1
{
	float s = 1.f - t;
	float t2 = t * t;
	float s2 = s * s;
	float t3 = t2 * t;
	return (3.f * b * s2 * t) + (3.f * c * s * t2) + t3;

	// same as this
	//float l1 = lerp(0, b, t);
	//float l2 = lerp(b, c, t);
	//float l3 = lerp(c, 1, t);
	//
	//return lerp(lerp(l1, l12, t), lerp(l2, l3, t), t);
}
float bezier5(float b, float c, float d, float e, float t) // a = 0 and d = 1
{
	// this implementation is not the fastest
	float a1 = lerp(0, b, t);
	float a2 = lerp(b, c, t);
	float a3 = lerp(c, d, t);
	float a4 = lerp(d, e, t);
	float a5 = lerp(e, 1, t);

	float b1 = lerp(a1, a2, t);
	float b2 = lerp(a2, a3, t);
	float b3 = lerp(a3, a4, t);
	float b4 = lerp(a4, a5, t);

	float c1 = lerp(b1, b2, t);
	float c2 = lerp(b2, b3, t);
	float c3 = lerp(b3, b4, t);

	float d1 = lerp(c1, c2, t);
	float d2 = lerp(c2, c3, t);

	return lerp(d1, d2, t);
}
float bezier7(float b, float c, float d, float e, float f, float g, float t) // a = 0 and d = 1
{
	float s = 1.f - t;
	float s2 = t * t;
	float s3 = s2 * t;
	float s4 = s2 * s2;
	float s5 = s3 * s2;
	float s6 = s3 * s3;

	float t2 = t  * t;
	float t3 = t2 * t;
	float t4 = t2 * t2;
	float t5 = t3 * t2;
	float t6 = t3 * t3;
	float t7 = t3 * t4;

	return (7.f * b * s6 * t) + (21.f * c * s5 * t2) + (35.f * d * s4 * t3) * (35.f * e * s3 * t4) + (21.f * f * s2 * t5) + (7.f * g * s * t6) + t7;
}
float bounce(float t, float a = -.45, float b = .25, float c = .55, float d = .75)
{
	return 1 - abs(bezier5(a, b, c, d, 1 - t));
	//return 1 - abs(bezier3(-.45, 0, 1 - t));
}

float smoothstep(float a, float b, float amount)
{
	// Use this cubic interpolation (smoothstep) instead, for a smooth appearance:
	return (b - a) * (3.0 - amount * 2.0) * amount * amount + a;
	// Use (smoothstep) for an even smoother result with a second derivative equal to zero on boundaries:
	//return (b - a) * (amount * (amount * 6.0 - 15.0) * amount * amount * amount + 10.0) + a;
}

// specialized noise

float perlin(float x, float y)
{
	const auto dot_grid_gradient = [](float ix, float iy, float x, float y)
	{
		float n = TWOPI * randfns(ix, iy);

		vec2 gradient = vec2(cos(n), sin(n));
		vec2 distance = { x - ix, y - iy };

		return dot(distance, gradient);
	};

	// grid coordinates
	int X = (int)x;
	int Y = (int)y;

	// interpolation weights (could also use higher order polynomial/s-curve here)
	float wx = x - (float)X;
	float wy = y - (float)Y;

	// interpolate between grid point gradients
	float a, b, c, d;

	a  = dot_grid_gradient(X + 0, Y + 0, x, y);
	b  = dot_grid_gradient(X + 1, Y + 0, x, y);
	c = smoothstep(a, b, wx);

	a  = dot_grid_gradient(X + 0, Y + 1, x, y);
	b  = dot_grid_gradient(X + 1, Y + 1, x, y);
	d = smoothstep(a, b, wx);

	return (smoothstep(c, d, wy) + 1.f) / 2.f;
}
float perlin(float x)
{
	return lerp(random_normalized_float(floor(x)), random_normalized_float(ceil(x)), fract(x));
}

// signed perlin
float perlins(float x, float y)
{
	return (perlin(x, y) * 2) - 1;
}

float worley(vec2 uv, float columns, float rows)
{
	vec2 index_uv = floor(vec2(uv.x * columns, uv.y * rows));
	vec2 fract_uv = fract(vec2(uv.x * columns, uv.y * rows));

	float minimum_dist = 1.0;

	for (int y = -1; y <= 1; y++) {
	for (int x = -1; x <= 1; x++)
	{
		vec2 neighbor = vec2(float(x), float(y));
		vec2 point = vec2(glm::fract(sin(dot(index_uv + neighbor, vec2(12.9898, 78.233))) * 43758.5453123)); // random

		vec2 diff = neighbor + point - fract_uv;
		float dist = length(diff);
		minimum_dist = glm::min(minimum_dist, dist);
	} }

	return minimum_dist;
}
vec2 voronoi(vec2 uv, float columns, float rows)
{
	vec2 index_uv = floor(vec2(uv.x * columns, uv.y * rows));
	vec2 fract_uv = fract(vec2(uv.x * columns, uv.y * rows));

	float minimum_dist = 1.0;
	vec2 minimum_point = {};

	for (int y = -1; y <= 1; y++) {
	for (int x = -1; x <= 1; x++)
	{
		vec2 neighbor = vec2(float(x), float(y));
		vec2 point = vec2(glm::fract(sin(dot(index_uv + neighbor, vec2(12.9898, 78.233))) * 43758.5453123)); // random

		vec2 diff = neighbor + point - fract_uv;
		float dist = length(diff);

		if (dist < minimum_dist)
		{
			minimum_dist = dist;
			minimum_point = point;
		}
	} }

	return minimum_point;
}

// misc utilities

vec3 shake(float trauma) // perlin shake
{
	uint offset = random_uint() % 64;
	float o1 = ((perlin((trauma + offset + 0) * 1000) * 2) - 1) * trauma;
	float o2 = ((perlin((trauma + offset + 1) * 2000) * 2) - 1) * trauma;
	float o3 = ((perlin((trauma + offset + 2) * 3000) * 2) - 1) * trauma;
	return vec3(o1, o2, o3);
}
mat3 point_at(vec3 dir, vec3 up)
{
	// assumed to be normalized
	vec3 f = dir; //front
	vec3 r = cross(up, f); //right
	vec3 u = up;

	mat3 result(1);
	result[0][0] = r.x;
	result[1][0] = r.y;
	result[2][0] = r.z;
	result[0][1] = u.x;
	result[1][1] = u.y;
	result[2][1] = u.z;
	result[0][2] = f.x;
	result[1][2] = f.y;
	result[2][2] = f.z;

	return inverse(result);
}

// fast fourier transforms

#include <complex>
typedef std::complex<double> Complex;

Complex gaussian_random_complex()
{
	float x1, x2, w;
	do {
		x1 = random_normalized_float_signed();
		x2 = random_normalized_float_signed();
		w = x1 * x1 + x2 * x2;
	} while (w > 1.f);
	w = sqrt((-2.f * log(w)) / w);
	return Complex(x1 * w, x2 * w);
}

// fast fourier : result stored in input array
void fft(Complex* input, uint N)
{
	uint target_index = 0;
	uint bit_mask;

	for (uint i = 0; i < N; ++i)
	{
		if (target_index > i) // compute twiddle factors?
		{
			//swap(input[target_index], input[i]);
			Complex temp = input[target_index];
			input[target_index] = input[i];
			input[i] = temp;
		}

		bit_mask = N;
		while (target_index & (bit_mask >>= 1)) // while bit is 1 : bit_mask = bit_mask >> 1
		{
			// Drop bit : ~ = bitwise NOT
			target_index &= ~bit_mask; // target_index = target_index & (~bit_mask)
		}

		target_index |= bit_mask; // target_index = target_index | bit_mask
	}

	for (uint i = 1; i < N; i <<= 1) // cycle for all bit positions of initial signal
	{
		uint  next  = i << 1;
		float angle = -PI / i; // inverse fft uses PI not -PI
		float sine  = sin(.5 * angle); // supplementary sin

		// multiplier for trigonometric recurrence
		Complex mult = Complex(-2.0 * sine * sine, sin(angle));
		Complex factor = 1.0; // start transform factor

		for (uint j = 0; j < i; ++j) { // iterations through groups with different transform factors
		for (uint k = j; k < N; k += next) // iterations through pairs within group
		{
			uint match = k + i;
			Complex product = input[match] * factor;
			input[match] = input[k] - product;
			input[k]    += product;
		} factor = mult * factor + factor; }
	}
}
void ifft(Complex* input, uint N, bool scale = true)
{
	uint target_index = 0;
	uint bit_mask;

	for (uint i = 0; i < N; ++i)
	{
		if (target_index > i) // compute twiddle factors?
		{
			//swap(input[target_index], input[i]);
			Complex temp = input[target_index];
			input[target_index] = input[i];
			input[i] = temp;
		}

		bit_mask = N;
		while (target_index & (bit_mask >>= 1)) // while bit is 1 : bit_mask = bit_mask >> 1
		{
			// Drop bit : ~ = bitwise NOT
			target_index &= ~bit_mask; // target_index = target_index & (~bit_mask)
		}

		target_index |= bit_mask; // target_index = target_index | bit_mask
	}

	for (uint i = 1; i < N; i <<= 1) // cycle for all bit positions of initial signal
	{
		uint  next  = i << 1;
		float angle = PI / i; // inverse fft uses PI not -PI
		float sine  = sin(.5 * angle); // supplementary sin

		// multiplier for trigonometric recurrence
		Complex mult = Complex(-2.0 * sine * sine, sin(angle));
		Complex factor = 1.0; // start transform factor

		for (uint j = 0; j < i; ++j) { // iterations through groups with different transform factors
		for (uint k = j; k < N; k += next) // iterations through pairs within group
		{
			uint match = k + i;
			Complex product = input[match] * factor;
			input[match] = input[k] - product;
			input[k]    += product;
		} factor = mult * factor + factor; }
	}

	if (scale) for (int i = 0; i < N; ++i) { input[i] *= 1.f / N; } // normalize output array
}
void fft2D(Complex* input, uint N)
{
	Complex* subarray = Alloc(Complex, N); // num_rows = num_columns = N

	for (uint n = 0; n < N; n++) // fft the columns
	{
		for (int i = 0; i < N; ++i) { subarray[i] = input[(i * N) + n]; }
		fft(subarray, N);
		for (int i = 0; i < N; ++i) { input[(i * N) + n] = subarray[i]; }
	}

	for (int n = 0; n < N; ++n) // fft the rows
	{
		for (int i = 0; i < N; ++i) { subarray[i] = input[(n * N) + i]; }
		fft(subarray, N);
		for (int i = 0; i < N; ++i) { input[(n * N) + i] = subarray[i]; }
	}

	free(subarray);
}
void ifft2D(Complex* input, uint N, bool scale = false)
{
	Complex* subarray = Alloc(Complex, N); // num_rows = num_columns = N

	for (uint n = 0; n < N; n++) // ifft the columns
	{
		for (int i = 0; i < N; ++i) { subarray[i] = input[(i * N) + n]; }
		ifft(subarray, N, scale);
		for (int i = 0; i < N; ++i) { input[(i * N) + n] = subarray[i]; }
	}

	for (int n = 0; n < N; ++n) // ifft the rows
	{
		for (int i = 0; i < N; ++i) { subarray[i] = input[(n * N) + i]; }
		ifft(subarray, N, scale);
		for (int i = 0; i < N; ++i) { input[(n * N) + i] = subarray[i]; }
	}

	free(subarray);
}