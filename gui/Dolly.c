#include "Dolly.h"
#include "../global.h"

#include <SDL2/SDL_image.h>

#include "Camera.h"
#include <string.h>

void Dolly_init(Dolly* self) {
    Vector_init(&self->surfaces);
    Vector_init(&self->textures);

    self->rect.x = 0;
    self->rect.y = 0;
    self->rect.w = 64;
    self->rect.h = 64;

    self->offset = 5;
}

void Dolly_init_with_sprites(Dolly* self, SDL_Renderer* window_renderer, const char* file_name, int num) {
    Dolly_init(self);
    Dolly_setSprites(self, window_renderer, file_name, num);
}

int Dolly_setSprites(Dolly* self, SDL_Renderer* window_render, const char* file_name, int num) {
    char* name = strdup(file_name);
    if (num != 1) 
        name[(strchr(name, '.') - name) - 2] = '0';
    for(int i = 0; i < num; i++) {
        if (num != 1)
            name[(strchr(name, '.') - name) - 1] = '0' + i;
        SDL_Surface* surface = IMG_Load(name);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(window_render, surface);
        if (!surface || !texture) {
            printf("error loading texture file %s: %s\n", name, SDL_GetError());
            return 0;
        }
        Vector_push(&self->surfaces, surface);
        Vector_push(&self->textures, texture);
    }
    free(name);
    return 1;
}

void Dolly_render(Dolly* self, SDL_Renderer* window_renderer, const Camera* cam) {
    SDL_Rect rect_copy = self->rect;
    SDL_Rect camera_rect;
    for(int i = 0; i < self->textures.num; i++) {
        rect_copy.y -= self->offset;
        Camera_transform_rect(cam, &rect_copy, &camera_rect);
        SDL_RenderCopyEx(
                window_renderer, 
                Vector_get(&self->textures, i),
                NULL, 
                &camera_rect, 
                (double) (self->angle) * 180.0f / M_PI ,
                NULL, 
                SDL_FLIP_NONE
        );
    }
}

void Dolly_setPos(Dolly* self, int x, int y) {
    self->rect.x = x;
    self->rect.y = x;
}
void Dolly_translate(Dolly* self, int dx, int dy) {
    self->rect.x += dx;
    self->rect.y += dy;
}

void Dolly_setAngle(Dolly* self, float angle) { self->angle = angle; }
void Dolly_rotate(Dolly* self, float dTheta) { self->angle += dTheta; }

int Dolly_getX(Dolly* self) {return self->rect.x;}
int Dolly_getY(Dolly* self) {return self->rect.y;}

void Dolly_delete(Dolly* self) {
    for(int i = 0; i < self->surfaces.num; i++) {
        SDL_FreeSurface(Vector_get(&self->surfaces, i));
    }
    for(int i = 0; i < self->textures.num; i++) {
        SDL_DestroyTexture(Vector_get(&self->textures, i));
    }
    Vector_delete(&self->surfaces);
    Vector_delete(&self->textures);
}