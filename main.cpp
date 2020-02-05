#include <cstdint>
#include <SDL2/SDL.h>
#include <portaudio.h>
#include "kiss_fft/kiss_fft.h"
#include <cmath>

using namespace std;

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 400

#define SAMPLE_RATE (44100)

//SDL.
SDL_Event event; //Create Event Handler.
SDL_Renderer *renderer; //Create Renderer.
SDL_Window *window; //Create Window.
SDL_Texture *texture; //Create a texture to store framebuffer.
void init_sdl ();

// Create frame buffer.
struct RGB {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} frameBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

// Port audio.
PaStream *stream;
void init_audio();

// Kiss fft setup.
kiss_fft_cfg cfg = kiss_fft_alloc( 1024 ,0 ,0,0 );


void update_screen (float *input);
static int pa_callback( const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData );
void line(int x0, int y0, int x1, int y1);

// Utils.
float find_max_level (kiss_fft_cpx *data, int n);

int main(int argv, char** args){
    init_sdl();
    init_audio();


    int running = 1;

    while (running) {

        while(SDL_PollEvent( &event )) {
            if (SDL_PollEvent(&event) && event.type == SDL_QUIT){ //If close button pressed.
                running = 0;
            }
        }
    }

    Pa_StopStream( stream );
    Pa_CloseStream( stream );
}

void update_screen (float *input){
    // Clear.
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            frameBuffer[y][x].red = 0;
            frameBuffer[y][x].blue = 0;
            frameBuffer[y][x].green = 0;
        }
    }

    // Spectrogram.
    // DO fft.
    kiss_fft_cpx fft_in[1024];
    kiss_fft_cpx fft_out[1024];
    for (int i = 0; i < 1024; i++) {
        fft_in[i].r = input[i];
    }
    kiss_fft( cfg , fft_in , fft_out );

    // Find max for scaling.
    float max = find_max_level(fft_out, 1024);
    if (max < 1.0) {
        max = 1.0f;
    }

    // Draw to screen.
    int previous_y = SCREEN_HEIGHT - 1;
    int out_max = 400;
    int out_max_x = 0;
    for (int x = 1; x < 256; x+=1) {
        fft_out[x].r = fabsf(fft_out[x].r);
        float scaled_val = fft_out[x].r / (max + 2.0f);
        int y = SCREEN_HEIGHT - ((int) (scaled_val * (float) (SCREEN_HEIGHT))) - 1;
        line((x - 1) * 4, previous_y, x * 4, y);
        previous_y = y;
        if (y < out_max) {
            out_max = y;
            out_max_x = x * 4;
        }
    }

    // Draw line at max.
    for (int y = 0; y < 400; y++){
        frameBuffer[y][out_max_x].red = 255;
    }

    // oscilloscope.
    previous_y = SCREEN_HEIGHT / 2;
    for (int x = 1; x < 1024; x+=1) {
        if (input[x] < 0.99f && input[x] > -0.99f) {
            int y = SCREEN_HEIGHT / 2 + (int)(input[x] * (float) (SCREEN_HEIGHT / 2));
            line((x - 1), previous_y, x, y);
            previous_y = y;
        }
    }

    SDL_UpdateTexture(texture, NULL, frameBuffer, SCREEN_WIDTH * sizeof(uint8_t) * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// Draw line between points.
// FROM https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.2B.2B.
void line(int x0, int y0, int x1, int y1) {

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;){
        frameBuffer[y0][x0].red = 255;
        frameBuffer[y0][x0].green = 255;
        frameBuffer[y0][x0].blue = 255;
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

float find_max_level (kiss_fft_cpx *data, int n) {
    int max = 0.0f;
    for (int i = 0; i < n; i++) {
        if (fabsf(data[i].r) > max) {
            max = fabsf(data[i].r);
        }
    }
    return max;
}

static int pa_callback( const void *input, void *output, unsigned long frameCount,
                        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData ) {

    update_screen ((float *)input);

    return 0;
}

void init_audio() {
    Pa_Initialize();
    Pa_OpenDefaultStream(&stream, 1, 1, paFloat32, SAMPLE_RATE, 1024, pa_callback, NULL);
    Pa_StartStream( stream );
}

void init_sdl (){
    //Start SDL.
    SDL_Init( SDL_INIT_VIDEO );

    //Initialize the window and renderer.
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    //Create Texture for buffering.
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    //Set the render Draw Color to White.
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    //Clear Screen (White).
    SDL_RenderClear(renderer);

    //Display on screen.
    SDL_RenderPresent(renderer);
}

