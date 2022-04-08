make -C minilibx_opengl
gcc main.c	\
	minilibx_opengl/libmlx.a	\
	libGLEW.2.2.0.dylib \
	libglfw.3.3.dylib \
	-framework AppKit -framework OpenGL
