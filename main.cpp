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
    SHAPE,
    ELIPSE,
    LINE
}ToolType;

#define DEFAULTWIDTH 800
#define DEFAULTHEIGHT 600

int WIDTH = DEFAULTWIDTH, HEIGHT = DEFAULTHEIGHT;
int CANVASWIDTH = DEFAULTWIDTH, CANVASHEIGHT = 480;
int VIEWPORTWIDTH = WIDTH - CANVASWIDTH, VIEWPORTHEIGHT = HEIGHT - CANVASHEIGHT;
int VIEWPORTX = 0, VIEWPORTY = 0 + VIEWPORTHEIGHT;

int ELEMENTSCALE = 2;

TTF_Font* systemFont;

toolbarRect defaultbar = {0,32,8,1,16,16};
toolbarRect filebar = {0,0,4,1,16,16};
toolbarRect palleteBar = {256,0,8,4,8,8};
toolbarRect fillOptionsBar = {64,64,1,1,16,16};
toolbarRect polyOptionsBar = {96,64,3,1,16,16};
SDL_Rect toleranceSliderRect = {96,64,128,16};
SDL_Rect polySliderRect = {96,96,128,16};
SDL_Rect rSliderRect = {256,64,128,16};
SDL_Rect gSliderRect = {256,80,128,16};
SDL_Rect bSliderRect = {256,96,128,16};
toolbar* fillbar;
toolbar* polybar;
Slider* toleranceSlider;
Slider* polySlider;

Uint32* pixelGrid;
char* fileName = NULL;

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
Uint32 valueR = 0;
Uint32 valueG = 0;
Uint32 valueB = 0;
Uint32 tolerance = 0;
bool fillmode = true;
Uint32 polyFillPercent = 50;
bool fillpoly = false;
int polyVerts = 3;
int brushSize = 1;
StringMessage* toolMessage;
SDL_Renderer* renderer;
SDL_Texture* canvas;

SDL_Texture* texCont;
SDL_Texture* texGlob;
SDL_Texture* texFillCursor;
SDL_Texture* texUnFillPoly;
SDL_Texture* texFillPoly;

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

int saveAs(void* p){
    char message[] = "CHECK CONSOLE";
    SDL_Rect r = {0,0,CANVASWIDTH,CANVASHEIGHT};
    SDL_Color red = {255,0,0,0};
    StringMessage* consoleMessage = createStringMessage(renderer,message,r,red,systemFont);
    renderStringMessage(renderer,consoleMessage);
    SDL_RenderPresent(renderer);
    printf("\n");
    printf("Enter filename to save(.png will be automatically appended) ");
    char* newfileName = new char[128];
    scanf("%127s",newfileName);
    size_t l = strlen(newfileName);
    strcat(newfileName,".png\0");
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(pixelGrid,CANVASWIDTH,CANVASHEIGHT,32,CANVASWIDTH * sizeof(Uint32),SDL_PIXELFORMAT_ARGB8888);
    IMG_SavePNG(surf,newfileName);
    fileName = newfileName;
    SDL_FreeSurface(surf);
    destroyStringMessage(consoleMessage);
    return 0;
}

int save(void* p){
    if (fileName == NULL) return saveAs(p);
    
    SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(pixelGrid,CANVASWIDTH,CANVASHEIGHT,32,CANVASWIDTH * sizeof(Uint32),SDL_PIXELFORMAT_ARGB8888);
    IMG_SavePNG(surf,fileName);
    SDL_FreeSurface(surf);
    printf("Saved To: ");
    printf("%s",fileName);
    printf("\n");
    return 0;
}

int open(void* p){
    char message[] = "CHECK CONSOLE";
    SDL_Rect r = {0,0,CANVASWIDTH,CANVASHEIGHT};
    SDL_Color red = {255,0,0,0};
    StringMessage* consoleMessage = createStringMessage(renderer,message,r,red,systemFont);
    renderStringMessage(renderer,consoleMessage);
    SDL_RenderPresent(renderer);
    printf("\n");
    printf("Enter filename to open(specify file extension) ");
    char* newfileName = new char[128];
    scanf("%127s",newfileName);
    size_t l = strlen(newfileName);
    for (size_t i = 0; i < l; i++)
    {
        if (newfileName[i] == '.') newfileName[i] = 0;
        
    }
    strcat(newfileName,".png");
    if (fileName != NULL) delete fileName;
    fileName = newfileName;

    SDL_Surface* imageSurface = IMG_Load(newfileName);
    SDL_UnlockSurface(imageSurface);
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    for (size_t i = 0; i < CANVASWIDTH * CANVASHEIGHT; i++)
    {
        Uint8 r;
        Uint8 g;
        Uint8 b;
        Uint32* pixels = (Uint32*)imageSurface->pixels;
        SDL_GetRGB(pixels[i],imageSurface->format,&r,&g,&b);

        pixelGrid[i] = SDL_MapRGBA(format,r,g,b,255);
    }
    SDL_UpdateTexture(canvas,NULL,pixelGrid,CANVASWIDTH * sizeof(Uint32));
    SDL_FreeFormat(format);
    SDL_FreeSurface(imageSurface);
    destroyStringMessage(consoleMessage);
    return 0;
}

int newDoc(void* p){
    char message[] = "CHECK CONSOLE";
    SDL_Rect r = {0,0,CANVASWIDTH,CANVASHEIGHT};
    SDL_Color red = {255,0,0,0};
    StringMessage* consoleMessage = createStringMessage(renderer,message,r,red,systemFont);
    renderStringMessage(renderer,consoleMessage);
    SDL_RenderPresent(renderer);
    printf("\n");
    printf("Enter new project name ");
    char* newfileName = new char[128];
    scanf("%127s",newfileName);
    size_t l = strlen(newfileName);
    strcat(newfileName,".png\0");
    if(fileName != NULL) delete fileName;
    fileName = newfileName;

    memset(pixelGrid,0xFFFFFFFF,CANVASWIDTH * CANVASHEIGHT * sizeof(Uint32));
    SDL_UpdateTexture(canvas,NULL,pixelGrid,CANVASWIDTH * sizeof(Uint32));
    destroyStringMessage(consoleMessage);
    return 0;
}



int setColour(void* p){
    toolbarButton* button = ((toolbarButton*)p);
    int bpp = 8;
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    
    selectedColor = SDL_MapRGBA(format,button->r,button->g,button->b,255);
    //printf("%08x\n",selectedColor);
    SDL_FreeFormat(format);
    return 1;
}

void setColourR(int x){
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    Uint8 r, g, b, a;
    SDL_GetRGBA(selectedColor,format,&r,&g,&b,&a);
    r = (Uint8)x;
    selectedColor = SDL_MapRGBA(format,r,g,b,255);
    SDL_FreeFormat(format);
}

void setColourG(int x){
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    Uint8 r, g, b, a;
    SDL_GetRGBA(selectedColor,format,&r,&g,&b,&a);
    g = (Uint8)x;
    selectedColor = SDL_MapRGBA(format,r,g,b,255);
    SDL_FreeFormat(format);
}

void setColourB(int x){
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    Uint8 r, g, b, a;
    SDL_GetRGBA(selectedColor,format,&r,&g,&b,&a);
    b = (Uint8)x;
    selectedColor = SDL_MapRGBA(format,r,g,b,255);
    SDL_FreeFormat(format);
}

void showPolyOptions(){
    polybar->mode = 0;
    polySlider->hide = !fillpoly;
}

void hidePolyOptions(){
    polybar->mode = 1;
    polySlider->hide = 1;
}

int setToolPencil(void* p){
    selectedTool = PENCIL;
    printf("Selected Tool: Pencil\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Pencil");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    hidePolyOptions();
    return 1;
}
int setToolPaint(void* p){
    selectedTool = PAINT;
    printf("Selected Tool: Paint\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Brush");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    hidePolyOptions();
    return 1;
}
int setToolFill(void* p){
    selectedTool = FILL;
    printf("Selected Tool: Fill\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Fill");
    fillbar->mode = 0;
    toleranceSlider->hide = 0;
    hidePolyOptions();
    return 1;
}
int setToolPicker(void* p){
    selectedTool = PICKER;
    printf("Selected Tool: Colour Picker\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Picker");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    hidePolyOptions();
    return 1;
}
int setToolRPoly(void* p){
    
    toolbarButton * b = (toolbarButton*)p;
    if (selectedTool == SHAPE)
    {
        if (fillpoly)
        {
            fillpoly = false;
            b->Texture = texUnFillPoly;
        }
        else{
            fillpoly = true;
            b->Texture = texFillPoly;
        }
    }
    showPolyOptions();

    
    selectedTool = SHAPE;
    printf("Selected Tool: Shape(");
    printf("%i",polyVerts);
    printf(" Vertices)\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: RPoly");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    return 1;
}

    int setPolyVertsMinus(void* p){
        polyVerts--;
        if (polyVerts <= 2) polyVerts = 3;
        printf("Number of Vertices: ");
        printf("%i", polyVerts);
        printf("\n");
        return 1;
    }

    int setPolyVertsPlus(void* p){
        polyVerts++;
        printf("Number of Vertices: ");
        printf("%i", polyVerts);
        printf("\n");
        return 1;
    }

    int setPolyVertsConsole(void* p){

        return 1;
    }

int setToolElipse(void* p){
    selectedTool = ELIPSE;
    printf("Selected Tool: Elipse\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Elipse");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    hidePolyOptions();
    return 1;
}

int setToolLine(void* p){
    selectedTool = LINE;
    printf("Selected Tool: Line\n");
    updateStringMessage(renderer,toolMessage,"Selected Tool: Line");
    fillbar->mode = 1;
    toleranceSlider->hide = 1;
    hidePolyOptions();
    return 1;
}

int main( int argc, char* argv[] )
{

    SDL_Init( SDL_INIT_EVERYTHING);
    TTF_Init();
    SDL_Window * window = SDL_CreateWindow("CRUSTER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window,-1,0);
    canvas = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING,CANVASWIDTH,CANVASHEIGHT);
    TTF_Font* font = TTF_OpenFont("resources/Cobol Bold.ttf",32);
    systemFont = font;
    SDL_Color white = {0,0,0,255};
    SDL_Rect debugMessageBox{388,0,128,32};
    char message[] = "Selected Tool: Pencil";
    
    StringMessage* debugMessage = createStringMessage(renderer,message,debugMessageBox,white,font);
    toolMessage = debugMessage;
    Uint32 * pixels = new Uint32[CANVASHEIGHT * CANVASWIDTH];
    pixelGrid = pixels;
    memset(pixels,0xFFFFFFFF,CANVASWIDTH * CANVASHEIGHT * sizeof(Uint32));
    SDL_UpdateTexture(canvas,NULL,pixelGrid,CANVASWIDTH * sizeof(Uint32));
    
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

    toolbarButton* polyButtons = new toolbarButton[polyOptionsBar.w * polyOptionsBar.h];

    for (size_t i = 0; i < polyOptionsBar.w * polyOptionsBar.h; i++)
    {
        polyButtons[i] = createButton(nullptr,nullptr,HLMODE_UNCHANGING);
    }
    
    
    SDL_Surface * imageSurface = IMG_Load("resources/icons/new.bmp");
    SDL_Texture * tex1 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[0] = createButton(&newDoc,tex1,HLMODE_UNCHANGING);
    imageSurface = SDL_LoadBMP("resources/icons/open.bmp");
    SDL_Texture * tex2 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[1] = createButton(open,tex2,HLMODE_UNCHANGING);
    imageSurface = SDL_LoadBMP("resources/icons/save.bmp");
    SDL_Texture * tex3 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[2] = createButton(&save,tex3,HLMODE_UNCHANGING);
    imageSurface = IMG_Load("resources/icons/saveas.png");
    SDL_Texture * texSaveAs = SDL_CreateTextureFromSurface(renderer,imageSurface);
    filebuttons[3] = createButton(&saveAs,texSaveAs,HLMODE_UNCHANGING);
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
    imageSurface = IMG_Load("resources/icons/regularPoly.png");
    texUnFillPoly = SDL_CreateTextureFromSurface(renderer,imageSurface);
    imageSurface = IMG_Load("resources/icons/regularPolyF.png");
    texFillPoly = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[4] = createButton(&setToolRPoly,texUnFillPoly,HLMODE_UNSELECTED);
    imageSurface = IMG_Load("resources/icons/elipse.png");
    SDL_Texture * tex9 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[5] = createButton(&setToolElipse,tex9,HLMODE_UNSELECTED);
    imageSurface = IMG_Load("resources/icons/line.png");
    SDL_Texture * tex10 = SDL_CreateTextureFromSurface(renderer,imageSurface);
    toolButtons[6] = createButton(&setToolLine,tex10,HLMODE_UNSELECTED);

    toolbarButton *fillButtons = new toolbarButton[1];
    imageSurface = IMG_Load("resources/icons/contiguous.png");
    texCont = SDL_CreateTextureFromSurface(renderer,imageSurface);
    fillButtons[0] = createButton(setFillMode,texCont,HLMODE_TOGGLE_ON);
    imageSurface = IMG_Load("resources/icons/global.png");
    texGlob = SDL_CreateTextureFromSurface(renderer,imageSurface);

    imageSurface = IMG_Load("resources/icons/fillcursor.png");
    texFillCursor = SDL_CreateTextureFromSurface(renderer,imageSurface);

    imageSurface = IMG_Load("resources/icons/arrowL.png");
    SDL_Texture * texL = SDL_CreateTextureFromSurface(renderer,imageSurface);
    polyButtons[0] = createButton(&setPolyVertsMinus,texL,HLMODE_UNCHANGING);
    imageSurface = IMG_Load("resources/icons/textEntry.png");
    SDL_Texture * texTE = SDL_CreateTextureFromSurface(renderer,imageSurface);
    polyButtons[1] = createButton(nullptr,texTE,HLMODE_UNCHANGING);
    imageSurface = IMG_Load("resources/icons/arrowR.png");
    SDL_Texture * texR = SDL_CreateTextureFromSurface(renderer,imageSurface);
    polyButtons[2] = createButton(&setPolyVertsPlus,texR,HLMODE_UNCHANGING);

    MultiSlider *sliders = new MultiSlider[5];
    {
        Slider sldTol = {toleranceSliderRect,0,1,50,1,SDL_MAX_UINT32,&tolerance,0x000000FF};
        Slider sldPoly = {polySliderRect,0,1,50,0,100,&polyFillPercent,0x00000011};
        sliders[0].s = sldTol;
        sliders[1].s = sldPoly;

        EventSlider sldR = {rSliderRect,1,0,0,0,255,&setColourR,0xFFFF0000};
        EventSlider sldG = {gSliderRect,1,0,0,0,255,&setColourG,0xFF00FF00};
        EventSlider sldB = {bSliderRect,1,0,0,0,255,&setColourB,0xFF0000FF};
        sliders[2].es = sldR;
        sliders[3].es = sldG;
        sliders[4].es = sldB;
    }
    toleranceSlider = &sliders[0].s;
    polySlider = &sliders[1].s;

    toolbar toolbars[] = {TB_Create(defaultbar,toolButtons),TB_Create(filebar,filebuttons),TB_Create(palleteBar,palleteButtons),TB_Create(fillOptionsBar,fillButtons),TB_Create(polyOptionsBar,polyButtons)};
    toolbars[3].mode = 1;
    toolbars[4].mode = 1;
    fillbar = &toolbars[3];
    polybar = &toolbars[4];
    int numToolbars = 5;
    

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

    int last_mouse_x = -1;
    int last_mouse_y = -1;

    int x = 0;
    int y = 0;

    bool drawing = false;

    bool shapeClicked = false;
    SDL_Point shapePoint = {0,0};
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
                if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
                    SDL_GetWindowSize(window,&WIDTH,&HEIGHT);
                    viewport.w = WIDTH - viewport.x;
                    viewport.h = HEIGHT - viewport.y;
                };
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
                last_mouse_x = -1;
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
                    case SHAPE:
                    case LINE:
                        if (shapeClicked)
                        {
                            shapeClicked = false;
                            if (selectedTool == LINE)
                            {
                                drawBresenham(pixels,CANVASWIDTH,CANVASHEIGHT,shapePoint.x,shapePoint.y,localx,localy,selectedColor);
                            }
                            else if (selectedTool == SHAPE)
                            {
                                drawPoly(pixelGrid,CANVASWIDTH,CANVASHEIGHT,shapePoint.x,shapePoint.y,localx,localy,polyVerts,fillpoly,(100 - polyFillPercent),selectedColor);
                            }
                            
                            
                            SDL_UpdateTexture(canvas,NULL,pixels,CANVASWIDTH * sizeof(Uint32));
                        }
                        else{
                            shapeClicked = true;
                            shapePoint.x = localx;
                            shapePoint.y = localy;
                        }
                        
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
                        //drawSquare(pixels,brush,snapshot,selectedColor);
                        if (last_mouse_x == -1 || last_mouse_y == -1)
                        {
                            last_mouse_x = localx;
                            last_mouse_y = localy;
                        }
                        else{
                            drawBresenham(pixelGrid,snapshot.w,snapshot.h,last_mouse_x,last_mouse_y,localx,localy,selectedColor);
                            last_mouse_x = localx;
                            last_mouse_y = localy;
                        }
                        
                        
                    }
                    else{
                        if (last_mouse_x == -1 || last_mouse_y == -1)
                        {
                            last_mouse_x = localx;
                            last_mouse_y = localy;
                        }
                        else{
                            drawThickLine(pixels,snapshot.w,snapshot.h,localx,localy,last_mouse_x,last_mouse_y,brushSize,selectedColor);
                            last_mouse_x = localx;
                            last_mouse_y = localy;
                        }
                        
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
        renderAllSliders(renderer,sliders,5,ELEMENTSCALE,mouse_x,mouse_y,mouse_down);
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
            else if (selectedTool == LINE && shapeClicked)
            {
                SDL_RenderDrawLine(renderer,shapePoint.x + viewport.x,shapePoint.y + viewport.y,mouse_x,mouse_y);
            }
            else if (selectedTool == SHAPE && shapeClicked)
            {
                renderPoly(renderer,shapePoint.x + viewport.x,shapePoint.y + viewport.y,mouse_x,mouse_y,polyVerts);
            }
            
            
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(15);
    }

    delete[] pixels;
    SDL_FreeSurface(imageSurface);
    SDL_DestroyTexture(canvas);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}