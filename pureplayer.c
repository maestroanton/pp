#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
#include <SDL2/SDL.h>
#elif _WIN32
#include "SDL.h"
#endif
#include "handle.h"

/* I know global variables suck but it is what it is. */

static float volume_slider = 1.0f; /* Volume. 0 being mute and 1 being full blast. */
static float panning_slider = 0.5f; /* Balance L-R. 0.5 is equal, 0 is full left and 1 is full right. */
static Uint8 *audioMemAddr = NULL;
static Uint32 audioLen = 0;
static SDL_AudioSpec audioSpec;
static SDL_AudioStream *data_stream;

SDL_bool load_wav(const char *fname, SDL_Window *win); /* Loads WAV and creates audio stream. */

int
main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) == -1) {
        panic_abort("Panic!", "Not able to init SNL.", NULL);
    }

    SDL_AudioDeviceID speaker;
    SDL_AudioSpec audio;
    SDL_Window   *window = NULL;
    SDL_Renderer *renderer = NULL;

    SDL_zero(audio);
    audio.freq = 48000;
    audio.channels = 2;
    audio.samples = 4096;
    audio.format = AUDIO_F32;
    audio.callback = NULL;

    speaker = SDL_OpenAudioDevice(NULL, 0, &audio, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(!speaker) {
        panic_abort("Error!", SDL_GetError(), NULL);
    }

    window = SDL_CreateWindow("Hello SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if(!window) {
        panic_abort("SDL_CreateWindow failed.", SDL_GetError(), NULL);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        panic_abort("SDL_CreateRenderer Failed.", SDL_GetError(), window);
    }

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    SDL_bool go_on = SDL_TRUE;
    SDL_bool paused = SDL_TRUE;

    const SDL_Rect rewind_rect = {120, 140, 100, 100};
    const SDL_Rect pause_rect = {480, 140, 100, 100};
    const SDL_Rect stop_rect = {360, 140, 100, 100};
    const SDL_Rect volume_rect = {120, 340, 460, 25};
    const SDL_Rect panning_rect = {120, 420, 460, 25};

    SDL_Rect panning_knob;
    SDL_memcpy(&panning_knob, &panning_rect, sizeof(SDL_Rect));
    panning_knob.w = 10;
    panning_knob.x = (panning_rect.x + (panning_rect.w / 2)) - panning_knob.w;

    SDL_Rect volume_knob;
    SDL_memcpy(&volume_knob, &volume_rect, sizeof(SDL_Rect));
    volume_knob.w = 10;
    volume_knob.x = (volume_rect.x + volume_rect.w) - (volume_knob.w);

    Uint8 red = 168, green = 128, blue = 128, alpha = 255;

    while(go_on) {
        SDL_Event evnt;
        while(SDL_PollEvent(&evnt)) {
            switch(evnt.type) {
                case SDL_QUIT: {
                    go_on = SDL_FALSE;
                    break;
                }

                case SDL_MOUSEBUTTONDOWN: {
                    const SDL_Point point = {evnt.button.x, evnt.button.y};
                    if(SDL_PointInRect(&point, &rewind_rect)) {
                        SDL_ClearQueuedAudio(speaker);
                        SDL_AudioStreamClear(data_stream);
                        if(SDL_AudioStreamPut(data_stream, audioMemAddr, audioLen) == -1) {
                            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
                            halt(data_stream, audioMemAddr, audioLen);
                        } else if(SDL_AudioStreamFlush(data_stream) == -1) {
                            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
                            halt(data_stream, audioMemAddr, audioLen);
                        }
                    } else if(SDL_PointInRect(&point, &pause_rect)) {
                        paused = paused ? SDL_FALSE : SDL_TRUE;
                        SDL_PauseAudioDevice(speaker, paused);
                    } else if(SDL_PointInRect(&point, &stop_rect)) {
                        halt(data_stream, audioMemAddr, audioLen);
                    }

                    break;
                }

                case SDL_MOUSEMOTION: {
                    const SDL_Point point = {evnt.motion.x, evnt.motion.y};
                    if(SDL_PointInRect(&point, &volume_rect) && (evnt.motion.state & SDL_BUTTON_LMASK)) {
                        float slide = (float)(point.x - volume_rect.x);
                        volume_slider = (slide / ((float)volume_rect.w));
                        if(slide <= volume_knob.w) {
                            volume_knob.x = (volume_rect.x);
                        } else {
                            volume_knob.x = (slide + volume_rect.x) - volume_knob.w;
                        }
                    } else if(SDL_PointInRect(&point, &panning_rect) && (evnt.motion.state & SDL_BUTTON_LMASK)) {
                            float slide = (float)(point.x - panning_rect.x);
                            panning_slider = (slide / ((float)panning_rect.w));
                            if(slide <= panning_knob.w) {
                                panning_knob.x = (panning_rect.x);
                            } else {
                                panning_knob.x = (slide + panning_rect.x) - panning_knob.w;
                            }
                        }
                    break;
                }

                case SDL_DROPFILE: {
                    load_wav(evnt.drop.file, window);
                    SDL_free(evnt.drop.file);
                    break;
                }
            }
        }

        if(SDL_GetQueuedAudioSize(speaker) < 8192) {
            const int bytes_remaining = SDL_AudioStreamAvailable(data_stream);
            if(bytes_remaining > 0) {
                const int new_bytes = SDL_min(bytes_remaining, 32 * 1024);
                static Uint8 converted_buffer[32 * 1024];
                SDL_AudioStreamGet(data_stream, converted_buffer, new_bytes);
                float *to_buffer = (float *) converted_buffer;
                const int n_samples = (new_bytes / sizeof(float));

                SDL_assert((n_samples % 2) == 0); /* Assert we have stereo sound. */

                /* Change volume. (FIXME: non-linear) */
                if(volume_slider != 1.0f) {
                    for(int i = 0; i < n_samples; i++) {
                        to_buffer[i] *= volume_slider;
                    }
                }

                /* Change panning. (L-R speaker) */
                /* Sample 0 - Left | Sample 1 - Right. */
                if(panning_slider < 0.5f) {
                    for(int i = 0; i < n_samples; i += 2) {
                        to_buffer[i+1] *= panning_slider;
                        }
                } else if(panning_slider > 0.5f) {
                    for(int i = 0; i < n_samples; i += 2) {
                        to_buffer[i] *= (1.0f - panning_slider);
                    }
                }

                SDL_QueueAudio(speaker, converted_buffer, new_bytes);
            }
        }

        SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rewind_rect);
        SDL_RenderFillRect(renderer, &pause_rect);
        SDL_RenderFillRect(renderer, &stop_rect);
        SDL_RenderFillRect(renderer, &volume_rect);
        SDL_RenderFillRect(renderer, &panning_rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &volume_knob);
        SDL_RenderFillRect(renderer, &panning_knob);

        SDL_RenderPresent(renderer);
    }

    SDL_CloseAudioDevice(speaker);
    SDL_FreeWAV(audioMemAddr);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

SDL_bool
load_wav(const char *fname, SDL_Window *win)
{
    SDL_FreeAudioStream(data_stream);
    SDL_FreeWAV(audioMemAddr);
    data_stream = NULL;
    audioMemAddr = NULL;
    audioLen = 0;

    if(SDL_LoadWAV(fname, &audioSpec, &audioMemAddr, &audioLen) == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), win);
        goto failed;
    }

    data_stream = SDL_NewAudioStream(audioSpec.format, audioSpec.channels, audioSpec.freq, AUDIO_F32, 2, 48000);
    if(!data_stream) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), win);
        goto failed;

    }

    if(SDL_AudioStreamPut(data_stream, audioMemAddr, audioLen) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), win);
        goto failed;
    }

    if(SDL_AudioStreamFlush(data_stream) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), win);
        goto failed;
    }

    return SDL_TRUE;

    failed:
        halt(data_stream, audioMemAddr, audioLen);
        return SDL_FALSE;
}
