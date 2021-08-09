typedef struct Color Color;
struct Color
{
    u8 r;
    u8 g;
    u8 b;
    u8 a;
};
typedef struct VertexData VertexData;
struct VertexData
{
    vec3 position;
    vec2 uv;
    Color color;
    f32 texture_slot;
};
typedef struct Camera Camera;
struct Camera
{
    vec3 position;
    vec3 direction;
    vec3 up;
};
typedef enum SpriteType
{
    TYPE_SPRITE,




























    TYPE_SUBSPRITE,
}
SpriteType;
typedef struct Sprite Sprite;
struct Sprite
{
    u32 main_sprite;
    vec2 uvs[4];
    char *name;
    void *data;
    i32 width;
    i32 height;
    i32 channels;

    u32 id;
    u32 slot;
    SpriteType type;
};
typedef struct CharMetric CharMetric;
struct CharMetric
{
    u16 x0;
    u16 y0;
    u16 x1;
    u16 y1;
    f32 xoff;
    f32 yoff;
    f32 xadvance;
};
typedef struct Font Font;
struct Font
{
    u32 *sprite_handles;
    CharMetric *char_metrics;
    u16 num_chars;
    u32 font_size;
    u32 font_sprite_handle;
};
typedef enum RenderEntryType
{
    RENDER_ENTRY_QuadEntry,








































































    RENDER_ENTRY_TriangleEntry,
}
RenderEntryType;
typedef struct RenderEntryHeader RenderEntryHeader;
struct RenderEntryHeader
{
    RenderEntryType entry_type;
};
typedef enum ShaderId
{
    SHADER_ID_NORMAL,



















































































    SHADER_ID_HUE_QUAD,




















































































    SHADER_ID_SB_QUAD,





















































































    NUM_SHADERS,
}
ShaderId;
typedef struct QuadEntry QuadEntry;
struct QuadEntry
{
    vec3 position;
    vec2 size;
    Color color;
    u32 sprite_handle;

    ShaderId shader_id;
};
typedef struct TriangleEntry TriangleEntry;
struct TriangleEntry
{
    vec3 points[3];
    Color color;
    u32 sprite_handle;

    ShaderId shader_id;
};
typedef struct SortElement SortElement;
struct SortElement
{
    u32 entry_offset;
    u32 key;
};
typedef struct RenderSetup RenderSetup;
struct RenderSetup
{
    mat4 projection;
    Camera camera;
};
typedef enum UniformType
{
    UNIFORM_F32,


























































































































    UNIFORM_F64,



























































































































    UNIFORM_I32,




























































































































    UNIFORM_U32,





























































































































    UNIFORM_VEC2,






























































































































    UNIFORM_VEC3,































































































































    UNIFORM_VEC4,
































































































































    UNIFORM_MAT4,

































































































































    UNIFORM_TEXTURE2D,
}
UniformType;
typedef struct UniformTypeInfo UniformTypeInfo;
struct UniformTypeInfo
{
    UniformType type;
    u32 size;
};
typedef struct Uniform Uniform;
struct Uniform
{
    UniformTypeInfo type_info;
    void *data;
    char *name;

    Uniform *next;
};
typedef struct Shader Shader;
struct Shader
{
    u32 bind_id;
    Uniform *uniforms;
    i32 num_uniforms;
};
typedef struct Assets Assets;
struct Assets
{
    MemoryArena arena;

    u32 *loaded_sprite_queue;
    u32 num_queued_sprites;

    Sprite *sprites;
    u32 num_sprites;

    Shader shaders[NUM_SHADERS];
};
typedef struct RenderGroup RenderGroup;
struct RenderGroup
{
    RenderSetup setup;
    Assets *assets;

    u8 *push_buffer_base;
    u32 push_buffer_size;
    u32 push_buffer_capacity;

    u32 sort_element_count;
};
#define MAX_VERTICES (100000)
#define MAX_INDICES (150000)
typedef struct Renderer Renderer;
struct Renderer
{
    VertexData vertices[MAX_VERTICES];
    u32 vertex_count;
    u32 indices[MAX_INDICES];
    u32 indices_count;

    u32 VBO;
    u32 EBO;
    u32 VAO;

    u32 slot;

    vec3 light_pos;

    i32 screen_width;
    i32 screen_height;

    Assets *assets;

    RenderGroup render_groups[10];
    u32 render_group_count;

    u32 white_sprite;
};
