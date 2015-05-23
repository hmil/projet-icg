#include "icg_common.h"
#include "Map.h"
#include "FrameBuffer.h"
#include "Water.h"
#include "ScreenQuad.h"
#include "Cube.h"
#include "bezier.h"
#include "point.h"


#define EDIT_CURVES

int width=1280, height=720;

Map world;
Water water;
FrameBuffer fb_mirrored(width, height);
FrameBuffer fb_quad(width, height);
ScreenQuad sqad;
Cube skybox;

vec3 cam_pos(72, 1, 52);
//vec3 cam_pos(0, 1, 0);
vec3 cam_look;
vec2 angles(0, 0);
vec2 old_angles;
vec2 old_mouse_pos;

vec3 sky_color(0.60, 0.70, 0.85);
//vec3 sky_color(1, 0, 0);

#define MOVE_INC	0.015f
#define KEY_FWD		0
#define KEY_BWD		1
#define KEY_LEFT	2
#define KEY_RIGHT	3
bool keys[] = { false, false, false, false };
bool speedup = false;

mat4 model, view, projection;

bool is_dragging;

#ifdef EDIT_CURVES

#define POS_CURVE_N  2
#define LOOK_CURVE_N 2

BezierCurve cam_pos_curve[POS_CURVE_N];
BezierCurve cam_look_curve[LOOK_CURVE_N];
std::vector<ControlPoint> cam_pos_points;
std::vector<ControlPoint> cam_look_points;
int selected_point(-1);

GLuint _pid_bezier;
GLuint _pid_point;
GLuint _pid_point_selection;

void initCurves() {
	/// Compile the shaders here to avoid the duplication
	_pid_bezier = opengp::load_shaders("shaders/bezier_vshader.glsl", "shaders/bezier_fshader.glsl");
	if (!_pid_bezier) exit(EXIT_FAILURE);

	_pid_point = opengp::load_shaders("shaders/point_vshader.glsl", "shaders/point_fshader.glsl");
	if (!_pid_point) exit(EXIT_FAILURE);

	_pid_point_selection = opengp::load_shaders("shaders/point_selection_vshader.glsl", "shaders/point_selection_fshader.glsl");
	if (!_pid_point_selection) exit(EXIT_FAILURE);

	///--- init cam_pos_curve
	cam_pos_points.push_back(ControlPoint(72.0f, 1.0f, 52.0f, 3));
	cam_pos_points.push_back(ControlPoint(72.0f, 1.5f, 52.0f, 2));
	cam_pos_points.push_back(ControlPoint(71.0f, 1.5f, 52.0f, 1));
	cam_pos_points.push_back(ControlPoint(71.0f, 1.0f, 52.0f, 0));
	cam_pos_points.push_back(ControlPoint(71.0f, 0.5f, 52.0f, 4));
	cam_pos_points.push_back(ControlPoint(71.0f, 0.5f, 53.0f, 5));
	cam_pos_points.push_back(ControlPoint(71.0f, 1.0f, 53.0f, 6));

	for (unsigned int i = 0; i < cam_pos_points.size(); i++) {
		cam_pos_points[i].id() = i;
		cam_pos_points[i].init(_pid_point, _pid_point_selection);
	}

	for (unsigned int i = 0; i < POS_CURVE_N; i++) {
		cam_pos_curve[i].init(_pid_bezier);
		cam_pos_curve[i].set_points(cam_pos_points[i * 3].position(), cam_pos_points[i * 3 + 1].position(), cam_pos_points[i * 3 + 2].position(), cam_pos_points[i * 3 + 3].position());
	}

	cam_look_points.push_back(ControlPoint(70.0f, 0.5f, 53.0f, 100));
	cam_look_points.push_back(ControlPoint(70.0f, 0.5f, 53.0f, 101));
	cam_look_points.push_back(ControlPoint(70.01f, 0.5f, 53.0f, 102));
	cam_look_points.push_back(ControlPoint(70.0f, 0.5f, 52.0f, 103));
	cam_look_points.push_back(ControlPoint(70.0f, 0.5f, 51.0f, 104));
	cam_look_points.push_back(ControlPoint(70.0f, 1.0f, 51.0f, 105));
	cam_look_points.push_back(ControlPoint(70.0f, 1.5f, 51.0f, 106));

	for (unsigned int i = 0; i < cam_look_points.size(); i++) {
		cam_look_points[i].id() = i + 100;
		cam_look_points[i].init(_pid_point, _pid_point_selection);
	}

	for (unsigned int i = 0; i < LOOK_CURVE_N; i++) {
		cam_look_curve[i].init(_pid_bezier);
		cam_look_curve[i].set_points(cam_look_points[i * 3].position(), cam_look_points[i * 3 + 1].position(), cam_look_points[i * 3 + 2].position(), cam_look_points[i * 3 + 3].position());
	}
}
#endif

void init(){
	glClearColor(sky_color(0), sky_color(1), sky_color(2), /*solid*/1.0);
    glEnable(GL_DEPTH_TEST);


#ifdef EDIT_CURVES
	initCurves();
#endif

	world.init(vec2(cam_pos(0), cam_pos(2)), sky_color);
	fb_mirrored.init(true);
	fb_quad.init(true);
	skybox.init();

	water.init(fb_mirrored.getColorAttachment());

	sqad.init(fb_quad.getColorAttachment(), fb_quad.getDepthAttachment());



	glViewport(0, 0, width, height);
}

void display(){

    opengp::update_title_fps("FrameBuffer");
    glViewport(0,0,width,height);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

    ///--- Setup view-projection matrix
    float ratio = width / (float) height;
    projection = Eigen::perspective(45.0f, ratio, 0.01f, 10.0f);
    vec3 cam_up(0.0f, 1.0f, 0.0f);

	model = mat4::Identity();
	// For simplicity: water is at height 0
	// And the whole map is translated down by water_level units
	// TODO: move hardcoded water level smw else
	model(1, 3) = -0.3f;
	mat4 skybox_model = model;

	mat4 world_model(model);
    
	// Floats get rough at high values so we keep the position near 0
	vec2 cam_pos_memo(cam_pos(0), cam_pos(2));
	cam_pos(0) = 0;
	cam_pos(2) = 0;

	// compute camera view from angles
	cam_look(0) = cam_pos(0) - sin(angles(0))*cos(angles(1));
	cam_look(2) = cam_pos(2) + cos(angles(0))*cos(angles(1));
	cam_look(1) = cam_pos(1) - sin(angles(1));

	view = Eigen::lookAt(cam_pos, cam_look, cam_up);
	
	vec3 cam_look2 = cam_look;
	cam_look2(1) = cam_look(1) - cam_pos(1);
	mat4 skybox_view = Eigen::lookAt(vec3(0, 0, 0), cam_look2, cam_up);

	// mirror camera
	cam_pos(1) = -cam_pos(1);
	cam_look(1) = -cam_look(1);
	mat4 mirrored_view = Eigen::lookAt(cam_pos, cam_look, cam_up);
	cam_pos(1) = -cam_pos(1); // reset cam pos
	
	fb_quad.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (cam_pos(1) > 0)
			glEnable(GL_CLIP_DISTANCE0);

		skybox.draw(skybox_model, mirrored_view, projection);
		world.draw(model, mirrored_view, projection, cam_pos(1));
		glDisable(GL_CLIP_DISTANCE0);
	fb_quad.unbind();
	fb_mirrored.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		vec3 sfx_mirror_cam_pos(cam_pos_memo(0), -cam_pos(1), cam_pos_memo(1));
		sqad.draw(mirrored_view, projection, sfx_mirror_cam_pos);
	fb_mirrored.unbind();
	
	fb_quad.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.draw(skybox_model, skybox_view, projection);

		// restore camera
		cam_pos(0) = cam_pos_memo(0);
		cam_pos(2) = cam_pos_memo(1);

#ifdef EDIT_CURVES
		model(0, 3) = -cam_pos(0);
		model(2, 3) = -cam_pos(2);
		for (unsigned int i = 0; i < cam_pos_points.size(); i++) {
			cam_pos_points[i].draw(model, view, projection);
		}

		for (unsigned int i = 0; i < cam_look_points.size(); i++) {
			cam_look_points[i].draw(model, view, projection);
		}

		for (unsigned int i = 0; i < POS_CURVE_N; i++) {
			cam_pos_curve[i].draw(model, view, projection);
		}
		for (unsigned int i = 0; i < LOOK_CURVE_N; i++) {
			cam_look_curve[i].draw(model, view, projection);
		}
#endif
		world.draw(world_model, view, projection, cam_pos(1));
		water.draw(mat4::Identity(), view, projection);
	fb_quad.unbind();

	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sqad.draw(view, projection, cam_pos);


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
	water.update(vec2(cam_pos(0), cam_pos(2)));

	display();
}


// Transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 transform_screen_coords(int x, int y) {
	return vec2(2.0f * (float)x / width - 1.0f,
		2.0f * (float)y / height - 1);
}


void render_selection() {
	glViewport(0, 0, width, height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw control points for selection
	for (unsigned int i = 0; i < cam_pos_points.size(); i++) {
		cam_pos_points[i].draw_selection(model, view, projection);
	}

	for (unsigned int i = 0; i < cam_look_points.size(); i++) {
		cam_look_points[i].draw_selection(model, view, projection);
	}
}

/*
	Takes a world-coordinate point, a 2D screen-coordinate position
	and moves the world point to the position pointed at on the screen
	without changing it's depth
*/
bool unproject(int mouse_x, int mouse_y, vec3 &p) {

	// Screen-space unprojection
	// 1) Compute the inverse of VP
	mat4 pv = (projection * view * model).inverse();

	vec4 point = projection * view * model * vec4(p(0), p(1), p(2), 1.0f);

	// 2) Find new screen-space coordinates from mouse position
	float mx = (float)mouse_x / width - 0.5;
	float my = (-1.0f*mouse_y) / height + 0.5;

	vec4 homocoord = pv * vec4(mx * 2, my * 2, point(2) / point(3), 1);

	// 3) Obtain object coordinates p
	p = vec3(homocoord(0), homocoord(1), homocoord(2)) / homocoord(3);

	return true;
}

void mouse_button(int button, int action) {
	int x_i, y_i;

	is_dragging = false;

#ifdef EDIT_CURVES
	static int x_pressed = 0, y_pressed = 0;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		int x = 0, y = 0;
		glfwGetMousePos(&x, &y);
		x_pressed = x; y_pressed = y;

		render_selection();

		glFlush();
		glFinish();

		unsigned char res[4];
		glReadPixels(x, height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
		selected_point = res[0];

		if (selected_point >= 0 && selected_point < cam_pos_points.size()) {
			cam_pos_points[selected_point].selected() = true;
		}
		else if (selected_point >= 100 && selected_point < cam_look_points.size() + 100) {
			cam_look_points[selected_point - 100].selected() = true;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		if (selected_point >= 0 && selected_point < cam_pos_points.size()) {
			cam_pos_points[selected_point].selected() = false;
		}
		if (selected_point >= 100 && selected_point < cam_look_points.size() + 100) {
			cam_look_points[selected_point - 100].selected() = false;
		}

		int x = 0, y = 0;
		glfwGetMousePos(&x, &y);
		if (x == x_pressed && y == y_pressed) {
			return;
		}

		if (selected_point >= 0 && selected_point < cam_pos_points.size()) {
			unproject(x, y, cam_pos_points[selected_point].position());

			for (unsigned int i = 0; i < POS_CURVE_N; i++) {
				cam_pos_curve[i].set_points(cam_pos_points[i * 3].position(), cam_pos_points[i * 3 + 1].position(), cam_pos_points[i * 3 + 2].position(), cam_pos_points[i * 3 + 3].position());
			}
		}

		if (selected_point >= 100 && selected_point < cam_look_points.size() + 100) {
			unproject(x, y, cam_look_points[selected_point - 100].position());

			for (unsigned int i = 0; i < LOOK_CURVE_N; i++) {
				cam_look_curve[i].set_points(cam_look_points[i * 3].position(), cam_look_points[i * 3 + 1].position(), cam_look_points[i * 3 + 2].position(), cam_look_points[i * 3 + 3].position());
			}
		}
	}
#endif

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetMousePos(&x_i, &y_i);
		old_mouse_pos = transform_screen_coords(x_i, y_i);
		old_angles = angles;
		is_dragging = true;
	}
}

void mouse_pos(int x, int y) {
	if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && is_dragging) {
		vec2 p = transform_screen_coords(x, y);
		angles = old_angles + p - old_mouse_pos;
	}
}

void keyboard(int key, int action) {
	if (action == GLFW_RELEASE) {

		switch (key) {
		case 'R': H += 0.05f; break;
		case 'F': H -= 0.05f; break;
		case 'T': lacunarity += 0.34567f; break;
		case 'G': lacunarity -= 0.34567f; break;
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
#ifdef EDIT_CURVES
		case 'C': 
			// output curves ctrl points
			std::cout << "-- curves dump --" << std::endl << "position:" << std::endl;
			for (unsigned int i = 0; i < POS_CURVE_N; i++) {
				cam_pos_curve[i].print_points();
			}
			std::cout << "look:" << std::endl;
			for (unsigned int i = 0; i < LOOK_CURVE_N; i++) {
				cam_look_curve[i].print_points();
			}


#endif
		case ' ':
			speedup = true;
			return;

		default: break;
		}
	}
}

void cleanup(){
	world.cleanup();
	fb_mirrored.cleanup();

#ifdef EDIT_CURVES
	glDeleteProgram(_pid_bezier);
	glDeleteProgram(_pid_point);
	glDeleteProgram(_pid_point_selection);
#endif
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
