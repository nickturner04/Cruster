#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>

typedef struct toolbarRect{
    Uint16 x;
    Uint16 y;
    Uint8 w;
    Uint8 h;
    Uint8 bw;
    Uint8 bh;
}toolbarRect;

typedef struct toolbarButton{
    int (*Pressed)();
    SDL_Texture *Texture;
}toolbarButton;

toolbarButton createButton(int (*event)(), SDL_Texture *texture){
    toolbarButton out = {event,texture};
    return out;
}

typedef struct toolbar{
    toolbarRect rect;
    toolbarButton * buttons;
}toolbar;

toolbar TB_Create(toolbarRect rect, toolbarButton * buttons){
    toolbar newToolbar;
    newToolbar.rect = rect;
    newToolbar.buttons = buttons;
    return newToolbar;
}

void drawCircle(SDL_Renderer* renderer, int cx, int cy, int radius){
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

toolbarButton* drawToolbar(SDL_Renderer* renderer, toolbar *toolbarObject, int scale, int m_x, int m_y){
    toolbarButton* selectedButton = NULL;

    toolbarRect bar = toolbarObject->rect;
    SDL_SetRenderDrawColor(renderer,110,104,89,255);
    SDL_Rect rect = {bar.x,bar.y,bar.bw * bar.w * scale, bar.bh * bar.h * scale};
    SDL_RenderFillRect(renderer,&rect);
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderDrawRect(renderer,&rect);
    for (size_t i = 0; i < bar.w; i++)
    {
        for (size_t j = 0; j < bar.h; j++)
        {
            SDL_Rect box = {rect.x + bar.bw * i * scale,rect.y + bar.bh * j * scale, bar.bw * scale, bar.bh * scale};
            if (i < 6)
            {
                SDL_RenderCopy(renderer, toolbarObject->buttons[i].Texture, NULL, &box);
            }

            SDL_SetRenderDrawColor(renderer,0,0,0,255);
            
            if (i == m_x && j == m_y)
            {
                SDL_SetRenderDrawColor(renderer,196,180,84,255);
                selectedButton = &toolbarObject->buttons[i * j];
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

toolbarButton* renderAllToolbars(SDL_Renderer* renderer,toolbar* toolbars, int numToolbars, int scale, int m_x, int m_y){
    toolbarButton* selectedButton = NULL;
    for (size_t i = 0; i < numToolbars; i++)
    {
        int boxpos_x = -1;
        int boxpos_y = -1;
        if (m_y > toolbars[i].rect.y && m_y < toolbars[i].rect.y + toolbars[i].rect.h * toolbars[i].rect.bh * scale && m_x < toolbars[i].rect.x + toolbars[i].rect.w * toolbars[i].rect.bw * scale)
        {
            int x = m_x - toolbars[i].rect.x;
            int y = m_y - toolbars[i].rect.y;
            boxpos_x = (int)(x ) / (toolbars[i].rect.bh * scale);
            boxpos_y = (int)(y ) / (toolbars[i].rect.bw * scale);
            printf("B_X: %i", x, "%s B_Y: %i", boxpos_y, "%s\n");
        }
        selectedButton = drawToolbar(renderer,&toolbars[i],scale,boxpos_x,boxpos_y);
    }
    return selectedButton;
}
#ifdef __cplusplus
}
#endif