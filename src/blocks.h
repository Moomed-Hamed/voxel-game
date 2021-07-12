#include "camera.h"

#define BLOCK_AIR	0

#define BLOCK_STONE	1 // solids
#define BLOCK_DIRT	2
#define BLOCK_GRASS	3
#define BLOCK_SAND	4
#define BLOCK_WOOD	5
#define BLOCK_ORES	6

#define BLOCK_WATER	7 // fluids
#define BLOCK_LAVA	8
#define BLOCK_MILK	9
#define BLOCK_OIL	10

#define BLOCK_FURNACE 11 // machines

#define BLOCK_CROP	12 // other
#define BLOCK_FLORA	13

typedef uint32 BlockID;

// rendering

struct Solid_Drawable { vec3 position; float tex_offset; };
struct Fluid_Drawable { vec3 position; };