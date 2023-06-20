#include <SDL.h>
#include <stdlib.h>
#include <iostream>

void
panic_abort(const char *title, const char *text, SDL_Window *win)
{
    std::cerr << title << text;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, win);

    SDL_Quit();
    exit(1);
}

void
halt(SDL_AudioStream *stream, Uint8 *buf, Uint32 len)
{
    if(stream) {
        SDL_FreeAudioStream(stream);
    }
    if(buf) {
        SDL_FreeWAV(buf);
    }
    stream = NULL;
    buf = NULL;
    len = 0;
}
