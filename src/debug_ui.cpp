global_variable Color bg_main_color = {43, 46, 74, 255};
global_variable Color faint_bg_color = {25, 25, 45, 255};
global_variable Color faint_red_color = {144, 55, 33, 255};
global_variable Color faint_redblue_color = {83, 53, 74, 255};
global_variable Color red_color = {232, 69, 69, 255};

#define PADDING 5.0f
#define MAX_NAME_WIDTH 120

#define WINDOW_NO_TITLEBAR 0x1
// WARNING: Macro madness bellow, don't look at it for too long!!

#define UI_Float32Editbox(debug, var, name) \
    AnyTypeMultibox(debug, var, (sizeof(*var)/sizeof(f32)), name, f32, false)

#define UI_Float64Editbox(debug, var, name) \
    AnyTypeMultibox(debug, var, (sizeof(*var)/sizeof(f64)), name, f64, false)

#define UI_Int32Editbox(debug, var, name) \
    AnyTypeMultibox(debug, var, (sizeof(*var)/sizeof(i32)), name, i32, true)

#define UI_Int8Editbox(debug, var, name) \
    AnyTypeMultibox(debug, var, (sizeof(*var)/sizeof(i8)), name, i8, true)

#define UI_UInt8Editbox(debug, var, name) \
    AnyTypeMultibox(debug, var, (sizeof(*var)/sizeof(u8)), name, u8, true)


#define AnyTypeMultibox(debug, v, num_components, var_name, type, is_int) \
{ \
    f32 total_width = (debug->current_window->size.x - MAX_NAME_WIDTH - 2*PADDING) - ((num_components-1) * PADDING); \
    type* iterator = (type*)v; \
    DrawEditbox(debug, iterator, (*iterator), var_name, V2(total_width/num_components, 20), type, is_int) \
 \
    for (u32 i = 1; i < num_components; ++i) \
    { \
        ui_cursor_sameline(debug); \
        iterator++; \
        DrawEditbox(debug, iterator, (*iterator), 0, V2(total_width/num_components, 20), type, is_int) \
    } \
} \

#define DrawEditbox(debug, pointer, value, var_name, size, type, is_int) \
{ \
    RenderGroup* group = debug->render_group; \
    u32 layer = debug->current_window ? debug->current_window->layer : 0; \
    if (var_name) \
    { \
        ui_cursor_newline(debug, size.x + MAX_NAME_WIDTH, size.y); \
    } \
    else \
    { \
        ui_cursor_newline(debug, size.x, size.y); \
    } \
    vec2 pos = debug->draw_cursor; \
    Color color = bg_main_color; \
    b8 is_editing_box = false; \
    if (var_name) \
    { \
        if (name_is_too_long(&debug->font, var_name)) \
        { \
            char* temp_name = string_copy(&debug->arena, var_name); \
            truncate_var_name(temp_name); \
            draw_text(debug->render_group,  \
                       &debug->font, temp_name,  \
                       V2(pos.x, pos.y + (size.y - debug->font.font_size)),  \
                       layer + LAYER_FRONT); \
        } \
        else \
        { \
            draw_text(debug->render_group,  \
                       &debug->font, var_name,  \
                       V2(pos.x, pos.y + (size.y - debug->font.font_size)),  \
                       layer + LAYER_FRONT); \
        } \
    } \
    pos.x += MAX_NAME_WIDTH; \
 \
 \
    char* text = get_text_to_draw(&debug->arena, value); \
 \
    if (is_active(debug, pointer)) \
    { \
        if (button_pressed(debug->input.escape)) \
        { \
            debug->active_item = 0; \
        } \
        ui_process_text_input(debug); \
 \
        if (button_pressed(debug->input.enter)) \
        { \
            i8 sign = 1; \
            if (debug->text_input_buffer[0] == '-') \
            { \
                for (u32 i = 0; i < debug->text_insert_index; ++i) \
                { \
                    debug->text_input_buffer[i] = debug->text_input_buffer[i+1]; \
                } \
                sign = -1; \
            } \
 \
            type number = 0; \
            if (is_int) \
            { \
                number = (type)atoi(debug->text_input_buffer); \
            } \
            else \
            { \
                number = (type)atof(debug->text_input_buffer); \
            } \
            *pointer = number * sign; \
            debug->active_item = 0;\
        } \
 \
        is_editing_box = true; \
        draw_text(debug->render_group, &debug->font, debug->text_input_buffer, V2(pos.x, pos.y + (size.y - debug->font.font_size)), layer + LAYER_FRONT); \
        debug->text_input_buffer[debug->text_insert_index] = '\0'; \
 \
        f32 cursor_x = pos.x + get_text_width(&debug->font, debug->text_input_buffer); \
        push_quad(debug->render_group, V2(cursor_x, pos.y), V2(3, size.y), faint_red_color, layer + LAYER_FRONT); \
    } \
    else \
    { \
        if (is_hot(debug, pointer)) \
        { \
            color.r *= 2; \
            color.g *= 2; \
            color.b *= 2; \
            if (button_pressed(debug->input.left_mouse_button)) \
            { \
                set_active(debug, pointer); \
                fill_text_buffer(debug, value); \
                debug->editbox_value_to_set = iterator; \
                debug->editbox_value_to_set_size = sizeof(type); \
                debug->editbox_value_is_int = is_int; \
            } \
        } \
        draw_text(debug->render_group,  \
                   &debug->font, \
                   text,  \
                   V2(pos.x + (size.x / 2),  \
                      pos.y + (size.y - debug->font.font_size)), \
                   layer + LAYER_FRONT, \
                   TEXT_ALIGN_MIDDLE); \
    } \
 \
    push_quad(group, pos, size, color, layer + LAYER_MID); \
 \
    if (window_is_focused(debug, debug->current_window)) \
    { \
        if (point_is_inside_rect(debug->input.mouse.position, pos, size)) \
        { \
            debug->hot_item = pointer; \
        } \
    } \
} \


internal void
ui_process_text_input(DebugState* debug)
{
    GameInput* input = &debug->input;

    if (input->character != '\0')
    {
        char c = input->character;
        if ((c >= 48 && c < 58) ||
            c == 46 ||
            c == 43 ||
            c == 45)
        {
            debug->text_input_buffer[debug->text_insert_index] = input->character;
            debug->text_input_buffer[debug->text_insert_index + 1] = '\0';
            ++debug->text_insert_index;
        }
    }
    if (button_pressed(input->backspace) && 
        modifier_pressed(input, SHIFT_MODIF))
    {
        debug->text_insert_index = 0;
        debug->text_input_buffer[0] = '\0';
    }
    else if (button_pressed(input->backspace) &&
        debug->text_insert_index)
    {
            --debug->text_insert_index;
            debug->text_input_buffer[debug->text_insert_index] = '\0';
    }
}

internal b8 
window_is_focused(DebugState* debug, UiWindow* window)
{
    return debug->focused_window == window;
}

internal void
ui_set_window_focus(DebugState* debug, UiWindow* window)
{
    // TODO: this is a hack
    if (debug->top_layer >= 10000)
    {
        debug->top_layer = 0;
    }
    debug->top_layer += 20;
    window->layer = debug->top_layer;
    debug->focused_window = window;
}


// TODO: handle collisions
internal UiWindow*
get_window(DebugState* debug, char* key)
{
    u64 hash = 5381;
    i32 c;
    
    while((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }

    u16 hash_index = hash % ArrayCount(debug->window_table);

    UiWindow* ui_window = debug->window_table + hash_index;

    return ui_window;
}

internal void
set_window_position(DebugState* debug, vec2 position, char* window_name)
{
    get_window(debug, window_name)->position = position;
}

internal void
set_window_size(DebugState* debug, vec2 size, char* window_name)
{
    get_window(debug, window_name)->size = size;
}

internal b8
is_hot(DebugState* debug, void* id)
{
    return debug->next_hot_item == id;
}


internal b8
is_active(DebugState* debug, void* id)
{
    return debug->active_item == id;
}

internal void
set_active(DebugState* debug, void* id)
{
    debug->hot_item = 0;
    debug->active_item = id;
}

internal void
set_hot(DebugState* debug, void* id)
{

    debug->hot_item = id;
}


enum TextAlign
{
    TEXT_ALIGN_MIDDLE,
    TEXT_ALIGN_LEFT,
};


internal inline void
ui_cursor_sameline(DebugState* debug)
{
    debug->is_newline = false;
}

internal inline vec2
ui_cursor_newline(DebugState* debug, f32 w, f32 h)
{
    if (debug->is_newline)
    {
        debug->draw_cursor.x = debug->current_window->position.x + PADDING;
        debug->draw_cursor.y -= h + PADDING;
    }
    else
    {
        debug->draw_cursor.x += w + PADDING;
    }
    debug->is_newline = true;

    return debug->draw_cursor;
}

internal b8
name_is_too_long(Font* font, char* name)
{
    f32 xadvance = 0.0f;
    while(*name)
    {
        u32 index = *name-32;
        xadvance += font->char_metrics[index].xadvance;
        name++;
    }

    return (xadvance > MAX_NAME_WIDTH);
}

internal void
truncate_var_name(char* name)
{
    u32 len = string_length(name);
    name[len-1] = '\0';
    name[len-2] = '.';
    name[len-3] = '.';
}

internal char*
get_text_to_draw(MemoryArena* arena, u64 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%lu", value);

    return result;
}

internal char*
get_text_to_draw(MemoryArena* arena, f64 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%.2f", value);

    return result;
}

internal char*
get_text_to_draw(MemoryArena* arena, i64 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%li", value);

    return result;
}

internal char*
get_text_to_draw(MemoryArena* arena, u32 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%d", value);

    return result;
}

internal char*
get_text_to_draw(MemoryArena* arena, i32 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%d", value);

    return result;
}

internal char*
get_text_to_draw(MemoryArena* arena, f32 value)
{
    char* result = PushMemory(arena, char, 128);
    snprintf(result, 128, "%.2f", value);

    return result;
}

internal void
delete_trailing_zeros(DebugState* debug)
{
    debug->text_insert_index = string_length(debug->text_input_buffer);

    for (u32 i = debug->text_insert_index-1;;--i)
    {
        if (debug->text_input_buffer[i] == '0')
        {
            --debug->text_insert_index;
        }
        else
        {
            break;
        }
    }

    debug->text_input_buffer[debug->text_insert_index] = '\0';
}
internal void
fill_text_buffer(DebugState* debug, f64 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%f",
             value);
    delete_trailing_zeros(debug);
}

internal void
fill_text_buffer(DebugState* debug, f32 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%f",
             value);
    delete_trailing_zeros(debug);
}

internal void
fill_text_buffer(DebugState* debug, i64 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%li",
             value);
    debug->text_insert_index = string_length(debug->text_input_buffer);
    debug->text_input_buffer[debug->text_insert_index] = '\0';
}

internal void
fill_text_buffer(DebugState* debug, u64 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%lu",
             value);
    debug->text_insert_index = string_length(debug->text_input_buffer);
    debug->text_input_buffer[debug->text_insert_index] = '\0';
}

internal void
fill_text_buffer(DebugState* debug, u32 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%u",
             value);
    debug->text_insert_index = string_length(debug->text_input_buffer);
    debug->text_input_buffer[debug->text_insert_index] = '\0';
}

internal void
fill_text_buffer(DebugState* debug, i32 value)
{
    snprintf(debug->text_input_buffer, 
             ArrayCount(debug->text_input_buffer), 
             "%i",
             value);
    debug->text_insert_index = string_length(debug->text_input_buffer);
    debug->text_input_buffer[debug->text_insert_index] = '\0';
}

internal Font
ui_load_font(MemoryArena* arena, Platform* platform, Assets* assets, char* font_path, i32 font_size)
{
    Font font = {};
    font.char_subsprites = PushMemory(arena, SubSprite, NUM_ASCII);
    font.char_metrics = PushMemory(arena, CharMetric, NUM_ASCII);
    font.num_chars = NUM_ASCII;


    font.font_sprite_handle = create_empthy_sprite(assets, 512, 512, 1);
    font.font_size = font_size;

    Sprite* font_sprite = get_loaded_sprite(assets, font.font_sprite_handle);
    FileResult font_file = platform->read_entire_file(font_path);

    if (font_file.data)
    {

        stbtt_bakedchar char_metrics[NUM_ASCII];
        i32 result = stbtt_BakeFontBitmap((u8*)font_file.data, 
                                          0, (f32)font_size,
                                          (u8*)font_sprite->data, 
                                          font_sprite->width, 
                                          font_sprite->height,
                                          32, NUM_ASCII, 
                                          char_metrics);
        if (!result)
        {
            PRINT("Failed to bake font map\n");
        }

        for(u8 i = 0; i < NUM_ASCII; ++i)
        {
            #define IFLOOR(x) ((int) floor(x))
            stbtt_aligned_quad q;
            // i32 index = *text-32;
            stbtt_bakedchar* b = char_metrics + i;

            font.char_metrics[i].x0 = b->x0;
            font.char_metrics[i].y0 = b->y0;
            font.char_metrics[i].x1 = b->x1;
            font.char_metrics[i].y1 = b->y1;
            font.char_metrics[i].xoff = b->xoff;
            font.char_metrics[i].yoff = b->yoff;
            font.char_metrics[i].xadvance = b->xadvance;

            f32 ipw = 1.0f / font_sprite->width;
            f32 iph = 1.0f / font_sprite->height;

            q.s0 = b->x0 * ipw;
            q.t0 = b->y0 * iph;
            q.s1 = b->x1 * ipw;
            q.t1 = b->y1 * iph;

            SubSprite* subsprite = font.char_subsprites + i;

            subsprite->uvs[0] = {q.s1, q.t1};
            subsprite->uvs[1] = {q.s1, q.t0};
            subsprite->uvs[2] = {q.s0, q.t0};
            subsprite->uvs[3] = {q.s0, q.t1};

            subsprite->sprite_sheet = font.font_sprite_handle;
        }
        platform->free_file_memory(font_file.data);
    }
    else
    {
        PRINT("Failed to load font [%s] file.", font_path);
    }


    return font;
}

internal f32
get_text_width(Font* font, char* text)
{
    f32 out = 0;

    while(*text)
    {
        u32 index = *text-32;
        out += font->char_metrics[index].xadvance;

        text++;
    }

    return out;
}

internal void
draw_text(RenderGroup* render_group,
           Font* font,
           char* text,
           vec2 position,
           u32 layer,
           TextAlign align = TEXT_ALIGN_LEFT,
           Color color = {255, 255, 255, 254})
{
    if (align == TEXT_ALIGN_MIDDLE)
    {
        f32 text_width = get_text_width(font, text);
        position.x -= (text_width / 2.0f);
    }

    while(*text)
    {
        if (*text >= 32 && *text <= 128)
        {
            #define IFLOOR(x) ((int) floor(x))
            i32 index = *text-32;
            CharMetric* cm = font->char_metrics + index;

            i32 round_x = IFLOOR((position.x + cm->xoff) + 0.5f);
            i32 round_y = IFLOOR((position.y - cm->yoff) + 0.5f);

            vec2 pos = V2(round_x, round_y);
            vec2 size = V2(cm->x1 - cm->x0,
                           -cm->y1 + cm->y0);

            push_quad(render_group, font->char_subsprites + index, pos, size, color, layer);

            position.x += cm->xadvance;
        }
        text++;
    }
}

// TODO: title is not a good pointer for id
internal void
ui_submenu_titlebar(DebugState* debug, vec2 position, vec2 size, char* title)
{
    Color color = bg_main_color;
    u32 layer = debug->current_window ? debug->current_window->layer : 0;

    if (is_hot(debug, title))
    {
        color = faint_red_color;
        if (button_pressed(debug->input.left_mouse_button))
        {
            color = faint_redblue_color;
            b8* active = &debug->sub_menus[debug->sub_menu_index].is_active;
            *active = !(*active);
        }
    }

    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position,
                                 position,
                                 size))
        {
            debug->hot_item = title;
        }
    }

    push_quad(debug->render_group, position, size, color, layer + LAYER_MID);
    draw_text(debug->render_group, 
               &debug->font, 
               title, 
               V2(position.x + (size.x / 2),
                  position.y + 2),
               layer + LAYER_FRONT,
               TEXT_ALIGN_MIDDLE);
}

internal void
imediate_ui(DebugState* debug, GameInput* input, Assets* assets, Renderer* ren)
{
    if (debug->temp_arena.arena)
    {
        end_temporary_memory(&debug->temp_arena);
    }
    debug->temp_arena = begin_temporary_memory(&debug->arena);
    debug->prev_mouse_pos = debug->input.mouse.position;
    debug->input = *input;
    debug->screen_width = ren->screen_width;
    debug->screen_height = ren->screen_height;
    debug->current_window = 0;
    debug->sub_menu_index = 0;

    mat4 projection = mat4_orthographic((f32)ren->screen_width,
                                        (f32)ren->screen_height);
    debug->render_group = setup_render_group(&debug->arena,
                                             projection,
                                             ren, 
                                             assets);
    debug->next_hot_item = debug->hot_item;
    debug->hot_item = 0;


    if (button_pressed(input->left_mouse_button))
    {
        UiWindow** windows =  PushMemory(&debug->arena, 
                                             UiWindow*, 
                                             ArrayCount(debug->window_table));
        u32 len = 0;

        for (u32 i = 0; i < ArrayCount(debug->window_table); ++i)
        {
            UiWindow* window = debug->window_table + i;
            if (window->size.x)
            {
                windows[len] = window;
                len++;
            }
        }
        // TODO: learn better sort..

        // Sort by layer
        for (i32 i = 0; i < (i32)len; ++i)
        {
            UiWindow* win = windows[i];

            i32 j = i - 1;
            while (j >= 0 && windows[j]->layer < win->layer)
            {
                windows[j+1] = windows[j];
                --j;
            }
            windows[j+1] = win;
        }


        // Top window should always be focused
        UiWindow* focused_window = windows[0];
        if (point_is_inside_rect(input->mouse.position,
                              focused_window->position,
                              focused_window->size))
        {
            return;
        }

        for (u32 i = 1; i < len; ++i)
        {
            UiWindow* window = windows[i];
            if (point_is_inside_rect(input->mouse.position,
                                  window->position,
                                  window->size))
            {
                ui_set_window_focus(debug, window);
                break;
            }
        }
    }
}

internal void
push_window(DebugState* debug, UiWindow* window)
{
    StackedWindow* sw = debug->window_stack + debug->num_stacked_windows;
    sw->window = window;
    sw->previous_draw_cursor = debug->draw_cursor;

    ++debug->num_stacked_windows;
    ASSERT(debug->num_stacked_windows < ArrayCount(debug->window_stack));
}

internal StackedWindow*
pop_window(DebugState* debug)
{
    --debug->num_stacked_windows;
    return debug->window_stack + debug->num_stacked_windows;
}


internal void
ui_window_begin(DebugState* debug, char* title, vec2 pos = V2(0), vec2 size = V2(400, 500), u32 style_flags = 0)
{
    UiWindow* window = get_window(debug, title);

    ASSERT(window);

    if (debug->current_window)
    {
        push_window(debug, debug->current_window);
        debug->do_pop_window = true;
    }

    debug->current_window = window;
    u32 layer = window->layer;


    if (window->position.x <= 0 && window->position.y <= 0)
    {
        window->size = size;
        window->position = pos;
    }


    Color color = faint_red_color;

    if (is_active(debug, window))
    {
        if (button_down(debug->input.left_mouse_button))
        {
            color = faint_red_color;
            vec2 mouse_delta = (debug->input.mouse.position - debug->prev_mouse_pos);
            window->position = window->position + mouse_delta;
        }
        else
        {
            debug->active_item = 0;
        }
    }
    else if (is_hot(debug, window))
    {
        color = red_color;
        if (button_pressed(debug->input.left_mouse_button) ||
            button_down(debug->input.left_mouse_button))
        {
            set_active(debug, window);
        }
    }

    // we start from top
    debug->draw_cursor = V2(window->position.x,
                            window->position.y + window->size.y);

    push_quad(debug->render_group, 
              debug->current_window->position,
              debug->current_window->size,
              faint_bg_color,
              debug->current_window->layer + LAYER_BACKMID);

    if ((style_flags & WINDOW_NO_TITLEBAR) != WINDOW_NO_TITLEBAR)
    {
        vec2 titlebar_size = V2((f32)window->size.x, (f32)debug->font.font_size);
        debug->draw_cursor.y -= titlebar_size.y;
        vec2 titlebar_pos = debug->draw_cursor;


        // TODO window focus
        if (!debug->focused_window)
        {
            ui_set_window_focus(debug, window);
        }


        if (window_is_focused(debug, debug->current_window))
        {
            if (point_is_inside_rect(debug->input.mouse.position,
                                     titlebar_pos,
                                     titlebar_size))
            {
                set_hot(debug, window);
            }
        }

        push_quad(debug->render_group, titlebar_pos, titlebar_size, color, layer + LAYER_MID);
        draw_text(debug->render_group, 
                &debug->font,
                title, 
                V2(titlebar_pos.x + (titlebar_size.x / 2),
                   titlebar_pos.y + 4.0f),
                layer + LAYER_FRONT,
                TEXT_ALIGN_MIDDLE);
    }

}

internal void
ui_window_end(DebugState* debug)
{
    if (debug->do_pop_window)
    {
        StackedWindow* sw = pop_window(debug);
        debug->current_window = sw->window;
        debug->draw_cursor = sw->previous_draw_cursor;
        debug->do_pop_window = false;
    }
    else
    {
        debug->current_window = 0;
    }
}

internal void
ui_fps(DebugState* debug)
{
    RenderGroup* group = debug->render_group;
    char* fps_text = "%.2f fps";
    char out[32];
    sprintf(out, fps_text, debug->game_fps);
    vec2 position = V2(0.0f, (f32)debug->screen_height - debug->font.font_size);

    draw_text(debug->render_group, &debug->font, out, position, 100);
}



internal void
ui_checkbox(DebugState* debug, b8* toggle_var, char* var_name)
{
    RenderGroup* group = debug->render_group;

    vec2 size = V2(20);
    vec2 pos = ui_cursor_newline(debug, size.x, size.y);
    u32 layer = debug->current_window ? debug->current_window->layer : 0;

    if (name_is_too_long(&debug->font, var_name))
    {
        var_name = string_copy(&debug->arena, var_name);
        truncate_var_name(var_name);
    }

    draw_text(debug->render_group, 
               &debug->font, 
               var_name, 
               V2(pos.x, pos.y + (size.y - debug->font.font_size)),
               layer + LAYER_FRONT);
    pos.x += MAX_NAME_WIDTH;

    push_quad(group, pos, size, bg_main_color, layer + LAYER_BACKMID);

    Color check_color = {100, 100, 100, 255};

    if (is_active(debug, toggle_var))
    {
        debug->active_item = 0;
    }
    else if (is_hot(debug, toggle_var))
    {
        check_color = faint_red_color;
        if (button_pressed(debug->input.left_mouse_button))
        {
            set_active(debug, toggle_var);
            *toggle_var = !(*toggle_var);
        }
    }

    if (*toggle_var)
    {
        check_color.r *= 2;
        check_color.g *= 2;
        check_color.b *= 2;
    }

    push_quad(group, pos, size, bg_main_color, layer + LAYER_BACKMID);
    size -= 8;
    pos += 4;
    push_quad(group, pos, size, check_color, layer + LAYER_BACKMID);

    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position,
                            pos,
                            size))
        {
            debug->hot_item = {toggle_var};
        }
    }
}

internal b8
ui_button(DebugState* debug, char* name, vec2 pos = V2(0), vec2 size = V2(80, 40))
{
    RenderGroup* group = debug->render_group;


    if (debug->current_window)
        pos = ui_cursor_newline(debug, size.x, size.y);


    Color button_color = bg_main_color;
    b8 result = false;
    u32 layer = debug->current_window ? debug->current_window->layer : 0;


    if (is_active(debug, name))
    {
        if (button_released(debug->input.left_mouse_button))
        {
            result = true;
            debug->active_item = 0;
        }
        button_color = faint_red_color;
    }
    else if (is_hot(debug, name))
    {
        if (button_pressed(debug->input.left_mouse_button))
        {
            set_active(debug, name);
        }
    }

    push_quad(group, pos, size, button_color, layer + LAYER_MID);
    vec2 text_pos = V2(
        pos.x + (size.x / 2.0f),
        pos.y + (size.y / 2.0f) - (debug->font.font_size / 4.0f)
    );
    draw_text(group, &debug->font, name, text_pos, layer + LAYER_FRONT, TEXT_ALIGN_MIDDLE);


    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position,
                            pos,
                            size))
        {
            debug->hot_item = {name};
        }
    }


    return result;
}


internal void
ui_slider(DebugState* debug, 
          f32 min,
          f32 max, 
          f32* value, 
          char* var_name)
{
    RenderGroup* group = debug->render_group;

    vec2 bar_size = V2(debug->current_window->size.x - MAX_NAME_WIDTH - 2*PADDING, 20.0f);
    vec2 pos = ui_cursor_newline(debug, bar_size.x + MAX_NAME_WIDTH, bar_size.y);

    u32 layer = debug->current_window ? debug->current_window->layer : 0;

    if (name_is_too_long(&debug->font, var_name))
    {
        var_name = string_copy(&debug->arena, var_name);
        truncate_var_name(var_name);
    }
    draw_text(debug->render_group, 
               &debug->font, 
               var_name, 
               V2(pos.x, pos.y + (bar_size.y - debug->font.font_size)), 
               layer + LAYER_FRONT);

    pos.x += MAX_NAME_WIDTH;

    vec2 button_position = pos;
    vec2 button_size = V2(10, bar_size.y);
    button_position.y -= (button_size.y - bar_size.y) / 2.0f;

    f32 range = max - min;


    Color button_color = faint_redblue_color;

    if (is_active(debug, value))
    {
        if (button_down(debug->input.left_mouse_button))
        {
            button_color = red_color;
            f32 slide_amount = (debug->input.mouse.position.x - pos.x);
            slide_amount = CLAMP(slide_amount, 0.0f, bar_size.x - button_size.x);

            f32 update_value = slide_amount / (bar_size.x - button_size.x);

            *value = (range * update_value) + min;
        }
        else
        {
            debug->active_item = 0;
        }
    }
    else if (is_hot(debug, value))
    {
        button_color = faint_red_color;
        if (button_pressed(debug->input.left_mouse_button))
        {
            set_active(debug, value);
        }
    }

    push_quad(group, pos, bar_size, bg_main_color, layer + LAYER_MID);


    vec2 text_pos = V2(pos.x+(bar_size.x/2), pos.y + (bar_size.y - debug->font.font_size));
    char text[32];
    snprintf(text, 32, "%.2f", *value);
    draw_text(debug->render_group, &debug->font, text, text_pos, layer + LAYER_FRONT, TEXT_ALIGN_MIDDLE);

    button_position.x += (((*value)-min)/range) * (bar_size.x - button_size.x);
    push_quad(group, button_position, button_size, button_color, layer + LAYER_FRONT);

    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position, button_position, button_size))
        {
            debug->hot_item = value;
        }
    }

}

internal void
hue_slider(DebugState* debug, Colorpicker* cp)
{

    Color cursor_color = COLOR(255);
    vec2 size = V2(20, 200);
    ui_cursor_newline(debug, size.x, size.y);
    vec2 pos = debug->draw_cursor;

    if (is_active(debug, &cp->hue))
    {
        if (button_down(debug->input.left_mouse_button))
        {
            cursor_color.a = 180;
            cp->hue = (debug->input.mouse.position.y - pos.y) / size.y;
            cp->hue = CLAMP(cp->hue, 0.0f, 1.0f);
        }
        else
        {
            debug->active_item = 0;
        }
    }
    else if (is_hot(debug, &cp->hue))
    {
        if (button_pressed(debug->input.left_mouse_button))
        {
            set_active(debug, &cp->hue);
        }
    }
    Shader* hue_shader = get_shader(debug->render_group->assets, SHADER_ID_HUE_QUAD);
    vec2 cursor_pos = V2(pos.x, pos.y + (cp->hue * size.y));

    set_shader_uniform(hue_shader, "u_size", V2(20, 200), vec2);
    set_shader_uniform(hue_shader, "u_position", pos, vec2);

    push_quad(debug->render_group, 
              pos,
              size,
              COLOR(100, 0, 40, 255),
              debug->current_window->layer + LAYER_FRONT,
              0,
              SHADER_ID_HUE_QUAD);

    push_quad(debug->render_group,
              cursor_pos,
              V2(20, 4),
              cursor_color,
              debug->current_window->layer + LAYER_FRONT + 10.0f,
              0,
              SHADER_ID_NORMAL);

    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position, pos, size))
        {
            debug->hot_item = &cp->hue;
        }
    }


}

internal void
hsb_picker(DebugState* debug, Colorpicker* cp)
{
    vec2 pos = debug->draw_cursor;
    vec2 size = V2(200, 200);
    Color cursor_color = COLOR(255);
    // TODO: this should use ui_same_line() API
    pos.x += 40;

    if (is_active(debug, &cp->saturation))
    {
        if (button_down(debug->input.left_mouse_button))
        {
            if (point_is_inside_rect(debug->input.mouse.position,
                                     pos,
                                     size))
            {
                cursor_color.a = 180;
                cp->saturation = (debug->input.mouse.position.x - pos.x) / size.x;
                cp->brightness = (debug->input.mouse.position.y - pos.y) / size.y;
            }
        }
        else
        {
            debug->active_item = 0;
        }
    }
    else if (is_hot(debug, &cp->saturation))
    {
        if (button_pressed(debug->input.left_mouse_button))
        {
            set_active(debug, &cp->saturation);
        }
    }

    vec2 cursor_pos = pos + (size * V2(cp->saturation, cp->brightness));

    Shader* sb_shader = get_shader(debug->render_group->assets, SHADER_ID_SB_QUAD);
    set_shader_uniform(sb_shader, "u_size", V2(200, 200), vec2);
    set_shader_uniform(sb_shader, "u_position", pos, vec2);
    set_shader_uniform(sb_shader, "u_hue", cp->hue, f32);

    push_quad(debug->render_group, 
              pos,
              size,
              COLOR(255),
              debug->current_window->layer + LAYER_FRONT,
              0,
              SHADER_ID_SB_QUAD);

    push_quad(debug->render_group, 
              cursor_pos,
              V2(4.0f),
              cursor_color,
              debug->current_window->layer + LAYER_FRONT + 10.0f,
              0,
              SHADER_ID_NORMAL);

    if (window_is_focused(debug, debug->current_window))
    {
        if (point_is_inside_rect(debug->input.mouse.position, pos, size))
        {
            debug->hot_item = &cp->saturation;
        }
    }

}

internal Color 
hsb2rgb(vec3 c)
{
    vec3 rgb = CLAMP_V3(ABS_V3(FMOD_V3(V3(0.0f, 4.0f, 2.0f) + c.x * 6.0f,
                             6.0f) - 3.0f) - 1.0f,
                     0.0f,
                     1.0f );
    rgb = rgb*rgb*(rgb * -2.0f + 3.0f);

    c =  vec3_mix(V3(1.0f), rgb, c.y) * c.z;
    Color result;
    result.r = c.x * 255;
    result.g = c.y * 255;
    result.b = c.z * 255;
    result.a = 255;

    return result;
}

internal Colorpicker*
get_colorpicker(DebugState* debug, void* key)
{
    u64 hash = ((u64)key) >> 3;

    u16 hash_index = hash % ArrayCount(debug->colorpickers);

    Colorpicker* cp = debug->colorpickers + hash_index;

    // TODO: currenty not handling collisions..
    do
    {
        if (cp->key == key)
        {
            break;
        }

        if (!(cp->next) && !(cp->key))
        {
            cp->key = key;
        }

    } while(true);


    return cp;
}

internal void
ui_color_picker(DebugState* debug, Color* var, char* var_name)
{
    vec2 size = V2(200, debug->font.font_size);
    Color color = *var;

    u32 layer = debug->current_window ? debug->current_window->layer : 0;
    vec2 pos = ui_cursor_newline(debug, size.x, size.y);


    if (name_is_too_long(&debug->font, var_name))
    {
        var_name = string_copy(&debug->arena, var_name);
        truncate_var_name(var_name);
    }

    draw_text(debug->render_group, 
            &debug->font, 
            var_name, 
            V2(pos.x, pos.y + (size.y - debug->font.font_size)), 
            layer + LAYER_FRONT);
    pos.x += MAX_NAME_WIDTH;

    push_quad(debug->render_group, pos, size, color, layer + LAYER_MID);


    UiWindow* colorpicker_window = get_window(debug, "Color Picker");
    Colorpicker* colorpicker = get_colorpicker(debug, (void*)var);

    if (button_pressed(debug->input.left_mouse_button))
    {
        if (!colorpicker->is_active)
        {
            if (window_is_focused(debug, debug->current_window))
            {
                if (point_is_inside_rect(debug->input.mouse.position, pos, size))
                {
                    vec2 window_size = V2(260 + 2*PADDING, 300);
                    vec2 window_pos = debug->input.mouse.position;
                    ui_set_window_focus(debug, colorpicker_window);

                    if ((window_pos.y + window_size.y) >= debug->screen_height ||
                        (window_pos.x + window_size.x) >= debug->screen_width)
                    {
                        window_pos.x = debug->screen_width - window_size.x;
                        window_pos.y = debug->screen_height - window_size.y;
                    }
                    set_window_position(debug, window_pos, "Color Picker");
                    set_window_size(debug, window_size, "Color Picker");

                    colorpicker->color = *var;
                    colorpicker->is_active = true;
                }
            }
        }
        else
        {
            if (!point_is_inside_rect(debug->input.mouse.position, 
                                      colorpicker_window->position, colorpicker_window->size))
            {
                colorpicker->is_active = false;
            }
        }
    }

    if (colorpicker->is_active)
    {


        ui_window_begin(debug, "Color Picker", 
                        colorpicker_window->position, colorpicker_window->size);

                        

        hue_slider(debug, colorpicker);
        hsb_picker(debug, colorpicker);

        *var = hsb2rgb(V3(colorpicker->hue,
                          colorpicker->saturation,
                          colorpicker->brightness));



        UI_UInt8Editbox(debug, var, "Color");

        ui_window_end(debug);

    }

}

internal void
ui_init(DebugState* debug, Platform* platform, Assets* assets)
{
#ifdef PLATFORM_WIN32
        debug->font = ui_load_font(&debug->arena,
                                           platform,
                                           assets,
                                           "../consola.ttf",
                                           16);
#elif PLATFORM_LINUX
        debug->font = ui_load_font(&debug->arena,
                                           platform,
                                           assets,
                                           "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
                                           16);
#endif
        for (u32 i = 0; i < ArrayCount(debug->sub_menus); ++i)
        {
            debug->sub_menus[i].is_active = true;
        }
}



internal b8
ui_submenu(DebugState* debug, char* title)
{
    debug->draw_cursor.y -=  debug->font.font_size + PADDING;
    debug->draw_cursor.x = debug->current_window->position.x + PADDING;


    ui_submenu_titlebar(debug, 
                           debug->draw_cursor, 
                           V2(debug->current_window->size.x - 2*PADDING,
                             (f32)debug->font.font_size),
                           title);

    b8 active = debug->sub_menus[debug->sub_menu_index].is_active;

    ASSERT(debug->sub_menu_index < ArrayCount(debug->sub_menus));
    ++debug->sub_menu_index;

    return active;
}


