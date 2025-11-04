#include "http.h"
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ring.h"
#include <time.h>
#include "vec3.h"
#include "material.h"
#include "scene.h"
#include "cube.h"
#include "sphere.h"
#include "camera.h"
#include "plugin.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#define SEED time(NULL)

void intToStr(uint64_t num, char *str) {
    sprintf(str, "%d", num);
}

void server_start() {
    
    srand(SEED);
    int Connection = 1;
    int back = 5;
    int s, c;
    struct sockaddr_in sin = {0};
    struct sockaddr_in cin = {0};
    s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
        perror("socket");
        return;
    }

    //memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8177);
    unsigned int a = 0;
    //localhost:8177
    a |= (127 << 24);
    a |= (0 << 16);
    a |= (0 << 8);
    a |= 1;
    sin.sin_addr.s_addr = htonl(a);
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
        perror("bind");
        close(s);
        return;
    }
    
    if (listen(s, back) == -1) {
        perror("listen");
        close(s);
        return;
    }
    socklen_t temp = sizeof(cin);
    while(Connection == 1) {
    c = accept(s, (struct sockaddr*)&cin, &temp);
    if (c == -1) {
        perror("accept");
        close(s);
        return;
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
    ssize_t n;
    
    int request = 0;
    int header = 0;
    int data = 0;
    char Accept[256];
    char ContType[256];
    int Content_Length = 0;
    
    struct Camera cam;
    struct Scene *scene;
    int camera_dimensions = 0;
    int camera_look_from = 0;
    int camera_look_at = 0;
    int camera_up = 0;
    int camera_max_depth = 0;
    int camera_vfov = 0;
    int camera_focus_dist = 0;
    int scene_bg = 0;
    int num_cubes = 0;
    int num_spheres = 0;
    int max_cubes = 5;
    int max_spheres = 5;
    struct Cube * cu = malloc(sizeof(struct Cube)*5);
    struct Sphere * sp = malloc(sizeof(struct Sphere)*5);
    int threads = 0;
    int thread_num;
    int samples = 0;
    int samp_num;
    int sampMeth = 0;
    SamplingStrategy sampMethType;
    Pixel *pixels;
    RingBuffer * rb = rb_new(256);
    char buf[256];


    while ((n = recv(c, buf, 256, 0)) > 0) {
        char parse[256] = "";
        rb_write(rb, buf, n);
        if (request == 0) {
            for(int i  = 0; i < 256; i++) {
                if (!rb_pop(rb, &parse[i])) {
                    n = recv(c, buf, 256, 0);
                    rb_write(rb, buf, n);
                    if (n == 0) {
                        perror("request");
                        return;
                    }
                }
                else {
                    if (parse[i] == '\n') {
                        request = 1;
                        break;
                    }
                }
                
            }
            memset(parse, 0, sizeof(parse));
        }

        if (header == 0) {
            for(int i = 0; i < 256; i++) {
                if (!rb_pop(rb, &parse[i])) {
                    n = recv(c, buf, 256, 0);
                    rb_write(rb, buf, n);
                    if (n == 0) {
                        perror("header");
                        return;
                    }
                }
                else {
                    if (parse[i] == '\n') {
                        parse[i+1] = '\0';
                        if (strcmp(parse, "\r\n") == 0) {
                            header = 1;
                            break;
                        }
                        else {
                            char dataType[256], dataVal[256];
                            int ret;
                            ret = sscanf(parse, "%s %s", dataType, dataVal);
                            printf("dataType: %s\n", dataType);
                            printf("dataVal: %s\n", dataVal);

                            if (strcmp(dataType, "Accept:") == 0) {
                                strcpy(ContType, dataVal);
                                if (strcmp(dataVal, "image/bmp") == 0) {
                                    strcpy(Accept, "bmp");
                                }
                                else if (strcmp(dataVal, "image/x-raytrace-output") == 0) {
                                    strcpy(Accept, "rto");
                                }
                                else if (strcmp(dataVal, "image/x-portable-pixmap") == 0) {
                                    strcpy(Accept, "ppm");
                                }
                                else {
                                    strcpy(Accept, "err");
                                }
                            }
                            else if (strcmp(dataType, "Content-Length:") == 0) {
                                ret = sscanf(dataVal, "%d", &Content_Length);
                                printf("length: %d\n", Content_Length);
                            }
                            else if (strcmp(dataType, "Connection:") == 0) {
                                if(strcmp(dataVal, "keep-alive") == 0) {
                                    Connection = 1;
                                    printf("Connection: c\n");
                                }
                                else {
                                    Connection = 0;
                                    printf("Connection: dc\n");
                                }
                            }
                            
                        }
                        memset(parse, 0, sizeof(parse));
                        i = -1;
                    }
                }
                
            }
            char err2[64] = "HTTP/1.1 411 Length Required\r\n";
            char err3[64] = "HTTP/1.1 415 Unsupported Media Type\r\n";
            if (Content_Length == 0) {
                printf("err2");
                send(c, err2, strlen(err2), 0);
                return;
            }
            if (strcmp(Accept, "err") == 0) {
                printf("err3");
                send(c, err3, strlen(err3), 0);
                return;
            }
            printf("reached data\n");
            if (data == 0) {
                printf("exec data\n");

                char *str = malloc(Content_Length + 1); // +1 for '\0'
                if (!str) {
                    perror("malloc");
                    return;
                }

                size_t total_read = 0;

                // Read any leftover bytes already in the ring buffer
                size_t from_rb = rb_read(rb, str, Content_Length);
                total_read += from_rb;

                // Continue reading until Content-Length is satisfied
                while (total_read < (size_t)Content_Length) {
                    ssize_t n = recv(c, str + total_read, Content_Length - total_read, 0);
                    if (n <= 0) {
                        perror("recv");
                        free(str);
                        return;
                    }
                    total_read += n;
                }

                str[Content_Length] = '\0';
                

                for (int i = 0; i<strlen(str); i++) {
                    if (str[i] == '=' || str[i] == '+' || str[i] == '&') {
                        str[i] = ' ';
                    }
                }
                printf("%s\n", str);
                char* token = strtok(str, " ");
                // Keep printing tokens while one of the
                // delimiters present in str[].
                while (token != NULL) {
                    //printf("%s\n", token);
                    if(strcmp(token, "camera_dimensions") == 0) {
                        printf("dimensions\n");
                        camera_dimensions = 1;
                        int x,y;
                        token = strtok(NULL, " ");
                        sscanf(token, "%d", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%d", &y);
                        cam = camera(x,y);
                        printf("%d %d\n", x,y);
                    }
                    else if(strcmp(token, "camera_look_from") == 0) {
                        printf("look_from\n");
                        camera_look_from = 1;
                        double x,y,z;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &y);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &z);
                        cam.look_from = vec3(x,y,z);
                        printf("%lf %lf %lf\n", x,y,z);
                    }
                    else if(strcmp(token, "camera_up") == 0) {
                        printf("camera_up\n");
                        camera_up = 1;
                        double x,y,z;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &y);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &z);
                        cam.v_up = vec3(x,y,z);
                        printf("%lf %lf %lf\n", x,y,z);
                    }
                    else if(strcmp(token, "camera_look_at") == 0) {
                        printf("look_at\n");
                        camera_look_at = 1;
                        double x,y,z;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &y);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &z);
                        cam.look_at = vec3(x,y,z);
                        printf("%lf %lf %lf\n", x,y,z);
                    }
                    else if(strcmp(token, "camera_max_depth") == 0) {
                        printf("depth\n");
                        camera_max_depth = 1;
                        int depth;
                        token = strtok(NULL, " ");
                        sscanf(token, "%d", &depth);
                        cam.max_depth = depth;
                        printf("%d\n", depth);
                    }
                    else if(strcmp(token, "camera_vfov") == 0) {
                        printf("vfov\n");
                        camera_vfov = 1;
                        double fov;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &fov);
                        cam.vfov = fov;
                        printf("%lf\n", fov);
                    }
                    else if(strcmp(token, "camera_focus_dist") == 0) {
                        printf("camera_focus_dist\n");
                        camera_focus_dist = 1;
                        double dist;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &dist);
                        cam.focus_dist = dist;
                        printf("%lf\n", dist);
                    }
                    else if(strcmp(token, "scene_bg") == 0) {
                        printf("scene_bg\n");
                        scene_bg = 1;
                        double r,g,b,r2,g2,b2;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &r);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &g);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &b);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &r2);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &g2);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &b2);
                        scene = scene_new(vec3(r, g, b), vec3(r2, g2, b2));
                        printf("%lf %lf %lf, %lf %lf %lf\n", r,g,b,r2,g2,b2);
                    }
                    else if(strcmp(token, "cube") == 0) {
                        printf("cube\n");
                        double x, y, z, w, h, d;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &y);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &z);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &w);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &h);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &d);
                        token = strtok(NULL, " ");
                        if (strcmp(token, "metal") == 0) {
                            double mx, my, mz, mr;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mx);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &my);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mz);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mr);
                            if (num_cubes == max_cubes) {
                                cu = realloc(cu, sizeof(struct Cube)*(max_cubes+1));
                                max_cubes++;
                            }
                            cu[num_cubes++] = cube(vec3(x,y,z),w,h,d,material_metal(vec3(mx,my,mz),mr));
                            printf("lambertian %lf %lf %lf, %lf %lf %lf, %lf %lf %lf, %lf\n", x,y,z,w,h,d,mx,my,mz, mr);
                        }
                        else if (strcmp(token, "lambertian") == 0) {
                            double lx, ly, lz;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &lx);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &ly);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &lz);
                            if (num_cubes == max_cubes) {
                                cu = realloc(cu, sizeof(struct Cube)*(max_cubes+1));
                                max_cubes++;
                            }
                            cu[num_cubes++] = cube(vec3(x,y,z),w,h,d,material_lambertian(vec3(lx,ly,lz)));
                            printf("lambertian %lf %lf %lf, %lf %lf %lf, %lf %lf %lf\n", x,y,z,w,h,d,lx,ly,lz);
                        }
                        else if (strcmp(token, "dielectric") == 0) {
                            double ri;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &ri);
                            if (num_cubes == max_cubes) {
                                cu = realloc(cu, sizeof(struct Cube)*(max_cubes+1));
                                max_cubes++;
                            }
                            cu[num_cubes++] = cube(vec3(x,y,z),w,h,d,material_dielectric(ri));
                            printf("dielectric %lf %lf %lf, %lf %lf %lf, %lf\n", x,y,z,w,h,d,ri);
                        }
                    }
                    else if(strcmp(token, "sphere") == 0) {
                        printf("sphere\n");
                        double x, y, z, r;
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &x);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &y);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &z);
                        token = strtok(NULL, " ");
                        sscanf(token, "%lf", &r);
                        token = strtok(NULL, " ");
                        if (strcmp(token, "metal") == 0) {
                            double mx, my, mz, mr;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mx);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &my);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mz);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &mr);
                            if (num_spheres == max_spheres) {
                                sp = realloc(sp, sizeof(struct Sphere)*(max_spheres+1));
                                max_spheres++;
                            }
                            sp[num_spheres++] = sphere(vec3(x,y,z),r,material_metal(vec3(mx,my,mz),mr));
                            printf("metal %lf %lf %lf, %lf, %lf %lf %lf, %lf\n", x, y,z,r,mx,my,mz,mr);
                        }
                        else if (strcmp(token, "lambertian") == 0) {
                            double lx, ly, lz;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &lx);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &ly);
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &lz);
                            if (num_spheres == max_spheres) {
                                sp = realloc(sp, sizeof(struct Sphere)*(max_spheres+1));
                                max_spheres++;
                            }
                            sp[num_spheres++] = sphere(vec3(x,y,z),r,material_lambertian(vec3(lx,ly,lz)));
                            printf("lambertian %lf %lf %lf, %lf, %lf %lf %lf\n", x,y,z,r,lx,ly,lz);
                        }
                        else if (strcmp(token, "dielectric") == 0) {
                            double ri;
                            token = strtok(NULL, " ");
                            sscanf(token, "%lf", &ri);
                            if (num_spheres == max_spheres) {
                                sp = realloc(sp, sizeof(struct Sphere)*(max_spheres+1));
                                max_spheres++;
                            }
                            sp[num_spheres++] = sphere(vec3(x,y,z),r,material_dielectric(ri));
                            printf("dielectric %lf %lf %lf, %lf, %lf\n", x,y,z,r,ri);
                        }
                    }
                    else if(strcmp(token, "render") == 0) {
                        printf("render\n");
                        token = strtok(NULL, " ");
                        threads = 1;
                        sscanf(token, "%d", &thread_num);
                        token = strtok(NULL, " ");
                        samples = 1;
                        sscanf(token, "%d", &samp_num);
                        cam.samples_per_pixel = samp_num;
                        token = strtok(NULL, " ");
                        sampMeth = 1;
                        if (strcmp(token, "box") == 0) {
                            sampMethType = SAMP_BOX;
                        }
                        else if (strcmp(token, "x") == 0) {
                            sampMethType = SAMP_LINEAR_X;
                        }
                        else if (strcmp(token, "y") == 0) {
                            sampMethType = SAMP_LINEAR_Y;
                        }
                        cam.sampling_strategy = sampMethType;
                        printf("%d %d %d\n", thread_num, samp_num, sampMethType);
                    }
                    token = strtok(NULL, " ");
                    
                }
                camera_init(&cam);
                free(str);
                char err1[32] = "HTTP/1.1 400 Bad Request\r\n";
                char succ[32] = "HTTP/1.1 200 OK\r\n";
                printf("%d\n", camera_dimensions + camera_look_from +
                camera_look_at + camera_up + camera_max_depth + camera_vfov +
                camera_focus_dist + scene_bg + threads + samples + sampMeth);
                printf("%d %d %d %d %d %d %d %d %d %d %d\n", camera_dimensions, camera_look_from,
                camera_look_at, camera_up, camera_max_depth, camera_vfov,
                camera_focus_dist, scene_bg, threads, samples, sampMeth);
                if (camera_dimensions + camera_look_from +
                camera_look_at + camera_up + camera_max_depth + camera_vfov +
                camera_focus_dist + scene_bg + threads + samples + sampMeth != 11) {
                    send(c, err1, strlen(err1), 0);
                    printf("err1");
                }
                else {
                    send(c, succ, strlen(succ), 0);
                    printf("succ");
                }
                char Content_type[256] = "Content-Type: ";
                strcat(Content_type, ContType);
                strcat(Content_type, "\r\n");
                send(c, Content_type, strlen(Content_type), 0);
                char Content_disposition[256] = "Content-Disposition: attachment; filename=\"rt_image.";
                strcat(Content_disposition, Accept);
                strcat(Content_disposition, "\"\r\n");
                send(c, Content_disposition, strlen(Content_disposition), 0);
                char content_length[256] = "Content-Length: ";
                uint64_t size = 0;
                for (int i = 0;i < num_spheres;i+=1) {
                    scene_add(scene, &sp[i].object);
                }
                for (int i = 0;i < num_cubes;i+=1) {
                    scene_add(scene, &cu[i].object);
                }
                printf("Threads: %d\n", thread_num);
                pixels = camera_mt_render(&cam, scene, thread_num);
                if (strcmp(Accept, "rto") == 0) {
                    size = 4 + cam.image_height*cam.image_width*3;
                }
                else if (strcmp(Accept, "bmp") == 0) {
                    size = ((cam.image_width*cam.image_height)*3)+54;
                }
                else if (strcmp(Accept, "ppm") == 0) {
                    FILE* fout = tmpfile(); 
                    plugin_write_stream(fout, "ppm", cam.image_width, cam.image_height, pixels);
                    fflush(fout);
                    fseek(fout, 0, SEEK_END);
                    size = ftell(fout);
                    fclose(fout);
                }
                char temp[256];
                intToStr(size, temp);
                strcat(content_length, temp);
                strcat(content_length, "\r\n");
                send(c, content_length, strlen(content_length), 0);

                char connection[256] = "Connection: ";
                if (Connection == 1) {
                    strcat(connection, "keep-alive\r\n");
                }
                else {
                    strcat(connection, "close\r\n");
                }
                send(c, connection, strlen(connection), 0);
                
                char Access_Control_Allow_Origin[256] = "Access-Control-Allow-Origin: *\r\n";
                send(c, Access_Control_Allow_Origin, strlen(Access_Control_Allow_Origin), 0);
                char linebreak[3] = "\r\n";
                send(c, linebreak, strlen(linebreak), 0);
                //printf("Threads: %d\n", );
                printf("Camera setup:\n");
                printf("  dim %d x %d\n", cam.image_width, cam.image_height);
                printf("%lf %lf %lf  look_from\n", cam.look_from.x, cam.look_from.y, cam.look_from.z);
                printf("%lf %lf %lf  look_at\n", cam.look_at.x, cam.look_at.y, cam.look_at.z);
                printf("%lf %lf %lf  v_up\n", cam.v_up.x, cam.v_up.y, cam.v_up.z);
                printf("  vfov: %lf\n", cam.vfov);
                printf("  focus_dist: %lf\n", cam.focus_dist);
                printf("  max_depth: %d\n", cam.max_depth);
                printf("Scene objects: %d spheres, %d cubes\n", num_spheres, num_cubes);
                if (NULL != pixels) {
                    FILE *sock_fp = fdopen(dup(c), "wb"); // duplicate FD
                    if (!sock_fp) {
                        perror("fdopen");
                    } else {
                        plugin_write_stream(sock_fp, Accept,
                                            cam.image_width, cam.image_height,
                                            pixels);
                        /*plugin_write("./test.bmp",
                        "bmp",
                        cam.image_width,
                        cam.image_height,
                        pixels);*/
                        //fflush(sock_fp);
                        //fclose(sock_fp); // important!
                    }
                }
                else {
                    printf("camera_render returned NULL!\n");
                }

            
            }
            
            data = 1;
            break;
        }
        
        
        
    }
    
    close(c);
    free(cu);
    free(sp);
    data = 0;
    header = 0;
    request = 0;
    Content_Length = 0;
    Accept[0] = '\0';
    ContType[0] = '\0';
    scene_free(scene);
    //free(scene);
    free(pixels);
    rb_free(rb);
    if (n < 0) {
        perror("recv");
        return;
    }

    }
    close(s);
    
        

    
    printf("Server closed.\n");

    return;

}

void server_stop() {

}