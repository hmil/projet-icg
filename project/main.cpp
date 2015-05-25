#include "icg_common.h"
#include "Map.h"
#include "FrameBuffer.h"
#include "Water.h"
#include "ScreenQuad.h"
#include "Cube.h"
#include "bezier.h"
#include "point.h"


#define EDIT_CURVES
// #define RECORDING

#define BEZIER_SPEED 0.01
#define SEA_LEVEL 0.3f

enum NAVIGATION_MODE {
	NAVIGATION,
	BEZIER
} navmode;

int width=1280, height=720;

Map world;
Water water;
FrameBuffer fb_mirrored(width, height);
FrameBuffer fb_quad(width, height);
FrameBuffer fb_main(width, height);
ScreenQuad sqad;
Cube skybox;

vec3 cam_pos(62, 1, 53);
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
int current_frame(0);

mat4 model, view, projection;

bool is_dragging;

#define POS_CURVE_N  17
#define TIME_PER_FRAME 0.06

BezierCurve cam_pos_curve[POS_CURVE_N];
BezierCurve cam_look_curve[POS_CURVE_N];
std::vector<ControlPoint> cam_pos_points;
std::vector<ControlPoint> cam_look_points;
int selected_point(-1);

GLuint _pid_bezier;
GLuint _pid_point;
GLuint _pid_point_selection;


#ifdef RECORDING

unsigned char *imgData;

void saveBMP(int w, int h, const unsigned char* image, ostream &stream) {

	unsigned char file[14] = {
		'B', 'M', // magic
		0, 0, 0, 0, // size in bytes
		0, 0, // app data
		0, 0, // app data
		40 + 14, 0, 0, 0 // start of data offset
	};
	unsigned char info[40] = {
		40, 0, 0, 0, // info hd size
		0, 0, 0, 0, // width
		0, 0, 0, 0, // heigth
		1, 0, // number color planes
		24, 0, // bits per pixel
		0, 0, 0, 0, // compression is none
		0, 0, 0, 0, // image bits size
		0x13, 0x0B, 0, 0, // horz resoluition in pixel / m
		0x13, 0x0B, 0, 0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
		0, 0, 0, 0, // #colors in pallete
		0, 0, 0, 0, // #important colors
	};

	int padSize = (4 - (w * 3) % 4) % 4;
	int sizeData = w*h * 3 + h*padSize;
	int sizeAll = sizeData + sizeof(file) + sizeof(info);

	file[2] = (unsigned char)(sizeAll);
	file[3] = (unsigned char)(sizeAll >> 8);
	file[4] = (unsigned char)(sizeAll >> 16);
	file[5] = (unsigned char)(sizeAll >> 24);

	info[4] = (unsigned char)(w);
	info[5] = (unsigned char)(w >> 8);
	info[6] = (unsigned char)(w >> 16);
	info[7] = (unsigned char)(w >> 24);

	info[8] = (unsigned char)(h);
	info[9] = (unsigned char)(h >> 8);
	info[10] = (unsigned char)(h >> 16);
	info[11] = (unsigned char)(h >> 24);

	info[20] = (unsigned char)(sizeData);
	info[21] = (unsigned char)(sizeData >> 8);
	info[22] = (unsigned char)(sizeData >> 16);
	info[23] = (unsigned char)(sizeData >> 24);

	stream.write((char*)file, sizeof(file));
	stream.write((char*)info, sizeof(info));

	unsigned char pad[3] = { 0, 0, 0 };

	for (long y = 0; y<h; y++)
	{
		for (int x = 0; x<w; x++)
		{
			stream.write((const char*)(&image[3*(x + y * width)]), 3);
		}
		stream.write((char*)pad, padSize);
	}
}
void saveFrame() {
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, imgData);
	ofstream imgFile;
	char filename[32];
	sprintf(filename, "../output/out_%d.bmp", current_frame);
	imgFile.open(filename, std::ios_base::out | std::ios_base::binary);
	saveBMP(width, height, imgData, imgFile);
	imgFile.close();
}

#endif // RECORDING

void initCurves() {
	/// Compile the shaders here to avoid the duplication
	_pid_bezier = opengp::load_shaders("shaders/bezier_vshader.glsl", "shaders/bezier_fshader.glsl");
	if (!_pid_bezier) exit(EXIT_FAILURE);

	_pid_point = opengp::load_shaders("shaders/point_vshader.glsl", "shaders/point_fshader.glsl");
	if (!_pid_point) exit(EXIT_FAILURE);

	_pid_point_selection = opengp::load_shaders("shaders/point_selection_vshader.glsl", "shaders/point_selection_fshader.glsl");
	if (!_pid_point_selection) exit(EXIT_FAILURE);

	///--- init cam_pos_curve



position:
cam_pos_points.push_back(ControlPoint(62.9204f, 2.11262f, 53.7955f, 1));
cam_pos_points.push_back(ControlPoint(63.3887f, 2.02122f, 52.118f, 2));
cam_pos_points.push_back(ControlPoint(64.3069f, 2.1395f, 50.6564f, 3));
cam_pos_points.push_back(ControlPoint(65.2888f, 2.15024f, 50.0381f, 4));
cam_pos_points.push_back(ControlPoint(66.2219f, 2.16902f, 49.5043f, 5));
cam_pos_points.push_back(ControlPoint(67.1242f, 2.09966f, 48.873f, 6));
cam_pos_points.push_back(ControlPoint(68.0811f, 2.04761f, 48.8726f, 7));
cam_pos_points.push_back(ControlPoint(69.1115f, 2.08351f, 48.8614f, 8));
cam_pos_points.push_back(ControlPoint(70.1095f, 2.45104f, 49.2645f, 9));
cam_pos_points.push_back(ControlPoint(70.9751f, 2.32527f, 50.6283f, 10));
cam_pos_points.push_back(ControlPoint(72.2888f, 2.17102f, 51.8177f, 11));
cam_pos_points.push_back(ControlPoint(73.8861f, 0.91267f, 52.2872f, 12));
cam_pos_points.push_back(ControlPoint(75.0142f, 1.03194f, 52.6334f, 13));
cam_pos_points.push_back(ControlPoint(75.5908f, 1.0346f, 53.1321f, 14));
cam_pos_points.push_back(ControlPoint(76.4547f, 0.947925f, 54.5051f, 15));
cam_pos_points.push_back(ControlPoint(77.1455f, 0.99788f, 54.8179f, 16));
cam_pos_points.push_back(ControlPoint(77.9095f, 1.12481f, 55.4286f, 17));
cam_pos_points.push_back(ControlPoint(78.4052f, 1.00798f, 55.6928f, 18));
cam_pos_points.push_back(ControlPoint(79.1353f, 1.18747f, 56.1969f, 19));
cam_pos_points.push_back(ControlPoint(79.6382f, 1.20899f, 56.6266f, 20));
cam_pos_points.push_back(ControlPoint(80.5462f, 0.997388f, 58.2867f, 21));
cam_pos_points.push_back(ControlPoint(80.1258f, 0.956428f, 59.0802f, 22));
cam_pos_points.push_back(ControlPoint(79.969f, 1.00764f, 59.8901f, 23));
cam_pos_points.push_back(ControlPoint(78.0666f, 1.21057f, 60.6207f, 24));
cam_pos_points.push_back(ControlPoint(77.0299f, 1.09031f, 59.9879f, 25));
cam_pos_points.push_back(ControlPoint(76.2456f, 1.00912f, 59.4651f, 26));
cam_pos_points.push_back(ControlPoint(76.1279f, 0.834948f, 58.762f, 27));
cam_pos_points.push_back(ControlPoint(76.2272f, 0.898529f, 58.3275f, 28));
cam_pos_points.push_back(ControlPoint(76.6348f, 0.309602f, 58.3864f, 29));
cam_pos_points.push_back(ControlPoint(77.0617f, 0.106423f, 58.4357f, 30));
cam_pos_points.push_back(ControlPoint(77.4972f, 0.044952f, 58.3656f, 31));
cam_pos_points.push_back(ControlPoint(77.9114f, 0.0564439f, 58.3079f, 32));
cam_pos_points.push_back(ControlPoint(78.7697f, 0.184217f, 58.0087f, 33));
cam_pos_points.push_back(ControlPoint(79.2613f, 0.401773f, 57.83f, 34));
cam_pos_points.push_back(ControlPoint(79.7492f, 0.552262f, 57.7633f, 35));
cam_pos_points.push_back(ControlPoint(80.4678f, 0.593623f, 57.4382f, 36));
cam_pos_points.push_back(ControlPoint(80.9557f, 0.789814f, 57.5709f, 37));
cam_pos_points.push_back(ControlPoint(81.2324f, 0.909865f, 57.6799f, 38));
cam_pos_points.push_back(ControlPoint(81.9105f, 0.709966f, 57.8259f, 39));
cam_pos_points.push_back(ControlPoint(82.1205f, 0.744281f, 57.1301f, 40));
cam_pos_points.push_back(ControlPoint(82.3428f, 0.919922f, 56.6353f, 41));
cam_pos_points.push_back(ControlPoint(82.3595f, 0.929432f, 55.0815f, 42));
cam_pos_points.push_back(ControlPoint(82.3117f, 0.926831f, 54.3855f, 43));
cam_pos_points.push_back(ControlPoint(82.2058f, 1.0561f, 53.6898f, 44));
cam_pos_points.push_back(ControlPoint(82.8506f, 0.878034f, 53.3828f, 45));
cam_pos_points.push_back(ControlPoint(83.8879f, 0.802315f, 53.9009f, 46));
cam_pos_points.push_back(ControlPoint(84.8927f, 0.715543f, 54.282f, 47));
cam_pos_points.push_back(ControlPoint(85.3214f, 0.612899f, 55.5229f, 48));
cam_pos_points.push_back(ControlPoint(86.996f, 0.613258f, 54.8913f, 49));
cam_pos_points.push_back(ControlPoint(88.6674f, 0.549471f, 54.1753f, 50));
cam_pos_points.push_back(ControlPoint(89.8695f, 0.509852f, 51.8716f, 51));
cam_pos_points.push_back(ControlPoint(89.6406f, 0.454212f, 51.2013f, 52));
look:
cam_look_points.push_back(ControlPoint(68.9428f, 1.84098f, 47.6271f, 101));
cam_look_points.push_back(ControlPoint(69.3261f, 1.96152f, 47.96f, 102));
cam_look_points.push_back(ControlPoint(69.6747f, 1.98957f, 48.4459f, 103));
cam_look_points.push_back(ControlPoint(70.0459f, 1.99236f, 48.4773f, 104));
cam_look_points.push_back(ControlPoint(70.4157f, 2.05567f, 48.5859f, 105));
cam_look_points.push_back(ControlPoint(70.6945f, 2.14503f, 48.7679f, 106));
cam_look_points.push_back(ControlPoint(70.939f, 1.64665f, 49.2294f, 107));
cam_look_points.push_back(ControlPoint(71.296f, 1.3033f, 49.7153f, 108));
cam_look_points.push_back(ControlPoint(71.6218f, 1.44213f, 50.2711f, 109));
cam_look_points.push_back(ControlPoint(72.3463f, 1.05114f, 51.7582f, 110));
cam_look_points.push_back(ControlPoint(73.0046f, 1.0778f, 52.5253f, 111));
cam_look_points.push_back(ControlPoint(73.4157f, 1.08437f, 52.864f, 112));
cam_look_points.push_back(ControlPoint(73.886f, 1.06622f, 53.2058f, 113));
cam_look_points.push_back(ControlPoint(74.3558f, 0.966875f, 53.6904f, 114));
cam_look_points.push_back(ControlPoint(74.8533f, 1.12688f, 53.5013f, 115));
cam_look_points.push_back(ControlPoint(76.2312f, 0.322981f, 56.6035f, 116));
cam_look_points.push_back(ControlPoint(76.8177f, 0.736968f, 57.1319f, 117));
cam_look_points.push_back(ControlPoint(75.7336f, 0.204478f, 59.72f, 118));
cam_look_points.push_back(ControlPoint(76.6859f, 0.143176f, 58.5181f, 119));
cam_look_points.push_back(ControlPoint(77.1882f, 0.444209f, 57.0577f, 120));
cam_look_points.push_back(ControlPoint(77.2011f, 0.0654193f, 56.5809f, 121));
cam_look_points.push_back(ControlPoint(77.6165f, 0.246621f, 56.5174f, 122));
cam_look_points.push_back(ControlPoint(77.985f, 0.283994f, 56.4625f, 123));
cam_look_points.push_back(ControlPoint(78.628f, 0.321057f, 56.6942f, 124));
cam_look_points.push_back(ControlPoint(78.6934f, 0.468758f, 56.8392f, 125));
cam_look_points.push_back(ControlPoint(78.82f, 0.475013f, 57.5258f, 126));
cam_look_points.push_back(ControlPoint(78.9993f, 0.281416f, 58.0725f, 127));
cam_look_points.push_back(ControlPoint(78.4476f, 0.145205f, 58.2791f, 128));
cam_look_points.push_back(ControlPoint(77.3939f, -0.173369f, 58.5733f, 129));
cam_look_points.push_back(ControlPoint(78.7703f, -0.17559f, 58.4542f, 130));
cam_look_points.push_back(ControlPoint(79.5734f, -0.167618f, 58.2698f, 131));
cam_look_points.push_back(ControlPoint(80.3956f, 0.261965f, 58.1866f, 132));
cam_look_points.push_back(ControlPoint(80.6748f, 0.270244f, 58.0932f, 133));
cam_look_points.push_back(ControlPoint(81.2167f, 0.233927f, 57.882f, 134));
cam_look_points.push_back(ControlPoint(81.9574f, 0.250172f, 57.7894f, 135));
cam_look_points.push_back(ControlPoint(83.3611f, 0.0861395f, 57.923f, 136));
cam_look_points.push_back(ControlPoint(83.3864f, 0.279f, 57.258f, 137));
cam_look_points.push_back(ControlPoint(83.4657f, 0.418687f, 56.7398f, 138));
cam_look_points.push_back(ControlPoint(82.0864f, 0.149573f, 55.1861f, 139));
cam_look_points.push_back(ControlPoint(82.6407f, 0.858131f, 54.3597f, 140));
cam_look_points.push_back(ControlPoint(82.9102f, 1.20127f, 54.0629f, 141));
cam_look_points.push_back(ControlPoint(83.8738f, 0.568238f, 53.9944f, 142));
cam_look_points.push_back(ControlPoint(84.4917f, 0.666351f, 54.5446f, 143));
cam_look_points.push_back(ControlPoint(84.8258f, 0.632537f, 54.8678f, 144));
cam_look_points.push_back(ControlPoint(85.8143f, 0.798242f, 55.9753f, 145));
cam_look_points.push_back(ControlPoint(87.3291f, 0.512318f, 55.468f, 146));
cam_look_points.push_back(ControlPoint(88.5109f, 0.324828f, 55.0029f, 147));
cam_look_points.push_back(ControlPoint(88.6998f, 0.527363f, 54.5573f, 148));
cam_look_points.push_back(ControlPoint(90.0354f, 0.408933f, 53.7348f, 149));
cam_look_points.push_back(ControlPoint(90.1793f, 0.677614f, 52.8305f, 150));
cam_look_points.push_back(ControlPoint(90.2064f, 1.0422f, 51.2021f, 151));
cam_look_points.push_back(ControlPoint(89.1084f, 1.25819f, 50.3483f, 152));

	for (unsigned int i = 0; i < cam_pos_points.size(); i++) {
		cam_pos_points[i].id() = i;
		cam_pos_points[i].init(_pid_point, _pid_point_selection);
	}

	for (unsigned int i = 0; i < POS_CURVE_N; i++) {
		cam_pos_curve[i].init(_pid_bezier);
		cam_pos_curve[i].set_points(cam_pos_points[i * 3].position(), cam_pos_points[i * 3 + 1].position(), cam_pos_points[i * 3 + 2].position(), cam_pos_points[i * 3 + 3].position());
	}


	for (unsigned int i = 0; i < cam_look_points.size(); i++) {
		cam_look_points[i].id() = i + 100;
		cam_look_points[i].init(_pid_point, _pid_point_selection);
	}

	for (unsigned int i = 0; i < POS_CURVE_N; i++) {
		cam_look_curve[i].init(_pid_bezier);
		cam_look_curve[i].set_points(cam_look_points[i * 3].position(), cam_look_points[i * 3 + 1].position(), cam_look_points[i * 3 + 2].position(), cam_look_points[i * 3 + 3].position());
	}
}

void init(){
	glClearColor(sky_color(0), sky_color(1), sky_color(2), /*solid*/1.0);
    glEnable(GL_DEPTH_TEST);


	initCurves();

	world.init(vec2(cam_pos(0), cam_pos(2)), sky_color);
	fb_mirrored.init(true);
	fb_quad.init(true);
	fb_main.init(true);
	skybox.init();

	water.init(fb_main.getColorAttachment(), fb_mirrored.getColorAttachment());

	sqad.init(fb_quad.getColorAttachment(), fb_quad.getDepthAttachment());

#ifdef RECORDING
	navmode = BEZIER;
	current_frame = 0;
	imgData = (unsigned char*)calloc(width*height * 3, sizeof(char));
#endif

	glViewport(0, 0, width, height);
}

void display(){

    opengp::update_title_fps("FrameBuffer");
    glViewport(0,0,width,height);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glEnable(GL_BLEND); // not working on my laptop

    ///--- Setup view-projection matrix
    float ratio = width / (float) height;
    projection = Eigen::perspective(45.0f, ratio, 0.01f, 10.0f);
    vec3 cam_up(0.0f, 1.0f, 0.0f);

	// Floats get rough at high values so we keep the position near 0
	vec2 cam_pos_memo(cam_pos(0), cam_pos(2));
	cam_pos(0) = 0;
	cam_pos(2) = 0;

	model = mat4::Identity();
	// For simplicity: water is at height 0
	// And the whole map is translated down by water_level units
	// TODO: move hardcoded water level smw else
	model(1, 3) = -SEA_LEVEL;
	mat4 skybox_model = model;

	mat4 world_model(model);

	float time = glfwGetTime();
	// In bezier mode, use per frame timing to get perfect movie synchro
	if (navmode == BEZIER) {
		time = current_frame * TIME_PER_FRAME;
	}
	else {
		// compute camera view from angles
		cam_look(0) = cam_pos(0) - sin(angles(0))*cos(angles(1));
		cam_look(2) = cam_pos(2) + cos(angles(0))*cos(angles(1));
		cam_look(1) = cam_pos(1) - sin(angles(1));
	}



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
		sqad.draw(mirrored_view, projection, sfx_mirror_cam_pos, time);
	fb_mirrored.unbind();

	fb_main.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.draw(skybox_model, skybox_view, projection);
		world.draw(model, view, projection, cam_pos(1));
	fb_main.unbind();

	
	fb_quad.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.draw(skybox_model, skybox_view, projection);
		// restore camera
		cam_pos(0) = cam_pos_memo(0);
		cam_pos(2) = cam_pos_memo(1);

#if defined(EDIT_CURVES) && !defined(RECORDING)
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
		for (unsigned int i = 0; i < POS_CURVE_N; i++) {
			cam_look_curve[i].draw(model, view, projection);
		}
#endif
		world.draw(world_model, view, projection, cam_pos(1));
		water.draw(mat4::Identity(), view, projection);
	fb_quad.unbind();



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	sqad.draw(view, projection, cam_pos, time);

#ifdef RECORDING
	saveFrame();
#endif
}

void update() {

	if (navmode == NAVIGATION) {
		const int coeff = speedup ? 3 : 1;
		// update pos
		if (keys[KEY_FWD]) {
			cam_pos(2) += coeff * MOVE_INC*cos(angles(0))*cos(angles(1));
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
	}
	else {
		if (current_frame * BEZIER_SPEED < POS_CURVE_N) {
			current_frame++;

			float position = current_frame * BEZIER_SPEED;
			double curve_i;
			float offset = modf(position, &curve_i);

			cam_pos_curve[(int)curve_i].sample_point(offset, cam_pos);
			cam_pos(1) -= SEA_LEVEL;
			cam_look_curve[(int)curve_i].sample_point(offset, cam_look);
			cam_look(0) -= cam_pos(0); // because camera is centered at 0,0
			cam_look(2) -= cam_pos(2);
			cam_look(1) -= SEA_LEVEL;
		}
	}

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

			for (unsigned int i = 0; i < POS_CURVE_N; i++) {
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
		case 'C':
			// output curves ctrl points
			std::cout << "-- curves dump --" << std::endl << "position:" << std::endl;
			for (unsigned int i = 0; i < POS_CURVE_N; i++) {
				cam_pos_curve[i].print_points();
			}
			std::cout << "look:" << std::endl;
			for (unsigned int i = 0; i < POS_CURVE_N; i++) {
				cam_look_curve[i].print_points();
			}
			break;
		case 'B':
			std::cout << "Bezier mode" << std::endl;
			navmode = BEZIER;
			current_frame = 0;
			break;
		case 'N':
			std::cout << "Navigation mode" << std::endl;
			navmode = NAVIGATION;
			break;
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

	glDeleteProgram(_pid_bezier);
	glDeleteProgram(_pid_point);
	glDeleteProgram(_pid_point_selection);

#ifdef RECORDING
	free(imgData);
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
