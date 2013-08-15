// window.cpp
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

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

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <cmath>

#include <png++/png.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <glibmm/main.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "SFMLWidget/SFMLWidget.h"

void check_error(const char * at)
{
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;
    std::cerr<<"OpenGL Error at "<<at<<": "<<gluErrorString(e)<<std::endl;
}

GLuint compile_shader(const char * filename, GLenum shader_type)
{
    std::ifstream in(filename, std::ios::binary | std::ios::in);
    std::vector <char> buff;

    if(in)
    {
        in.seekg(0, std::ios::end);
        size_t in_size = in.tellg();
        in.seekg(0, std::ios::beg);

        buff.resize(in_size + 1);
        buff.back() = '\0';
        in.read(&buff[0], in_size);

        if(!in)
            return 0;
    }
    else
        return 0;

    GLuint shader = glCreateShader(shader_type);

    const char * src = &buff[0];
    glShaderSource(shader, 1, &src, NULL);

    glCompileShader(shader);

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if(compile_status != GL_TRUE)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetShaderInfoLog(shader, log_length, NULL, &log[0]);

        std::cerr<<"Error compiling shader: "<<filename<<std::endl;
        std::cerr<<&log[0]<<std::endl;

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

GLuint link_shader_prog(const std::vector<GLuint> & shaders)
{
    GLuint prog = glCreateProgram();
    for(auto &i: shaders)
        glAttachShader(prog, i);

    glLinkProgram(prog);

    GLint link_status;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);

    if(link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetProgramInfoLog(prog, log_length, NULL, &log[0]);

        std::cerr<<"Error linking program: "<<std::endl;
        std::cerr<<&log[0]<<std::endl;

        glDeleteProgram(prog);

        return 0;
    }

    return prog;

}

std::vector<float> read_png(const char * filename)
{
    png::image<png::rgba_pixel> image(filename);
    std::vector<float> data(image.get_height() * image.get_width() * 4);

    for(size_t r = 0; r < image.get_height(); ++r)
    {
        for(size_t c = 0; c < image.get_width(); ++c)
        {
            data[(image.get_width() * r + c) * 4 + 0] = (float)image[r][c].red / 255.0f;
            data[(image.get_width() * r + c) * 4 + 1] = (float)image[r][c].green / 255.0f;
            data[(image.get_width() * r + c) * 4 + 2] = (float)image[r][c].blue / 255.0f;
            data[(image.get_width() * r + c) * 4 + 3] = (float)image[r][c].alpha / 255.0f;
        }
    }

    return data;
}

class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings= sf::ContextSettings()):
        SFMLWidget(mode, size_reqest), perspective(1.0f), shader_prog(0)

    {
        std::cout<<"constructing"<<std::endl;
        signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
        signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
        signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::rotate), 25);
        Glib::signal_idle().connect(sigc::mem_fun(*this, &Graph_disp::idle));

        rotation_y = 0.0f;
        rotation_z = 0.0f;

        // calculate normals
        for(auto &i: tet_coords)
        {
            tet_normals.push_back(-glm::normalize(i));
        }
    }

    ~Graph_disp()
    {
        if(shader_prog != 0)
            glDeleteProgram(shader_prog);
        if(shader_prog_line != 0)
            glDeleteProgram(shader_prog_line);
        // this is so bad...
        if(vao_line != 0)
            glDeleteVertexArrays(1, &vao_line);
        if(vao[0] != 0)
            glDeleteVertexArrays(4, &vao[0]);
        // if(buffer != 0)
        //     glInvalidateBufferData(buffer);
        glDeleteTextures(textures.size(), &textures[0]);
    }

    void realize()
    {
        // openGL initialization should go here
        std::cout<<"I just realized something..."<<std::endl;

        // init glew
        if(glewInit() != GLEW_OK)
        {
            std::cerr<<"Error loading glew. Aborting"<<std::endl;
            exit(EXIT_FAILURE);
        }

        // init GL state vars
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthRangef(0.0f, 1.0f);
        glLineWidth(5.0f);

        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // build shader programs
        GLuint vert_shader = compile_shader("src/tet.vert", GL_VERTEX_SHADER);
        GLuint frag_shader = compile_shader("src/tet.frag", GL_FRAGMENT_SHADER);

        if(frag_shader == 0 || vert_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog = link_shader_prog(std::vector<GLuint> {vert_shader, frag_shader});
        if(shader_prog == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        vert_shader = compile_shader("src/line.vert", GL_VERTEX_SHADER);
        frag_shader = compile_shader("src/line.frag", GL_FRAGMENT_SHADER);

        if(frag_shader == 0 || vert_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog_line = link_shader_prog(std::vector<GLuint> {vert_shader, frag_shader});
        if(shader_prog_line == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        // generate buffers for verts and colors
        vao.resize(5);
        buffer.resize(vao.size());
        glGenVertexArrays(vao.size(), &vao[0]);
        for(size_t i = 0; i < vao.size(); ++i)
        {
            glBindVertexArray(vao[i]);

            std::vector<glm::vec3> verts;
            std::vector<glm::vec2> texs;
            std::vector<glm::vec3> normals;

            switch(i)
            {
            case 0:
                for(int j = 0; j < 3; ++j)
                    normals.push_back(tet_normals[3]);
                texs.push_back(tex_coords[0]); texs.push_back(tex_coords[1]); texs.push_back(tex_coords[2]);
                verts.push_back(tet_coords[0]); verts.push_back(tet_coords[2]); verts.push_back(tet_coords[1]);
                break;
            case 1:
                for(int j = 0; j < 3; ++j)
                    normals.push_back(tet_normals[2]);
                texs.push_back(tex_coords[0]); texs.push_back(tex_coords[1]); texs.push_back(tex_coords[2]);
                verts.push_back(tet_coords[0]); verts.push_back(tet_coords[1]); verts.push_back(tet_coords[3]);
                break;
            case 2:
                for(int j = 0; j < 3; ++j)
                    normals.push_back(tet_normals[1]);
                texs.push_back(tex_coords[0]); texs.push_back(tex_coords[1]); texs.push_back(tex_coords[2]);
                verts.push_back(tet_coords[0]); verts.push_back(tet_coords[3]); verts.push_back(tet_coords[2]);
                break;
            case 3:
                for(int j = 0; j < 3; ++j)
                    normals.push_back(tet_normals[0]);
                texs.push_back(tex_coords[0]); texs.push_back(tex_coords[1]); texs.push_back(tex_coords[2]);
                verts.push_back(tet_coords[3]); verts.push_back(tet_coords[1]); verts.push_back(tet_coords[2]);
                break;
            case 4:
                // add the ground plane
                texs.push_back(glm::vec2(0.0f, 0.0f)); texs.push_back(glm::vec2(5.0f, 0.0f));
                texs.push_back(glm::vec2(5.0f, 5.0f)); texs.push_back(glm::vec2(0.0f, 5.0f));
                verts.push_back(glm::vec3(-10.0f, -1.0f, -10.0f)); verts.push_back(glm::vec3(10.0f, -1.0f, -10.0f));
                verts.push_back(glm::vec3(10.0f, -1.0f, 10.0f)); verts.push_back(glm::vec3(-10.0f, -1.0f, 10.0f));
                for(int j = 0; j < 4; ++j)
                    normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            }

            glGenBuffers(1, &buffer[i]);
            glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3)
                + texs.size() * sizeof(glm::vec2) + normals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(glm::vec3), &verts[0]);
            glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), texs.size() * sizeof(glm::vec2), &texs[0]);
            glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3) + texs.size() * sizeof(glm::vec2),
                normals.size() * sizeof(glm::vec3), &normals[0]);

            glVertexAttribPointer(vert_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(vert_pos);

            glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 0, (void *)(verts.size() * sizeof(glm::vec3)));
            glEnableVertexAttribArray(tex);

            glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 0, (void *)(verts.size() * sizeof(glm::vec3) + texs.size() * sizeof(glm::vec2)));
            glEnableVertexAttribArray(normal);
        }

        glGenVertexArrays(1, &vao_line);
        glBindVertexArray(vao_line);

        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> vert_colors;

        for(size_t i = 0; i < 4; ++i)
        {
            vert_colors.push_back(colors[CYAN]); vert_colors.push_back(colors[CYAN]);
            verts.push_back(glm::normalize(tet_coords[i]) * -sqrtf(6.0f) / 12.0f); verts.push_back(verts.back() + tet_normals[i] * 0.25f);

            for(size_t j = 0; j < 4; ++j)
            {
                if(j == i)
                    continue;
                vert_colors.push_back(colors[MAGENTA]); vert_colors.push_back(colors[MAGENTA]);
                verts.push_back(tet_coords[i]); verts.push_back(tet_coords[i] + tet_normals[j] * 0.25f);
            }

        }

        glGenBuffers(1, &buffer_line);
        glBindBuffer(GL_ARRAY_BUFFER, buffer_line);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3) + vert_colors.size() * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(glm::vec3), &verts[0]);
        glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), vert_colors.size() * sizeof(glm::vec4), &vert_colors[0]);

        glVertexAttribPointer(vert_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(vert_pos);

        glVertexAttribPointer(vert_color, 4, GL_FLOAT, GL_TRUE, 0, (void *)(verts.size() * sizeof(glm::vec3)));
        glEnableVertexAttribArray(vert_color);

        // load images
        glEnable(GL_TEXTURE_2D);

        std::vector<std::vector<float>> texture_data;

        try
        {
            texture_data.push_back(read_png("img/tet-red.png"));
            texture_data.push_back(read_png("img/tet-green.png"));
            texture_data.push_back(read_png("img/tet-blue.png"));
            texture_data.push_back(read_png("img/tet-yellow.png"));
            texture_data.push_back(read_png("img/floor.png"));
        }
        catch(std::exception &e)
        {
            std::cerr<<"Error reading image file: "<<e.what()<<std::endl;
            exit(EXIT_FAILURE);
        }

        textures.resize(texture_data.size());
        glGenTextures(textures.size(), &textures[0]);

        check_error("after gen");

        for(size_t i = 0; i < textures.size(); ++i)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexStorage2D(GL_TEXTURE_2D, (int)(log2(512)) + 1, GL_RGBA8, 512, 512);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGBA, GL_FLOAT, &texture_data[i][0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        std::cout<<"OpenGL version: "<<glGetString(GL_VERSION)<<std::endl;
        std::cout<<"GLSL version: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
        std::cout<<"Vendor: "<<glGetString(GL_VENDOR)<<std::endl;
        std::cout<<"Renderer: "<<glGetString(GL_RENDERER)<<std::endl;

        int bits;
        glGetIntegerv(GL_MAJOR_VERSION, &bits);
        std::cout<<"Major version: "<<bits<<std::endl;
        glGetIntegerv(GL_MINOR_VERSION, &bits);
        std::cout<<"Minor version: "<<bits<<std::endl;

        glGetIntegerv(GL_SAMPLES, &bits);
        std::cout<<"Samples: "<<bits<<std::endl;
    }

    void resize(Gtk::Allocation & allocation)
    {
        std::cout<<"allocating"<<std::endl;

        if(m_refGdkWindow)
        {
            glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);

            float aspect = (float)glWindow.getSize().x / (float)glWindow.getSize().y;
            perspective = glm::perspective(30.0f, aspect, 0.1f, 1000.0f);

            invalidate();
        }
    }

    bool draw(const Cairo::RefPtr<Cairo::Context> & cr)
    {
        // std::cout<<"drawing"<<std::endl;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 rotate_z_mat = glm::rotate(glm::mat4(), rotation_z, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 rotate_y_mat = glm::rotate(glm::mat4(), rotation_y, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 view = glm::lookAt(cam.pos, cam.forward, cam.up);

        // glm::vec3 light_pos_eye = glm::vec3(0.0f, 0.0f, -9.0f);
        glm::vec3 light_pos_eye = glm::vec3(perspective * view * glm::vec4(light_pos, 1.0f));

        glUseProgram(shader_prog);
        glUniform3fv(glGetUniformLocation(shader_prog, "ambient_color"), 1, &ambient_color[0]);
        glUniform3fv(glGetUniformLocation(shader_prog, "light_color"), 1, &light_color[0]);
        glUniform3fv(glGetUniformLocation(shader_prog, "light_pos"), 1, &light_pos_eye[0]);
        glUniform3fv(glGetUniformLocation(shader_prog, "cam_forward"), 1, &cam.forward[0]);
        glUniform1f(glGetUniformLocation(shader_prog, "light_shiny"), light_shiny);
        glUniform1f(glGetUniformLocation(shader_prog, "light_strength"), light_strength);
        glUniform1f(glGetUniformLocation(shader_prog, "const_atten"), const_atten);
        glUniform1f(glGetUniformLocation(shader_prog, "linear_atten"), linear_atten);
        glUniform1f(glGetUniformLocation(shader_prog, "quad_atten"), quad_atten);

        // draw ground
        glm::mat4 view_model = view;
        glm::mat4 view_model_perspective = perspective * view_model;
        glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

        glUniformMatrix4fv(glGetUniformLocation(shader_prog, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_prog, "view_model"), 1, GL_FALSE, &view_model[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(shader_prog, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

        glBindVertexArray(vao.back());
        glBindTexture(GL_TEXTURE_2D, textures[vao.size() - 1]);
        glDrawArrays(GL_QUADS, 0, 4);

        view_model = view * rotate_z_mat * rotate_y_mat;
        view_model_perspective = perspective * view_model;
        normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

        glUniformMatrix4fv(glGetUniformLocation(shader_prog, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_prog, "view_model"), 1, GL_FALSE, &view_model[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(shader_prog, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

        check_error("draw");

        for(size_t i = 0; i < vao.size() - 1; ++i)
        {
            glBindVertexArray(vao[i]);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glUseProgram(shader_prog_line);
        glUniformMatrix4fv(glGetUniformLocation(shader_prog_line, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
        glBindVertexArray(vao_line);
        glDrawArrays(GL_LINES, 0, 32);

        display();
        return true;
    }

    bool idle()
    {
        if(dynamic_cast<Gtk::Window *>(get_toplevel())->is_active())
        {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                std::cout<<"You are having a bad problem"<<std::endl;
        }
        return true;
    }

    bool rotate()
    {
        rotation_z += .8;
        if(rotation_z > 360.0f)
            rotation_z -= 360.0f;

        rotation_y += .4f;
        if(rotation_y > 360.0f)
            rotation_y -= 360.0f;

        invalidate();
        return true;
    }

private:
    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator=(const Graph_disp &) = delete;
    Graph_disp & operator=(const Graph_disp &&) = delete;

    float rotation_y;
    float rotation_z;

    glm::mat4 perspective;

    struct
    {
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, -3.0f);
        glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    } cam;

    GLuint shader_prog;
    GLuint shader_prog_line;

    std::vector<GLuint> vao;
    GLuint vao_line;
    std::vector<GLuint> buffer;
    GLuint buffer_line;

    enum {vert_pos = 0, tex, normal};
    const GLuint vert_color = 1;

    // coords are X,Z,Y in sane-coords™
    const std::vector<glm::vec3> tet_coords =
    {
        glm::vec3(0.0f, sqrtf(6.0f) / 4.0f, 0.0f),
        glm::vec3(-0.5f, -sqrtf(6.0f) / 12.0f, -sqrtf(3.0f) / 6.0f),
        glm::vec3(0.5f, -sqrtf(6.0f) / 12.0f, -sqrtf(3.0f) / 6.0f),
        glm::vec3(0.0f, -sqrtf(6.0f) / 12.0f, sqrtf(3.0f) / 3.0f)
    };

    const std::vector<glm::vec4> colors =
    {
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.1f, 0.5f, 0.1f, 1.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    enum {RED = 0, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};

    const std::vector<glm::vec2> tex_coords = 
    {
        glm::vec2(0.5f, 1.0f - sqrtf(3.0f) / 2.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f)
    };

    std::vector<GLuint> textures;

    std::vector<glm::vec3> tet_normals;

    glm::vec3 ambient_color = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 light_pos = glm::vec3(-0.5f, 1.0f, -1.0f);
    float light_shiny = 0.8f;
    float light_strength = 0.8f;
    float const_atten = 1.0f;
    float linear_atten = 0.5f;
    float quad_atten = 0.0f;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window gtk_window;
    Gtk::VBox main_box;

    Graph_disp gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)); // these do nothing yet - future SFML version should enable them

    gl_window.show();
    main_box.pack_start(gl_window);
    main_box.show();

    gtk_window.add(main_box);

    Gtk::Main::run(gtk_window);
    return 0;
}
