/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   mlx_fragment.c                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: goosterl <goosterl@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/04/04 12:07:14 by goosterl      #+#    #+#                 */
/*   Updated: 2022/04/08 17:32:20 by goosterl      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "lib/GL/glew.h"
#include "lib/GLFW/glfw3.h"

typedef struct s_fragprog
{
	GLFWwindow	*gl_win;
	GLuint		shader;
	GLuint		vao;
	GLuint		vbo;
	GLuint		ssbo;
	void		*ssbo_data;
	GLuint		environment;
	GLuint		texID;
	int 		(*loop_func)(void *);
	void		*fparam;
	//	uniform
	float		res[2];
	float		time;
}	t_fragprog;

static GLboolean	init_glfw(void)
{
	glewExperimental = GL_TRUE;
	if (!glfwInit())
	{
		printf("glfw init failed\n");
		return (GL_FALSE);
	}
	
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	return (GL_TRUE);
}

static GLboolean	link_program(GLuint program_id)
{
	GLint	info_len;
	GLint	result;
	GLchar	*log;

	glLinkProgram(program_id);
	glGetProgramiv(program_id, GL_LINK_STATUS, &result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_len);
	if (info_len > 0)
	{
		log = (GLchar *)malloc(info_len + 1);
		glGetShaderInfoLog(program_id, info_len, NULL, &log[0]);
		printf("LINKER LOG:\n%s\n", log);
	}
	return (program_id * result);
}

static GLuint	compile_shader(const GLchar *shader_source, uint32_t type)
{
	GLuint	shader_id;
	GLint	result;
	GLint	info_len;
	GLchar	*log;

	// printf("SHADER_SRC: %s\n", shader_source);
	shader_id = glCreateShader(type);
	result = GL_FALSE;
	glShaderSource(shader_id, 1, &shader_source, NULL);
	glCompileShader(shader_id);
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &info_len);
	if (info_len > 0)
	{
		log = (GLchar *)malloc(info_len + 1);
		glGetShaderInfoLog(shader_id, info_len, NULL, &log[0]);
		printf("SHADER LOG:\n%s\n", log);
		free(log);
		
	}
	return (shader_id * result);
}

static void	set_res(GLFWwindow *win, int w, int h)
{
	(void)win;
	glViewport(0,0,w,h);
}

void		mlx_fragment_catch_pixels(t_fragprog *prog, int (*func)(void *))
{
	prog->loop_func = func;
}

t_fragprog	*mlx_new_fragment_program(int32_t width, int32_t height, const char *title, const char *shader_src)
{
	t_fragprog	*prog = (t_fragprog *)calloc(1, sizeof(t_fragprog));
	const char	*vert_src = "#version 330 core\n"
		"layout (location = 0) in vec2 pos;\n"
		"void main() {\n"
		"	gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}\n";
	init_glfw();
    prog->gl_win = glfwCreateWindow(width, height, "fract-ol", NULL, NULL);
    if (!prog->gl_win)
	{
        glfwTerminate();
        fprintf(stderr, "fatal error: Failed to create GLFW window\n");
        return (NULL);
    }
    glfwMakeContextCurrent(prog->gl_win);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printf("glew could not init\n");
		return (NULL);
	}
	glfwSetInputMode(prog->gl_win, GLFW_STICKY_KEYS, GL_TRUE);

    // glViewport(0, 0, width, height);
    // glfwSetFramebufferSizeCallback(prog->gl_win, set_res);
	// set_res(prog->gl_win, 512, 512);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    GLuint	vert_shader = compile_shader(vert_src, GL_VERTEX_SHADER);
	GLuint	frag_shader = compile_shader(shader_src, GL_FRAGMENT_SHADER);
	if (vert_shader == 0 || frag_shader == 0)
		return (NULL);
	prog->shader = glCreateProgram();
	glAttachShader(prog->shader, vert_shader);
	glAttachShader(prog->shader, frag_shader);
	if (link_program(prog->shader) == GL_FALSE)
		return (NULL);
	glDetachShader(prog->shader, vert_shader);
	glDetachShader(prog->shader, frag_shader);
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

    glGenVertexArrays(1, &prog->vao);
    glGenBuffers(1, &prog->vbo);
    glBindVertexArray(prog->vao);
    glBindBuffer(GL_ARRAY_BUFFER, prog->vbo);

    const static GLfloat vertices[] = {
		-1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,
		1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,  1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    prog->res[0] = (float)width;
	prog->res[1] = (float)height;
    glUniform2fv(glGetUniformLocation(prog->shader, "iResolution"), 1, &prog->res[0]);

	prog->ssbo_data = malloc(width * height * 4);
	glGenBuffers(1, &prog->ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, prog->ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, width * height * 4, prog->ssbo_data, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	
    // render loop
	return (prog);
}

int mlx_fragment_add_skybox(t_fragprog *prog, void *buffer, int width, int height, const char *name)
{
	glGenTextures(1, &prog->environment);
	glBindTexture(GL_TEXTURE_2D, prog->environment);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	prog->texID = (glGetUniformLocation(prog->shader, name), 0);
	return (1);
}

int	mlx_fragment_start_loop(t_fragprog *prog)
{
	float time  = 10.0f;
    while (glfwGetKey(prog->gl_win, GLFW_KEY_ESCAPE ) != GLFW_PRESS
		&& !glfwWindowShouldClose(prog->gl_win))
	{
		time += 0.05;
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);
        glBindVertexArray(prog->vao);
        glUseProgram(prog->shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prog->environment);
		glUniform1i(prog->texID, 0);
        glUniform1f(glGetUniformLocation(prog->shader, "iTime"), time);
        glUniform2fv(glGetUniformLocation(prog->shader, "iResolution"), 1, &prog->res[0]);
        glBindBuffer(GL_ARRAY_BUFFER, prog->vbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
		glfwSwapBuffers(prog->gl_win);
		// if (prog->loop_func != NULL)
		// {
		// 	int size = (int)prog->res[0] * (int)prog->res[1] * 3;
		// 	void	*pixbuffer = malloc(size);

		// 	GLint vp[4];
    	// 	glGetIntegerv(GL_VIEWPORT, vp);

		// 	glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// 	glReadBuffer(GL_FRONT);
		// 	glReadPixels(vp[0],
		// 		vp[1],
		// 		vp[2],
		// 		vp[3], GL_BGR, GL_UNSIGNED_BYTE, pixbuffer);
		// 	prog->loop_func(pixbuffer);
		// 	free(pixbuffer);
		// }
		
        
		
		
		time = fmod(time, 10000.f);
    }
    return (1);
}
