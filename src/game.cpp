// 090035212100108

#include <math.h>
#include <stdio.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include "common.h"
#include "platform.h"

Platform* g_Platform;

#include "log.h"
#include "headers/memory.h"
#include "memory.cpp"
#include "array.cpp"
#include "hashmap.cpp"
#include "string.cpp"
#include "headers/math.h"
#include "math.cpp"
#include "headers/input.h"
#include "input.cpp"
#include "headers/renderer.h"
#include "renderer.cpp"
#include "headers/debug.h"
#include "headers/game.h"
#include "render_group.cpp"
#include "asset.cpp"
#include "entity.cpp"

#include "generated_print.cpp"

#include "debug_ui.cpp"


// NOTE: Debug data

global_variable b8 global_is_edit_mode = 0;

internal void
GamePlaySound(GameSoundBuffer* game_sound, GameState* game_state)
{
    i32 wave_period = game_sound->samples_per_sec / game_state->tone_hz;
    
    i16* samples = game_sound->samples;
    for (i32 sample_index = 0;
         sample_index < game_sound->sample_count;
         ++sample_index)
    {
        f32 sine_value = sinf(game_state->t_sine);
        i16 sample_value = (i16)(sine_value * game_state->tone_volume);
        *samples = sample_value;
        samples++;
        *samples = sample_value;
        samples++;
        game_state->t_sine += 2.0f * PI_F * (1.0f / (f32)wave_period);
        if (game_state->t_sine > 2.0f * PI_F)
        {
            game_state->t_sine -= 2.0f * PI_F;
        }
    }
}


internal b8
IsColliding(vec2 p1, vec2 s1, vec2 p2, vec2 s2)
{
    return (p1.x < p2.x + s2.x &&
            p1.x + s1.x > p1.x &&
            p1.y < p2.y + s2.y &&
            p1.y + s1.y > s2.y);
    
}

internal void
AddForce(Rigidbody* rigidbody, vec2 force)
{
    rigidbody->acceleration += force;
}


internal ParticleEmitter
CreateParticleEmitter(MemoryArena* arena,
                      WorldState* world,
                      vec2 min_vel,
                      vec2 max_vel,
                      u32 particle_spawn_rate,
                      Color color,
                      vec2 size,
                      u32 num_particles)
{
    ParticleEmitter pa = {};
    
    pa.min_vel = min_vel;
    pa.max_vel = max_vel;
    
    pa.particle_spawn_rate = particle_spawn_rate;
    
    pa.color = color;
    pa.size = size;
    
    pa.max_particles = num_particles;
    
    pa.particles = PushMemory(arena, Particle, num_particles);
    
    return pa;
}

internal f32
RandomBetweenFloats(f32 min, f32 max)
{
    f32 random = ((f32)rand()) / (f32)RAND_MAX;
    f32 diff = max - min;
    f32 r = random * diff;
    return min + r;
}

internal vec2
RandomBetweenVectors(vec2 min, vec2 max)
{
    return V2(RandomBetweenFloats(min.x, max.x),
              RandomBetweenFloats(min.y, max.y));
}

internal void
ParticleUpdate(GameState* game_state, Array* entities)
{
    WorldState* world = &game_state->world;
    float dt = game_state->delta_time;
    
    for (u32 i = 0; i < entities->count; ++i)
    {
        EntityId entity = *ArrayGet(entities, i, EntityId);
        ParticleEmitter* emitter = GetComponent(world, entity, ParticleEmitter);
        
        for (u32 particle_index = 0; 
             particle_index < emitter->max_particles;
             ++particle_index)
        {
            Particle* particle = emitter->particles + particle_index;
            // Simulate
            particle->position += particle->velocity * dt;
            
            //Render
            PushQuad(game_state->render_group, 
                     particle->position,
                     emitter->size,
                     emitter->color,
                     LAYER_MID);
        }
        
        // Spawn new particles
        for (u32 i = 0; i < emitter->particle_spawn_rate; ++i)
        {
            if (emitter->particle_index >= emitter->max_particles)
            {
                emitter->particle_index = 0;
            }
            
            Particle* particle = emitter->particles + emitter->particle_index;
            particle->position = emitter->position;
            particle->velocity = RandomBetweenVectors(emitter->min_vel,
                                                      emitter->max_vel);
            
            ++emitter->particle_index;
        }
    }
}

internal void 
RenderUpdate(GameState* game_state, Array* entities)
{
    WorldState* world = &game_state->world;
    for (u32 i = 0; i < entities->count; ++i)
    {
        EntityId entity = *ArrayGet(entities, i, EntityId);
        Transform* transform = GetComponent(world, entity, Transform);
        Render* render = GetComponent(world, entity, Render);
        
        PushQuad(game_state->render_group,
                 transform->position,
                 transform->scale,
                 render->color,
                 render->layer,
                 render->sprite);
        
    }
}

internal void 
PhysicsUpdate(GameState* game_state, Array* entities)
{
    WorldState* world = &game_state->world;
    f32 dt = game_state->delta_time;
    
    for (u32 i = 0; i < entities->count; ++i)
    {
        EntityId entity = *ArrayGet(entities, i, EntityId);
        Transform* transform = GetComponent(world, entity, Transform);
        Rigidbody* rigidbody = GetComponent(world, entity, Rigidbody);
        
        if (rigidbody->mass <= 0.0f)
            return;
        
        transform->position += rigidbody->velocity * dt;
        
        rigidbody->acceleration /= rigidbody->mass;
        
        rigidbody->velocity += rigidbody->acceleration * dt;
        
        f32 drag = powf(0.5f, dt);
        rigidbody->velocity *= drag;
        
        rigidbody->acceleration = V2(0.0f);
        
    }
}

internal void
InitGrid(WorldState* world)
{
    world->grid_width = 50;
    world->grid_height = 50;
    float offset = 4;
    
    for (u16 x = 0; x < world->grid_width; ++x)
    {
        for (u16 y = 0; y < world->grid_height; ++y)
        {
            EntityId chunk = NewEntity(world);
            
            AddComponent(world, chunk, Transform);
            AddComponent(world, chunk, Render);
            
            Render* render = GetComponent(world, chunk, Render);
            Transform* transform = GetComponent(world, chunk, Transform);
            
            vec2 size = V2(40, 40);
            
            *render = 
            {
                0,
                NewColor(120, 20, 40, 255),
                LAYER_BACKMID,
            };
            *transform = 
            {
                V2((size.x + offset) * x, (size.y + offset) * y),
                size,
                V2(0),
            };
        }
    }
}

extern "C" PLATFORM_API void
GameMainLoop(f32 delta_time, GameMemory* memory, GameSoundBuffer* game_sound, GameInput* input)
{
    GameState* game_state = (GameState*)memory->permanent_storage;
    game_state->delta_time = delta_time;
    Renderer* ren = &game_state->renderer;
    input->mouse.position.y = ren->screen_height - input->mouse.position.y;
    
    if (ren->screen_width != memory->screen_width ||
        ren->screen_height != memory->screen_height)
    {
        ren->screen_width = memory->screen_width;
        ren->screen_height = memory->screen_height;
        
        for (u32 i = 0; i < ren->render_group_count; ++i)
        {
            ren->render_groups[i].setup.projection = Mat4Orthographic(ren->screen_width,
                                                                      ren->screen_height);
        }
    }
    
    
    if (!memory->is_initialized)
    {
        memory->is_initialized = true;
        g_Platform = &memory->platform;
        
        
        InitArena(&game_state->arena, 
                  (memory->permanent_storage_size - sizeof(GameState)),
                  (u8*)memory->permanent_storage + sizeof(GameState));
        
        InitArena(&game_state->flush_arena, 
                  (memory->temporary_storage_size),
                  (u8*)memory->temporary_storage);
        
        AssetsInit(&game_state->assets, &game_state->flush_arena);
        
        memory->debug = PushMemory(&game_state->arena, DebugState);
        SubArena(&game_state->arena, &memory->debug->arena, Megabytes(12));
        
        
        //ren->camera.up = V3(0, 1, 0);
        //ren->camera.direction = V3(0, 0, 1);
        //ren->camera.position.z = 9000.0f;
        
        ren->assets = &game_state->assets;
        
        // NOTE: needs to be first image loaded
        Assert(game_state->assets.num_sprites == 0);
        ren->white_sprite = LoadSprite(g_Platform, &game_state->assets, "../assets/white.png");
        
        g_Platform->InitRenderer(&game_state->renderer);
        game_state->render_group = CreateRenderGroup(&game_state->flush_arena, Mat4Orthographic((f32)ren->screen_width, (f32)ren->screen_height),
                                                     CreateCamera(Vec3Up(), Vec3Forward(), V3(0.0f, 0.0f, 300.0f)),
                                                     &game_state->renderer,
                                                     &game_state->assets);
        
        
        UiInit(memory->debug, ren, &memory->platform, &game_state->assets);
        
        
        game_state->minotaur_sprite = LoadSprite(g_Platform, &game_state->assets, "../assets/minotaur.png");
        game_state->hero_sprite_sheet = LoadSprite(g_Platform, &game_state->assets, 
                                                   "../assets/platform_metroidvania asset pack v1.01/herochar sprites(new)/herochar_spritesheet(new).png");
        game_state->goblin_sprite_sheet = LoadSprite(g_Platform, &game_state->assets, 
                                                     "../assets/platform_metroidvania asset pack v1.01/enemies sprites/bomber goblin/goblin_bomber_spritesheet.png");
        game_state->hero_sprite = SubspriteFromSprite(&game_state->assets,
                                                      game_state->hero_sprite_sheet,
                                                      0, 11,
                                                      16, 16);
        game_state->goblin_sprite = SubspriteFromSprite(&game_state->assets,
                                                        game_state->goblin_sprite_sheet,
                                                        0, 1,
                                                        16, 16);
        
        game_state->backgroud_sprite = LoadSprite(g_Platform, &game_state->assets,
                                                  "../assets/platform_metroidvania asset pack v1.01/tiles and background_foreground/background.png");
        
        
        WorldState* world = &game_state->world;
        
        InitWorld(&game_state->arena, world);
        ComponentType components[] = 
        {
            ComponentTransform,
            ComponentRender,
        };
        RegisterSystem(world, components, ArrayCount(components), RenderUpdate);
        
        components[0] = ComponentTransform;
        components[1] = ComponentRigidbody;
        
        RegisterSystem(world, components, ArrayCount(components), PhysicsUpdate);
        
        ComponentType comps[] = {ComponentParticleEmitter};
        RegisterSystem(world, comps, ArrayCount(comps), ParticleUpdate);
        
        EntityId player = NewEntity(world);
        AddComponent(world, player, Transform);
        AddComponent(world, player, Render);
        AddComponent(world, player, Rigidbody);
        AddComponent(world, player, ParticleEmitter);
        
        ParticleEmitter* player_particles = GetComponent(world, player, ParticleEmitter);
        *player_particles = CreateParticleEmitter(&game_state->flush_arena, world, V2(-100), V2(100), 4, NewColor(255), V2(30), 10000);
        
        Transform* player_transform = GetComponent(world, player, Transform);
        *player_transform = {
            V2(200, 200),
            V2(60, 60),
            V2(0),
        };
        Render* player_render = GetComponent(world, player, Render);
        *player_render = {
            game_state->hero_sprite,
            NewColor(255),
            LAYER_FRONT
        };
        
        Rigidbody* player_rigidbody = GetComponent(world, player, Rigidbody);
        
        *player_rigidbody = {
            V2(0),
            V2(0),
            1.0f,
        };
        
        EntityId goblin = NewEntity(world);
        AddComponent(world, goblin, Render);
        AddComponent(world, goblin, Transform);
        
        Render* goblin_render = GetComponent(world, goblin, Render);
        *goblin_render = {game_state->goblin_sprite, NewColor(255), LAYER_FRONT};
        
        
        InitGrid(world);
        
    }
    
    
    
    UpdateSystems(game_state);
    
    if (!game_state->is_free_camera)
    {
        EntityId player = 0;
        Rigidbody* player_rigid = GetComponent(&game_state->world, player, Rigidbody);
        Transform* player_tran = GetComponent(&game_state->world, player, Transform);
        
        if (ButtonDown(input->move_left))
        {
            AddForce(player_rigid, V2(-1000.0f, 0.0f));
        }
        if (ButtonDown(input->move_right))
        {
            AddForce(player_rigid, V2(1000.0f, 0.0f));
        }
        if (ButtonDown(input->move_up))
        {
            AddForce(player_rigid, V2(0.0f, 1000.0f));
        }
        
        if (player_tran->position.y <= 100.0f)
        {
            AddForce(player_rigid, V2(0.0f, 3000.0f));
        }
        
    }
    
    // TOggle edit mode
    if (ButtonPressed(input->f1))
    {
        global_is_edit_mode = !global_is_edit_mode;
        if (global_is_edit_mode)
        {
            game_state->render_group->setup.projection = Mat4Perspective(ren->screen_width,
                                                                         ren->screen_height,
                                                                         90.0f,
                                                                         1.0f,
                                                                         100000.0f);
        }
        else
        {
            game_state->render_group->setup.projection = Mat4Orthographic((f32)ren->screen_width, (f32)ren->screen_height);
        }
    }
    
    {
        
        Camera* cam = &game_state->render_group->setup.camera;
        
        if (input->mouse.wheel_delta)
        {
            f32 scroll_amount = (f32)input->mouse.wheel_delta;
            cam->position.z -= scroll_amount;
        }
        
        if (ButtonDown(input->move_left))
        {
            cam->position.x -= 5.0f;
        }
        if (ButtonDown(input->move_right))
        {
            cam->position.x += 5.0f;
        }
        if (ButtonDown(input->move_up))
        {
            cam->position.y += 5.0f;
        }
        if (ButtonDown(input->move_down))
        {
            cam->position.y -= 5.0f;
        }
    }
    
    
    
    
    if (global_is_edit_mode)
    {
        UiStart(memory->debug, input, &game_state->assets, ren);
        
        UiFps(memory->debug);
        
        UiWindowBegin(memory->debug, "Window");
        
        if (UiSubmenu(memory->debug, "Camera"))
        {
            Camera* camera = &game_state->render_group->setup.camera;
            UiFloat32Editbox(memory->debug, &camera->up, "up");
            UiFloat32Editbox(memory->debug, &camera->direction, "direction");
            UiFloat32Editbox(memory->debug, &camera->position, "position");
        }
        
        if (UiSubmenu(memory->debug, "Player"))
        {
            EntityId player = 0;
            Rigidbody* rigid = GetComponent(&game_state->world, player, Rigidbody);
            Transform* trans = GetComponent(&game_state->world, player, Transform);
            Render* render = GetComponent(&game_state->world, player, Render);
            
            UiCheckbox(memory->debug, &game_state->is_free_camera, "free camera");
            UiFloat32Editbox(memory->debug, &rigid->velocity, "velocity");
            UiFloat32Editbox(memory->debug, &rigid->mass, "mass");
            
            UiFloat32Editbox(memory->debug, &trans->position, "position");
            UiFloat32Editbox(memory->debug, &trans->rotation, "rotation");
            UiFloat32Editbox(memory->debug, &trans->scale, "scale");
            
            UiColorpicker(memory->debug, &render->color, "color");
        }
        
        
        UiWindowEnd(memory->debug);
        
        EndTemporaryMemory(&memory->debug->temp_arena);
    }
    
    
    
    g_Platform->EndFrame(ren);
}




