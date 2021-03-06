#ifndef PROJECTILERENDERER_H
#define PROJECTILERENDERER_H

#include "../gui/Camera.h"
#include <SDL2/SDL.h>

void Proj_init_all_sprites(SDL_Renderer *window_renderer);
void Proj_cleanup_all_sprites();
void Proj_render_all(SDL_Renderer *renderer, const Camera *cam);

#endif
