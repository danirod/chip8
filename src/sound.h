#ifndef SOUND_H_
#define SOUND_H_

#include <SDL2/SDL.h>

SDL_AudioSpec* init_audiospec(void);
void dispose_audiospec(SDL_AudioSpec*);

#endif // SOUND_H_
