//  mlx_shaders.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <OpenGL/gl3.h>
#include "mlx_int.h"


void display_log(GLuint object, void (*param_func)(), void (*getlog_func)())
{
  GLint log_length;
  char *log;

  param_func(object, GL_INFO_LOG_LENGTH, &log_length);
  log = malloc(log_length);
  getlog_func(object, log_length, NULL, log);
  fprintf(stderr, "%s", log);
  free(log);
}


int mlx_shaders_pixel(glsl_info_t *glsl)
{
  char  *source;
  int	length;
  GLint action_ok;

  glsl->pixel_vshader = glCreateShader(GL_VERTEX_SHADER);
  source = strdup("#version 110 \n"
		  "attribute vec2 position;"
		  "varying vec2 texcoord;"
		  "void main()"
		  "{"
		  " gl_Position = vec4( position, 0.0, 1.0);"
		  " texcoord = vec2(position[0]+1.0, 1.0 - position[1]) / 2.0;"
		  "}");
  length = strlen(source);
  glShaderSource(glsl->pixel_vshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->pixel_vshader);
  free(source);

  glGetShaderiv(glsl->pixel_vshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile pixel vshader :\n");
    display_log(glsl->pixel_vshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }

  glsl->pixel_fshader = glCreateShader(GL_FRAGMENT_SHADER);
  source = strdup("#version 110 \n"
		  "uniform sampler2D texture;"
		  "varying vec2 texcoord;"
		  "void main()"
		  "{"
		  " gl_FragColor = vec4(1, 0, 1, 0);"
		  "}");
  length = strlen(source);
  glShaderSource(glsl->pixel_fshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->pixel_fshader);
  free(source);

  glGetShaderiv(glsl->pixel_fshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile pixel fshader :\n");
    display_log(glsl->pixel_fshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }

  glsl->pixel_program = glCreateProgram();
  glAttachShader(glsl->pixel_program, glsl->pixel_vshader);
  glAttachShader(glsl->pixel_program, glsl->pixel_fshader);
  glLinkProgram(glsl->pixel_program);

  glGetProgramiv(glsl->pixel_program, GL_LINK_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to link pixel shader program:\n");
    display_log(glsl->pixel_program, glGetProgramiv, glGetProgramInfoLog);
    return (1);
  }

  glFlush();

  return (0);
}


int mlx_shaders_image(glsl_info_t *glsl)
{
  char  *source;
  int	length;
  GLint action_ok;

  glsl->image_vshader = glCreateShader(GL_VERTEX_SHADER);
  source = strdup("#version 110 \n"
		  "attribute vec2 position;"
		  "uniform vec2 winhalfsize;"
		  "uniform vec2 imagepos;"
		  "uniform vec2 imagesize;"
		  "varying vec2 texcoord;"
		  "void main()"
		  "{"
		  " texcoord = position / imagesize;"
		  " vec2 pos = position - winhalfsize + imagepos;"
		  " pos = pos / winhalfsize;"
		  " gl_Position = vec4( pos, 0.0, 1.0);"
		  "}");
  length = strlen(source);
  glShaderSource(glsl->image_vshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->image_vshader);
  free(source);

  glGetShaderiv(glsl->image_vshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile image vshader :\n");
    display_log(glsl->image_vshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }
	//	main
  glsl->image_fshader = glCreateShader(GL_FRAGMENT_SHADER);
  source = strdup("#version 110 \n"
		  "uniform sampler2D texture;"
		  "uniform float iTime;"
		  "varying vec2 texcoord;"
			"float mandelbulb_sdf(vec3 pos) {"
			"	float Power = 10.0;"
    		"	if(length(pos) > 1.5) return length(pos) - 1.2;"
			"	vec3 z = pos;"
			"	float dr = 1.0, r = 0.0, theta, phi;"
			"	for (int i = 0; i < 100; i++) {"
			"		r = length(z);"
			"		if (r>1.5) break;"
			"		dr =  pow( r, Power-1.0)*Power*dr + 1.0;"
			"		theta = acos(z.z/r) * Power + iTime;"
			"		phi = atan(z.y,z.x) * Power - iTime;"
			"		float sinTheta = sin(theta);"
			"		z = pow(r,Power) * vec3(sinTheta*cos(phi), sinTheta*sin(phi), cos(theta)) + pos;"
			"	}"
			"	return 0.5*log(r)*r/dr;"
			"}"
			
			"float get_dist(vec3 p)"
			"{"
			"	float t = 11.0;"
			"	vec3 s = vec3(0, 1, 3);"
				
			"	float r = 1.0;"
			"	float sd = length(p - s.xyz) - r;"
			"	return (mandelbulb_sdf(p));"
			"	float pd = p.y;"
			"	return min(sd, pd);"
			"}"
			
			"vec3 get_normal(vec3 p)"
			"{"
			"	vec2 e = vec2(0.1, 0);"
			"	float d = get_dist(p);"
			"	vec3 n = vec3("
			"		d-get_dist(p-e.xyy),"
			"		d-get_dist(p-e.yxy),"
			"		d-get_dist(p-e.yyx)"
			"	);"
			"	return normalize(n);"
			"}"
			
			"float march(vec3 ro, vec3 rd)"
			"{"
			"	float steps = 0.;"
			"	float total_dist = 0.0;"
			"	for(int i = 0; i < 100; i++)"
			"	{"
			"		steps += 1.;"
			"		vec3 p = ro + rd * total_dist;"
			"		float dist = get_dist(p);"
			"		total_dist += dist;"
			"		if(dist < 0.0000001 || total_dist > 400.0)"
			"		{	break ;	}"
			"	}"
			"	return total_dist;"
			"}"
			
			"void main()"
			"{"
			"	mat3 lookat_dir;"
			"	vec2 uv = (texcoord - 0.5) * 5.0;"
			"	float rotfact = iTime / 10.0;"
			"	vec3 ro = vec3(-sin(rotfact), 0, cos(rotfact)) * 1.5;"
			"	ro = vec3(1, 0, 1);"
			"	vec3 lookat = normalize(ro);"
			"	vec3 side = cross(lookat, vec3(0,1,0));"
			"	vec3 up = vec3(0, 1, 0);"
			"	lookat_dir[0] = side;"
			"	lookat_dir[1] = up;"
			"	lookat_dir[2] = lookat;"
  			"	vec3 rd = normalize(vec3(uv.x, uv.y, -1));"
			"	rd *= lookat_dir;"
			"	float dst = march(ro, rd) / 2.0;"
			"	vec3 ld = vec3(cos(iTime), 0, sin(iTime));"
			"	vec3 phit = ro + rd * dst;"
			"	float amt = dot(-ld, get_normal(phit));"
			"	float cosang = pow(dot(-ld, get_normal(phit)), 128.0);"
			"	amt += cosang;"
			"	gl_FragColor = vec4(amt, cosang, amt, 0.0);"
			"}");

// void	mat_lookat(t_mat dst, t_vec origin, t_vec look_at, t_vec head)
// {
// 	t_vec	forward;
// 	t_vec	side;
// 	t_vec	up;

// 	forward = normalize(look_at - origin);
// 	side = normalize(cross(forward, head));
// 	up = cross(side, forward);

// 	mat_init(dst);
// 	dst[0][0] = side[0];
// 	dst[1][0] = side[1];
// 	dst[2][0] = side[2];
// 	dst[0][1] = up[0];
// 	dst[1][1] = up[1];
// 	dst[2][1] = up[2];
// 	dst[0][2] = -forward[0];
// 	dst[1][2] = -forward[1];
// 	dst[2][2] = -forward[2];
// 	dst[3][0] = -dot(side, origin);
// 	dst[3][1] = -dot(up, origin);
// 	dst[3][2] = dot(forward, origin);
// }

  length = strlen(source);
  glShaderSource(glsl->image_fshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->image_fshader);
  free(source);

  glGetShaderiv(glsl->image_fshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile image fshader :\n");
    display_log(glsl->image_fshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }

  glsl->image_program = glCreateProgram();
  glAttachShader(glsl->image_program, glsl->image_vshader);
  glAttachShader(glsl->image_program, glsl->image_fshader);
  glLinkProgram(glsl->image_program);

  glGetProgramiv(glsl->image_program, GL_LINK_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to link image shader program:\n");
    display_log(glsl->image_program, glGetProgramiv, glGetProgramInfoLog);
    return (1);
  }

  glFlush();

  return (0);
}




int mlx_shaders_font(glsl_info_t *glsl)
{
  char  *source;
  int	length;
  GLint action_ok;

  glsl->font_vshader = glCreateShader(GL_VERTEX_SHADER);
  source = strdup("#version 110 \n"
		  "attribute vec2 position;"
		  "uniform vec2 winhalfsize;"
		  "uniform vec2 fontposinwin;"
		  "uniform vec2 fontposinatlas;"
		  "uniform vec2 fontatlassize;"
		  "varying vec2 texcoord;"
		  "void main()"
		  "{"
#ifdef STRINGPUTX11
		  " texcoord = (position * vec2(1.4, -1.4) + fontposinatlas ) / fontatlassize;"
#else
		  " texcoord = (position * vec2(1.0, -1.0) + fontposinatlas ) / fontatlassize;"
#endif
		  " vec2 pos = position - winhalfsize + fontposinwin;"
		  " pos = pos / winhalfsize;"
		  " gl_Position = vec4( pos, 0.0, 1.0);"
		  "}");
  length = strlen(source);
  glShaderSource(glsl->font_vshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->font_vshader);
  free(source);

  glGetShaderiv(glsl->font_vshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile font vshader :\n");
    display_log(glsl->font_vshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }

  glsl->font_fshader = glCreateShader(GL_FRAGMENT_SHADER);
  source = strdup("#version 110 \n"
		  "uniform sampler2D texture;"
		  "uniform vec4 color;"
		  "varying vec2 texcoord;"
		  "void main()"
		  "{"
		  " gl_FragColor = color * vec4(0,0,1, 0);"
		  "}");
  length = strlen(source);
  glShaderSource(glsl->font_fshader, 1, (const GLchar**)&source, &length);
  glCompileShader(glsl->font_fshader);
  free(source);

  glGetShaderiv(glsl->font_fshader, GL_COMPILE_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to compile font fshader :\n");
    display_log(glsl->font_fshader, glGetShaderiv, glGetShaderInfoLog);
    return (1);
  }

  glsl->font_program = glCreateProgram();
  glAttachShader(glsl->font_program, glsl->font_vshader);
  glAttachShader(glsl->font_program, glsl->font_fshader);
  glLinkProgram(glsl->font_program);

  glGetProgramiv(glsl->font_program, GL_LINK_STATUS, &action_ok);
  if (!action_ok) {
    fprintf(stderr, "Failed to link font shader program:\n");
    display_log(glsl->font_program, glGetProgramiv, glGetProgramInfoLog);
    return (1);
  }

  glFlush();

  return (0);
}



int mlx_shaders(glsl_info_t *glsl)
{
  return (mlx_shaders_pixel(glsl) + mlx_shaders_image(glsl) + mlx_shaders_font(glsl));
}
