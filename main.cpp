#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "customrendering.h"
#include "customdrawing.h"

#define DEFAULTWIDTH 800
#define DEFAULTHEIGHT 600

int WIDTH = DEFAULTWIDTH, HEIGHT = DEFAULTHEIGHT;
int CANVASWIDTH = 640, CANVASHEIGHT = 480;
int VIEWPORTWIDTH = WIDTH - CANVASWIDTH, VIEWPORTHEIGHT = HEIGHT - CANVASHEIGHT;
int VIEWPORTX = 0, VIEWPORTY = 0 + VIEWPORTHEIGHT;

int ELEMENTSCALE = 2;

toolbarRect defaultbar = {0,32,8,1,16,16};
toolbarRect filebar = {0,0,3,1,16,16};

//Default empty function that is called for an event
int DefaultEmpty(){ return -1;}
//Event Pointers
//Outside Viewport
int (*E_Click)() = &DefaultEmpty;
//Inside viewport
int (*E_Mousedown)() = &DefaultEmpty;
int (*E_Mouseup)() = &DefaultEmpty;

int v = IMG_INIT_AVIF;

int main( int argc, char* argv[] )
{
    SDL_Init( SDL_INIT_EVERYTHING);

    SDL_Window * window = SDL_CreateWindow("CRUSTER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer * renderer = SDL_CreateRenderer(window,-1,0);
    SDL_Texture * canvas = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,CANVASWIDTH,CANVASHEIGHT);
    Uint32 * pixels = new Uint32[CANVASHEIGHT * CANVASWIDTH];
    memset(pixels,255,CANVASWIDTH * CANVASHEIGHT * sizeof(Uint32));
    
    
    toolbarButton *filebuttons = new toolbarButton[filebar.w * filebar.h];
    for (size_t i = 0; i < filebar.w * filebar.h; i++)
    {
        filebuttons[i] = createButton(nullptr,nullptr);
    }

    toolbarButton *toolButtons = new toolbarButton[defaultbar.w * defaultbar.h];
    for (size_t i = 0; i < defaultbar.w * defaultbar.h; i++)
    {
        toolButtons[i] = createButton(nullptr,nullptr);
    }
    
    SDL_Surface * imageSurface = IMG_Load("resources/icons/new.bmp");
    SDL_Texture * tex1 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[0] = createButton(nullptr,tex1);
    imageSurface = SDL_LoadBMP("resources/icons/open.bmp");
    SDL_Texture * tex2 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[1] = createButton(nullptr,tex2);
    imageSurface = SDL_LoadBMP("resources/icons/save.bmp");
    SDL_Texture * tex3 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[2] = createButton(nullptr,tex3);
    imageSurface = SDL_LoadBMP("resources/icons/pencil.bmp");
    SDL_Texture * tex4 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[0] = createButton(nullptr,tex4);
    imageSurface = SDL_LoadBMP("resources/icons/paint.bmp");
    SDL_Texture * tex5 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[1] = createButton(nullptr,tex5);
    imageSurface = SDL_LoadBMP("resources/icons/fill.bmp");
    SDL_Texture * tex6 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[2] = createButton(nullptr,tex6);
    imageSurface = IMG_Load("resources/icons/picker.png");
    SDL_Texture * tex7 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[3] = createButton(nullptr,tex7);
    imageSurface = IMG_Load("resources/icons/shape.png");
    SDL_Texture * tex8 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[4] = createButton(nullptr,tex8);

    toolbar toolbars[] = {TB_Create(defaultbar,toolButtons),TB_Create(filebar,filebuttons)};
    int numToolbars = 2;
    
    if (imageSurface == NULL)
    {
        std::cout <<SDL_GetError() << std::endl;
        
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

    bool running = true;


    while (running)
    {
        bool mouseInViewport = mouse_x > viewport.x && mouse_y > viewport.y && mouse_x < viewport.x + viewport.w && mouse_y < viewport.y + viewport.h;
        if (SDL_PollEvent( &windowEvent))
        {
            switch (windowEvent.type)
            {
            case SDL_QUIT:
                running = false;
            continue;

            case SDL_WINDOWEVENT:
                if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED);
            break;

            case SDL_MOUSEMOTION:
                mouse_x = windowEvent.motion.x;
                mouse_y = windowEvent.motion.y;
            break;

            case SDL_MOUSEBUTTONDOWN:
                if (mouseInViewport)
                {
                    drawing = true;
                }
                else{

                }
                
            break;

            case SDL_MOUSEBUTTONUP:
                drawing = false;
            break;

            case SDL_KEYDOWN:
                switch (windowEvent.key.keysym.sym)
                {
                default:
                    break;
                }
            break;

            default:
                break;
            }
           
        }
        
        bool drawSpecialCursor = false;
        if (mouseInViewport)
            {
                SDL_ShowCursor(0);
                drawSpecialCursor = true;
                if (drawing)
                {
                    printf("X: %i",mouse_x ,"%s Y: %i",mouse_y,"\n");
                    int localx = mouse_x - viewport.x;
                    int localy = mouse_y - viewport.y;
                    //SDL_Point p = GetTextureCoordinate(localx,localy,viewport.w,viewport.h,CANVASWIDTH,CANVASHEIGHT);
                    pixels[localy * CANVASWIDTH + localx] = 0;
                    SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
                    
                }
            }
        else
        {
            SDL_ShowCursor(1);
            drawSpecialCursor = false;
        }
        SDL_SetRenderDrawColor(renderer,148,148,148,255);
        SDL_Rect rect = {0,0,WIDTH,HEIGHT};
        SDL_RenderFillRect(renderer,&rect);
        
        SDL_RenderClear(renderer);
        toolbarButton* selectedButton = renderAllToolbars(renderer,toolbars,2,2,mouse_x,mouse_y);
        //SDL_RenderCopy(renderer,tex,NULL,NULL);
        SDL_RenderCopy(renderer, canvas, &snapshot, &viewport);
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderDrawRect(renderer, &viewport);
        
        //drawToolbar(renderer,&filebar,1,0,0);
        if (drawSpecialCursor)
        {
            SDL_SetRenderDrawColor(renderer,148,148,148,255);
            SDL_RenderDrawLine(renderer,mouse_x - 5, mouse_y,mouse_x + 5, mouse_y);
            SDL_RenderDrawLine(renderer,mouse_x, mouse_y - 5,mouse_x, mouse_y + 5);
            drawCircle(renderer,mouse_x,mouse_y,10);
        }
        SDL_RenderPresent(renderer);
       SDL_Delay(17);
    }

    delete[] pixels;
    SDL_FreeSurface(imageSurface);
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}