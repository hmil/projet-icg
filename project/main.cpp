#include "icg_common.h"
#include "HeightmapGenerator.h"
#include "Grid.h"
#include "FrameBuffer.h"
#include "Tile.h"


#define TEXTURE_SIZE 2048

int width=1280, height=720;

HeightmapGenerator generator;
Tile *tile;
Tile *tile2;

vec3 cam_pos(0, 0.5, -2);
vec2 angles(0, 0);
vec2 old_angles;
vec2 old_mouse_pos;

#define MOVE_INC	0.01
#define KEY_FWD		0
#define KEY_BWD		1
#define KEY_LEFT	2
#define KEY_RIGHT	3
bool keys[] = { false, false, false, false };


void init(){
    glClearColor(0,0,0, /*solid*/1.0 );    
    glEnable(GL_DEPTH_TEST);
	
	generator.init(1024);
	

	tile = new Tile(&generator, 0, 0);
	tile2 = new Tile(&generator, 1, 0);

	tile->generateHeightmap();
	tile2->generateHeightmap();
	glViewport(0, 0, width, height);
}

void display(){ 

	// update pos
	if (keys[KEY_FWD]) {
		cam_pos(2) += MOVE_INC*cos(angles(0))*cos(angles(1));
		cam_pos(1) -= MOVE_INC*sin(angles(1));
		cam_pos(0) -= MOVE_INC*sin(angles(0))*cos(angles(1));
	}
	else if (keys[KEY_BWD]) {
		cam_pos(2) -= MOVE_INC*cos(angles(0))*cos(angles(1));
		cam_pos(1) += MOVE_INC*sin(angles(1));
		cam_pos(0) += MOVE_INC*sin(angles(0))*cos(angles(1));
	}
	if (keys[KEY_LEFT]) {
		cam_pos(2) += MOVE_INC*sin(angles(0))*cos(angles(1));
		cam_pos(0) += MOVE_INC*cos(angles(0))*cos(angles(1));
	}
	else if (keys[KEY_RIGHT]) {
		cam_pos(2) -= MOVE_INC*sin(angles(0))*cos(angles(1));
		cam_pos(0) -= MOVE_INC*cos(angles(0))*cos(angles(1));
	}
	vec3 cam_look = cam_pos + vec3(0, 0, 1);


    opengp::update_title_fps("FrameBuffer");   
    glViewport(0,0,width,height);
    
    ///--- Setup view-projection matrix
    float ratio = width / (float) height;
    static mat4 projection = Eigen::perspective(45.0f, ratio, 0.1f, 10.0f);
    vec3 cam_up(0.0f, 1.0f, 0.0f);

	mat4 rotation = mat4::Identity();
	rotation.block(0, 0, 3, 3) = Eigen::AngleAxisf(angles(1), vec3::UnitX()).matrix() * Eigen::AngleAxisf(angles(0), vec3::UnitY()).matrix();

    mat4 view = Eigen::lookAt(cam_pos, cam_look, cam_up);

	mat4 model = mat4::Identity();
	model(1, 3) = -1;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	tile->draw(model, rotation * view, projection, TEXTURE_SIZE);

	mat4 tr = mat4::Identity();
	tr(0, 3) = 4;
	tile2->draw(model * tr, rotation * view, projection, TEXTURE_SIZE);
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
			break;
		case 'A':
			keys[KEY_LEFT] = false;
			break;
		case 'S':
			keys[KEY_BWD] = false;
			break;
		case 'D':
			keys[KEY_RIGHT] = false;
			break;
		default: return;
		}
		tile->generateHeightmap();
		std::cout << "H=" << H << " lacunarity=" << lacunarity << " octaves=" << octaves << std::endl;
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
		case 'W': 
			keys[KEY_FWD] = true;
			break;
		case 'A': 
			keys[KEY_LEFT] = true;
			break;
		case 'S': 
			keys[KEY_BWD] = true;
			break;
		case 'D': 
			keys[KEY_RIGHT] = true;
			break;
		default: break;
		}
	}
}

void cleanup(){
	delete(tile);
	delete(tile2);
}

int main(int, char**){
    glfwInitWindowSize(width, height);
    glfwCreateWindow();
    glfwDisplayFunc(display);
	glfwSetMouseButtonCallback(mouse_button);
	glfwSetMousePosCallback(mouse_pos);
	glfwSetKeyCallback(keyboard);
    init();
    // glfwSwapInterval(0); ///< disable VSYNC (allows framerate>30)
    glfwMainLoop();
	cleanup();
    return EXIT_SUCCESS;
}
