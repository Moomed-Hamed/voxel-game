#include "intermediary.h"

#define WINDOW_ERROR(str) out("WINDOW ERROR: " << str)

struct Window
{
	GLFWwindow* instance;
	uint screen_width, screen_height;
};

void init_window(Window* window, uint screen_width, uint screen_height, const char* window_name = "window")
{
	window->screen_width  = screen_width;
	window->screen_height = screen_height;

	if (!glfwInit()) { WINDOW_ERROR("GLFW ERROR : could not init glfw"); stop; return; }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window->instance = glfwCreateWindow(screen_width, screen_height, window_name, NULL, NULL);
	if (!window->instance) { glfwTerminate(); WINDOW_ERROR("no window instance"); stop; return; }

	glfwMakeContextCurrent(window->instance);
	glfwSwapInterval(1); // disable v-sync

	//Capture the cursor
	glfwSetInputMode(window->instance, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
void update_window(Window window)
{
	glfwPollEvents();
	glfwSwapBuffers(window.instance);
}
void shutdown_window()
{
	glfwTerminate();
}

struct Button
{
	char is_pressed;
	char was_pressed;
	u16  id;
};

struct Mouse
{
	double raw_x, raw_y;   // pixel coordinates
	double norm_x, norm_y; // normalized screen coordinates
	double dx, dy;  // pos change since last frame in pixels

	Button right_button, left_button;
};

void update_mouse(Mouse* mouse, Window window)
{
	static Mouse prev_mouse = {};

	glfwGetCursorPos(window.instance, &mouse->raw_x, &mouse->raw_y);

	mouse->dx = mouse->raw_x - prev_mouse.raw_x; // what do these mean again?
	mouse->dy = prev_mouse.raw_y - mouse->raw_y;

	prev_mouse.raw_x = mouse->raw_x;
	prev_mouse.raw_y = mouse->raw_y;

	mouse->norm_x = ((uint)mouse->raw_x % window.screen_width) / (double)window.screen_width;
	mouse->norm_y = ((uint)mouse->raw_y % window.screen_height) / (double)window.screen_height;

	mouse->norm_y = 1 - mouse->norm_y;
	mouse->norm_x = (mouse->norm_x * 2) - 1;
	mouse->norm_y = (mouse->norm_y * 2) - 1;

	bool key_down = (glfwGetMouseButton(window.instance, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);

	if (key_down)
	{
		mouse->right_button.was_pressed = (mouse->right_button.is_pressed == true);
		mouse->right_button.is_pressed = true;
	}
	else
	{
		mouse->right_button.was_pressed = (mouse->right_button.is_pressed == true);
		mouse->right_button.is_pressed = false;
	}

	key_down = (glfwGetMouseButton(window.instance, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	if (key_down)
	{
		mouse->left_button.was_pressed = (mouse->left_button.is_pressed == true);
		mouse->left_button.is_pressed = true;
	}
	else
	{
		mouse->left_button.was_pressed = (mouse->left_button.is_pressed == true);
		mouse->left_button.is_pressed = false;
	}
}

#define NUM_KEYBOARD_BUTTONS 20
struct Keyboard
{
	union
	{
		Button buttons[NUM_KEYBOARD_BUTTONS];

		struct
		{
			Button ESC, SPACE;
			Button W, A, S, D;
			Button T, G, F, H;
			Button E, R, Y, X;
			Button SHIFT, CTRL;
			Button UP, DOWN, LEFT, RIGHT;
		};
	};
};

void init_keyboard(Keyboard* keyboard)
{
	keyboard->ESC = { false, false, GLFW_KEY_ESCAPE };
	keyboard->SPACE = { false, false, GLFW_KEY_SPACE };

	keyboard->W = { false, false, GLFW_KEY_W };
	keyboard->A = { false, false, GLFW_KEY_S };
	keyboard->S = { false, false, GLFW_KEY_A };
	keyboard->D = { false, false, GLFW_KEY_D };

	keyboard->T = { false, false, GLFW_KEY_T };
	keyboard->G = { false, false, GLFW_KEY_G };
	keyboard->F = { false, false, GLFW_KEY_F };
	keyboard->H = { false, false, GLFW_KEY_H };

	keyboard->E = { false, false, GLFW_KEY_E };
	keyboard->R = { false, false, GLFW_KEY_R };
	keyboard->Y = { false, false, GLFW_KEY_Y };
	keyboard->X = { false, false, GLFW_KEY_X };

	keyboard->UP    = { false, false, GLFW_KEY_UP };
	keyboard->DOWN  = { false, false, GLFW_KEY_DOWN };
	keyboard->LEFT  = { false, false, GLFW_KEY_LEFT };
	keyboard->RIGHT = { false, false, GLFW_KEY_RIGHT };

	keyboard->SHIFT = { false, false, GLFW_KEY_LEFT_SHIFT };
	keyboard->CTRL  = { false, false, GLFW_KEY_LEFT_CONTROL };
}
void update_keyboard(Keyboard* keyboard, Window window)
{
	for (uint i = 0; i < NUM_KEYBOARD_BUTTONS; ++i)
	{
		bool key_down = (glfwGetKey(window.instance, keyboard->buttons[i].id) == GLFW_PRESS);

		if (key_down)
		{
			keyboard->buttons[i].was_pressed = (keyboard->buttons[i].is_pressed == true);
			keyboard->buttons[i].is_pressed = true;
		}
		else
		{
			keyboard->buttons[i].was_pressed = (keyboard->buttons[i].is_pressed == true);
			keyboard->buttons[i].is_pressed = false;
		}
	}
}