/* included by renderer-gl3.c -- do not compile separately */
static const char *vshader_gbuffer_src =
"#version 330\n"
"uniform mat4 vmatrix;\n"
"uniform mat4 vpmatrix;\n"
"layout(location = 0) in vec3 vposition;\n"
"layout(location = 1) in vec2 vtexcoord;\n"
"layout(location = 2) in vec3 vnormal;\n"
"layout(location = 3) in vec4 vtangent;\n"
"smooth out float fdepth;\n"
"smooth out vec2 ftexcoord;\n"
"smooth out mat3 tbnmatrix;\n"
"void main() {\n"
"  fdepth    = -(vmatrix * vec4(vposition, 1.0)).z;\n"
"  ftexcoord = vtexcoord;\n"
"\n"
"  vec3 vbitangent = cross(vnormal, vtangent.xyz) * vtangent.w;\n"
"  tbnmatrix = mat3(vmatrix[0].xyz, vmatrix[1].xyz, vmatrix[2].xyz) * mat3(vtangent.xyz, vbitangent, vnormal);\n"
"\n"
"  gl_Position = vpmatrix * vec4(vposition, 1.0);\n"
"}\n";

static const char *fshader_gbuffer_src =
"#version 330\n"
"uniform sampler2D tdiffuse;\n"
"uniform sampler2D tnormal;\n"
"uniform sampler2D tspecular;\n"
"uniform sampler2D temissive;\n"
"smooth in float fdepth;\n"
"smooth in vec2  ftexcoord;\n"
"smooth in mat3  tbnmatrix;\n"
"layout(location = 0) out float gdepth;\n"
"layout(location = 1) out vec4  gdiffuse;\n"
"layout(location = 2) out vec2  gnormal;\n"
"layout(location = 3) out vec4  gspecular;\n"
"layout(location = 4) out vec4  gemissive;\n"
"void main() {\n"
"  vec3 n_local = texture(tnormal, ftexcoord).xyz * 2.0 - 1.0;\n"
"  vec3 n_world = normalize(tbnmatrix * n_local);\n" /* this is actually eye-space, not world-space */
"\n"
"  gdepth    = fdepth;\n"
"  gdiffuse  = texture(tdiffuse, ftexcoord);\n"
"  gnormal   = n_world.xy;\n"
"  gspecular = texture(tspecular, ftexcoord);\n"
"  gemissive = texture(temissive, ftexcoord);\n"
"}\n";

static const char *vshader_minimal_src = 
"#version 330\n"
"uniform mat4 mvpmatrix;\n"
"layout(location = 0) in vec3 vcoord;\n"
"void main () {\n"
"  gl_Position  = mvpmatrix * vec4(vcoord, 1.0);\n"
"}\n";

static const char *fshader_flatcolor_src = 
"#version 330\n"
"uniform vec3 color;\n"
"layout(location = 0) out vec4 outcolor;\n"
"void main() {\n"
"  outcolor = vec4(color, 1.0);\n"
"}\n";

static const char *vshader_pointlight_src = 
"#version 330\n"
"layout(std140) uniform pointlight {\n"
"  vec4  position;\n"
"  vec4  color;\n"
"} light;\n"
"uniform mat4 vmatrix;\n"
"layout(location = 0) in vec2 vcoord;\n"
"layout(location = 1) in vec3 vray;\n"
"smooth out vec3 fray;\n"
"smooth out vec2 ftexcoord;\n"
"smooth out vec3 flightpos;\n"
"void main () {\n"
"  fray      = vray;\n"
"  ftexcoord = vcoord * 0.5 + 0.5;\n"
"  flightpos = (vmatrix * light.position).xyz;\n"
"  gl_Position = vec4(vcoord, 0.0, 1.0);\n"
"}\n";

static const char *fshader_pointlight_src =
"#version 330\n"
"layout(std140) uniform pointlight {\n"
"  vec4  position;\n"
"  vec4  color;\n"
"} light;\n"
"uniform sampler2D tdepth;\n"
"uniform sampler2D tdiffuse;\n"
"uniform sampler2D tnormal;\n"
"uniform sampler2D tspecular;\n"
"uniform vec3 viewpos;\n"
"smooth in vec3 fray;\n"
"smooth in vec2 ftexcoord;\n"
"smooth in vec3 flightpos;\n"
"layout(location = 0) out vec4 color;\n"
"void main () {\n"
"  float depth = texture(tdepth, ftexcoord).r;\n"
"  vec3  coord = fray * depth;\n"
"  float dist  = distance(flightpos, coord);\n"
"  float luminance = light.color.a / (dist * dist);\n"
""
"  vec3 norm;\n"
"  norm.xy = texture(tnormal, ftexcoord).xy;\n"
"  norm.z  = sqrt(1.0 - dot(norm.xy, norm.xy));\n"
""
"  vec3 lightdir = normalize(flightpos - coord);\n"
"  vec3 eyedir   = -normalize(fray);\n"
"  vec3 reflect  = 2.0 * norm * dot(norm, lightdir) - lightdir;\n"
""
"  vec3 diff  = texture(tdiffuse, ftexcoord).rgb;\n"
"  vec4 spec  = texture(tspecular, ftexcoord);\n"
"  color.rgb  = diff * max(0.0, dot(norm, lightdir));\n" /* diffuse */
"  color.rgb += spec.rgb * pow(max(0.0, dot(reflect, eyedir)), spec.a*255.0);\n" /* specular */
"  color.rgb *= light.color.rgb * luminance;\n"
"  color.a    = 1.0;\n"
"}\n";

static const char *vshader_envlight_src = 
"#version 330\n"
"layout(location = 0) in vec2 vcoord;\n"
"layout(location = 1) in vec3 vray;\n"
"smooth out vec3 fray;\n"
"smooth out vec2 ftexcoord;\n"
"void main () {\n"
"  fray      = vray;\n"
"  ftexcoord = vcoord * 0.5 + 0.5;\n"
"  gl_Position = vec4(vcoord, 0.0, 1.0);\n"
"}\n";

static const char *fshader_envlight_src =
"#version 330\n"
"layout(std140) uniform envlight {\n"
"  vec4  ambient;\n"
"  vec4  color;\n"
"  vec4  direction;\n"
"} light;\n"
"uniform mat4 vmatrix;\n"
"uniform sampler2D tdepth;\n"
"uniform sampler2D tdiffuse;\n"
"uniform sampler2D tnormal;\n"
"uniform sampler2D tspecular;\n"
"uniform sampler2D temissive;\n"
"uniform sampler2D trandnorm;\n"
"uniform vec3 viewpos;\n"
"smooth in vec3 fray;\n"
"smooth in vec2 ftexcoord;\n"
"layout(location = 0) out vec4 color;\n"
"void main () {\n"
"  float depth = texture(tdepth, ftexcoord).r;\n"
"  vec3  coord = fray * depth + viewpos;\n"
""
"  vec3 norm;\n"
"  norm.xy = texture(tnormal, ftexcoord).xy;\n"
"  norm.z  = sqrt(1.0 - dot(norm.xy, norm.xy));\n"
""
//"  float occlusion = 0.0;\n"
//"  for (int i=0; i < 8; i++) {\n"
//"    vec3 offset  = texture(trandnorm, ftexcoord * 13.0).xyz * 2.0 - 1.0;\n"
//"    if (dot(norm, offset) < 0) { offset = -offset; }\n"
//"    vec3 sscoord = vpmatrix * (coord + offset)
//"  }\n"
""
"  vec3 lightdir = normalize(mat3(vmatrix[0].xyz, vmatrix[1].xyz, vmatrix[2].xyz) * -light.direction.xyz);\n"
"  vec3 eyedir   = -normalize(fray);\n"
"  vec3 reflect  = 2.0 * norm * dot(norm, lightdir) - lightdir;\n"
""
"  vec3 diff  = texture(tdiffuse, ftexcoord).rgb;\n"
"  vec4 spec  = texture(tspecular, ftexcoord);\n"
"  vec4 glow  = texture(temissive, ftexcoord);\n"
"  color.rgb += diff * max(0.0, dot(norm, lightdir));\n" /* diffuse */
"  color.rgb += spec.rgb * pow(max(0.0, dot(reflect, eyedir)), spec.a*255.0);\n" /* specular */
"  color.rgb *= light.color.rgb * light.color.a;\n" /* diffuse/specular scale */
"  color.rgb += diff * light.ambient.rgb * light.ambient.a;\n" /* ambient */
"  color.rgb += glow.rgb * exp2(glow.a * 16.0 - 8.0);\n" /* emissive */
"  color.a    = 1.0;\n"
"}\n";

static const char *vshader_tonemap_src = 
"#version 330\n"
"layout(location = 0) in vec2 vcoord;\n"
"smooth out vec2 ftexcoord;\n"
"void main () {\n"
"  ftexcoord = vcoord * 0.5 + 0.5;\n"
"  gl_Position = vec4(vcoord, 0.0, 1.0);\n"
"}\n";

static const char *fshader_tonemap_src = 
"#version 330\n"
"uniform sampler2D tcomposite;\n"
"smooth in vec2 ftexcoord;\n"
"layout(location = 0) out vec4 color;\n"
"void main() {\n"
"  vec3 c = texture(tcomposite, ftexcoord).rgb;\n"
"  color = vec4(1.0 / (1.0 + exp(-8.0 * pow(c, vec3(0.8)) + 4.0)) - 0.018, 1.0);\n"
"}\n";

static const char *vshader_blit_src =
"#version 330\n"
"uniform vec2 size;\n"
"uniform vec2 offset;\n"
"layout(location = 0) in vec2 vcoord;\n"
"smooth out vec2 ftexcoord;\n"
"void main () {\n"
"  ftexcoord = vcoord * 0.5 + 0.5;\n"
"  gl_Position = vec4(vcoord * size + offset, 0.0, 1.0);\n"
"}\n";

static const char *fshader_blit_src = 
"#version 330\n"
"uniform sampler2D image;\n"
"smooth in vec2 ftexcoord;\n"
"layout(location = 0) out vec4 color;\n"
"void main() {\n"
"  color = texture(image, ftexcoord);\n"
"}\n";
