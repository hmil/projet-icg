#pragma once
#include "icg_common.h"

class Grid{
protected:
    GLuint _vao;          ///< vertex array object
    GLuint _vbo_position; ///< memory buffer for positions
    GLuint _vbo_index;    ///< memory buffer for indice
    GLuint _pid;          ///< GLSL shader program ID
    GLuint _tex;          ///< Texture ID
    GLuint _num_indices;  ///< number of vertices to render
    
	int grid_dim = 512;

public:    
    void init(GLuint tex_coord){
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
            // reach from [-1, -1] to [1, 1].

            // Vertex position of the triangles.
			float step = 2.0f / grid_dim;
			for (int i = 0; i < grid_dim+1; ++i) {
				for (int j = 0; j < grid_dim+1; ++j) {
					vertices.push_back(-1.0f + step*i);	vertices.push_back(-1.0f + step*j);
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
		/*
		glGenTextures(1, &_tex);
		glBindTexture(GL_TEXTURE_2D, _tex);
		glfwLoadTexture2D("_grid/grid_texture.tga", 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		*/

		_tex = tex_coord;
		glBindTexture(GL_TEXTURE_2D, _tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// Texture uniforms
		GLuint tex_id = glGetUniformLocation(_pid, "tex");
		glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);
        
        // to avoid the current object being polluted
        glBindVertexArray(0);
    }
           
    void cleanup(){
        glDeleteBuffers(1, &_vbo_position);
        glDeleteBuffers(1, &_vbo_index);
        glDeleteVertexArrays(1, &_vao);
        glDeleteProgram(_pid);
        glDeleteTextures(1, &_tex);
    }
    
    void draw(const mat4& MVP){
        glUseProgram(_pid);
        glBindVertexArray(_vao);
        // Bind textures
        glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _tex);

        // Setup MVP
        GLuint MVP_id = glGetUniformLocation(_pid, "mvp");
        glUniformMatrix4fv(MVP_id, 1, GL_FALSE, MVP.data());


        // Draw
        glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
