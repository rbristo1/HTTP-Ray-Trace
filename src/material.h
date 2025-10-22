#pragma once
#include "vec3.h"
#include <stdbool.h>

typedef enum {
    MT_LAMBERTIAN,
    MT_METAL,
    MT_DIELECTRIC,
} MaterialType;

struct Material {
    MaterialType    type;
    union {
        struct {
            Color albedo;
        } lambertian;
        struct {
            Color albedo;
            double fuzz;
        } metal;
        struct {
            double refraction_index;
        } dielectric;
    };
};

struct Ray;
struct HitRecord;

struct Material material_lambertian(Color albedo);
struct Material material_metal(Color albedo, double fuzz);
struct Material material_dielectric(double refraction_index);
bool material_scatter(const struct Material *material, const struct HitRecord *rec, const struct Ray *r_in, Color *attenuation, struct Ray *scattered);
bool metal_scatter(const struct Material *material,
                      const struct HitRecord *rec,
                      const struct Ray *r_in,
                      Color *attenuation,
                      struct Ray *scattered);
bool lambertian_scatter(const struct Material *material,
                      const struct HitRecord *rec,
                      const struct Ray *r_in,
                      Color *attenuation,
                      struct Ray *scattered);
bool dielectric_scatter(const struct Material *material,
                      const struct HitRecord *rec,
                      const struct Ray *r_in,
                      Color *attenuation,
                      struct Ray *scattered);



Vec3 material_reflect(Vec3 v, Vec3 n);
Vec3 material_refract(Vec3 uv, Vec3 n, double eoe);
