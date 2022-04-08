/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: goosterl <goosterl@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/04/01 11:46:56 by goosterl      #+#    #+#                 */
/*   Updated: 2022/04/08 17:35:09 by goosterl      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "minilibx_opengl/mlx.h"
#include "bitmap.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char	*get_shader_source(const char *file_path)
{
	size_t	file_size;
	FILE	*shader_stream;
	char	*shader_src;

	shader_stream = fopen(file_path, "rb");
	shader_src = NULL;
	if (shader_stream != NULL)
	{
		fseek(shader_stream, 0, SEEK_END);
		file_size = ftell(shader_stream);
		fseek(shader_stream, 0, SEEK_SET);
		shader_src = (char *)calloc(1, file_size + 1);
		if (shader_src)
			fread(shader_src, 1, file_size, shader_stream);
		shader_src[file_size] = '\0';
		fclose(shader_stream);
		return (shader_src);
	}
	printf("Could not locate shader source\n");
	return (NULL);
}

static uint8_t	*get_file_source(FILE *stream)
{
	size_t	file_size;
	uint8_t	*source = NULL;

	fseek(stream, 0, SEEK_END);
	file_size = ftell(stream);
	fseek(stream, 0, SEEK_SET);
	source = (uint8_t *)calloc(1, file_size + 1);
	if (source)
		fread(source, 1, file_size, stream);
	source[file_size] = '\0';
	fclose(stream);
	return (source);
}

t_bitmap	load_bitmap(const char *file_path, void **buffer)
{
	t_bitmap	bitmap;
	void		*bitmap_ptr;
	uint8_t		*file_source;
	FILE		*file_stream;
	void		*buf;

	memset(&bitmap, 0, sizeof(t_bitmap));
	if (file_path == NULL)
		return (bitmap);
	file_stream = fopen(file_path, "rb");
	if (file_stream == NULL)
	{
		printf("Could not locate env.bmp...\n");
		return (bitmap);
	}
	file_source = get_file_source(file_stream);
	bitmap_ptr = &bitmap;
	if (file_source[0] == 'B' && file_source[1] == 'M')
	{
		memcpy(bitmap_ptr, file_source, 13);
		memcpy((bitmap_ptr) + 13, file_source + 13, 40);
		if (bitmap.bits_per_pixel != 32)
		{
			printf("Sorry! no 24bpp bitmaps supported at the moment...");
			return (bitmap);
		}
		buf = (uint8_t *)malloc(bitmap.image_size);
		memcpy(buf, file_source + bitmap.offset, bitmap.image_size);
	}
	else
		printf("NOT A VALID BITMAP\n");
	*buffer = buf;
	return (bitmap);
}

int	main(void)
{
	t_bitmap	env_bm;
	void		*env_buffer = NULL;

	char *file_src = get_shader_source("fract-ol.glsl");
	env_bm = load_bitmap("env.bmp", &env_buffer);
	if (env_bm.bits_per_pixel != 32)
		return (0);
	void *obj = mlx_new_fragment_program(512, 512, "AAAA", file_src);
	mlx_fragment_add_skybox(obj, env_buffer, env_bm.width, env_bm.height, "texture1");
	mlx_fragment_start_loop(obj);

	return (1);
}
