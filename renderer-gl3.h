#ifndef KL_RENDERER_GL3_H
#define KL_RENDERER_GL3_H

#include "renderer.h"

#include "camera.h"
#include "model.h"

int kl_gl3_init();

void kl_gl3_begin_pass_gbuffer(kl_mat4f_t *vmatrix, kl_mat4f_t *vpmatrix);
void kl_gl3_end_pass_gbuffer();
void kl_gl3_draw_pass_gbuffer(kl_model_t *model);

void kl_gl3_begin_pass_lighting(kl_mat4f_t *vmatrix, kl_vec3f_t *viewpos, kl_vec3f_t *rays);
void kl_gl3_end_pass_lighting();
void kl_gl3_draw_pass_lighting(kl_mat4f_t *mvpmatrix, unsigned int light);

void kl_gl3_begin_pass_debug();
void kl_gl3_end_pass_debug();
void kl_gl3_draw_pass_debug(kl_mat4f_t *mvpmatrix, float r, float g, float b);

void kl_gl3_composite(); /* apply tonemapping, postprocessing, and output to screen */
void kl_gl3_debugtex(); /* show output of the g-buffer */

unsigned int kl_gl3_upload_vertdata(void *data, int n);
void kl_gl3_update_vertdata(unsigned int vbo, void *data, int n);
unsigned int kl_gl3_upload_tris(unsigned int *data, int n);
unsigned int kl_gl3_upload_texture(void *data, int w, int h, int format, int type);
unsigned int kl_gl3_upload_light(kl_vec3f_t *position, float r, float g, float b, float a);
void kl_gl3_free_texture(unsigned int texture);
unsigned int kl_gl3_define_attribs(int tris, kl_render_attrib_t *cfg, int n);

#endif /* KL_RENDERER_GL3_H */
