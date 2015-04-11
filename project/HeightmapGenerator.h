#pragma once
#include "icg_common.h"

class HeightmapGenerator {
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID 
    GLuint _vbo; ///< memory buffer
	GLuint _grad;

	int _resolution;
    
public:    
	void init(const int resolution){

		_resolution = resolution;

        ///--- Compile the shaders
        _pid = opengp::load_shaders("shaders/heightmap.vert.glsl", "shaders/heightmap.frag.glsl");
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
            GLuint vpoint_id = glGetAttribLocation(_pid, "position");
            glEnableVertexAttribArray(vpoint_id);
            glVertexAttribPointer(vpoint_id, 2, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);
        }
		// Perlin gradients
		{
			std::vector<GLfloat> gradients;

			srand(1234); // deterministic for debugging purpose

			for (int i = 0; i < resolution; ++i) {
				gradients.push_back((float)rand() / RAND_MAX - 0.5); // x
				gradients.push_back((float)rand() / RAND_MAX - 0.5); // y
				// gradients.push_back(0); // z
			}

			///--- Buffer
			glGenTextures(1, &_grad);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, _grad);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RG16F, resolution, 0, GL_RG, GL_FLOAT, gradients.data());
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			// glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);// 1D tex don't need it ?

			// Texture uniforms
			GLuint tex_id = glGetUniformLocation(_pid, "gradients");
			glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
		}
        
        ///--- to avoid the current object being polluted
        glBindVertexArray(0);
        glUseProgram(0);
    }
           
    void cleanup(){
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, &_vao);
		glDeleteProgram(_pid);
		glDeleteTextures(1, &_grad);
    }
    
	void draw(float H, float lacunarity, int octaves){
        glUseProgram(_pid);
        glBindVertexArray(_vao);
		
			GLuint uni_id;

			uni_id = glGetUniformLocation(_pid, "resolution");
			glUniform1f(uni_id, _resolution);
			uni_id = glGetUniformLocation(_pid, "H");
			glUniform1f(uni_id, H);
			uni_id = glGetUniformLocation(_pid, "lacunarity");
			glUniform1f(uni_id, lacunarity);
			uni_id = glGetUniformLocation(_pid, "octaves");
			glUniform1i(uni_id, octaves);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, _grad);
            
            ///--- Draw
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);        
        glUseProgram(0);
    }
};
