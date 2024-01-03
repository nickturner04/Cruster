#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include<math.h>

typedef struct StringMessage{
    SDL_Rect rect;
    SDL_Color color;
    TTF_Font* font;
    SDL_Texture * texture;
}StringMessage;

StringMessage* createStringMessage(SDL_Renderer* renderer, char* message, SDL_Rect rect, SDL_Color color, TTF_Font* font){


    StringMessage stringMessage = {rect,color,font,NULL};
    SDL_Surface* surface = TTF_RenderText_Solid(font,message,color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer,surface);
    SDL_FreeSurface(surface);
    stringMessage.texture = texture;
    StringMessage* out = (StringMessage*)malloc(sizeof(StringMessage));
    *out = stringMessage;
    return out;
}

void updateStringMessage(SDL_Renderer* renderer, StringMessage* stringMessage,char* message){
    SDL_DestroyTexture(stringMessage->texture);
    SDL_Surface* surface = TTF_RenderText_Solid(stringMessage->font,message,stringMessage->color);
    stringMessage->texture = SDL_CreateTextureFromSurface(renderer,surface);
    SDL_FreeSurface(surface);
}

void renderStringMessage(SDL_Renderer* renderer, StringMessage* message){
    SDL_RenderCopy(renderer,message->texture,NULL,&message->rect);
}

void destroyStringMessage(StringMessage* message){
    SDL_DestroyTexture(message->texture);
    free(message);
}

typedef struct toolbarRect{
    Uint16 x;
    Uint16 y;
    Uint8 w;
    Uint8 h;
    Uint8 bw;
    Uint8 bh;
}toolbarRect;
//Different modes that affect how the button is drawn to the screen
typedef enum HighlightMode{
    HLMODE_SELECTED,
    HLMODE_UNSELECTED,
    HLMODE_TOGGLE_ON,
    HLMODE_TOGGLE_OFF,
    HLMODE_COLOR,
    HLMODE_UNCHANGING
}HighlightMode;
//The button struct containing a function pointer to it's specific method, as well as it's colour and texture
typedef struct toolbarButton{
    int (*Pressed)(void* p);
    SDL_Texture *Texture;
    int highlightmode;
    int r;
    int g;
    int b;
}toolbarButton;

toolbarButton createButton(int (*event)(void* p), SDL_Texture *texture, int highlight){
    toolbarButton out = {event,texture,highlight,110,104,89};
    return out;
}

typedef struct toolbar{
    toolbarRect rect;
    toolbarButton * buttons;
    int mode;
}toolbar;

toolbar TB_Create(toolbarRect rect, toolbarButton * buttons){
    toolbar newToolbar = {rect,buttons,0};
    return newToolbar;
}
//A slider that changes the value of a variable
typedef struct Slider{
    SDL_Rect rect;
    int type;
    int hide;
    int fillPercent;
    Uint32 min;
    Uint32 max;
    Uint32* link;
    Uint32 colour;
}Slider;
//A slider that calls a function whenever the value changes
typedef struct EventSlider{
    SDL_Rect rect;
    int type; //The first two fields of both sliders are the same so that in a union, the second field can be used to work out which kind of slider it is
    int hide;
    int val;
    int min;
    int max;
    void (*Event)(int x);
    Uint32 colour;
}EventSlider;

typedef union MultiSlider
{//Union so that both sliders can be stored in a single array
    EventSlider es;
    Slider s;
};


void renderCircle(SDL_Renderer* renderer, int cx, int cy, int radius){
    const int diameter = radius * 2;

    int x = radius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - diameter;
    while (x >= y)
    {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);

        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);

        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);

        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);


        if (error <= 0)
        {
  	        ++y;
  	        error += ty;
  	        ty += 2;
        }

        if (error > 0)
        {
  	        --x;
  	        tx += 2;
  	        error += (tx - diameter);
        }

    }
}

//Renders an outline of a polygon for drawing
void renderPoly(SDL_Renderer* renderer, int cx, int cy, int px, int py, int verts){
    double rx = px - cx, ry = py - cy;
    double angle = 2 * M_PI / verts;
    //uses vectors to do this

    for (size_t i = 0; i < verts; i++)
    {
        double b = angle * i;
        double vx = cx;
        double vy = cy;
        //formula for rotating vector
        vx += cos(b) * rx - sin(b) * ry;
        vy += sin(b) * rx + cos(b) * ry;
        SDL_RenderDrawLine(renderer,cx,cy,(int)vx,(int)vy);

    }
}

void drawCircleF(SDL_Renderer* renderer, int cx, int cy, int radius){
    const int diameter = radius * 2;

    int x = radius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - diameter;
    while (x >= y)
    {
        // Each of the following renders an octant of the circle

        SDL_RenderDrawLine(renderer,cx + x, cy - y,cx + x, cy + y);
        SDL_RenderDrawLine(renderer,cx - x, cy - y,cx - x, cy + y);
        SDL_RenderDrawLine(renderer,cx + y, cy - x,cx + y, cy + x);
        SDL_RenderDrawLine(renderer,cx - y, cy - x,cx - y, cy + x);

        if (error <= 0)
        {
  	        ++y;
  	        error += ty;
  	        ty += 2;
        }

        if (error > 0)
        {
  	        --x;
  	        tx += 2;
  	        error += (tx - diameter);
        }

    }
}

void drawEventSlider(SDL_Renderer * renderer, EventSlider * slider, int scale, int m_x, int m_down){

    if (m_down && m_x != -1)
    {
        float f = m_x / (float)slider->rect.w;

        slider->val = slider->min + (int)((float)(slider->max - slider->min) * f);
        slider->Event(slider->val);
    }
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    Uint8 r, g, b, a;
    SDL_GetRGBA(slider->colour,format,&r,&g,&b,&a);
    SDL_FreeFormat(format);
    SDL_SetRenderDrawColor(renderer,r,g,b,a);
    float progress = (float)slider->val / (float)slider->max;
    int fill = (int)(progress * (float)slider->rect.w);
    SDL_Rect progressRect = {slider->rect.x,slider->rect.y,fill,slider->rect.h};
    SDL_RenderFillRect(renderer,&progressRect);

    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderDrawRect(renderer,&slider->rect);
}

void drawSlider(SDL_Renderer * renderer, Slider * slider, int scale, int m_x, int m_down){

    if (m_down && m_x != -1)
    {
        float f = m_x / (float)slider->rect.w;
        slider->fillPercent = (int)(100.0f * f);

        Uint32 t = slider->min + (Uint32)((float)(slider->max - slider->min) * f);
        *slider->link = t;
    }
    
    
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    float progress = (float)slider->fillPercent / 100.0f;
    int fill = (int)(progress * (float)slider->rect.w);
    SDL_Rect progressRect = {slider->rect.x,slider->rect.y,fill,slider->rect.h};
    SDL_RenderFillRect(renderer,&progressRect);

    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderDrawRect(renderer,&slider->rect);
}
//renders all the sliders from an array, calling a different function depending on the type of the slider
void renderAllSliders(SDL_Renderer * renderer, MultiSlider* sliders, int numSliders, int scale, int m_x, int m_y, int m_down){
    for (size_t i = 0; i < numSliders; i++)
    {
        if (sliders[i].s.hide) continue;
        int pos_x = -1;
        if (m_y > sliders[i].s.rect.y && m_y < sliders[i].s.rect.y + sliders[i].s.rect.h && m_x <= sliders[i].s.rect.x + sliders[i].s.rect.w && m_x >= sliders[i].s.rect.x)
        {
            pos_x = m_x - sliders[i].s.rect.x;
        }
        if (sliders[i].s.type == 0)
        {
            drawSlider(renderer,&sliders[i].s,scale,pos_x,m_down);
        }
        else{
            drawEventSlider(renderer,&sliders[i].es,scale,pos_x,m_down);
        }
        
    }
    
}
//Draws a toolbar onto the screen, taking the mouse position into account to work out which button is hovered over
toolbarButton* drawToolbar(SDL_Renderer* renderer, toolbar *toolbarObject, int scale, int m_x, int m_y, int m_down){
    toolbarButton* selectedButton = NULL;

    toolbarRect bar = toolbarObject->rect;
    SDL_SetRenderDrawColor(renderer,110,104,89,255);
    SDL_Rect rect = {bar.x,bar.y,bar.bw * bar.w * scale, bar.bh * bar.h * scale};
    //SDL_RenderFillRect(renderer,&rect);
    //SDL_SetRenderDrawColor(renderer,0,0,0,255);
    //SDL_RenderDrawRect(renderer,&rect);
    for (size_t i = 0; i < bar.w; i++)
    {
        for (size_t j = 0; j < bar.h; j++)
        {
            if (m_down && (toolbarObject->buttons + i)->highlightmode == 0)(toolbarObject->buttons + i)->highlightmode = 1;
            
            SDL_Rect box = {rect.x + bar.bw * i * scale,rect.y + bar.bh * j * scale, bar.bw * scale, bar.bh * scale};
            
            toolbarButton button = toolbarObject->buttons[j * bar.w + i];
            if (button.highlightmode == 1)
            {
                SDL_SetTextureBlendMode(button.Texture,SDL_BLENDMODE_MUL);
            }
            SDL_SetRenderDrawColor(renderer,button.r,button.g,button.b,255);
            SDL_RenderFillRect(renderer,&box);
            SDL_RenderCopy(renderer, button.Texture, NULL, &box);
            SDL_SetTextureBlendMode(button.Texture,SDL_BLENDMODE_NONE);
            

            SDL_SetRenderDrawColor(renderer,0,0,0,255);
            
            if (i == m_x && j == m_y)//check if button is hovered
            {
                SDL_SetRenderDrawColor(renderer,196,180,84,255);
                selectedButton = toolbarObject->buttons + (j * bar.w + i);
                if (m_down){

                    
                    if (selectedButton->Pressed != nullptr)
                    {
                        selectedButton->Pressed(selectedButton);
                        if (selectedButton->highlightmode == 1)
                        {
                            selectedButton->highlightmode = 0;
                        }
                    }
                    else{
                        printf("\n WARNING: Button Event is NULL\n");
                    }
                        
                    
                }
            }
            
            SDL_RenderDrawRect(renderer,&box);
        }
        
    }

    return selectedButton;

    //if (m_x == -1 || m_y || -1) return;
    
    //SDL_SetRenderDrawColor(renderer,196,180,84,255);
    //SDL_Rect box = {m_x * scale * 16, m_y * scale * 16, 16 * scale, 16 * scale};
    //SDL_RenderDrawRect(renderer, &box);

}
//renders many toolbars onto the screen
toolbarButton* renderAllToolbars(SDL_Renderer* renderer,toolbar* toolbars, int numToolbars, int scale, int m_x, int m_y, int m_down){
    
    toolbarButton* selectedButton = NULL;
    for (size_t i = 0; i < numToolbars; i++)
    {
        if (toolbars[i].mode) continue;//skips if it is hidden
        
        int boxpos_x = -1;
        int boxpos_y = -1;
        int m_down_local = m_down;
        if (m_y > toolbars[i].rect.y && m_y < toolbars[i].rect.y + toolbars[i].rect.h * toolbars[i].rect.bh * scale && m_x < toolbars[i].rect.x + toolbars[i].rect.w * toolbars[i].rect.bw * scale)
        {
            //works out selected button
            int x = m_x - toolbars[i].rect.x;
            int y = m_y - toolbars[i].rect.y;
            boxpos_x = (int)(x ) / (toolbars[i].rect.bh * scale);
            boxpos_y = (int)(y ) / (toolbars[i].rect.bw * scale);
        }
        else{
            m_down_local = false;
        }
        selectedButton = drawToolbar(renderer,&toolbars[i],scale,boxpos_x,boxpos_y,m_down_local);
        
    }
    
    return selectedButton;
}
#ifdef __cplusplus
}
#endif