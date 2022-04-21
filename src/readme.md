### Blocks & Items (items.h)

Every block has an ID number. The numbers are ordered to make handling them in code easier.
blocks with the same texture on all sides come first, then blocks with
potentially different textures on every side, then fluids come last(air = 0).

### Chunks and Generation (chunk.h & world.h)

- Active chunk : loaded in memory, fully simulated; always 9 of these
- Border chunk : loaded in memory, partly simulated; always 7 of these
- Primed chunk : loaded in memory, not simulated; this is what render distance controls

Every frame (eventually every tick?) a list is made of the coordinates of active, border,
and primed chunks based on which chunk the player is currently in. This list is compared to
the list of currently loaded chunks, any chunks that need to be loaded are loaded, and any chunks
that need to be unloaded are unloaded.

### GUI (gui.h)

- Quad Drawable : shape with solid color
- Icon Drawable : quad with an image on it (can be transparent)

The entire gui is redrawn every frame in the update(GUI) function. The order that gui elements are added matters,
so there is a set order that elements are drawn in. The hotbar icons are drawn first, then the inventory screen
icons are drawn(if the player has their inventory open). Then the hotbar & inventory quads are drawn(behind 
the icons).

The way the renderer figures out which icons to draw and where depends on the icons being added to the render buffer
in a specific order: [hotbar]-[backpack]-[interactable]. Hotbar items are always first in the buffer,
then(if open) the inventory items, then (if present) the items in whichever interactable the player is
using(chest, furnace, etc.)

### Player (player.h)

- equipped_item : number corresponding to which hotbar item is equipped
- selected_item : item that the player is holding with the mouse(i think?)
- opened_items : pointer to the inventory of whatever the player is interacting with(chest, furnace, etc.)

The entire gui is redrawn every frame in the update(GUI) function. The order that gui elements are added matters,
so there is a set order that elements are drawn in. The hotbar icons are drawn first, then the inventory screen
icons are drawn(if the player has their inventory open). Then the hotbar & inventory quads are drawn(behind 
the icons).

### World (world.h)

- world_item : item in the world that does not belong to an inventory(eg. when a block is broken)

## Engine

### Window (window.h)

window.h is where the project code starts, it contains the boilerplate include and the code for launching
a window, OpenGL(graphics), and OpenAL(audio). It also contains the code for keyboard and mouse processing.

### Particles



### Rendering (renderer.h)

renderer.h contains code for loading meshes, shaders, and animation data(skeletons & poses)

#### Shaders

- assets/shaders/plain/ : shaders for drawing static meshes (position + normal) (no animated meshes rn)
- assets/shaders/transform/ : plain shader + additional position & rotation parameters
- lighting.frag & lighting.vert : lighting shaders (see PBR(#PBR-:-Physically-Based-Rendering) section)

To render a mesh you need a shader, assets/shaders/

#### PBR : Physically Based Rendering