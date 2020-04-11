#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#include "../SDL_FontCache/SDL_FontCache.h"

#include "../gui/Window.h"
#include "../gui/Dolly.h"
#include "../gui/Camera.h"
#include "../gui/Button.h"
#include "../gui/TextBox.h"
#include "../gui/Menu.h"

#include "Map.h"
#include "MapRenderer.h"

#ifdef WIN32
    #define strdup _strdup
#endif

enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NUM_BINDS,
};

static const int BINDS[] = {
    SDLK_w,
    SDLK_s,
    SDLK_a,
    SDLK_d,
};
static int bind_pressed[NUM_BINDS];
static int camera_speed = 200.0f;

enum mouse_binds {
    LEFT_BUTTON,
    RIGHT_BUTTON,
    MIDDLE_BUTTON,
    NUM_BUTTONS,
};
static int mouse_button_pressed[NUM_BUTTONS];

enum ITEMS {
    SAVE_BTN,
    LOAD_BTN,
    SAVE_BOX,
    TILE_BTN,
    SIZE_BOX,
    NUM_ITEMS,
};

static int IDS[NUM_ITEMS];
#define MAX_NUM_TILES 64
static int TILE_IDS[64];
static int num_tiles = 0;

static int brush_size = 1;

static FC_Font* font = NULL;
static Window* window;

static Menu top_menu;

void init_top_menu();

int main(int argc, char** argv)
{
    window = Window_init();

    Camera cam;
    Camera_init(&cam, window->renderer);

    MapRenderer_init(window->renderer);

    Map map;
    Map_init(&map, NULL);

    font = FC_CreateFont();
    FC_LoadFont(font, window->renderer, "../res/fonts/RobotoMono-Bold.ttf", 16, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);

    if (!font) {
        printf("Error loading font: %s\n", SDL_GetError());
    }

    init_top_menu();
    
    int selected_tile = 0;
    int screen_button_pressed = 0;

    int current_time = SDL_GetTicks();
    int dt = 0;
    SDL_Event e;
    int done = 0;
    while (!done)
    {
        screen_button_pressed = 0;
        dt = SDL_GetTicks() - current_time;
        current_time = SDL_GetTicks();
        float float_dt = ((float) dt) / 1000.0f;

        while (SDL_PollEvent(&e))
        {
            Menu_pass_event(&top_menu, window->renderer, &e);
            switch (e.type)
            {
            case SDL_KEYDOWN:
                for(int i = 0; i < NUM_BINDS; i++) {
                    if (e.key.keysym.sym == BINDS[i]) {
                        bind_pressed[i] = 1;
                    }
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        const TextBox* box = Menu_get_selected_textbox();
                        if (box) {
                            int size;
                            if (sscanf(box->buffer, "%d", &size)) {
                                brush_size = size;
                            }
                        }
                    }
                } 
                break;
            case SDL_KEYUP:
                for(int i = 0; i < NUM_BINDS; i++) {
                    if (e.key.keysym.sym == BINDS[i]) {
                        bind_pressed[i] = 0;
                    }
                } 
                break;
            case SDL_MOUSEBUTTONDOWN: {
                if(e.button.button == SDL_BUTTON_LEFT) {
                    mouse_button_pressed[LEFT_BUTTON] = 1;
                }
            } break;
            case SDL_MOUSEBUTTONUP:
                if(e.button.button == SDL_BUTTON_LEFT) {
                    mouse_button_pressed[LEFT_BUTTON] = 0;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (e.wheel.y > 0)
                    Camera_zoom(&cam, 1.1f);
                else if (e.wheel.y < 0)
                    Camera_zoom(&cam, 0.9f);
                break;
            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    SDL_Rect viewport;
                    SDL_RenderGetViewport(window->renderer, &viewport);

                    Camera_set_size(&cam, viewport.w, viewport.h);
                } break;
                } break;
            case SDL_QUIT:
                done = 1;
                break;
            }
        }

        //if we're typing in textbox dont move camera around
        if (Menu_get_selected_textbox()) {
            for(int i = 0; i < NUM_BINDS; i++) {
                bind_pressed[i] = 0;
            }
        }

        //moving camera
        float move_amnt = camera_speed * float_dt;
        if (bind_pressed[UP])    { Camera_translate(&cam, 0, -move_amnt); }
        if (bind_pressed[DOWN])  { Camera_translate(&cam, 0, move_amnt);  }
        if (bind_pressed[LEFT])  { Camera_translate(&cam, -move_amnt, 0); }
        if (bind_pressed[RIGHT]) { Camera_translate(&cam, move_amnt, 0);  }

        screen_button_pressed = 0;
        //check for button presses in top menu
        if (top_menu.selected_button >= 0) {
            if (top_menu.selected_button == IDS[SAVE_BTN]) {
                Map_save(&map, Menu_get_textbox(&top_menu, IDS[SAVE_BOX])->buffer);
            }
            else if(top_menu.selected_button == IDS[LOAD_BTN]) {
                Map_load(&map, Menu_get_textbox(&top_menu, IDS[SAVE_BOX])->buffer);
            }
            else if (top_menu.selected_button == IDS[TILE_BTN]) {
                for(int i = 0; i < num_tiles; i++) {
                    Button* btn = Menu_get_button(&top_menu, TILE_IDS[i]);
                    btn->is_hidden = !btn->is_hidden;
                }
            }
            for(int i = 0; i < num_tiles; i++) {
                if (top_menu.selected_button == TILE_IDS[i]) {
                    int atlas_w = MapRenderer_get_texture_width();
                    int atlas_h = MapRenderer_get_texture_height();

                    Button* tile_btn = Menu_get_button(&top_menu, IDS[TILE_BTN]);
                    tile_btn->srcrect.x = i % (atlas_w / 64) * 64;
                    tile_btn->srcrect.y = i / (atlas_w / 64) * 64;
                    selected_tile = i;
                }
            }
            top_menu.selected_button = -1;
            screen_button_pressed = 1;
            for(int i = 0; i < NUM_BUTTONS; i++) {
                mouse_button_pressed[i] = 0;
            }
        }

        //brush painting, unless a menu button was pressed
        int x,y;
        SDL_GetMouseState(&x, &y);
        if (mouse_button_pressed[LEFT_BUTTON] && !screen_button_pressed) {
            int screen_button_pressed = 0;
            if (!screen_button_pressed) {
                int worldx, worldy;
                Camera_get_mousestate_relative(&cam, &worldx, &worldy);
                worldx /= 64;
                worldy /= 64;
                int radius = brush_size - 1;
                for(int brush_x = worldx - radius; brush_x <= worldx + radius; brush_x++) {
                    for(int brush_y = worldy -radius; brush_y <= worldy + radius; brush_y++) {
                        Map_set_tile(&map, selected_tile, brush_x, brush_y);
                    }
                }
            }
        }

        //rendering
        Window_clear(window);
        Map_render(&map, window->renderer, &cam);
        Menu_render(&top_menu, window->renderer);
        Window_present(window);
    }
   
    MapRenderer_deinit();
    Window_delete(window);
}

void init_top_menu() {
    Menu_init(&top_menu);

    //save text box
    TextBox save_box;
    TextBox_init(&save_box, "output file name", font);
    save_box.x = 25;
    save_box.y = -30;
    save_box.box_width = 200;

    TTF_Font* font_ttf = TTF_OpenFont("../res/fonts/RobotoMono-Bold.ttf", 16);
    if (!font_ttf) {
        printf("Unable to load font: %s\n", SDL_GetError());
    }
    SDL_Color white = {255, 255, 255 ,255};

    //save and load buttons
    Button save_button;
    Button load_button;

    Button_init_text(&save_button, window->renderer, font_ttf, "save", white);
    Button_init_text(&load_button, window->renderer, font_ttf, "load", white);
    save_button.rect.x = 25;
    save_button.rect.y = -60;
    load_button.rect.x = 90;
    load_button.rect.y = -60;

    //tile button
    Button tile_button;
    Button_init_texture(&tile_button, MapRenderer_gettexture());
    tile_button.rect.x = 0;
    tile_button.rect.y = 0;
    tile_button.rect.w = 64;
    tile_button.rect.h = 64;

    //init tile selector/atlast buttons
    int atlas_w = MapRenderer_get_texture_width();
    int atlas_h = MapRenderer_get_texture_height();
    Button temp;
    int atlas_startx = 70;
    int atlas_starty = 20;

    int index = 0;
    for(int y = 0; y < atlas_h / 64; y++) {
        for(int x = 0; x < atlas_w / 64; x++) {
            if (index >= MAX_NUM_TILES) {
                printf("Error: too many tiles in texture!!!\n");
                break;
            }
            Button_init_texture(&temp, MapRenderer_gettexture());
            temp.rect.x = atlas_startx + x * 64;
            temp.rect.y = atlas_starty + y * 64;
            temp.rect.w = 64;
            temp.rect.h = 64;

            temp.srcrect.x = x * 64;
            temp.srcrect.y = y * 64;
            temp.srcrect.w = 64;
            temp.srcrect.h = 64;
            TILE_IDS[index] = Menu_add_button(&top_menu, temp);
            index++;
        }
    }
    num_tiles = index;

    //now do the cursor size box
    TextBox size_box;
    TextBox_init(&size_box, "cursor", font);
    TextBox_append_char(&size_box, '1');
    size_box.x = 0;
    size_box.y = 96;
    size_box.box_width = 64;

    IDS[SIZE_BOX] = Menu_add_textbox(&top_menu, size_box);

    IDS[SAVE_BTN] = Menu_add_button(&top_menu, save_button);
    IDS[LOAD_BTN] = Menu_add_button(&top_menu, load_button);
    IDS[SAVE_BOX] = Menu_add_textbox(&top_menu, save_box);
    IDS[TILE_BTN] = Menu_add_button(&top_menu, tile_button);
}
