#include "icg_common.h"
#include "HeightmapGenerator.h"
#include "Grid.h"
#include "FrameBuffer.h"
#include "trackball.h"

#define TEXTURE_SIZE 1600

int width=1280, height=720;

HeightmapGenerator generator;
Grid grid;
FrameBuffer fb(TEXTURE_SIZE, TEXTURE_SIZE);
Trackball trackball;

mat4 trackball_matrix;
mat4 old_trackball_matrix;


static float H = 0.7f;
static float lacunarity = 1.92;
static int octaves = 7;


void generateHeightmap() {
	fb.bind();
	glViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	generator.draw(H, lacunarity, octaves);
	glViewport(0, 0, width, height);
	fb.unbind();
}

void init(){
    glClearColor(0,0.2,0.5, /*solid*/1.0 );    
    glEnable(GL_DEPTH_TEST);
	generator.init(1024);
	GLuint tex_coord = fb.init(true /* use interpolation */);
	grid.init(tex_coord);

	trackball_matrix = mat4::Identity();
	generateHeightmap();
}

void display(){ 
    opengp::update_title_fps("FrameBuffer");   
    glViewport(0,0,width,height);
    
    ///--- Setup view-projection matrix
    float ratio = width / (float) height;
    static mat4 projection = Eigen::perspective(45.0f, ratio, 0.1f, 10.0f);
    vec3 cam_pos(0.0f, 1.0f, -3.0f);
    vec3 cam_look(0.0f, .5f, 0.0f);
    vec3 cam_up(0.0f, 1.0f, 0.0f);
    mat4 view = Eigen::lookAt(cam_pos, cam_look, cam_up);
	mat4 VP = projection * view * trackball_matrix;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	grid.draw(VP);
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
		vec2 p = transform_screen_coords(x_i, y_i);
		trackball.begin_drag(p.x(), p.y());
		old_trackball_matrix = trackball_matrix;  // Store the current state of the model matrix.
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		glfwGetMousePos(&x_i, &y_i);
		vec2 p = transform_screen_coords(x_i, y_i);
	}
}

void mouse_pos(int x, int y) {
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		vec2 p = transform_screen_coords(x, y);
		trackball_matrix = trackball.drag(p(0), p(1)) * old_trackball_matrix;
	}
}

void keyboard(int key, int action) {
	if (action == GLFW_RELEASE) {
		switch (key) {
		case 'Q': H += 0.05; break;
		case 'A': H -= 0.05; break;
		case 'W': lacunarity += 0.34567; break;
		case 'S': lacunarity -= 0.34567; break;
		case 'E': octaves++; break;
		case 'D': octaves--; break;
		default: return;
		}
		generateHeightmap();
		std::cout << "H=" << H << " lacunarity=" << lacunarity << " octaves=" << octaves << std::endl;
	}
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
    return EXIT_SUCCESS;
}
