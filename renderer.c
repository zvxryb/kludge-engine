#include "renderer.h"

#include "renderer-gl3.h"

#include "bvhtree.h"
#include "plane.h"
#include "sphere.h"

#include <stdlib.h>

typedef struct light {
  kl_vec3f_t position;
  float      scale;
  unsigned int id;
} light_t;

static int checkfrustum(kl_sphere_t *bounds, kl_frustum_t *frustum);
static void draw_model(kl_model_t *model, kl_scene_t *scene);
static void draw_light(light_t *light, kl_scene_t *scene);
static void draw_bounds(kl_bvh_node_t *node, kl_scene_t *scene);

static kl_bvh_node_t *bvh_models = NULL;
static kl_bvh_node_t *bvh_lights = NULL;

static int debugmode = 0;

/* ------------------------- */
int kl_render_init() {
  return kl_gl3_init();
}

void kl_render_draw(kl_camera_t *cam) {
  static kl_scene_t   scene;
  static kl_frustum_t frustum;
  kl_camera_update_scene(cam, &scene);
  kl_camera_update_frustum(cam, &frustum);

  kl_gl3_update_scene(&scene);

  kl_gl3_begin_pass_gbuffer();
  kl_bvh_search(bvh_models, (kl_bvh_filter_cb)&checkfrustum, &frustum, (kl_bvh_result_cb)&draw_model, &scene);
  kl_gl3_end_pass_gbuffer();

  kl_gl3_begin_pass_lighting();
  kl_bvh_search(bvh_lights, (kl_bvh_filter_cb)&checkfrustum, &frustum, (kl_bvh_result_cb)&draw_light, &scene);
  kl_gl3_end_pass_lighting();

  kl_gl3_begin_pass_debug();
  kl_bvh_debug(bvh_models, (kl_bvh_debug_cb)&draw_bounds, &scene);
  kl_bvh_debug(bvh_lights, (kl_bvh_debug_cb)&draw_bounds, &scene);
  kl_gl3_end_pass_debug();

  kl_gl3_composite();

  kl_gl3_debugtex(debugmode);
}

void kl_render_set_debug(int mode) {
  debugmode = mode;
}

void kl_render_add_model(kl_model_t* model) {
  kl_bvh_insert(&bvh_models, &model->bounds, model);
}

void kl_render_set_envlight(kl_vec3f_t *direction, float amb_r, float amb_g, float amb_b, float amb_intensity, float diff_r, float diff_g, float diff_b, float diff_intensity) {
  kl_gl3_update_envlight(direction, amb_r, amb_g, amb_b, amb_intensity, diff_r, diff_g, diff_b, diff_intensity);
}

void kl_render_add_light(kl_vec3f_t *position, float r, float g, float b, float intensity) {
  /* 16 * sqrt(intensity) is the distance at which light contribution is less than 1/256 */
  float radius = 16.0f * sqrtf(intensity);

  light_t *light = malloc(sizeof(light_t));
  *light = (light_t){
    .position = *position,
    .scale    = radius, 
    .id       = kl_gl3_upload_light(position, r, g, b, intensity)
  };
  kl_sphere_t bounds = {
    .center = { .x = position->x, .y = position->y, .z = position->z },
    .radius = radius
  };
  kl_bvh_insert(&bvh_lights, &bounds, light);
}

unsigned int kl_render_upload_vertdata(void *data, int n) {
  return kl_gl3_upload_vertdata(data, n);
}

unsigned int kl_render_upload_tris(unsigned int *data, int n) {
  return kl_gl3_upload_tris(data, n);
}

unsigned int kl_render_upload_texture(void *data, int w, int h, int format, int type) {
  return kl_gl3_upload_texture(data, w, h, format, type);
}

void kl_render_free_texture(unsigned int texture) {
  kl_gl3_free_texture(texture);
}

unsigned int kl_render_define_attribs(int tris, kl_render_attrib_t *cfg, int n) {
  return kl_gl3_define_attribs(tris, cfg, n);
}

/* ------------------------- */
static int checkfrustum(kl_sphere_t *bounds, kl_frustum_t *frustum) {
  if (kl_plane_dist(&frustum->near, &bounds->center) > bounds->radius)
    return 0;
  if (kl_plane_dist(&frustum->far, &bounds->center) > bounds->radius)
    return 0;
  if (kl_plane_dist(&frustum->top, &bounds->center) > bounds->radius)
    return 0;
  if (kl_plane_dist(&frustum->bottom, &bounds->center) > bounds->radius)
    return 0;
  if (kl_plane_dist(&frustum->left, &bounds->center) > bounds->radius)
    return 0;
  if (kl_plane_dist(&frustum->right, &bounds->center) > bounds->radius)
    return 0;
  return 1;
}

static void draw_model(kl_model_t *model, kl_scene_t *scene) {
  kl_gl3_draw_pass_gbuffer(model);
}

static void draw_light(light_t *light, kl_scene_t *scene) {
  kl_mat4f_t scale, translation, modelmatrix, mvpmatrix;
  kl_mat4f_translation(&translation, &light->position);
  kl_mat4f_scale(&scale, light->scale, light->scale, light->scale);
  kl_mat4f_mul(&modelmatrix, &translation, &scale);
  kl_mat4f_mul(&mvpmatrix, &scene->vpmatrix, &modelmatrix);

  kl_gl3_draw_pass_lighting(&mvpmatrix, light->id);
}

static void draw_bounds(kl_bvh_node_t *node, kl_scene_t *scene) {

  float r = 1.0f;
  float g = 0.5f;
  float b = 0.0f;

  kl_vec3f_t position = node->header.bounds.center;
  float      radius   = node->header.bounds.radius;

  kl_mat4f_t scale, translation, modelmatrix, mvpmatrix;
  kl_mat4f_translation(&translation, &position);
  kl_mat4f_scale(&scale, radius, radius, radius);
  kl_mat4f_mul(&modelmatrix, &translation, &scale);
  kl_mat4f_mul(&mvpmatrix, &scene->vpmatrix, &modelmatrix);

  kl_gl3_draw_pass_debug(&mvpmatrix, r, g, b);
}

/* vim: set ts=2 sw=2 et */
