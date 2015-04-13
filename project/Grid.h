#pragma once
#include "icg_common.h"


static struct Light{
	vec3 Ia = vec3(0.1f, 0.1f, 0.1f);
	vec3 Id = vec3(0.85f, 0.9f, 1.0f);
	vec3 Is = vec3(1.0f, 1.0f, 1.0f);

	vec3 light_pos = vec3(0.0f, 5000.0f, 0.0f);

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
    GLuint _vao;          ///< vertex array object
    GLuint _vbo_position; ///< memory buffer for positions
    GLuint _vbo_index;    ///< memory buffer for indice
    GLuint _pid;          ///< GLSL shader program ID
    GLuint _num_indices;  ///< number of vertices to render
	GLuint _color_tex;
    
	int grid_dim = 512;

public:    
	void init(vec3 fogColor){
		
		// Compile the shaders
		_pid = opengp::load_shaders("shaders/grid.vert.glsl", "shaders/grid.frag.glsl");
        if(!_pid) exit(EXIT_FAILURE);       
        glUseProgram(_pid);
        
        // Vertex one vertex Array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
     
        // Vertex coordinates and indices
        {
            std::vector<GLfloat> vertices;
            std::vector<GLuint> indices;
            // Always two subsequent entries in 'vertices' form a 2D vertex position.

            // The given code below are the vertices for a simple quad.
            // Your grid should have the same dimension as that quad, i.e.,
            // reach from [-2, -2] to [2, 2].

            // Vertex position of the triangles.
			float step = 4.0f / grid_dim;
			for (int i = 0; i < grid_dim+1; ++i) {
				for (int j = 0; j < grid_dim+1; ++j) {
					vertices.push_back(-2.0f + step*i);	vertices.push_back(-2.0f + step*j);
				}
			}
            

            // And indices.
			for (int i = 0; i < grid_dim ; ++i) {
				for (int j = 0; j < grid_dim ; ++j) {
					indices.push_back((grid_dim+1) * i + j);
					indices.push_back((grid_dim + 1) * i + j + 1);
					indices.push_back((grid_dim + 1) * (i + 1) + j);
					indices.push_back((grid_dim + 1) * (i + 1) + j);
					indices.push_back((grid_dim + 1) * i + j + 1);
					indices.push_back((grid_dim + 1) * (i + 1) + j + 1);
				}
			}
            

            _num_indices = indices.size();

            // position buffer
            glGenBuffers(1, &_vbo_position);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo_position);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

            // vertex indices
            glGenBuffers(1, &_vbo_index);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_index);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

            // position shader attribute
            GLuint loc_position = glGetAttribLocation(_pid, "position");
            glEnableVertexAttribArray(loc_position);
            glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }

		// Load texture

		float colors[] = {
			0.15f, 0.05f, 0.05f,
			0.15f, 0.05f, 0.05f,
			0.6f, 0.7f, 0.35f,
			0.15f, 0.3f, 0.3f,
			0.2f, 0.4f, 0.3f,
			1.0f, 1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f
		};
		
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &_color_tex);
		glBindTexture(GL_TEXTURE_1D, _color_tex);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 7, 0, GL_RGB, GL_FLOAT, colors);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// Texture uniforms
		GLuint col_tex_id = glGetUniformLocation(_pid, "color_tex");
		glUniform1i(col_tex_id, 0 /*GL_TEXTURE0*/);


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
    }
    
    void draw(const mat4& model, const mat4& view, const mat4& projection, const int resolution, GLuint heightmap){
        glUseProgram(_pid);
        glBindVertexArray(_vao);
        // Bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, _color_tex);

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

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, heightmap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
		// Texture uniforms
		GLuint tex_id = glGetUniformLocation(_pid, "tex");
		glUniform1i(tex_id, 1);

        // Draw
        glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
