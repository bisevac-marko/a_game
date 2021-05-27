#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <dlfcn.h> // dlopen..
#include <time.h>
#include <limits.h>


// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#define Font FontTest
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#undef Font

#include "common.h"
#include "memory.h"
#include "string.cpp"
#include "log.h"
#include "math.h"
#include "debug.h"
#include "renderer.h"
#include "renderer.cpp"
#include "input.h"
#include "game.h"
#include "asset_loading.cpp"
#include "render_group.cpp"
#include "opengl_renderer.h"
#include "opengl_renderer.cpp"



// typedef XID GLXContextID;
// typedef XID GLXPixmap;
// typedef XID GLXDrawable;
// typedef XID GLXPbuffer;
// typedef XID GLXWindow;
// typedef XID GLXFBConfigID;
// typedef struct __GLXcontextRec *GLXContext#define KEYCODE_W           25
// typedef struct __GLXFBConfigRec *GLXFBConfig;
#define KEYCODE_W           25
#define KEYCODE_A           38
#define KEYCODE_S           39
#define KEYCODE_D           40
#define KEYCODE_Q           24
#define KEYCODE_E           26
#define KEYCODE_UP          111
#define KEYCODE_DOWN        116
#define KEYCODE_LEFT        113
#define KEYCODE_RIGHT       114
#define KEYCODE_ESCAPE      9
#define KEYCODE_BACKSPACE   22
#define KEYCODE_ENTER       36
#define KEYCODE_SPACE       65
#define KEYCODE_P           33
#define KEYCODE_L           46

#define KEYCODE_SHIFT_L     50
#define KEYCODE_SHIFT_R     62
#define KEYCODE_CTRL_L      37
#define KEYCODE_CTRL_R      105
#define KEYCODE_ALT_L       64
#define KEYCODE_ALT_R       108
#define KEYCODE_SUPER       133

#define KEYCODE_PLUS        21
#define KEYCODE_MINUS       20

#define KEYCODE_F1          67
#define KEYCODE_F10         76
#define KEYCODE_F11         95
#define KEYCODE_F12         96

#define KEYCODE_SHIFT_MASK  0x01
#define KEYCODE_CTRL_MASK   0x04
#define KEYCODE_ALT_MASK    0x08

#define LINUX_LEFT_MOUSE   1
#define LINUX_RIGHT_MOUSE  3
#define LINUX_MIDDLE_MOUSE 2
#define LINUX_EXT1_MOUSE   8
#define LINUX_EXT2_MOUSE   9

#define GLX_RGBA		    4
#define GLX_DOUBLEBUFFER	5
#define GLX_DEPTH_SIZE		12

OpenGLFunction(GLXContext, glXCreateContextAttribsARB, Display* d, GLXFBConfig config, GLXContext shareContext, Bool direct, const int *attribList);
OpenGLFunction(void, glXSwapIntervalEXT, Display* d, GLXDrawable drawable, const int inverval);

// #include "opengl_renderer.h"
// #include "opengl_renderer.cpp"

internal void
free_file_memory(void* memory)
{
    if (memory)
       free(memory);
}


internal FileResult
read_entire_file(char* file_name)
{
    struct stat s;
    FileResult result = {};
    i32 fd;

    if (stat(file_name, &s) == -1)
    {
        printf("Invalid file %s \n", file_name);
        return result;
    }
    
    result.size = s.st_size;
    fd = open(file_name, O_RDONLY);
    if (fd != -1)
    {
        result.data = malloc(s.st_size * sizeof(char));
        if (result.data)
        {
            if ((read(fd, result.data, s.st_size)) != -1)
            {
                // NOTE: file read successfully
            }
            else
            {
                // TODO: logging
                free_file_memory(result.data);
                result.data = NULL;
                perror("read");
                close(fd);

            }
        } 
        else
        {
            // TODO: logging
        }
    }
    else
    {
        // TODO: logging
        perror("open");
    }
    
    
    return result;
}

internal b8
write_entire_file(char* file_name, u32 size, void* memory)
{
    i32 fd = open(file_name, O_CREAT | O_RDWR);
    b8 result = 0;

    if (fd != -1)
    {
        if (write(fd, memory, size) != -1)
        {
            printf("Written file [%s] successfuly", file_name);
            result = true;
        }
        else
        {
            perror("write");
        }
    }
    else
    {
        perror("open");
    }
    
    close(fd);
    return result;
}

struct LinuxWindowInfo
{
    XSetWindowAttributes win_attrib;
    XVisualInfo* visual_info;
    GLXContext context;
};

internal void
linux_init_opengl()
{
    void* lib_GL = dlopen("libGL.so", RTLD_NOW);
    if (!lib_GL) {
        PRINT("libGL.so couldn't be loaded");
    }

    #define LinuxLoadOpenGLFunction(name) \
            name = (name##proc *) dlsym(lib_GL, #name); \
            if (!name) { \
                PRINT("OpenGL function " #name " couldn't be loaded.\n"); \
            }

    LinuxLoadOpenGLFunction(glAttachShader);
    LinuxLoadOpenGLFunction(glBindBuffer);
    LinuxLoadOpenGLFunction(glBindFramebuffer);
    LinuxLoadOpenGLFunction(glBufferData);
    LinuxLoadOpenGLFunction(glBufferSubData);
    LinuxLoadOpenGLFunction(glCheckFramebufferStatus);
    LinuxLoadOpenGLFunction(glClearBufferfv);
    LinuxLoadOpenGLFunction(glCompileShader);
    LinuxLoadOpenGLFunction(glCreateProgram);
    LinuxLoadOpenGLFunction(glCreateShader);
    LinuxLoadOpenGLFunction(glDeleteBuffers);
    LinuxLoadOpenGLFunction(glDeleteFramebuffers);
    LinuxLoadOpenGLFunction(glEnableVertexAttribArray);
    LinuxLoadOpenGLFunction(glDrawBuffers);
    LinuxLoadOpenGLFunction(glFramebufferTexture2D);
    LinuxLoadOpenGLFunction(glGenBuffers);
    LinuxLoadOpenGLFunction(glGenFramebuffers);
    LinuxLoadOpenGLFunction(glGetAttribLocation);
    LinuxLoadOpenGLFunction(glGetShaderInfoLog);
    LinuxLoadOpenGLFunction(glGetShaderiv);
    LinuxLoadOpenGLFunction(glGetUniformLocation);
    LinuxLoadOpenGLFunction(glLinkProgram);
    LinuxLoadOpenGLFunction(glShaderSource);
    LinuxLoadOpenGLFunction(glUniform1i);
    LinuxLoadOpenGLFunction(glUniform1f);
    LinuxLoadOpenGLFunction(glUniform2f);
    LinuxLoadOpenGLFunction(glUniform4f);
    LinuxLoadOpenGLFunction(glUniform1iv);
    LinuxLoadOpenGLFunction(glUniform2iv);
    LinuxLoadOpenGLFunction(glUniform3iv);
    LinuxLoadOpenGLFunction(glUniform4iv);
    LinuxLoadOpenGLFunction(glUniform1fv);
    LinuxLoadOpenGLFunction(glUniform2fv);
    LinuxLoadOpenGLFunction(glUniform3fv);
    LinuxLoadOpenGLFunction(glUniform4fv);
    LinuxLoadOpenGLFunction(glUniformMatrix4fv);
    LinuxLoadOpenGLFunction(glUseProgram);
    LinuxLoadOpenGLFunction(glVertexAttribPointer);
    LinuxLoadOpenGLFunction(glGenVertexArrays);
    LinuxLoadOpenGLFunction(glBindVertexArray);
    LinuxLoadOpenGLFunction(glDeleteShader);
    LinuxLoadOpenGLFunction(glGetProgramInfoLog);
    LinuxLoadOpenGLFunction(glGetProgramiv);
    LinuxLoadOpenGLFunction(glXCreateContextAttribsARB);
    LinuxLoadOpenGLFunction(glXSwapIntervalEXT);
    // LinuxLoadOpenGLFunction(glActiveTexture);
    // LinuxLoadOpenGLFunction(glGenTextures);
    // LinuxLoadOpenGLFunction(glBindTexture);
    // LinuxLoadOpenGLFunction(glPixelStorei);
    // LinuxLoadOpenGLFunction(glTexParameteri);
    // LinuxLoadOpenGLFunction(glTexImage2D);
    // LinuxLoadOpenGLFunction(glEnable);
    // LinuxLoadOpenGLFunction(glDisable);
    // LinuxLoadOpenGLFunction(glBlendFunc);
    // LinuxLoadOpenGLFunction(glClearColor);
    // LinuxLoadOpenGLFunction(glClear);
    // LinuxLoadOpenGLFunction(glViewport);
    // LinuxLoadOpenGLFunction(glDrawArrays);
    // LinuxLoadOpenGLFunction(glDrawElements);
}

internal LinuxWindowInfo
linux_opengl_prep(Display* display)
{
    LinuxWindowInfo result = {};

    i32 glx_major, glx_minor;
    i32 visual_attribs[] =
    {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      //GLX_SAMPLE_BUFFERS  , 1,
      //GLX_SAMPLES         , 4,
      None
    };

    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion(display, &glx_major, &glx_minor) || 
         ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1 ))
    {
        PRINT("Invalid GLX version");
    }
    i32 fbcount;
    GLXFBConfig* fbc = 
                glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);

    if (!fbc)
    {
        PRINT("Failed to retrive a framebuffer config");
    }
    i32 best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

    i32 i;
    for (i=0; i<fbcount; ++i)
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi)
        {
          i32 samp_buf, samples;
          glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
          glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES       , &samples);

          if ( best_fbc < 0 || (samp_buf && samples) > best_num_samp )
            best_fbc = i, best_num_samp = samples;
          if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
            worst_fbc = i, worst_num_samp = samples;
        }
        XFree(vi);
    }

    GLXFBConfig fbc_to_use = fbc[ best_fbc ];

    result.visual_info = glXGetVisualFromFBConfig(display, fbc_to_use);

    XFree(fbc);

    if (glXCreateContextAttribsARB)
    {
        i32 opengl_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if GAME_DEBUG
            | GLX_CONTEXT_DEBUG_BIT_ARB
#endif
            ,
#if 0
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
            0,
        };

        result.context = glXCreateContextAttribsARB(display, fbc_to_use, 0, true, opengl_attribs);
    }
    else
    {
        result.context = glXCreateContext(display, result.visual_info, NULL, GL_TRUE);
    }

    GLXDrawable drawable = glXGetCurrentDrawable();
    if (drawable) {
        glXSwapIntervalEXT(display, drawable, 1);
    }

    Colormap color_map;
    color_map = XCreateColormap(display, 
                                RootWindow(display, result.visual_info->screen), 
                                result.visual_info->visual, 
                                AllocNone);
    result.win_attrib.colormap = color_map;
    
    result.win_attrib.background_pixmap = None;
    result.win_attrib.border_pixel      = 0;
    result.win_attrib.event_mask        = KeyPressMask      | KeyReleaseMask    | ButtonPressMask   | 
                                       ButtonReleaseMask | PointerMotionMask | Button1MotionMask |
                                       Button2MotionMask | Button3MotionMask | Button4MotionMask | 
                                       Button5MotionMask | ButtonMotionMask;

    return result;
}

struct LinuxGameCode
{
    void* lib_game_handle;
    GameMainLoop main_loop;
};

internal LinuxGameCode
linux_load_game_code()
{
    LinuxGameCode result = {};
    result.lib_game_handle = dlopen("./libgame.so", RTLD_NOW | RTLD_LOCAL);

    if (result.lib_game_handle) 
    {
        result.main_loop = (GameMainLoop)dlsym(result.lib_game_handle, "game_main_loop");

        ASSERT(result.main_loop);
    }
    else
    {
        PRINT("Couldn't load game code");
        PRINT("Error: %s", dlerror());
        ASSERT(false);
    }

    return result;
}

internal void
linux_unload_game_code(LinuxGameCode game_code)
{
    if (game_code.lib_game_handle)
    {
        dlclose(game_code.lib_game_handle);
        game_code.lib_game_handle = 0;
    }
}

internal inline vec2
linux_get_mouse_position(Display *display, Window window)
{
    Window ret_root, ret_win;
    i32 root_x, root_y;
    i32 win_x, win_y;
    u32 mask;
    b8 query_success = XQueryPointer(display, window,
                                      &ret_root, &ret_win,
                                      &root_x, &root_y, &win_x, &win_y, &mask);
    
    vec2 result = {};
    if (query_success)
    {
        result.x = (f32)win_x;
        result.y = (f32)win_y;
    }
    return result;
}

internal void
linux_set_button_state(ButtonState* button_state, b8 is_down, b8 is_released)
{
    button_state->pressed = is_down && !(button_state->is_down);
    button_state->is_down = is_down;
    button_state->released = is_released;
}


i32 
main()
{
    Window window;
    Display *display = 0;


    char* default_display = getenv("DISPLAY");

    if (default_display)
    {
        display = XOpenDisplay(default_display);
    }
    else
    {
        PRINT("Couldn't open X11 window.");
        return 1;
    }
    
    linux_init_opengl();
    LinuxWindowInfo window_info = linux_opengl_prep(display);


    window = XCreateWindow(display, RootWindow(display, window_info.visual_info->screen), 
                              0, 0, 1280, 768, 0, window_info.visual_info->depth, 
                              InputOutput, 
                              window_info.visual_info->visual, 
                              CWBorderPixel|CWColormap|CWEventMask, &window_info.win_attrib);
    glXMakeCurrent(display, window, window_info.context);

    if (!window)
    {
        PRINT("Failed to create window");
    }

    XFree(window_info.visual_info);

    XStoreName(display, window, "Game");
    XMapWindow(display, window);


    GameMemory game_memory = {};
    game_memory.permanent_storage_size = Megabytes(64);
    game_memory.temporary_storage_size = Megabytes(128);

    game_memory.is_initialized = false;
    game_memory.permanent_storage = calloc(game_memory.permanent_storage_size, sizeof(u8));
    game_memory.temporary_storage = calloc(game_memory.temporary_storage_size, sizeof(u8));

    ASSERT(game_memory.permanent_storage);
    ASSERT(game_memory.temporary_storage);

    game_memory.platform.read_entire_file = read_entire_file;
    game_memory.platform.write_entire_file = write_entire_file;
    game_memory.platform.free_file_memory = free_file_memory;

// TODO: should be able to change graphics API on runtime
    game_memory.platform.init_renderer = opengl_init;
    game_memory.platform.end_frame = opengl_end_frame;

    LinuxGameCode linux_game_code = linux_load_game_code();

    GameInput game_input = {};

    while (1)
    {
        for (sizet i = 0; i < ArrayCount(game_input.buttons); ++i)
        {
            game_input.buttons[i].pressed = 0;
            game_input.buttons[i].released = 0;
        }
        game_input.character = '\0';


        while(XPending(display))
        {
            XEvent x_event = {};
            XNextEvent(display, &x_event);

            if (x_event.type == KeyPress)
            {
                char text_buffer[10];
                KeySym key;

                (XLookupString(&x_event.xkey, 
                              text_buffer,
                              10,
                              &key,
                              0));
                game_input.character = text_buffer[0];
            }
            if (x_event.type == KeyRelease && XEventsQueued(display, QueuedAfterReading))
            {
                XEvent next_event;
                XPeekEvent(display, &next_event);
                if ((next_event.type == KeyPress) &&
                    (next_event.xkey.time == x_event.xkey.time) &&
                    (next_event.xkey.keycode == x_event.xkey.keycode))
                {
                    XNextEvent(display, &x_event);
                    continue;
                }
            }

            switch (x_event.type)
            {
                case KeyRelease:
                case KeyPress:
                {
                    if (x_event.xkey.keycode == KEYCODE_W)
                    {
                        b8 is_down = (x_event.xkey.type == KeyPress);
                        game_input.move_up.is_down = is_down;
                        game_input.move_up.released = (x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_D)
                    {
                        game_input.move_right.is_down = (x_event.xkey.type == KeyPress);
                        game_input.move_right.released = (x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_A)
                    {
                        game_input.move_left.is_down = (x_event.xkey.type == KeyPress);
                        game_input.move_left.released = (x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_S)
                    {
                        game_input.move_down.is_down = (x_event.xkey.type == KeyPress);
                        game_input.move_down.released = (x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_ESCAPE)
                    {
                        linux_set_button_state(&game_input.escape,
                                               x_event.xkey.type == KeyPress,
                                               x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_BACKSPACE)
                    {
                        linux_set_button_state(&game_input.backspace,
                                               x_event.xkey.type == KeyPress,
                                               x_event.xkey.type == KeyRelease);
                    }
                    else if (x_event.xkey.keycode == KEYCODE_ENTER)
                    {
                        linux_set_button_state(&game_input.enter,
                                               x_event.xkey.type == KeyPress,
                                               x_event.xkey.type == KeyRelease);
                    }
                } break;
                case ButtonPress:
                case ButtonRelease:
                {
                    if (LINUX_LEFT_MOUSE)
                    {
                        b8 was_down = game_input.left_mouse_button.is_down;
                        b8 is_down = (x_event.type == ButtonPress);

                        game_input.left_mouse_button.pressed = (!was_down && is_down);
                        // PRINT("was_down %d", (!was_down && is_down));
                        game_input.left_mouse_button.is_down = is_down;
                        game_input.left_mouse_button.released = (x_event.type == ButtonRelease);
                    }
                    else if (LINUX_RIGHT_MOUSE)
                    {
                        b8 was_down = game_input.right_mouse_button.is_down;
                        b8 is_down = (x_event.type == ButtonPress);

                        game_input.right_mouse_button.pressed = (!was_down && is_down);
                        game_input.right_mouse_button.is_down = (x_event.type == ButtonPress);
                        game_input.right_mouse_button.released = (x_event.type == ButtonRelease);
                    }
                };
            }
        }
        game_input.mouse.position = linux_get_mouse_position(display, window);


        XWindowAttributes x_win_attribs;
        XGetWindowAttributes(display, window, &x_win_attribs);
        glViewport(0, 0, x_win_attribs.width, x_win_attribs.height);
        game_memory.screen_width = x_win_attribs.width;
        game_memory.screen_height = x_win_attribs.height;

        if (linux_game_code.lib_game_handle)
        {
            linux_game_code.main_loop(1.0f/60.0f, &game_memory, NULL, &game_input);
        }

        glXSwapBuffers(display, window);
    }

    XCloseDisplay(display);
    return 0;
}
