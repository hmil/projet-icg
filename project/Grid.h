#pragma once
#include "icg_common.h"


struct Light{
	vec3 Ia = vec3(0.1f, 0.1f, 0.1f);
	vec3 Id = vec3(0.85f, 0.9f, 1.0f);
	vec3 Is = vec3(1.0f, 1.0f, 1.0f);

	vec3 light_pos = vec3(2.0f, 2000.0f, 0.0);

	///--- Pass light properties to the shader
	void setup(GLuint _pid){
		glUseProgram(_pid);
		GLuint light_pos_id = glGetUniformLocation(_pid, "light_pos"); //Given in camera space
		GLuint Ia_id = glGetUniformLocation(_pid, "Ia");
		GLuint Id_id = glGetUniformLocation(_pid, "Id");
		GLuint Is_id = glGetUniformLocation(_pid, "Is");
		glUniform3fv(light_pos_id, ONE, light_pos.data());
		glUniform3fv(Ia_id, ONE, Ia.data());
		glUniform3fv(Id_id, ONE, Id.data());
		glUniform3fv(Is_id, ONE, Is.data());
	}

	vec3 get_spot_direction(float time) {
		return light_pos;
	}
};

class Grid : public Light{
protected:
    GLuint _vaos[3];          ///< vertex array object
    GLuint _vbo_positions[3]; ///< memory buffer for positions
    GLuint _vbo_indexes[3];    ///< memory buffer for indice

    GLuint _pid;          ///< GLSL shader program ID
    GLuint _grass_tex, _rock_tex, _snow_tex, _sand_tex;
    
	int grid_dim[3];


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
				indices.push_back((dim + 1) * i + j + 1);
				indices.push_back((dim + 1) * (i + 1) + j);
				indices.push_back((dim + 1) * (i + 1) + j);
				indices.push_back((dim + 1) * i + j + 1);
				indices.push_back((dim + 1) * (i + 1) + j + 1);
			}
		}
	}

public:

	enum Definition { HIGH_DEF, MEDIUM_DEF, LOW_DEF };

	Grid()
	{
		grid_dim[HIGH_DEF] = 512;
		grid_dim[MEDIUM_DEF] = 256;
		grid_dim[LOW_DEF] = 64;
	}

	void init(vec3 fogColor){
		
		// Compile the shaders
		_pid = opengp::load_shaders("shaders/grid.vert.glsl", "shaders/grid.frag.glsl");
        if(!_pid) exit(EXIT_FAILURE);       
        glUseProgram(_pid);
        
     
        // Vertex coordinates and indices
        GLuint loc_position = glGetAttribLocation(_pid, "position");

			
		for (int i = 0; i < 3; ++i) {
			std::vector<GLfloat> vertices;
			std::vector<GLuint> indices;

			// Vertex one vertex Array
			glGenVertexArrays(1, &_vaos[i]);
			glBindVertexArray(_vaos[i]);
			generateGrid(vertices, indices, grid_dim[i]);
			// position buffer
			glGenBuffers(1, &_vbo_positions[i]);
			glBindBuffer(GL_ARRAY_BUFFER, _vbo_positions[i]);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
			// vertex indices
			glGenBuffers(1, &_vbo_indexes[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_indexes[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
			// position shader attribute
			glEnableVertexAttribArray(loc_position);
			glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }

        /***************************/
        // Load textures
        /***************************/

		
        //// Grass
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &_grass_tex);
        glBindTexture(GL_TEXTURE_2D, _grass_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glfwLoadTexture2D("textures/grass.tga", 0);

        // Rock
        glActiveTexture(GL_TEXTURE2);
        glGenTextures(1, &_rock_tex);
        glBindTexture(GL_TEXTURE_2D, _rock_tex);
        glfwLoadTexture2D("textures/rock.tga", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Snow
        glActiveTexture(GL_TEXTURE3);
        glGenTextures(1, &_snow_tex);
        glBindTexture(GL_TEXTURE_2D, _snow_tex);
        glfwLoadTexture2D("textures/snow.tga", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Sand
        glActiveTexture(GL_TEXTURE4);
        glGenTextures(1, &_sand_tex);
        glBindTexture(GL_TEXTURE_2D, _sand_tex);
        glfwLoadTexture2D("textures/sand.tga", 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
		for (int i = 0; i < 3; ++i) {
			glDeleteBuffers(1, &_vbo_positions[i]);
			glDeleteBuffers(1, &_vbo_indexes[i]);
			glDeleteVertexArrays(1, &_vaos[i]);
		}
        glDeleteProgram(_pid);
        glDeleteTextures(1, &_grass_tex);
        glDeleteTextures(1, &_snow_tex);
        glDeleteTextures(1, &_sand_tex);
        glDeleteTextures(1, &_rock_tex);
    }
    
    void draw(const mat4& model, const mat4& view, const mat4& projection, const int resolution, GLuint heightmap, Definition def){
        glUseProgram(_pid);
        glBindVertexArray(_vaos[def]);
        // Bind textures
		glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _grass_tex);

		Light::setup(_pid);

		///--- Setup matrix stack
		GLint model_id = glGetUniformLocation(_pid, "model");
		glUniformMatrix4fv(model_id, ONE, DONT_TRANSPOSE, model.data());
		GLint view_id = glGetUniformLocation(_pid, "view");
		glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, view.data());
		GLint projection_id = glGetUniformLocation(_pid, "projection");
		glUniformMatrix4fv(projection_id, ONE, DONT_TRANSPOSE, projection.data());

		GLint resolution_id = glGetUniformLocation(_pid, "resolution");
		glUniform1i(resolution_id, resolution);

        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		// Texture uniforms
		GLuint tex_id = glGetUniformLocation(_pid, "tex");
        glUniform1i(tex_id, 0);

        // Draw
		
		int num_indices = 6 * grid_dim[def] * grid_dim[def];
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
