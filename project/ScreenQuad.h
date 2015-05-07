#pragma once
#include "icg_common.h"

class ScreenQuad{
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID 
    GLuint _vbo; ///< memory buffer
    GLuint _color_tex; ///< Texture ID
	GLuint _depth_tex; ///< Texture ID
public:
    void init(GLuint color_tex, GLuint depth_tex){ 
        
        ///--- Compile the shaders
        _pid = opengp::load_shaders("shaders/ScreenQuad.vert.glsl", "shaders/ScreenQuad.frag.glsl");
        if(!_pid) exit(EXIT_FAILURE);       
        glUseProgram(_pid);
        
        ///--- Vertex one vertex Array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
     
        ///--- Vertex coordinates
        {
            const GLfloat vpoint[] = { /*V1*/ -1.0f, -1.0f, 
                                       /*V2*/ +1.0f, -1.0f, 
                                       /*V3*/ -1.0f, +1.0f,
                                       /*V4*/ +1.0f, +1.0f };        
            ///--- Buffer
            glGenBuffers(1, &_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);
        
            ///--- Attribute
            GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
            glEnableVertexAttribArray(vpoint_id);
            glVertexAttribPointer(vpoint_id, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
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
        
        ///--- Load/Assign texture
		_color_tex = color_tex;
		glBindTexture(GL_TEXTURE_2D, _color_tex);
        GLuint color_tex_id = glGetUniformLocation(_pid, "color_tex");
        glUniform1i(color_tex_id, 0 /*GL_TEXTURE0*/);
		_depth_tex = depth_tex;
		glBindTexture(GL_TEXTURE_2D, _depth_tex);
		GLuint depth_tex_id = glGetUniformLocation(_pid, "depth_tex");
		glUniform1i(depth_tex_id, 1 /*GL_TEXTURE1*/);
    
        
        ///--- to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }
       
    void cleanup(){
        // TODO cleanup
    }
    
    void draw(const mat4& view, const mat4& projection, const vec3& cam_pos){
        glUseProgram(_pid);
        glBindVertexArray(_vao);
			
			mat4 VP = projection * view;
			mat4 VP_i = VP.inverse();

			GLint view_id = glGetUniformLocation(_pid, "VP_i");
			glUniformMatrix4fv(view_id, ONE, DONT_TRANSPOSE, VP_i.data());

			view_id = glGetUniformLocation(_pid, "cam_pos");
			glUniform3f(view_id, cam_pos(0), cam_pos(1), cam_pos(2));
			
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _color_tex);
            glUniform1f(glGetUniformLocation(_pid, "tex_width"), _width);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, _depth_tex);
            glUniform1f(glGetUniformLocation(_pid, "tex_height"), _height); 
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);        
        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
