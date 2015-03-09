#ifndef MY_SDL_H_
#define MY_SDL_H_

struct context_t
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* surface;
    SDL_Event event;
};

int init_context(struct context_t*);

void destroy_context(struct context_t*);

void render(struct context_t*, struct machine_t*);

#endif // MY_SDL_H_
