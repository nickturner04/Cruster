#include <iostream>
#include <SDL2/SDL.h>
#include "drawing.h"

#define DEFAULTWIDTH 800
#define DEFAULTHEIGHT 600

int WIDTH = DEFAULTWIDTH, HEIGHT = DEFAULTHEIGHT;
int CANVASWIDTH = 640, CANVASHEIGHT = 480;
int VIEWPORTWIDTH = WIDTH - CANVASWIDTH, VIEWPORTHEIGHT = HEIGHT - CANVASHEIGHT;
int VIEWPORTX = 0 + VIEWPORTWIDTH, VIEWPORTY = 0 + VIEWPORTHEIGHT;

int main( int argc, char* argv[] )
{
    SDL_Init( SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow("CRUSTER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer * renderer = SDL_CreateRenderer(window,-1,0);
    SDL_Texture * canvas = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,CANVASWIDTH,CANVASHEIGHT);
    Uint32 * pixels = new Uint32[CANVASHEIGHT * CANVASWIDTH];
    memset(pixels,255,CANVASWIDTH * CANVASHEIGHT * sizeof(Uint32));

    SDL_Surface * imageSurface = SDL_LoadBMP("resources/turkey.bmp");
    if (imageSurface == NULL)
    {
        std::cout << "Image not loaded" << SDL_GetError() << std::endl;
    }
    else{
        
    }
    

    if (NULL == window)
    {
        std::cout << "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_Event windowEvent;

    //variables

    int mouse_x = 0;
    int mouse_y = 0;

    int x = 0;
    int y = 0;

    int brushRadius = 25;

    bool drawing = false;

    SDL_Rect screen = {0,0,WIDTH,HEIGHT};
    SDL_Rect viewport = {VIEWPORTX,VIEWPORTY,CANVASWIDTH,CANVASHEIGHT};
    SDL_Rect snapshot = {0,0,CANVASHEIGHT,CANVASWIDTH};

    while (true)
    {
        SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
        if (SDL_PollEvent( &windowEvent))
        {
           if (SDL_QUIT == windowEvent.type){
                break;
           }
           else if (SDL_WINDOWEVENT == windowEvent.type)
           {
                if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED);
                
                
           }
           
           else if(SDL_MOUSEBUTTONDOWN == windowEvent.type){
            drawing = true;
           }
           else if(SDL_MOUSEBUTTONUP == windowEvent.type){
            drawing = false;
           }
           else if (SDL_MOUSEMOTION == windowEvent.type)
           {
            mouse_x = windowEvent.motion.x;
            mouse_y = windowEvent.motion.y;
           }
           
        }
        if (drawing)
        {
            printf("X: %i",mouse_x ,"%s Y: %i",mouse_y,"\n");
            
            
        }
        bool drawSpecialCursor = false;
        if (mouse_x > VIEWPORTX && mouse_y > VIEWPORTY)
            {
                SDL_ShowCursor(0);
                drawSpecialCursor = true;
            }
            else{
                SDL_ShowCursor(1);
                drawSpecialCursor = false;
            }
        SDL_SetRenderDrawColor(renderer,148,148,148,255);
        SDL_Rect rect = {0,0,WIDTH,HEIGHT};
        SDL_RenderFillRect(renderer,&rect);
        
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
        SDL_RenderCopy(renderer, canvas, &snapshot, &viewport);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderDrawRect(renderer, &viewport);

        if (drawSpecialCursor)
        {
            SDL_SetRenderDrawColor(renderer,148,148,148,255);
            SDL_RenderDrawLine(renderer,mouse_x - 5, mouse_y,mouse_x + 5, mouse_y);
            SDL_RenderDrawLine(renderer,mouse_x, mouse_y - 5,mouse_x, mouse_y + 5);
            drawCircle(renderer,mouse_x,mouse_y,10);
        }
        SDL_RenderPresent(renderer);
       
    }

    delete[] pixels;
    SDL_FreeSurface(imageSurface);
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}