#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define ESFOLADA "/home/anton/music/esfolada.wav"

/* I know global variables suck but it is what it is. */

static SDL_Window   *window = NULL;
static SDL_Renderer *renderer = NULL;

static float volume_slider = 1.0f; /* Volume. 0 being mute and 1 being full blast. */
static float balance_slider = 0.5f; /* Balance L-R. 0.5 is equal, 0 is full left and 1 is full right. */
static Uint8 *audioMemAddr = NULL;
static Uint32 audioLen = 0;
static SDL_AudioSpec audioSpec;
static SDL_AudioStream *data_stream;

void halt(void); /* Stops audio and cleans up. */
SDL_bool load_wav(const char *fname); /* Loads WAV and creates audio stream. */
void panic_abort(const char *title, const char *text); /* Cleans up and quits on catastrophic errors. */

int
main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) == -1) {
        panic_abort("Panic!", "Not able to init SNL.");
    }

    SDL_AudioDeviceID speaker;
    SDL_AudioSpec audio;

    SDL_zero(audio);
    audio.freq = 48000;
    audio.channels = 2;
    audio.samples = 4096;
    audio.format = AUDIO_F32;
    audio.callback = NULL;

    speaker = SDL_OpenAudioDevice(NULL, 0, &audio, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(!speaker) {
        panic_abort("Error!", SDL_GetError());
    }

    window = SDL_CreateWindow("Hello SDL!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    if(!window) {
        panic_abort("SDL_CreateWindow failed.", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if(!renderer) {
        panic_abort("SDL_CreateRenderer Failed.", SDL_GetError());
    }

    load_wav(ESFOLADA);

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    SDL_bool go_on = SDL_TRUE;
    SDL_bool paused = SDL_TRUE;

    const SDL_Rect rewind_rect = {120, 140, 100, 100};
    const SDL_Rect pause_rect = {480, 140, 100, 100};
    const SDL_Rect stop_rect = {360, 140, 100, 100};
    const SDL_Rect volume_rect = {120, 340, 460, 25};
    const SDL_Rect balance_rect = {120, 420, 460, 25};

    SDL_Rect balance_knob;
    SDL_memcpy(&balance_knob, &balance_rect, sizeof(SDL_Rect));
    balance_knob.w = 10;
    balance_knob.x = (balance_rect.x + (balance_rect.w / 2)) - balance_knob.w;

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
                            halt();
                        } else if(SDL_AudioStreamFlush(data_stream) == -1) {
                            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
                            halt();
                        }
                    } else if(SDL_PointInRect(&point, &pause_rect)) {
                        paused = paused ? SDL_FALSE : SDL_TRUE;
                        SDL_PauseAudioDevice(speaker, paused);
                    } else if(SDL_PointInRect(&point, &stop_rect)) {
                        halt();    
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
                        } else if(SDL_PointInRect(&point, &balance_rect) && (evnt.motion.state & SDL_BUTTON_LMASK)) {
                            float slide = (float)(point.x - balance_rect.x);
                            balance_slider = (slide / ((float)balance_rect.w));
                            if(slide <= balance_knob.w) {
                                balance_knob.x = (balance_rect.x);
                            } else {
                                balance_knob.x = (slide + balance_rect.x) - balance_knob.w;
                            }
                        }
                    break;
                }

                case SDL_DROPFILE: {
                    load_wav(evnt.drop.file);
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

                /* Change volume (but only if needed) */
                if(volume_slider != 1.0f) {
                    for(int i = 0; i < n_samples; i++) {
                        to_buffer[i] *= volume_slider;
                    }
                }

                /* Change balance (L-R speaker) */
                if(balance_slider != 0.5f) {
                    for(int i = 0; i < n_samples; i += 2) {
                        /* Even samples are left, odd samples are right (from 0) */
                        to_buffer[i] *= (1.0f - balance_slider); 
                        to_buffer[i+1] *= balance_slider;
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
        SDL_RenderFillRect(renderer, &balance_rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &volume_knob);
        SDL_RenderFillRect(renderer, &balance_knob);

        SDL_RenderPresent(renderer);
    }

    SDL_CloseAudioDevice(speaker);
    SDL_FreeWAV(audioMemAddr);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void
panic_abort(const char *title, const char *text)
{
    fprintf(stderr, "%s ... %s\n", title, text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, window);

    SDL_Quit();
    exit(1);
}

SDL_bool
load_wav(const char *fname)
{
    SDL_FreeAudioStream(data_stream);
    SDL_FreeWAV(audioMemAddr);
    data_stream = NULL;
    audioMemAddr = NULL;
    audioLen = 0;

    if(SDL_LoadWAV(fname, &audioSpec, &audioMemAddr, &audioLen) == NULL) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
        goto failed;
    }

    data_stream = SDL_NewAudioStream(audioSpec.format, audioSpec.channels, audioSpec.freq, AUDIO_F32, 2, 48000);
    if(!data_stream) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
        goto failed;

    }

    if(SDL_AudioStreamPut(data_stream, audioMemAddr, audioLen) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
        goto failed;
    }

    if(SDL_AudioStreamFlush(data_stream) == -1) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error!", SDL_GetError(), window);
        goto failed;
    }

    return SDL_TRUE;

    failed:
        halt();
        return SDL_FALSE;
}

void
halt(void)
{
    if(data_stream) {
        SDL_FreeAudioStream(data_stream);
    }
    if(audioMemAddr) {
        SDL_FreeWAV(audioMemAddr);
    }
    data_stream = NULL;
    audioMemAddr = NULL;
    audioLen = 0;
}
