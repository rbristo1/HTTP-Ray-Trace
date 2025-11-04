
all: src/main.c
	gcc -pthread -g -lm -o main src/main.c src/vec3.c src/camera.c src/scene.c src/plugin.c src/sphere.c src/ray.c src/interval.c src/cube.c src/hit.c src/material.c src/ring.c src/http.c

clean:
	rm main

