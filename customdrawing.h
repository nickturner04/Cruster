#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>

//pixel with all white and transparent alpha, represents an empty pixel within a buffer
const Uint32 empty = 0x00FFFFFF;

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
//Source code from "Wikipedia" starts here, it originates from: https://en.wikipedia.org/wiki/Flood_fill and uses a stack to fill in adjacent pixels with the same colour
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
//Source code from "Wikipedia" ends here

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

//copies all non empty pixels from a buffer to an image
void compImage(Uint32* dst, Uint32* src, size_t length){
    for (size_t i = 0; i < length; i++)
    {
        if(src[i] != empty) dst[i] = src[i];
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

//Source from Eike Anderson starts here, it originates from Programming Principles Lecture 10 and draws a pixel line
void drawBresenham(Uint32* pixels, int width, int height, int x1, int y1, int x2, int y2, Uint32 colour){

    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int e = (dx>dy ? dx : -dy)/2,e2;
    int x = x1, y = y1;

    for (;;)
    {
        pixels[y * width + x] = colour;
        if (x == x2 && y == y2) return;
        e2 = e;
        if (e2 > -dx)
        {
            e -= dy;
            x += sx;
        }
        if (e2 < dy)
        {
            e += dx;
            y += sy;
        }
    }
}

void drawBresenhamSafe(Uint32* pixels, int width, int height, int x1, int y1, int x2, int y2, Uint32 colour){

    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int e = (dx>dy ? dx : -dy)/2,e2;
    int x = x1, y = y1;


    for (;;)
    {
        if (x < 0 || y < 0 || x > width || y > height) goto d;
        pixels[y * width + x] = colour;
        d:

        if (x == x2 && y == y2) return;
        e2 = e;
        if (e2 > -dx)
        {
            e -= dy;
            x += sx;
        }
        if (e2 < dy)
        {
            e += dx;
            y += sy;
        }
    }
}

//Source from Eike Anderson ends here

void drawPoly(Uint32* pixels, int width, int height, int cx, int cy, int px, int py, int verts, int fill, int fillpercent, Uint32 colour){
    const double deg2rad = M_PI / 180.0;
    Uint32* buffer = (Uint32*)malloc(sizeof(Uint32) * width * height);
    for (size_t i = 0; i < width * height; i++)
    {
        buffer[i] = empty;
    }
    

    double rx = px - cx, ry = py - cy;
    double angle = 360.0 / verts;
    double mult = (double)fillpercent / 100.0;
    
    int xpoints[verts];
    int ypoints[verts];
    int xpointsF[verts];
    int ypointsF[verts];
    for (size_t i = 0; i < verts; i++)
    {
        double b = angle * i * deg2rad;
        double vx = cx;
        double vy = cy;
        double vxF = cx;
        double vyF = cy;


        double ax = cos(b) * rx - sin(b) * ry;
        double ay = sin(b) * rx + cos(b) * ry;
        vx += ax;
        vy += ay;
        vxF += ax * mult;
        vyF += ay * mult;
        //drawBresenhamSafe(pixels,width,height,cx,cy,(int)vx,(int)vy,colour);
        xpoints[i] = int(vx);
        ypoints[i] = int(vy);

        xpointsF[i] = int(vxF);
        ypointsF[i] = int(vyF);
    }
    for (size_t i = 1; i < verts; i++)
    {
        drawBresenhamSafe(buffer,width,height,xpoints[i - 1],ypoints[i - 1],xpoints[i],ypoints[i],colour);
    }
    drawBresenhamSafe(buffer,width,height,xpoints[0],ypoints[0],xpoints[verts - 1],ypoints[verts - 1],colour);
    if (fill){
        contiguousFill(buffer,cx,cy,width,height,colour,1);
        if (mult)
        {
            contiguousFill(buffer,cx,cy,width,height,colour,1);
            for (size_t i = 1; i < verts; i++)
            {
                drawBresenhamSafe(buffer,width,height,xpointsF[i - 1],ypointsF[i - 1],xpointsF[i],ypointsF[i],empty);
            }
            drawBresenhamSafe(buffer,width,height,xpointsF[0],ypointsF[0],xpointsF[verts - 1],ypointsF[verts - 1],empty);
            contiguousFill(buffer,cx,cy,width,height,empty,1);
        }
        
    }
    
    compImage(pixels,buffer,width * height);
    free(buffer);
}

void drawThickLine(Uint32* pixels, int cw, int ch, int x1, int y1, int x2, int y2, int w, Uint32 colour){
    Uint32* buffer = (Uint32*)malloc(sizeof(Uint32) * cw * ch);
    for (size_t i = 0; i < cw * ch; i++)
    {
        buffer[i] = empty;
    }
    
    double halfw = (double)w / 2.0;
    w = (int)halfw;

    double dx = y2 - y1;
    double dy = x2 - x1;

    double mpx = x1 + dy / 2.0;
    double mpy = y1 + dx / 2.0;

    double mag = sqrt(dx * dx + dy * dy);
    dx /= mag; dx *= halfw;
    dy /= mag; dy *= halfw;
    
    drawBresenhamSafe(buffer,cw,ch,x1 - (int)dx,y1 - (int)dy,x2 - (int)dx,y2 - (int)dy,colour);
    drawBresenhamSafe(buffer,cw,ch,x1 + (int)dx,y1 + (int)dy,x2 + (int)dx,y2 + (int)dy,colour);
    //drawBresenhamSafe(buffer,cw,ch,x1 + (int)dx, y1 + (int)dy,x1 - (int)dx,y1 - (int)dy,colour);
    //drawBresenham(buffer,cw,ch,x2 + (int)dx,y2 + (int)dy,y1 - (int)dy,y2 - (int)dy,colour);

    drawPoly(buffer,cw,ch,x1,y1,x1 + w,y1 + w,36,1,0,colour);
    drawPoly(buffer,cw,ch,x2,y2,x2 + w,y2 + w,36,1,0,colour);

    if (halfw > 2.0)
    {
        contiguousFill(buffer,mpx,mpy,cw,ch,colour,1);
    }
    compImage(pixels,buffer,cw * ch);
    free(buffer);
}

#ifdef __cplusplus
}
#endif