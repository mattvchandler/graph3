// gl_helpers.cpp
// texture and shader loading

// Copyright 2013 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <fstream>
#include <utility>

#include <gdkmm.h>

#include "gl_helpers.hpp"

// check for OpenGL error and print message
void check_error(const std::string & at)
{
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;
    std::cerr<<"OpenGL Error at "<<at<<": "<<gluErrorString(e)<<std::endl;
}

// compile a shader object
GLuint compile_shader(const std::string & filename, GLenum shader_type)
{
    // open shader file
    std::ifstream in(filename, std::ios::binary | std::ios::in);
    std::vector <char> buff;

    if(in)
    {
        in.seekg(0, std::ios::end);
        size_t in_size = in.tellg();
        in.seekg(0, std::ios::beg);

        buff.resize(in_size + 1);
        buff.back() = '\0';
        in.read(buff.data(), in_size);

        if(!in)
        {
            std::cerr<<"Error reading shader: "<<filename<<std::endl;
            return 0;
        }
    }
    else
    {
        std::cerr<<"Error opening shader: "<<filename<<std::endl;
        return 0;
    }

    // create shader object
    GLuint shader = glCreateShader(shader_type);

    // load & compile
    const char * src = buff.data();
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    // error handling
    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if(compile_status != GL_TRUE)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetShaderInfoLog(shader, log_length, NULL, log.data());

        std::cerr<<"Error compiling shader: "<<filename<<std::endl;
        std::cerr<<log.data()<<std::endl;

        glDeleteShader(shader);

        return 0;
    }
    return shader;
}

// link shader objects into shader progra,
GLuint link_shader_prog(const std::vector<GLuint> & shaders)
{
    // create program and load shader objects
    GLuint prog = glCreateProgram();
    for(auto &i: shaders)
        glAttachShader(prog, i);

    glLinkProgram(prog);

    // error handling
    GLint link_status;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);

    if(link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetProgramInfoLog(prog, log_length, NULL, log.data());

        std::cerr<<"Error linking program: "<<std::endl;
        std::cerr<<log.data()<<std::endl;

        glDeleteProgram(prog);

        return 0;
    }
    return prog;
}

// create & load a texture from a filename
GLuint create_texture_from_file(const std::string & filename)
{
    // greate GTK image object from file
    const Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_file(filename);
    int w = image->get_width();
    int h = image->get_height();

    // raw image data
    std::vector<float> float_data(w * h * 4);

    // get data and load into float array
    const guint8 * int_data = image->get_pixels();
    for(int r = 0; r < h; ++r)
    {
        for(int c = 0; c < w; ++c)
        {
            const guint8 * pix = int_data + r * image->get_rowstride() + c * image->get_n_channels();
            switch(image->get_n_channels())
            {
            case 1:
                float_data[(w * r + c) * 4 + 0] = pix[0] / 255.0f;
                float_data[(w * r + c) * 4 + 1] = pix[0] / 255.0f;
                float_data[(w * r + c) * 4 + 2] = pix[0] / 255.0f;
                float_data[(w * r + c) * 4 + 3] = 1.0f;
                break;
            case 3:
                float_data[(w * r + c) * 4 + 0] = pix[0] / 255.0f;
                float_data[(w * r + c) * 4 + 1] = pix[1] / 255.0f;
                float_data[(w * r + c) * 4 + 2] = pix[2] / 255.0f;
                float_data[(w * r + c) * 4 + 3] = 1.0f;
                break;
            case 4:
                float_data[(w * r + c) * 4 + 0] = pix[0] / 255.0f;
                float_data[(w * r + c) * 4 + 1] = pix[1] / 255.0f;
                float_data[(w * r + c) * 4 + 2] = pix[2] / 255.0f;
                float_data[(w * r + c) * 4 + 3] = pix[3] / 255.0f;
                break;
            default:
                break;
            }
        }
    }

    // create OpenGL texture
    glEnable(GL_TEXTURE_2D);
    GLuint tex;
    glGenTextures(1, &tex);

    // copy data to OpenGL
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, (int)(log2(std::min(w, h))) + 1,
        GL_RGBA8, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, float_data.data());

    // set texture properties
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    check_error("Texture generation for: " + filename);

    return tex;
}
