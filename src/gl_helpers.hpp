// gl_helpers.hpp
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

#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

// check for OpenGL error and print message
void check_error(const std::string & at);

// compile a shader object
GLuint compile_shader(const std::string & filename, GLenum shader_type);

// link shader objects into shader program,
GLuint link_shader_prog(const std::vector<GLuint> & shaders,
    const std::vector<std::pair<GLuint, std::string>> & attribs);

// create & load a texture from a filename
GLuint create_texture_from_file(const std::string & filename);

#endif // TEXTURE_H
