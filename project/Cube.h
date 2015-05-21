#pragma once
#include "icg_common.h"

class Cube {
private:
    GLuint _vao;          ///< vertex array object
    GLuint _vbo_position; ///< memory buffer for positions
    GLuint _vbo_index;    ///< memory buffer for indices
    GLuint _pid;          ///< GLSL shader program ID
    GLuint _num_indices;  ///< number of vertices to render
	GLuint _tex;

public:
    GLuint program_id() const{
        return _pid;
    }

    void init(){
        // Compile the shaders.
        _pid = opengp::load_shaders("shaders/cube.vert.glsl", "shaders/cube.frag.glsl");
        if(!_pid)
          exit(EXIT_FAILURE);
        glUseProgram(_pid);

        // Vertex one vertex array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        // Position buffer
		float dim = 4.9f;
		const GLfloat position[] = { -dim, -dim, dim,  // left, bottom, front
			dim, -dim, dim,  // right, bottom, front
			dim, dim, dim,  // right, top, front
			-dim, dim, dim,  // left, top, front
			-dim, -dim, -dim,  // left, bottom, back
			dim, -dim, -dim,  // right, bottom, back
			dim, dim, -dim,  // right, top, back
			-dim, dim, -dim }; // left, top, back*/

		

        glGenBuffers(1, &_vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo_position);
        glBufferData(GL_ARRAY_BUFFER, sizeof(position), position, GL_STATIC_DRAW);

        // position shader attribute
        GLuint loc_position = glGetAttribLocation(_pid, "position"); ///< Fetch attribute ID for vertex positions
        glEnableVertexAttribArray(loc_position); /// Enable it
        glVertexAttribPointer(loc_position, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

        // Index buffer
        const GLuint index[] = {0, 1, 2,  // Front face triangle 1
                                0, 2, 3,  // Front face triangle 2
                                1, 5, 6,  // Right face
                                1, 6, 2,
                                5, 4, 7,  // Back face
                                5, 7, 6,
                                4, 0, 3,  // Left face
                                4, 3, 7,
                                3, 2, 6,  // Top face
                                3, 6, 7,
                                1, 0, 4,  // Bottom face
                                1, 4, 5};

        _num_indices = sizeof(index) / sizeof(GLuint);

        glGenBuffers(1, &_vbo_index);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_index);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

		
		// TEX
		
		const char* const faces[] = {
			"textures/skybox/right.tga",
			"textures/skybox/left.tga",
			"textures/skybox/bottom.tga", // OK
			"textures/skybox/top.tga", // OK
			
			"textures/skybox/front.tga",
			"textures/skybox/back.tga"
		};

		glGenTextures(1, &_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _tex);
		for (int i = 0; i < 6; ++i) {
			GLFWimage img;
			if (glfwReadImage(faces[i], &img, GLFW_BUILD_MIPMAPS_BIT) != GL_TRUE) {
				fprintf(stderr, "Unable to load %s\n", faces[i]);
				exit(-1);
			}

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, img.Format, img.Width, img.Height, 0, img.Format, GL_UNSIGNED_BYTE, img.Data);
			glfwFreeImage(&img);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);	
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		// END TEX

		glBindVertexArray(0);
		glUseProgram(0);
    }

    void cleanup(){
        glDeleteBuffers(1, &_vbo_position);
        glDeleteBuffers(1, &_vbo_index);
        glDeleteVertexArrays(1, &_vao);
    }

    void draw(const mat4& model, const mat4& view, const mat4& projection){
		glDepthMask(GL_FALSE);
        glUseProgram(_pid);
		// Setup MVP
		mat4 MVP = projection*view*model;
		GLuint MVP_id = glGetUniformLocation(_pid, "mvp");
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, MVP.data());

		// Draw
        glBindVertexArray(_vao);
		glBindTexture(GL_TEXTURE_CUBE_MAP , _tex);
        glDrawElements(GL_TRIANGLES, _num_indices, GL_UNSIGNED_INT, 0);

		// Clean up
        glBindVertexArray(0);
        glUseProgram(0);
		glDepthMask(GL_TRUE);
    }
};
