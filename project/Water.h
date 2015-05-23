#pragma once
#include "icg_common.h"

class Water{
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID
    GLuint _vbo; ///< memory buffer
    GLuint _tex_mirror; ///< Texture ID of mirror texture

	const float PLANE_SIZE = 10;

	vec2 _cam_pos;

public:
    void init(GLuint tex_mirror){
        ///--- Compile the shaders
        _pid = opengp::load_shaders("shaders/water.vert.glsl", "shaders/water.frag.glsl");
        if(!_pid) exit(EXIT_FAILURE);
        glUseProgram(_pid);

        ///--- Vertex one vertex Array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        ///--- Vertex coordinates
        {
			const GLfloat vpoint[] = {
				/*V1*/ -PLANE_SIZE, 0.0f, -PLANE_SIZE,
				/*V2*/ -PLANE_SIZE, 0.0f, PLANE_SIZE,
				/*V3*/ PLANE_SIZE, 0.0f, -PLANE_SIZE,
				/*V4*/ +PLANE_SIZE, 0.0f, PLANE_SIZE };
            ///--- Buffer
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

            ///--- Attribute
            GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
            glEnableVertexAttribArray(vpoint_id);
            glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }

        ///--- Texture coordinates
        {
            const GLfloat vtexcoord[] = { /*V1*/ 0.0f, 0.0f,
                                          /*V2*/ 0.0f, 1.0f,
                                          /*V3*/ 1.0f, 0.0f,
                                          /*V4*/ 1.0f, 1.0f};

            ///--- Buffer
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

            ///--- Attribute
            GLuint vtexcoord_id = glGetAttribLocation(_pid, "vtexcoord");
            glEnableVertexAttribArray(vtexcoord_id);
            glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }

        _tex_mirror = tex_mirror;

        ///--- Texture uniforms
        GLuint tex_through_id = glGetUniformLocation(_pid, "tex_through");
		glUniform1i(tex_through_id, 0 /*GL_TEXTURE0*/);
        GLuint tex_mirror_id = glGetUniformLocation(_pid, "tex_mirror");
        glUniform1i(tex_mirror_id, 1 /*GL_TEXTURE1*/);

        ///--- to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }

	void update(const vec2& cam_pos) {
		_cam_pos = cam_pos;
	}

    void draw(const mat4& M, const mat4& V, const mat4& P){
        const mat4 MVP = P * V * M;
        glUseProgram(_pid);
		glBindVertexArray(_vao);
			glDisable(GL_CULL_FACE);

			GLuint tex_mirror_id = glGetUniformLocation(_pid, "time");
			glUniform1f(tex_mirror_id, glfwGetTime()*5);

            ///--- Bind textures
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _tex_mirror);

			glUniform2f(glGetUniformLocation(_pid, "cam_pos"), _cam_pos(0), _cam_pos(1));

            ///--- Setup MVP
            GLuint MVP_id = glGetUniformLocation(_pid, "MVP");
            glUniformMatrix4fv(MVP_id, 1, GL_FALSE, MVP.data());

            // -- Setup view
            GLuint V_id = glGetUniformLocation(_pid, "V");
            glUniformMatrix4fv(V_id, 1, GL_FALSE, V.data());

            // -- Setup model
            GLuint M_id = glGetUniformLocation(_pid, "M");
            glUniformMatrix4fv(M_id, 1, GL_FALSE, M.data());

            ///--- Draw
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
