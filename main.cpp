#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include "customrendering.h"
#include "customdrawing.h"

typedef enum ToolType{
    PENCIL,
    PAINT,
    FILL,
    PICKER,
    SHAPE
}ToolType;

#define DEFAULTWIDTH 800
#define DEFAULTHEIGHT 600

int WIDTH = DEFAULTWIDTH, HEIGHT = DEFAULTHEIGHT;
int CANVASWIDTH = DEFAULTWIDTH, CANVASHEIGHT = 480;
int VIEWPORTWIDTH = WIDTH - CANVASWIDTH, VIEWPORTHEIGHT = HEIGHT - CANVASHEIGHT;
int VIEWPORTX = 0, VIEWPORTY = 0 + VIEWPORTHEIGHT;

int ELEMENTSCALE = 2;

toolbarRect defaultbar = {0,32,8,1,16,16};
toolbarRect filebar = {0,0,3,1,16,16};
toolbarRect palleteBar = {256,0,8,4,8,8};
toolbarRect fillOptionsBar = {64,64,1,1,16,16};
SDL_Rect toleranceSliderRect = {96,64,128,16};
toolbar* fillbar;
Slider* toleranceSlider;

//Default empty function that is called for an event
int DefaultEmpty(){ return -1;}
//Event Pointers
//Outside Viewport
int (*E_Click)() = &DefaultEmpty;
//Inside viewport
int (*E_Mousedown)() = &DefaultEmpty;
int (*E_Mouseup)() = &DefaultEmpty;

ToolType selectedTool = PENCIL;
Uint32 selectedColor = 0;
Uint32 tolerance = 0;
SDL_Color selectedColorStruct = {0,0,0,0};
bool fillmode = true;
int brushSize = 1;

SDL_Texture* texCont;
SDL_Texture* texGlob;
SDL_Texture* texFillCursor;

int save(void* p){
    printf("Save Pressed!\n");
    return 1;
}

int setFillMode(void* p){
    toolbarButton* button = (toolbarButton*)p;
    if (button->highlightmode == HLMODE_TOGGLE_ON)
    {
        button->highlightmode = HLMODE_TOGGLE_OFF;
        button->Texture = texGlob;
        fillmode = false;
    }
    else{
        button->highlightmode = HLMODE_TOGGLE_ON;
        button->Texture = texCont;
        fillmode = true;
    }
    return 0;
}

int setColour(void* p){
    toolbarButton button = *((toolbarButton*)p);
    int bpp = 8;
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    
    selectedColor = SDL_MapRGBA(format,button.r,button.g,button.b,255);
    printf("%08x\n",selectedColor);
    SDL_GetRGBA(selectedColor,format,&selectedColorStruct.r,&selectedColorStruct.g,&selectedColorStruct.b,&selectedColorStruct.a);
    button.r = selectedColorStruct.r;
    button.g = selectedColorStruct.g;
    button.b = selectedColorStruct.b;
    return 1;
}

int setToolPencil(void* p){
    selectedTool = PENCIL;
    printf("Selected Tool: Pencil\n");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    return 1;
}
int setToolPaint(void* p){
    selectedTool = PAINT;
    printf("Selected Tool: Paint\n");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    return 1;
}
int setToolFill(void* p){
    selectedTool = FILL;
    printf("Selected Tool: Fill\n");
    fillbar->mode = 0;
    toleranceSlider->hide = 0;
    return 1;
}
int setToolPicker(void* p){
    selectedTool = PICKER;
    printf("Selected Tool: Colour Picker\n");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    return 1;
}
int setToolShape(void* p){
    selectedTool = SHAPE;
    printf("Selected Tool: Shape\n");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    return 1;
}

int main( int argc, char* argv[] )
{
    SDL_Init( SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_Window * window = SDL_CreateWindow("CRUSTER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer * renderer = SDL_CreateRenderer(window,-1,0);
    SDL_Texture * canvas = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,CANVASWIDTH,CANVASHEIGHT);
    TTF_Font* font = TTF_OpenFont("resources/Cobol Bold.ttf",32);
    SDL_Color white = {0,0,0,255};
    SDL_Rect debugMessageBox{0,128,512,64};
    char message[] = "M_X:000|M_Y:000|L_X:000|L_Y|000";
    
    StringMessage* debugMessage = createStringMessage(renderer,message,debugMessageBox,white,font);
    Uint32 * pixels = new Uint32[CANVASHEIGHT * CANVASWIDTH];
    memset(pixels,0xFFFFFFFF,CANVASWIDTH * CANVASHEIGHT * sizeof(Uint32));
    
    
    toolbarButton *filebuttons = new toolbarButton[filebar.w * filebar.h];
    for (size_t i = 0; i < filebar.w * filebar.h; i++)
    {
        filebuttons[i] = createButton(nullptr,nullptr,0);
    }

    toolbarButton *toolButtons = new toolbarButton[defaultbar.w * defaultbar.h];
    for (size_t i = 0; i < defaultbar.w * defaultbar.h; i++)
    {
        toolButtons[i] = createButton(nullptr,nullptr,0);
    }

    toolbarButton *palleteButtons = new toolbarButton[palleteBar.w * palleteBar.h];

    for (size_t i = 0; i < palleteBar.w * palleteBar.h; i++)
    {
        palleteButtons[i] = createButton(&setColour,nullptr,HLMODE_COLOR);
        float fraction = i / (float)(palleteBar.w * palleteBar.h);
        float r = (255.0f * fraction);
        palleteButtons[i].r = (int)r;
        palleteButtons[i].g = (int)r;
        palleteButtons[i].b = (int)r;
    }
    
    SDL_Surface * imageSurface = IMG_Load("resources/icons/new.bmp");
    SDL_Texture * tex1 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[0] = createButton(nullptr,tex1,HLMODE_UNCHANGING);
    imageSurface = SDL_LoadBMP("resources/icons/open.bmp");
    SDL_Texture * tex2 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[1] = createButton(nullptr,tex2,HLMODE_UNCHANGING);
    imageSurface = SDL_LoadBMP("resources/icons/save.bmp");
    SDL_Texture * tex3 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[2] = createButton(&save,tex3,HLMODE_UNCHANGING);
    imageSurface = SDL_LoadBMP("resources/icons/pencil.bmp");
    SDL_Texture * tex4 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[0] = createButton(&setToolPencil,tex4,HLMODE_SELECTED);
    imageSurface = SDL_LoadBMP("resources/icons/paint.bmp");
    SDL_Texture * tex5 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[1] = createButton(&setToolPaint,tex5,HLMODE_UNSELECTED);
    imageSurface = SDL_LoadBMP("resources/icons/fill.bmp");
    SDL_Texture * tex6 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[2] = createButton(&setToolFill,tex6,HLMODE_UNSELECTED);
    imageSurface = IMG_Load("resources/icons/picker.png");
    SDL_Texture * tex7 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[3] = createButton(&setToolPicker,tex7,HLMODE_UNSELECTED);
    imageSurface = IMG_Load("resources/icons/shape.png");
    SDL_Texture * tex8 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[4] = createButton(&setToolShape,tex8,HLMODE_UNSELECTED);

    toolbarButton *fillButtons = new toolbarButton[1];
    imageSurface = IMG_Load("resources/icons/contiguous.png");
    texCont = SDL_CreateTextureFromSurface(renderer,imageSurface);
    fillButtons[0] = createButton(setFillMode,texCont,HLMODE_TOGGLE_ON);
    imageSurface = IMG_Load("resources/icons/global.png");
    texGlob = SDL_CreateTextureFromSurface(renderer,imageSurface);

    imageSurface = IMG_Load("resources/icons/fillcursor.png");
    texFillCursor = SDL_CreateTextureFromSurface(renderer,imageSurface);

    

    Slider *sliders = new Slider[1];
    sliders[0] = {toleranceSliderRect,1,50,&tolerance,1,SDL_MAX_UINT32};
    toleranceSlider = &sliders[0];
    

    toolbar toolbars[] = {TB_Create(defaultbar,toolButtons),TB_Create(filebar,filebuttons),TB_Create(palleteBar,palleteButtons),TB_Create(fillOptionsBar,fillButtons)};
    toolbars[3].mode = 1;
    fillbar = &toolbars[3];
    int numToolbars = 4;
    

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
    int mouse_down = 0;

    int x = 0;
    int y = 0;

    bool drawing = false;

    SDL_Rect screen = {0,0,WIDTH,HEIGHT};
    SDL_Rect viewport = {VIEWPORTX,VIEWPORTY,CANVASWIDTH,CANVASHEIGHT};
    SDL_Rect snapshot = {0,0,CANVASWIDTH,CANVASHEIGHT};

    bool running = true;


    while (running)
    {
       
        bool mouseInViewport = mouse_x > viewport.x && mouse_y > viewport.y && mouse_x < viewport.x + viewport.w && mouse_y < viewport.y + viewport.h;
        int localx = mouse_x - viewport.x;
        int localy = mouse_y - viewport.y;
        if (localy < 0) localy = 0;
        
        int clicked = 0;
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
                    switch (selectedTool)
                    {
                    case PENCIL:
                        drawing = true;
                        break;
                    case PAINT:
                        drawing = true;
                    break;
                    case FILL:

                    break;
                    break;
                    default:
                        break;
                    }
                    
                }
                else{
                    mouse_down = 1;
                }
                
            break;

            case SDL_MOUSEBUTTONUP:
                if (mouseInViewport){
                    switch (selectedTool)
                    {
                    case PENCIL:
                        drawing = false;
                    break;
                    case PAINT:
                        drawing = false;
                    break;
                    case FILL:
                        
                        if (localy < 0) localy = 0;
                        fillmode ? contiguousFill(pixels,localx, localy,snapshot.w,snapshot.h,selectedColor,tolerance) : globalFill(pixels,localx, localy,snapshot.w,snapshot.h,selectedColor,tolerance);
                        SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
                    break;
                    default:
                    break;
                    }
                } 
                else{
                    clicked = 1;
                    mouse_down = 0;
                }
                
            break;

            case SDL_KEYDOWN:
                switch (windowEvent.key.keysym.sym)
                {
                    case SDLK_LEFTBRACKET:
                        brushSize -= 1;
                        if (brushSize == 0) brushSize = 1;
                    break;
                    case SDLK_RIGHTBRACKET:
                        brushSize += 1;
                    break;
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
                    
                    if (localy < 0) localy = 0;
                    //char* message = new char[256];
                    //(message + 0,"M_X:%i",mouse_x);
                    //sprintf(message + 8,"|M_X:%i",mouse_x);
                    //sprintf(message + 16,"|L_X:%i",localx);
                    //sprintf(message + 24,"|L_Y:%i",localy);
                    //puts(message);
                    //updateStringMessage(renderer,debugMessage,message);
                    //delete message;
                    //SDL_Point p = GetTextureCoordinate(localx,localy,viewport.w,viewport.h,CANVASWIDTH,CANVASHEIGHT);
                    //pixels[(localy * CANVASWIDTH) + localx] = selectedColor;
                    SDL_Rect brush = {localx - brushSize / 2, localy - brushSize / 2, (localx + brushSize) - localx, (localy + brushSize) - localy};
                    if (selectedTool == PENCIL)
                    {
                        drawSquare(pixels,brush,snapshot,selectedColor);
                    }
                    else{
                        drawCircle(pixels,localx,localy,brushSize,snapshot,selectedColor);
                    }
                    
                    SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
                    
                }
            }
        else
        {
            SDL_ShowCursor(1);
            drawSpecialCursor = false;
        }

         SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer,148,148,148,255);
        SDL_Rect rect = {0,0,WIDTH,HEIGHT};
        SDL_RenderFillRect(renderer,&rect);

        SDL_RenderCopy(renderer, canvas, NULL, &viewport);
        
        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderDrawRect(renderer, &viewport);

        renderStringMessage(renderer,debugMessage);
        
        
        toolbarButton* selectedButton = renderAllToolbars(renderer,toolbars,numToolbars,2,mouse_x,mouse_y,clicked);
        renderAllSliders(renderer,sliders,1,ELEMENTSCALE,mouse_x,mouse_y,mouse_down);
        //SDL_RenderCopy(renderer,tex,NULL,NULL);
        
        
        //drawToolbar(renderer,&filebar,1,0,0);
        if (drawSpecialCursor)
        {
            SDL_SetRenderDrawColor(renderer,148,148,148,255);
            SDL_RenderDrawLine(renderer,mouse_x - 5, mouse_y,mouse_x + 5, mouse_y);
            SDL_RenderDrawLine(renderer,mouse_x, mouse_y - 5,mouse_x, mouse_y + 5);
            if (selectedTool == PAINT)renderCircle(renderer,mouse_x,mouse_y,brushSize / 2);
            else if(selectedTool == PENCIL){
                SDL_Rect brush = {mouse_x - brushSize / 2, mouse_y - brushSize / 2, (mouse_x + brushSize) - mouse_x, (mouse_y + brushSize) - mouse_y};
                SDL_RenderDrawRect(renderer,&brush);
            }
            else if(selectedTool == FILL){
                SDL_Rect cursor = {mouse_x - 2,mouse_y - 30,32,32};
                SDL_RenderCopy(renderer,texFillCursor,NULL,&cursor);
            }
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