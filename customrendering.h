#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

typedef struct toolbarRect{
    Uint16 x;
    Uint16 y;
    Uint8 w;
    Uint8 h;
    Uint8 bw;
    Uint8 bh;
}toolbarRect;

typedef enum HighlightMode{
    HLMODE_SELECTED,
    HLMODE_UNSELECTED,
    HLMODE_TOGGLE_ON,
    HLMODE_TOGGLE_OFF,
    HLMODE_COLOR,
    HLMODE_UNCHANGING
}HighlightMode;

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

typedef struct Slider{
    SDL_Rect rect;
    int hide;
    int fillPercent;
    Uint32* link;
    Uint32 min;
    Uint32 max;
}Slider;

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

void drawSlider(SDL_Renderer * renderer, Slider * slider, int scale, int m_x, int m_down){

    if (m_down && m_x != -1)
    {
        float f = m_x / (float)slider->rect.w;
        slider->fillPercent = (int)(100.0f * f);

        Uint32 t = slider->min + (Uint32)((float)(slider->max - slider->min) * f);
        *slider->link = t;
        printf("Tolerance: %u",*slider->link);
    }
    
    
    SDL_SetRenderDrawColor(renderer,0,0,255,255);
    float progress = (float)slider->fillPercent / 100.0f;
    int fill = (int)(progress * (float)slider->rect.w);
    SDL_Rect progressRect = {slider->rect.x,slider->rect.y,fill,slider->rect.h};
    SDL_RenderFillRect(renderer,&progressRect);

    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderDrawRect(renderer,&slider->rect);
}

void renderAllSliders(SDL_Renderer * renderer, Slider* sliders, int numSliders, int scale, int m_x, int m_y, int m_down){
    for (size_t i = 0; i < numSliders; i++)
    {
        if (sliders[i].hide) continue;
        int pos_x = -1;
        if (m_y > sliders[i].rect.y && m_y < sliders[i].rect.y + sliders[i].rect.h && m_x <= sliders[i].rect.x + sliders[i].rect.w && m_x >= sliders[i].rect.x)
        {
            pos_x = m_x - sliders[i].rect.x;
        }
        drawSlider(renderer,&sliders[i],scale,pos_x,m_down);
    }
    
}

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
            
            if (i == m_x && j == m_y)
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

toolbarButton* renderAllToolbars(SDL_Renderer* renderer,toolbar* toolbars, int numToolbars, int scale, int m_x, int m_y, int m_down){

    if (m_down)
    {
        printf("\nClick: %i",m_down,"%s\n");
    }
    
    
    toolbarButton* selectedButton = NULL;
    for (size_t i = 0; i < numToolbars; i++)
    {
        if (toolbars[i].mode) continue;
        
        int boxpos_x = -1;
        int boxpos_y = -1;
        int m_down_local = m_down;
        if (m_y > toolbars[i].rect.y && m_y < toolbars[i].rect.y + toolbars[i].rect.h * toolbars[i].rect.bh * scale && m_x < toolbars[i].rect.x + toolbars[i].rect.w * toolbars[i].rect.bw * scale)
        {
            
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