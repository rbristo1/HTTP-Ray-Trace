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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#define SEED time(NULL)
int s;
bool die = false;
pid_t pid = -1;
pid_t *children = NULL;
size_t children_count = 0;
size_t children_cap = 0;
void intToStr(uint64_t num, char *str) {
    sprintf(str, "%d", num);
}
void add_child(pid_t pid) {
    if (children_count == children_cap) {
        children = realloc(children, sizeof(pid_t) * (children_cap+1));
        children_cap++;
    }
    children[children_count++] = pid;
}
void remove_child(pid_t pid) {
    for (size_t i = 0; i < children_count; ++i) {
        if (children[i] == pid) {
            children[i] = children[--children_count];
            return;
        }
    }
}
void removeChildren() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_child(pid);
    }
}
void sighandler(int sig) {
    if (SIGINT == sig) {
        fprintf(stderr, "Quitting!\n");
        if (pid == 0) {
            die = true;
            server_stop();
            return;
        }
        die = true;
        server_stop();
    }
    if (SIGQUIT == sig) {
        printf("SIGQUIT\n");
        shutdown(s, SHUT_RDWR);
        close(s);
        die = true;
    }
    if (SIGUSR1 == sig) {
        return;
    }
    if (SIGHUP == sig) {
        printf("SIGHUP\n");
        if (pid == 0) {
            die = true;
            return;
        }
        return;
    }
}
void servWorker(int c, struct sockaddr_in sin, struct sockaddr_in cin, int * Connection) {
    ssize_t n;
    char buf[256];
    n = recv(c, buf, 256, MSG_NOSIGNAL);
    if (n == -1) {
        
        return;
    }
    
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
    while (n >= 0) {
        
        char parse[256] = "";
        rb_write(rb, buf, n);
        if (request == 0) {

            for(int i  = 0; i < 256; i++) {
                if (!rb_pop(rb, &parse[i])) {
                    n = recv(c, buf, 256, MSG_NOSIGNAL);
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
                    n = recv(c, buf, 256, MSG_NOSIGNAL);
                    rb_write(rb, buf, n);
                    if (n == 0) {
                        perror("header");
                        return;
                    }
                    printf("%s", buf);
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
                                    *Connection = 1;
                                    printf("Connection: c\n");
                                }
                                else {
                                    *Connection = 0;
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
                send(c, err2, strlen(err2), MSG_NOSIGNAL);
                return;
            }
            if (strcmp(Accept, "err") == 0) {
                printf("err3");
                send(c, err3, strlen(err3), MSG_NOSIGNAL);
                return;
            }
            printf("reached data\n");
            if (data == 0) {
                printf("exec data\n");

                char *str = malloc(Content_Length + 1); 
                if (!str) {
                    perror("malloc");
                    return;
                }

                size_t total_read = 0;

                size_t from_rb = rb_read(rb, str, Content_Length);
                total_read += from_rb;

                while (total_read < (size_t)Content_Length) {
                    ssize_t n = recv(c, str + total_read, Content_Length - total_read, MSG_NOSIGNAL);
                    if (n <= 0) {
                        perror("recv");
                        free(str);
                        return;
                    }
                    total_read += n;
                }
                
                str[Content_Length] = '\0';
                printf("String: %s", str);

                for (int i = 0; i<strlen(str); i++) {
                    if (str[i] == '=' || str[i] == '+' || str[i] == '&') {
                        str[i] = ' ';
                    }
                }
                printf("%s\n", str);
                char* token = strtok(str, " ");
                while (token != NULL) {
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
                    send(c, err1, strlen(err1), MSG_NOSIGNAL);
                    printf("err1");
                }
                else {
                    send(c, succ, strlen(succ), MSG_NOSIGNAL);
                    printf("succ");
                }
                char Content_type[256] = "Content-Type: ";
                strcat(Content_type, ContType);
                strcat(Content_type, "\r\n");
                send(c, Content_type, strlen(Content_type), MSG_NOSIGNAL);
                char Content_disposition[256] = "Content-Disposition: attachment; filename=\"rt_image.";
                strcat(Content_disposition, Accept);
                strcat(Content_disposition, "\"\r\n");
                send(c, Content_disposition, strlen(Content_disposition), MSG_NOSIGNAL);
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
                send(c, content_length, strlen(content_length), MSG_NOSIGNAL);

                char connection[256] = "Connection: ";
                if (*Connection == 1 && !die) {
                    strcat(connection, "keep-alive\r\n");
                }
                else {
                    strcat(connection, "close\r\n");
                }
                send(c, connection, strlen(connection), MSG_NOSIGNAL);
                
                char Access_Control_Allow_Origin[256] = "Access-Control-Allow-Origin: *\r\n";
                send(c, Access_Control_Allow_Origin, strlen(Access_Control_Allow_Origin), MSG_NOSIGNAL);
                char linebreak[3] = "\r\n";
                send(c, linebreak, strlen(linebreak), MSG_NOSIGNAL);
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
                    FILE *sock_fp = fdopen(c, "wb"); 
                    if (!sock_fp) {
                        perror("fdopen");
                    } else {
                        plugin_write_stream(sock_fp, Accept,
                                            cam.image_width, cam.image_height,
                                            pixels);
                        fflush(sock_fp);
                        fclose(sock_fp);
                    }
                }
                else {
                    printf("camera_render returned NULL!\n");
                }

            
            }
            if (die) {
                shutdown(c, SHUT_RDWR);
                free(children);
                close(c);
            }
            
            data = 1;
            break;
        }
        
        
        
    }
    
    
    data = 0;
    header = 0;
    request = 0;
    Content_Length = 0;
    Accept[0] = '\0';
    ContType[0] = '\0';
    free(sp);
    free(cu);
    scene_free(scene);
    if (pixels) {
        free(pixels);
    }
    rb_free(rb);
    if (n < 0) {
        perror("recv");
        return;
    }
    if (die) {
        exit(EXIT_SUCCESS);
    }
}

void clientNew(int c, struct sockaddr_in sin, struct sockaddr_in cin);
void server_start() {
    signal(SIGINT, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGUSR1, sighandler);

    srand(SEED);
    int c;
    struct sockaddr_in sin = {0};
    struct sockaddr_in cin = {0};
    socklen_t temp = sizeof(cin);
    
    s = socket(AF_INET, SOCK_STREAM , 0);
    if (s == -1) {
        perror("socket");

        return;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(8177);

    unsigned int a = 0;
    a |= (127 << 24);
    a |= (0 << 16);
    a |= (0 << 8);
    a |= 1;
    sin.sin_addr.s_addr = htonl(a);

    if (bind(s, (struct sockaddr*)&sin, sizeof(sin)) == -1) {
        perror("bind");
        close(s);
        return;
    }

    if (listen(s, 5) == -1) {
        perror("listen");
        close(s);
        return;
    }

    printf("Server listening on %s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    while (true) {
        usleep(5000);
        if (!die) {
            c = accept(s, (struct sockaddr*)&cin, &temp);
        }
        
       

        
        if (c != -1) {
            
            printf("Connection accepted from %s:%d\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
            pid = fork();
            if (pid < 0) {
                perror("fork");
                close(c);
                continue;
            }

            if (pid == 0) {
                // Child process

                close(s); // child doesn't need the listening socket
                clientNew(c, sin, cin);
                //close(c);
                return;
            } else {
                add_child(pid);
                // Parent process
                close(c); 
            }

        }
        removeChildren();

        
        
    }
    close(s);
    
    printf("Server closed.\n");
}

void clientNew(int c, struct sockaddr_in sin, struct sockaddr_in cin) {
    int Connection = 1;
    while (Connection == 1 && !die) {
        servWorker(c, sin, cin, &Connection);
        usleep(200000);
        printf("Client Connected\n");
    }
    free(children);
    close(c);
    exit(0);
}
void server_stop() {
    shutdown(s, SHUT_RDWR);
    close(s);

    for (size_t i = 0; i < children_count; i++) {
        pid_t p = children[i];
        while (waitpid(p, NULL, 0) == -1) {
            if (errno == EINTR) continue;
            break;
        }
    }
    free(children);
    children = NULL;

    printf("###############################################################################################################################################################################################################################################################################################################################################\n");
    exit(EXIT_SUCCESS);
}