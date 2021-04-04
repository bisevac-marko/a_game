#if !defined(DEBUG_H)
#define DEBUG_H

#define MAX_NAME_LEN 9

struct UiItemId
{
    void* id;
};

enum InteractableUiItemType
{
    INTERACTABLE_TYPE_CLICK = 1,
    INTERACTABLE_TYPE_DRAG = 2,
};

struct InteractableUiItem
{
    InteractableUiItemType type;
    UiItemId id;
    InteractableUiItem *next_item;
};


struct DebugState
{
    MemoryArena arena;

    InteractableUiItem hot_item;
    InteractableUiItem next_hot_item;
    InteractableUiItem interacting_item;

    vec2 draw_cursor;
    vec2 prev_draw_cursor;


    f32 game_fps;
    vec2 mouse_pos;
    vec2 prev_mouse_pos;
    vec2 menu_pos;
};


internal inline void
print_vec3(vec3 v, char* name)
{
    DEBUG_PRINT("v.x: %.2f", v.x);
    DEBUG_PRINT("v.y: %.2f", v.y);
    DEBUG_PRINT("v.z: %.2f", v.z);
}

#endif
