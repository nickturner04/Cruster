#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>

typedef struct toolbar{
    Uint16 x;
    Uint16 y;
    Uint8 w;
    Uint8 h;
}toolbar;

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

void drawToolbar(SDL_Renderer* renderer, toolbar *bar, int scale, int m_x, int m_y){
    SDL_SetRenderDrawColor(renderer,110,104,89,255);
    SDL_Rect rect = {bar->x,bar->y,16 * bar->w * scale,16 * bar->h * scale};
    SDL_RenderFillRect(renderer,&rect);
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderDrawRect(renderer,&rect);
    for (size_t i = 0; i < 16; i++)
    {
        for (size_t j = 0; j < 2; j++)
        {
            SDL_SetRenderDrawColor(renderer,0,0,0,255);
            SDL_Rect box = {rect.x + 16 * i * scale,rect.y + 16 * j * scale,16 * scale,16 * scale};
            if (i == m_x && j == m_y)
            {
                SDL_SetRenderDrawColor(renderer,196,180,84,255);
            }
            
            SDL_RenderDrawRect(renderer,&box);
        }
        
    }

    //if (m_x == -1 || m_y || -1) return;
    
    //SDL_SetRenderDrawColor(renderer,196,180,84,255);
    //SDL_Rect box = {m_x * scale * 16, m_y * scale * 16, 16 * scale, 16 * scale};
    //SDL_RenderDrawRect(renderer, &box);
    

}
#ifdef __cplusplus
}
#endif