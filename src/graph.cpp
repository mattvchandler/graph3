// graph.cpp
// generic graphing class

// Copyright 2014 Matthew Chandler

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

#include <limits>

#include "gl_helpers.hpp"
#include "graph.hpp"

Graph::Graph():
    use_tex(false), valid_tex(false), color(1.0f, 1.0f, 1.0f), transparency(0.5),
    shininess(50.0f), specular(1.0f), grid_color(0.1f, 0.1f, 0.1f), normal_color(0.0f, 1.0f, 1.0f),
    draw_flag(true), transparent_flag(false), draw_normals_flag(false), draw_grid_flag(true),
    _tex(0), _vao(0), _vbo(0), _ebo(0), _num_indexes(0),
    _grid_vao(0), _grid_ebo(0), _grid_num_indexes(0),
    _normal_vao(0), _normal_vbo(0), _normal_num_indexes(0)
{}

Graph::~Graph()
{
    // free OpenGL resources
    if(_tex)
        glDeleteTextures(1, &_tex);

    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
    if(_ebo)
        glDeleteBuffers(1, &_ebo);

    if(_grid_vao)
        glDeleteVertexArrays(1, &_grid_vao);
    if(_grid_ebo)
        glDeleteBuffers(1, &_grid_ebo);

    if(_normal_vao)
        glDeleteVertexArrays(1, &_normal_vao);
    if(_normal_vbo)
        glDeleteBuffers(1, &_normal_vbo);
}

// draw graph geometry
void Graph::draw() const
{
    glBindVertexArray(_vao);
    glBindTexture(GL_TEXTURE_2D, _tex);

    glDrawElements(GL_TRIANGLE_STRIP, _num_indexes, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

// draw gridlines
void Graph::draw_grid() const
{
    glBindVertexArray(_grid_vao);

    glDrawElements(GL_LINE_STRIP, _grid_num_indexes, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}

// draw normals
void Graph::draw_normals() const
{
    glBindVertexArray(_normal_vao);

    glDrawArrays(GL_LINES, 0, _normal_num_indexes);

    glBindVertexArray(0);
}

// change texture given a filename
// deletes texture when empty filename is given
void Graph::set_texture(const std::string & filename)
{
    // free any existing texture
    if(_tex)
        glDeleteTextures(1, &_tex);

    // zero vars so we're in a good state if the texture creation line throws
    _tex = 0;
    valid_tex = false;

    if(filename.size() == 0)
        return;

    // load texture
    _tex = create_texture_from_file(filename);
    valid_tex = true;
}

sigc::signal<void, const std::string &> Graph::signal_cursor_moved()
{
    return _signal_cursor_moved;
}

// helper function to build OpenGL objects from verticies
void Graph::build_graph_geometry(size_t num_rows, size_t num_columns,
    const std::vector<glm::vec3> & coords,
    const std::vector<glm::vec2> & tex_coords,
    const std::vector<glm::vec3> & normals,
    const std::vector<bool> & defined)
{
    std::vector<GLuint> index;

    bool break_flag = true;

    // arrange verts as a triangle strip
    for(size_t row = 0; row < num_rows - 1; ++row)
    {
        for(size_t column = 0; column < num_columns - 1; ++column)
        {
            // 4 corner indexes
            int ul = row * num_columns + column;
            int ur = row * num_columns + column + 1;
            int ll = (row + 1) * num_columns + column;
            int lr = (row + 1) * num_columns + column + 1;

            // draw appropriate triangles for defined verticies
            if(defined[ul] && defined[ur] && defined[ll] && defined[lr])
            {
                index.push_back(ul);
                index.push_back(ll);
                break_flag = false;
            }
            else if(defined[ul] && defined[ur] && defined[ll])
            {
                index.push_back(ul);
                index.push_back(ll);
                index.push_back(ur);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ur] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                index.push_back(ul);
                index.push_back(lr);
                index.push_back(ur);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ll] && defined[lr])
            {
                index.push_back(ul);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ur] && defined[ll] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                index.push_back(ur);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
        }

        // finish row
        int ul = row * num_columns + num_columns - 1;
        int ll = (row + 1) * num_columns + num_columns - 1;

        if(!break_flag && defined[ul] && defined[ll])
        {
            index.push_back(ul);
            index.push_back(ll);
        }

        if(!break_flag)
            index.push_back(0xFFFFFFFF);
        break_flag = true;
    }

    // generate required OpenGL structures
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size() +
        sizeof(glm::vec3) * normals.size(), NULL, GL_STATIC_DRAW);

    // store vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * coords.size(), coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), sizeof(glm::vec2) * tex_coords.size(), tex_coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size(),
        sizeof(glm::vec3) * normals.size(), normals.data());

    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size()));
    glEnableVertexAttribArray(2);

    _num_indexes = index.size();

    // generate grid lines
    std::vector<GLuint> grid_index;

    // horizontal pass
    for(size_t i = 1; i < 10; ++i)
    {
        for(size_t column = 0; column < num_columns; ++column)
        {
            GLuint ind = (int)((float)num_rows * (float)i / 10.0f) * num_columns + column;
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFFFFFF);
        }
        grid_index.push_back(0xFFFFFFFF);
    }

    //vertical pass
    for(size_t i = 1; i < 10; ++i)
    {
        for(size_t row = 0; row < num_rows; ++row)
        {
            GLuint ind = row * num_columns + (int)((float)num_columns * (float)i / 10.0f);
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFFFFFF);
        }
        grid_index.push_back(0xFFFFFFFF);
    }

    // generate & load required OpenGL structures
    glGenVertexArrays(1, &_grid_vao);
    glBindVertexArray(_grid_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glGenBuffers(1, &_grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * grid_index.size(), grid_index.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size()));
    glEnableVertexAttribArray(2);

    _grid_num_indexes = grid_index.size();

    // lines for normal vectors
    std::vector<glm::vec3> normal_coords;

    for(size_t i = 0; i < coords.size(); ++i)
    {
        if(defined[i])
        {
            normal_coords.push_back(coords[i]);
            normal_coords.push_back(coords[i] + 0.1f * normals[i]);
        }
    }

    // generate & load required OpenGL structures
    glGenVertexArrays(1, &_normal_vao);
    glBindVertexArray(_normal_vao);

    glGenBuffers(1, &_normal_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _normal_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal_coords.size(), normal_coords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    _normal_num_indexes = normal_coords.size();

    glBindVertexArray(0);
}
