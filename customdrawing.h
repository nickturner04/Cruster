#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>
#include <stdlib.h>

typedef struct PointStack{
    size_t length;
    int head;
    SDL_Point * points;
}PointStack;

PointStack * PS_Create(size_t length){
    PointStack* stack = (PointStack*)malloc(sizeof(PointStack));
    *stack = {length,-1,NULL};
    stack->points = (SDL_Point*)malloc(sizeof(SDL_Point) * length);
    return stack;
}

void PS_Destroy(PointStack * stack){
    free(stack->points);
    free(stack);
}

int PS_Pop(PointStack* stack, SDL_Point* point){
    if (stack->head < 0) return 0;
    
    *point = stack->points[stack->head];
    stack->head--;
    return 1;
}
int PS_Peek(PointStack* stack, SDL_Point* point){
    if (stack->head < 0) return 0;
    if (point == NULL) return 1;
    *point = stack->points[stack->head];
    return 2;
}

int PS_Push(PointStack* stack, int x, int y){
    if (stack->head + 1 == stack->length) return 0;
    SDL_Point point = {x,y};
    stack->head ++;
    stack->points[stack->head] = point;
    return 1;
}

inline void PS_DynamicPush(PointStack* stack, int x, int y){
    int result = PS_Push(stack,x,y);
    if (result) return;
    
    stack->length *= 2;
    stack->points = (SDL_Point*)realloc(stack->points,stack->length * sizeof(SDL_Point));
    PS_Push(stack,x,y);
}

SDL_Point GetTextureCoordinate(int vp_x, int vp_y, int vp_w, int vp_h, int t_w, int t_h){
    SDL_Point out = {0,0};
    out.x = (int)((float)t_w * ((float)vp_x / (float)vp_w));
    out.y = int((float)t_h * ((float)vp_y / (float)vp_h));
    return out;
}

void contiguousFill(Uint32* pixels, int x, int y, int cw, int ch, Uint32 colour, int tolerance){
    Uint32 selectedColor = pixels[y * cw + x];
    if (selectedColor == colour) return;
    
    PointStack* s = PS_Create(cw * ch);
    PS_Push(s,x,y);
    while (PS_Peek(s,NULL))
    {
        SDL_Point n;
        PS_Pop(s,&n);
        if (pixels[n.y * cw + n.x] == selectedColor && n.x > 0 && n.y > 0 && n.x < cw && n.y < ch)
        {
            pixels[n.y * cw + n.x] = colour;
            PS_DynamicPush(s, n.x - 1,n.y);
            PS_DynamicPush(s, n.x + 1,n.y);
            PS_DynamicPush(s,n.x,n.y + 1);
            PS_DynamicPush(s,n.x,n.y - 1);
        }    }
    PS_Destroy(s);
}

void globalFill(Uint32* pixels, int x, int y, int cw, int ch, Uint32 colour, int tolerance){
    Uint32 selectedColor = pixels[y * cw + x];
    for (size_t i = 0; i < cw; i++)
    {
        for (size_t j = 0; j < ch; j++)
        {
            int diff = (int)pixels[j * cw + i] - (int)selectedColor;
            if (diff < 0) diff *= -1;
            if (diff <= tolerance) pixels[j * cw + i] = colour;      
            
        }
        
    }
    
}

void drawSquare(Uint32* pixels, SDL_Rect rect,SDL_Rect canvas, Uint32 colour){
    int intitial_i = rect.x < 0 ? 0 : rect.x;
    int initial_j = rect.y < 0 ? 0 : rect.y;
    int initial_w = rect.x + rect.w;
    for (int i = intitial_i; i < rect.x + rect.w; i++)
    {
        for (int j = initial_j; j < rect.y + rect.h; j++)
        {
            if ((i < 0 || j < 0 || i > canvas.w || j > canvas.h))continue;
            pixels[j * canvas.w + i] = colour;
        }
        
    }
    
}

inline void putPixel (Uint32* pixels, int x, int y, SDL_Rect canvas, Uint32 colour){
    if (x < 0 || y < 0 || x > canvas.w || y > canvas.h) return;
    pixels[y * canvas.w + x] = colour;
}

inline void plotCirclePoints(Uint32* pixels, int xc, int yc, int x, int y, SDL_Rect canvas,Uint32 Colour){
    putPixel(pixels,xc+x, yc+y, canvas, Colour); 
    putPixel(pixels,xc-x, yc+y, canvas, Colour); 
    putPixel(pixels,xc+x, yc-y, canvas, Colour); 
    putPixel(pixels,xc-x, yc-y, canvas, Colour); 
    putPixel(pixels,xc+y, yc+x, canvas, Colour); 
    putPixel(pixels,xc-y, yc+x, canvas, Colour); 
    putPixel(pixels,xc+y, yc-x, canvas, Colour); 
    putPixel(pixels,xc-y, yc-x, canvas, Colour); 
}

void drawCircle(Uint32* pixels, int x, int y, int r, SDL_Rect canvas, Uint32 colour){
    int x1 = 0, y1 = r;
    int d = 3 - 2 * r;
    plotCirclePoints(pixels,x,y,x1,y1,canvas,colour);
}

void drawBresenham(Uint32* pixels, int width, int height, int x1, int y1, int x2, int y2, Uint32 colour){
    int dx = x2 - x1;
    int dy = y2 - y1;
    int e = 2 *(dy - dx);
    int x = x1, y = y1;
    pixels[y * width + x] = colour;

    while (x != x2 && y != y2)
    {
        if (e < 0)
        {
            x++;
            e = e + 2 * dy;
        }
        else
        {
            x++;
            y++;
            e = e + 2 * (dy - dx);
        }
        pixels[y * width + x] = colour;   
    }
}

#ifdef __cplusplus
}
#endif