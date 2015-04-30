#include "icg_common.h"
#include "Map.h"
#include "FrameBuffer.h"
#include "Water.h"
#include "ScreenQuad.h"

int width=1280, height=720;

Map world;
Water water;
FrameBuffer fb_main(width, height);
FrameBuffer fb_mirrored(width, height);
ScreenQuad sqad;

vec3 cam_pos(2103, 1, 2125);
//vec3 cam_pos(0, 1, 0);
vec3 cam_look;
vec2 angles(0, 0);
vec2 old_angles;
vec2 old_mouse_pos;

vec3 sky_color(0.85, 0.90, 0.95);
// vec3 sky_color(1, 0, 0);

#define MOVE_INC	0.015
#define KEY_FWD		0
#define KEY_BWD		1
#define KEY_LEFT	2
#define KEY_RIGHT	3
bool keys[] = { false, false, false, false };
bool speedup = false;

void init(){
	glClearColor(sky_color(0), sky_color(1), sky_color(2), /*solid*/1.0);
    glEnable(GL_DEPTH_TEST);

	world.init(vec2(cam_pos(0), cam_pos(2)), sky_color);
	GLuint fb_main_tex = fb_main.init(true);
	GLuint fb_mirr_tex = fb_mirrored.init(true);
	water.init(fb_main_tex, fb_mirr_tex);

	sqad.init(fb_main_tex);



	glViewport(0, 0, width, height);
}

void display(){

    opengp::update_title_fps("FrameBuffer");
    glViewport(0,0,width,height);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ///--- Setup view-projection matrix
    float ratio = width / (float) height;
    static mat4 projection = Eigen::perspective(45.0f, ratio, 0.01f, 10.0f);
    vec3 cam_up(0.0f, 1.0f, 0.0f);

	mat4 model = mat4::Identity();
	// For simplicity: water is at height 0
	// And the whole map is translated down by water_level units
	// TODO: move hardcoded water level smw else
	model(1, 3) = -0.3f;
    
	// Floats get rough at high values so we keep the position near 0
	vec2 cam_pos_memo(cam_pos(0), cam_pos(2));
	cam_pos(0) = 0;
	cam_pos(2) = 0;

	// compute camera view from angles
	cam_look(0) = cam_pos(0) - sin(angles(0))*cos(angles(1));
	cam_look(2) = cam_pos(2) + cos(angles(0))*cos(angles(1));
	cam_look(1) = cam_pos(1) - sin(angles(1));

	mat4 view = Eigen::lookAt(cam_pos, cam_look, cam_up);

	// mirror camera
	cam_pos(1) = -cam_pos(1);
	cam_look(1) = -cam_look(1);
	mat4 mirrored_view = Eigen::lookAt(cam_pos, cam_look, cam_up);
	cam_pos(1) = -cam_pos(1); // reset cam pos

	// with clipping plane
	float clippingPlane[] = {0, 0.3, 0, 0.5};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	fb_mirrored.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CLIP_DISTANCE0);
		world.draw(model, mirrored_view, projection);
		glDisable(GL_CLIP_DISTANCE0);
	fb_mirrored.unbind();

	fb_main.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		world.draw(model, view, projection);
	fb_main.unbind();
	
	world.draw(model, view, projection);

	/*
	mat4 waterModel = mat4::Identity();
	waterModel(0, 3) = cam_pos(0);
	waterModel(2, 3) = cam_pos(2);
	waterModel(1, 3) = 0;
	*/
	water.draw(mat4::Identity(), view, projection);

	// restore camera
	cam_pos(0) = cam_pos_memo(0);
	cam_pos(2) = cam_pos_memo(1);

	//sqad.draw();
}

void update() {
	const int coeff = speedup ? 3 : 1;
	// update pos
	if (keys[KEY_FWD]) {
		cam_pos(2) +=  coeff * MOVE_INC*cos(angles(0))*cos(angles(1));
		cam_pos(1) -= coeff * MOVE_INC*sin(angles(1));
		cam_pos(0) -= coeff * MOVE_INC*sin(angles(0))*cos(angles(1));
	}
	else if (keys[KEY_BWD]) {
		cam_pos(2) -= coeff * MOVE_INC*cos(angles(0))*cos(angles(1));
		cam_pos(1) += coeff * MOVE_INC*sin(angles(1));
		cam_pos(0) += coeff * MOVE_INC*sin(angles(0))*cos(angles(1));
	}
	if (keys[KEY_LEFT]) {
		cam_pos(2) += coeff * MOVE_INC*sin(angles(0));
		cam_pos(0) += coeff * MOVE_INC*cos(angles(0));
	}
	else if (keys[KEY_RIGHT]) {
		cam_pos(2) -= coeff * MOVE_INC*sin(angles(0));
		cam_pos(0) -= coeff * MOVE_INC*cos(angles(0));
	}
	cam_look = cam_pos + vec3(0, 0, 1);

	world.update(vec2(cam_pos(0), cam_pos(2)));

	display();
}


// Transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 transform_screen_coords(int x, int y) {
	return vec2(2.0f * (float)x / width - 1.0f,
		2.0f * (float)y / height - 1);
}

void mouse_button(int button, int action) {
	int x_i, y_i;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetMousePos(&x_i, &y_i);
		old_mouse_pos = transform_screen_coords(x_i, y_i);
		old_angles = angles;
	}
}

void mouse_pos(int x, int y) {
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		vec2 p = transform_screen_coords(x, y);
		angles = old_angles + p - old_mouse_pos;
	}
}

void keyboard(int key, int action) {
	if (action == GLFW_RELEASE) {

		switch (key) {
		case 'R': H += 0.05; break;
		case 'F': H -= 0.05; break;
		case 'T': lacunarity += 0.34567; break;
		case 'G': lacunarity -= 0.34567; break;
		case 'Y': octaves++; break;
		case 'H': octaves--; break;
		case 'W':
			keys[KEY_FWD] = false;
			return;
		case ' ':
			speedup = false;
			return;
		case 'A':
			keys[KEY_LEFT] = false;
			return;
		case 'S':
			keys[KEY_BWD] = false;
			return;
		case 'D':
			keys[KEY_RIGHT] = false;
			return;
		default: return;
		}
		world.regenerateHeightmap();
		std::cout << "H=" << H << " lacunarity=" << lacunarity << " octaves=" << octaves << std::endl;
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
		case 'W':
			keys[KEY_FWD] = true;
			return;
		case 'A':
			keys[KEY_LEFT] = true;
			return;
		case 'S':
			keys[KEY_BWD] = true;
			return;
		case 'D':
			keys[KEY_RIGHT] = true;
			return;
		case ' ':
			speedup = true;
			return;

		default: break;
		}
	}
}

void cleanup(){
	world.cleanup();
	fb_main.cleanup();
	fb_mirrored.cleanup();
}

int main(int, char**){
    glfwInitWindowSize(width, height);
    glfwCreateWindow();
    glfwDisplayFunc(update);
	glfwSetMouseButtonCallback(mouse_button);
	glfwSetMousePosCallback(mouse_pos);
	glfwSetKeyCallback(keyboard);
    init();
    // glfwSwapInterval(0); ///< disable VSYNC (allows framerate>30)
    glfwMainLoop();
	cleanup();
    return EXIT_SUCCESS;
}
