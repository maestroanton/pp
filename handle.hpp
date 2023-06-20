#ifndef HANDLE_H
#define HANDLE_H

void halt(SDL_AudioStream *stream, Uint8 *buf, Uint32 len); /* Stops audio and cleans up. */
void panic_abort(const char *title, const char *text, SDL_Window *win); /* Cleans up and quits on catastrophic errors. */

#endif
