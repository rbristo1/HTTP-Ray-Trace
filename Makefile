
all: main.c
	gcc -lm -o main main.c vec3.c camera.c scene.c plugin.c sphere.c ray.c interval.c cube.c hit.c material.c

clean:
	rm main

