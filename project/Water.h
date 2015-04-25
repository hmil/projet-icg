#pragma once
#include "icg_common.h"

class Water{
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID 
    GLuint _vbo; ///< memory buffer
    GLuint _tex_mirror; ///< Texture ID of mirror texture
	GLuint _tex_through; ///< Texture ID of underneath texture

	const float PLANE_SIZE = 10;
    
public:    
    void init(GLuint tex_through, GLuint tex_mirror){
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
                                          /*V2*/ 1.0f, 0.0f, 
                                          /*V3*/ 0.0f, 1.0f,
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
		_tex_through = tex_through;
        
        ///--- Texture uniforms
        GLuint tex_through_id = glGetUniformLocation(_pid, "tex_through");
		glUniform1i(tex_through_id, 0 /*GL_TEXTURE0*/);
        GLuint tex_mirror_id = glGetUniformLocation(_pid, "tex_mirror");
        glUniform1i(tex_mirror_id, 1 /*GL_TEXTURE1*/);
        
        ///--- to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    void draw(const mat4& MVP){
        glUseProgram(_pid);
		glBindVertexArray(_vao);
			glDisable(GL_CULL_FACE);

			GLuint tex_mirror_id = glGetUniformLocation(_pid, "time");
			glUniform1f(tex_mirror_id, glfwGetTime()*5);

            ///--- Bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _tex_through);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _tex_mirror);
            
            ///--- Setup MVP
            GLuint MVP_id = glGetUniformLocation(_pid, "MVP");
            glUniformMatrix4fv(MVP_id, 1, GL_FALSE, MVP.data());   
            
            ///--- Draw
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
