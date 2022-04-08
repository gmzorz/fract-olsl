# fract-olsl
fract-ol project (42/Codam) bonus and showcase entry

Realtime 3D fractal renderer with attempted global illumination. Bit noisy but 60fps! Also did not realise it looped perfectly.

## usage

`./compile.sh`, then `./a.out`.

[VIDEO](https://www.youtube.com/watch?v=rUWgVIFgvHs)

## images

<p align="center">
<img src="images/one.png" width="512" height="512"></img>
</p>

<p align="center">
<img src="images/two.png" width="512" height="512"></img>
</p>

## todo

output backbuffer to video file (using [libgmavi](https://github.com/gmzorz/libgmavi)). The retrieval of pixels presented a bigger challenge than rendering a mandelbulb.
