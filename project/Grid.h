#pragma once
#include "icg_common.h"


struct Light{
	vec3 Ia = vec3(0.1f, 0.1f, 0.1f);
	vec3 Id = vec3(0.85f, 0.9f, 1.0f);
	vec3 Is = vec3(1.0f, 1.0f, 1.0f);

	vec3 light_dir = vec3(-1.0f, 0.75f, 0.0f);

	///--- Pass light properties to the shader
	void setup(GLuint _pid){
		glUseProgram(_pid);
		GLuint light_dir_id = glGetUniformLocation(_pid, "light_dir"); //Given in camera space
		GLuint Ia_id = glGetUniformLocation(_pid, "Ia");
		GLuint Id_id = glGetUniformLocation(_pid, "Id");
		GLuint Is_id = glGetUniformLocation(_pid, "Is");
		glUniform3fv(light_dir_id, ONE, light_dir.normalized().data());
		glUniform3fv(Ia_id, ONE, Ia.data());
		glUniform3fv(Id_id, ONE, Id.data());
		glUniform3fv(Is_id, ONE, Is.data());
	}

	vec3 get_spot_direction(float time) {
		return light_dir;
	}
};

class Grid : public Light{
protected:
    GLuint _vao;          ///< vertex array object
    GLuint _vbo_position; ///< memory buffer for positions
    GLuint _vbo_index;    ///< memory buffer for indice

    GLuint _pid;          ///< GLSL shader program ID
    GLuint _grass_tex, _rock_tex, _snow_tex, _sand_tex;
    
	int grid_dim;


	void generateGrid(std::vector<GLfloat> &vertices, std::vector<GLuint> &indices, int dim) {
		float step = 4.0f / dim;
		for (int i = 0; i < dim + 1; ++i) {
			for (int j = 0; j < dim + 1; ++j) {
				vertices.push_back(-2.0f + step*i);	vertices.push_back(-2.0f + step*j);
			}
		}


		// And indices.
		for (int i = 0; i < dim; ++i) {
			for (int j = 0; j < dim; ++j) {
				indices.push_back((dim + 1) * i + j);
				indices.push_back((dim + 1) * (i + 1) + j);
				indices.push_back((dim + 1) * i + j + 1);
				indices.push_back((dim + 1) * (i + 1) + j + 1);
			}
		}
	}

public:

	Grid() : grid_dim(32)
	{ }

	void init(vec3 fogColor){
		
		// Compile the shaders
		_pid = opengp::load_shaders("shaders/grid.vert.glsl", "shaders/grid.frag.glsl", "shaders/grid.geom.glsl", "shaders/grid.tc.glsl", "shaders/grid.te.glsl");
        if(!_pid) exit(EXIT_FAILURE);       
        glUseProgram(_pid);
        
     
        // Vertex coordinates and indices
        GLuint loc_position = glGetAttribLocation(_pid, "position");

			
		{
			std::vector<GLfloat> vertices;
			std::vector<GLuint> indices;

			// Vertex one vertex Array
			glGenVertexArrays(1, &_vao);
			glBindVertexArray(_vao);
			generateGrid(vertices, indices, grid_dim);
			// position buffer
			glGenBuffers(1, &_vbo_position);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo_position);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
			// vertex indices
			glGenBuffers(1, &_vbo_index);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_index);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
			// position shader attribute
			glEnableVertexAttribArray(loc_position);
			glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }

        /***************************/
        // Load textures
        /***************************/

		// common texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Use trilinear interpolation for minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// Use bilinear interpolation for magnification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		
        //// Grass
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &_grass_tex);
        glBindTexture(GL_TEXTURE_2D, _grass_tex);
		glfwLoadTexture2D("textures/grass.tga", 0);
		glGenerateMipmap(GL_TEXTURE_2D);

        // Rock
        glActiveTexture(GL_TEXTURE2);
        glGenTextures(1, &_rock_tex);
        glBindTexture(GL_TEXTURE_2D, _rock_tex);
        glfwLoadTexture2D("textures/rock.tga", 0);
		glGenerateMipmap(GL_TEXTURE_2D);

        // Snow
        glActiveTexture(GL_TEXTURE3);
        glGenTextures(1, &_snow_tex);
        glBindTexture(GL_TEXTURE_2D, _snow_tex);
        glfwLoadTexture2D("textures/snow.tga", 0);
		glGenerateMipmap(GL_TEXTURE_2D);

        // Sand
        glActiveTexture(GL_TEXTURE4);
        glGenTextures(1, &_sand_tex);
        glBindTexture(GL_TEXTURE_2D, _sand_tex);
        glfwLoadTexture2D("textures/sand.tga", 0);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Texture uniforms
        GLuint tex_id = glGetUniformLocation(_pid, "grass_tex");
        glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);
        tex_id = glGetUniformLocation(_pid, "rock_tex");
        glUniform1i(tex_id, 2 /*GL_TEXTURE2*/);
        tex_id = glGetUniformLocation(_pid, "snow_tex");
        glUniform1i(tex_id, 3 /*GL_TEXTURE3*/);
        tex_id = glGetUniformLocation(_pid, "sand_tex");
        glUniform1i(tex_id, 4 /*GL_TEXTURE4*/);


		// fog color
		GLuint fog_id = glGetUniformLocation(_pid, "fogColor");
		glUniform3f(fog_id, fogColor(0), fogColor(1), fogColor(2));

        // to avoid the current object being polluted
        glBindVertexArray(0);
    }
           
    void cleanup(){
		glDeleteBuffers(1, &_vbo_position);
		glDeleteBuffers(1, &_vbo_index);
		glDeleteVertexArrays(1, &_vao);
        glDeleteProgram(_pid);
        glDeleteTextures(1, &_grass_tex);
        glDeleteTextures(1, &_snow_tex);
        glDeleteTextures(1, &_sand_tex);
        glDeleteTextures(1, &_rock_tex);
    }
    
	void draw(const mat4& model, const mat4& view, const mat4& projection, const int resolution, GLuint heightmap, const float cam_height){

        glUseProgram(_pid);
        glBindVertexArray(_vao);

		// Prepare tesselation: use quad patches
		glPatchParameteri(GL_PATCH_VERTICES, 4);


        // Bind textures
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmap);
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _grass_tex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _rock_tex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _snow_tex);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, _sand_tex);

		Light::setup(_pid);

		
		glUniform1f(glGetUniformLocation(_pid, "cam_height"), cam_height);

		///--- Setup matrix stack
		GLint model_id = glGetUniformLocation(_pid, "model");
		glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, model.data());
		GLint view_id = glGetUniformLocation(_pid, "view");
		glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, view.data());
		GLint projection_id = glGetUniformLocation(_pid, "projection");
		glUniformMatrix4fv(projection_id, ONE, DONT_TRANSPOSE, projection.data());
		GLint MV_id = glGetUniformLocation(_pid, "MV");
		mat4 MV = view * model;
		glUniformMatrix4fv(MV_id, ONE, DONT_TRANSPOSE, MV.data());
		GLint MVP_id = glGetUniformLocation(_pid, "MVP");
		glUniformMatrix4fv(MVP_id, ONE, DONT_TRANSPOSE, ((mat4)(projection * MV)).data());

		// prebaked inverse
		mat4 model_i = model.transpose();
		model_i.inverse();
		GLint model_i_id = glGetUniformLocation(_pid, "model_i");
		glUniformMatrix4fv(model_i_id, ONE, DONT_TRANSPOSE, model_i.data());
		mat4 view_i = view;
		view_i.inverse();
		GLint view_i_id = glGetUniformLocation(_pid, "view_i");
		glUniformMatrix4fv(view_i_id, ONE, DONT_TRANSPOSE, view_i.data());

		GLint resolution_id = glGetUniformLocation(_pid, "resolution");
		glUniform1i(resolution_id, resolution);

		// Texture uniforms
		GLuint tex_id = glGetUniformLocation(_pid, "tex");
        glUniform1i(tex_id, 0);

        // Draw
		
		int num_indices = 4 * grid_dim * grid_dim;
		glDrawElements(GL_PATCHES, num_indices, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
