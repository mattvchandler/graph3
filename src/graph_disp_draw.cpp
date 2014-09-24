// graph_disp_draw.cpp
// Graphics display drawing code

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

#include "gl_helpers.hpp"
#include "graph_disp.hpp"

void Cursor::draw() const
{
    // load the vertexes and texture
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D, _tex);

    // draw the geometry
    glDrawArrays(GL_TRIANGLES, 0, _num_indexes);
}

void Axes::draw() const
{
    // load the vertices and texture
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // draw the geometry
    glDrawArrays(GL_LINES, 0, _num_indexes);
}

void Graph_disp::graph_draw_setup(std::unordered_map<std::string, GLuint> & uniforms,
    const Graph & graph, const glm::mat4 & view_model_perspective, const glm::mat4 & view_model,
    const glm::mat3 & normal_transform, const glm::vec3 & dir_light_dir, const glm::vec3 & dir_half_vec)
{
    glUniformMatrix4fv(uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
    glUniformMatrix4fv(uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
    glUniformMatrix3fv(uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

    // material properties
    if(!(graph.use_tex && graph.valid_tex))
    {
        glUniform4fv(uniforms["color"], 1, &graph.color[0]);
    }
    glUniform1f(uniforms["shininess"], graph.shininess);
    glUniform3fv(uniforms["specular"], 1, &graph.specular[0]);
    glUniform3fv(uniforms["dir_light_dir"], 1, &dir_light_dir[0]);
    glUniform3fv(uniforms["dir_half_vec"], 1, &dir_half_vec[0]);

    // light properties
    glUniform3fv(uniforms["cam_light_color"], 1, &cam_light.color[0]);
    glUniform1f(uniforms["cam_light_strength"], cam_light.strength);
    glUniform1f(uniforms["const_atten"], cam_light.const_atten);
    glUniform1f(uniforms["linear_atten"], cam_light.linear_atten);
    glUniform1f(uniforms["quad_atten"], cam_light.quad_atten);
    glUniform3fv(uniforms["dir_light_color"], 1, &dir_light.color[0]);
    glUniform1f(uniforms["dir_light_strength"], dir_light.strength);
}

// main drawing code
bool Graph_disp::draw(const Cairo::RefPtr<Cairo::Context> & unused)
{
    // set up global lights
    glClearColor(bkg_color.r, bkg_color.g, bkg_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_prog_tex);
    glUniform3fv(_prog_tex_uniforms["ambient_color"], 1, &ambient_color[0]);

    glUseProgram(_prog_color);
    glUniform3fv(_prog_color_uniforms["ambient_color"], 1, &ambient_color[0]);

    // set up viewmodel matrices
    glm::mat4 view_model;
    if(use_orbit_cam)
    {
        view_model = glm::rotate(glm::rotate(glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -_orbit_cam.r)),
            -_orbit_cam.phi, glm::vec3(1.0f, 0.0f, 0.0f)), -_orbit_cam.theta, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else
    {
        view_model = _cam.view_mat();
    }
    view_model = glm::scale(view_model, glm::vec3(_scale));

    glm::mat4 view_model_perspective = _perspective * view_model;
    glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

    glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

    // directional light
    glm::vec3 dir_light_dir = normal_transform * glm::normalize(-dir_light.pos);
    glm::vec3 dir_half_vec = glm::normalize(light_forward + dir_light_dir);

    // draw axes
    if(draw_axes_flag)
    {
        glUseProgram(_prog_line);

        glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
        glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
        glUniform4fv(_prog_line_uniforms["color"], 1, &_axes.color[0]);

        _axes.draw();

        check_error("draw axes");
    }

    // draw opaque graphs
    for(auto &graph: _graphs)
    {
        // draw geometry
        if(graph->draw_flag && !graph->transparent_flag)
        {
            if(graph->use_tex && graph->valid_tex)
            {
                // draw with texture
                glUseProgram(_prog_tex);
                graph_draw_setup(_prog_tex_uniforms, *graph,
                    view_model_perspective, view_model, normal_transform,
                    dir_light_dir, dir_half_vec);
            }
            else
            {
                // draw with one color
                glUseProgram(_prog_color);
                graph_draw_setup(_prog_color_uniforms, *graph,
                    view_model_perspective, view_model, normal_transform,
                    dir_light_dir, dir_half_vec);
            }
            check_error("draw geometry");

            graph->draw();
        }

        // draw grid
        if(graph->draw_grid_flag && !graph->transparent_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->grid_color[0]);

            graph->draw_grid();
            check_error("draw grid");
        }

        // draw normal vectors
        if(graph->draw_normals_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->normal_color[0]);

            graph->draw_normals();
            check_error("draw normals");
        }
    }

    // draw cursor
    if(draw_cursor_flag && _active_graph && _active_graph->cursor_defined())
    {
        glUseProgram(_prog_tex);

        glm::mat4 cursor_view_model = glm::scale(glm::translate(view_model, _active_graph->cursor_pos()), glm::vec3(0.25f / _scale));
        glm::mat4 cursor_view_model_perspective = _perspective * cursor_view_model;
        glm::mat3 cursor_normal_transform = glm::transpose(glm::inverse(glm::mat3(cursor_view_model)));

        glUniformMatrix4fv(_prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &cursor_view_model_perspective[0][0]);
        glUniformMatrix4fv(_prog_tex_uniforms["view_model"], 1, GL_FALSE, &cursor_view_model[0][0]);
        glUniformMatrix3fv(_prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &cursor_normal_transform[0][0]);

        // material properties
        glUniform1f(_prog_tex_uniforms["shininess"], _cursor.shininess);
        glUniform3fv(_prog_tex_uniforms["specular"], 1, &_cursor.specular[0]);
        glUniform3fv(_prog_tex_uniforms["dir_light_dir"], 1, &dir_light_dir[0]);
        glUniform3fv(_prog_tex_uniforms["dir_half_vec"], 1, &dir_half_vec[0]);

        _cursor.draw();
        check_error("draw cursor");
    }

    // 2nd pass to draw transparent graphs
    for(auto &graph: _graphs)
    {
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
        // draw geometry
        if(graph->draw_flag && graph->transparent_flag)
        {
            if(graph->use_tex && graph->valid_tex)
            {
                // draw with texture
                glUseProgram(_prog_tex);
                graph_draw_setup(_prog_tex_uniforms, *graph,
                    view_model_perspective, view_model, normal_transform,
                    dir_light_dir, dir_half_vec);
            }
            else
            {
                // draw with one color
                glUseProgram(_prog_color);
                graph_draw_setup(_prog_color_uniforms, *graph,
                    view_model_perspective, view_model, normal_transform,
                    dir_light_dir, dir_half_vec);
            }
            check_error("draw transparent geometry");

            graph->draw();
        }

        // draw grid
        if(graph->draw_grid_flag && graph->transparent_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->grid_color[0]);

            graph->draw_grid();
            check_error("draw grid");
        }
        glDepthMask(GL_TRUE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    display(); // swap display buffers
    return true;
}
