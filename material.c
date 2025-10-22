/* material.c */
#include "material.h"
#include "ray.h"
#include "vec3.h"
#include "hit.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

double random_double() {
    return rand() / (RAND_MAX + 1.0);
}
struct Material material_lambertian(Color albedo) {
    struct Material m;
    m.type = MT_LAMBERTIAN;
    m.lambertian.albedo = albedo;
    return m;
}

struct Material material_metal(Color albedo, double fuzz) {
    struct Material m;
    m.type = MT_METAL;
    m.metal.albedo = albedo;
    if (fuzz < 1.0) {
      m.metal.fuzz = fuzz;
    }
    else {
      m.metal.fuzz = 1.0;
    }
    return m;
}

struct Material material_dielectric(double refraction_index) {
    struct Material m;
    m.type = MT_DIELECTRIC;
    m.dielectric.refraction_index = refraction_index;
    return m;
}

#define RAND(mn, mx)         (mn + rand() % (mx-mn+1))
static Vec3 random_unit_vector(void) {
    Vec3 vec;
    double len2;
    int i;
    for (i = 0;i < 100000;i+=1) {
        vec = vec3_unit(vec3(RAND(-255, 255) / 255.0,
                             RAND(-255, 255) / 255.0,
                             RAND(-255, 255) / 255.0));
        len2 = vec3_len2(vec);
        if (1e-160 < len2 && len2 <= 1) {
            return vec3_div_val(vec, sqrt(len2));
        }
    }
    return vec3(0.17, 0.42, 0.57);
}

static bool near_zero(const Vec3 *vec) {
    return (vec->x < 0.001) && (vec->y < 0.001) && (vec->z < 0.001);
}

bool lambertian_scatter(const struct Material *material,
                               const struct HitRecord *rec,
                               const struct Ray *r_in,
                               Color *attenuation,
                               struct Ray *scattered)
{
    (void)r_in;
    Vec3 scatter_direction = vec3_add(rec->normal, random_unit_vector());

    if (near_zero(&scatter_direction)) {
        scatter_direction = rec->normal;
    }
    *scattered = ray(rec->point, scatter_direction);
    *attenuation = material->lambertian.albedo;

    return true;
}

bool metal_scatter(const struct Material *material,
                          const struct HitRecord *rec,
                          const struct Ray *r_in,
                          Color *attenuation,
                          struct Ray *scattered)
{
    Vec3 reflected;
    Vec3 fuzzvec;

    fuzzvec = vec3_mul_val(random_unit_vector(), material->metal.fuzz);
    reflected = vec3_add(vec3_unit(material_reflect(r_in->direction, rec->normal)), fuzzvec);
    *scattered = ray(rec->point, reflected);
    *attenuation = material->metal.albedo;
    return vec3_dot(scattered->direction, rec->normal) > 0;
}


static double material_dielectric_reflectance(double cosine, double ri)
{
    double r0 = (1 - ri) / (1 + ri);
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - cosine, 5);
}
bool dielectric_scatter(const struct Material *material,
                               const struct HitRecord *rec,
                               const struct Ray *r_in,
                               Color *attenuation,
                               struct Ray *scattered)
{
    double ri;
    double cos_theta;
    double sin_theta;
    bool cannot_refract;
    Vec3 direction;
    Vec3 unit_direction;

    *attenuation = vec3(1.0, 1.0, 1.0);
    ri = rec->front_face ? (1.0 / material->dielectric.refraction_index) : material->dielectric.refraction_index;
    unit_direction = vec3_unit(r_in->direction);
    cos_theta = fmin(vec3_dot(vec3_neg(unit_direction), rec->normal), 1.0);
    sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    cannot_refract = (ri * sin_theta) > 1.0;

    if (cannot_refract || material_dielectric_reflectance(cos_theta, ri) > random_double()) {
        direction = material_reflect(unit_direction, rec->normal);
    }
    else {
        direction = material_refract(unit_direction, rec->normal, ri);
    }

    *scattered = ray(rec->point, direction);

    return true;
}


bool material_scatter(const struct Material *material,
                             const struct HitRecord *rec,
                             const struct Ray *r_in,
                             Color *attenuation,
                             struct Ray *scattered)
{
    // choose based on material type:
    // dielectric_scatter(material, rec, r_in, attenuation, scattered);
    // metal_scatter(material, rec, r_in, attenuation, scattered);
    // lambertian_scatter(material, rec, r_in, attenuation, scattered);
    if (material->type == MT_LAMBERTIAN) {
      return lambertian_scatter(material, rec, r_in, attenuation, scattered);
    }
    else if (material->type == MT_METAL) {
      return metal_scatter(material, rec, r_in, attenuation, scattered);
    }
    else {
      return dielectric_scatter(material, rec, r_in, attenuation, scattered);
    }

}

Vec3 material_reflect(Vec3 v, Vec3 n) {
    double dot = vec3_dot(v, n);
    return vec3_sub(v, vec3_mul_val(n, 2.0 * dot));
}
Vec3 material_refract(Vec3 uv, Vec3 n, double eoe) {
  double aor = fmin(1.0, vec3_dot(vec3_mul_val(uv,-1), n));
  Vec3 prd = vec3_mul_val(vec3_add(vec3_mul_val(n, aor),uv),eoe);
  Vec3 parard = vec3_mul_val(n,-sqrt(fabs(1-vec3_len2(prd))));
  return vec3_add(prd, parard);

}
