/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   bitmap.h                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: goosterl <goosterl@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/04/05 13:29:02 by goosterl      #+#    #+#                 */
/*   Updated: 2022/04/05 13:31:32 by goosterl      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITMAP_H
# define BITMAP_H
# include <stdint.h>

#pragma pack(push,1)

//	Assuming windows standard

typedef struct s_bitmap
{
	uint8_t			id[2];
	uint32_t		file_size;
	uint32_t		reserved;
	uint32_t		offset;		// DIB size
	uint32_t		size;
	uint32_t		width;
	uint32_t		height;
	uint16_t		color_planes;
	uint16_t		bits_per_pixel;
	uint32_t		compression;
	uint32_t		image_size;
	uint32_t		hres;
	uint32_t		vres;
	uint32_t		colors;
	uint32_t		important_colors;
	int32_t			_GL_BUFFER_ID;
}	t_bitmap;

#pragma pack(pop)

#endif
