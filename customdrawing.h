#ifdef __cplusplus
extern "C"
{
#endif
#include <SDL2/SDL.h>

SDL_Point GetTextureCoordinate(int vp_x, int vp_y, int vp_w, int vp_h, int t_w, int t_h){
    SDL_Point out = {0,0};
    out.x = (int)((float)t_w * ((float)vp_x / (float)vp_w));
    out.y = int((float)t_h * ((float)vp_y / (float)vp_h));
    return out;
}

void Bresenham(Uint32* pixels, int width, int height, int x1, int y1, int x2, int y2){
    double gradient = (double)(y1 - y2) / (double)(x1 - x2);
    double gradient_i =(double)(x1 - x2) / (double)(y1 - y2);

    if (gradient > 1 || gradient < -1)
    {
        int xx1 = x2;
        int xx2 = x2;
        int yy1 = y1;
        int yy2 = y2;

        for (size_t y = yy1; y < yy2; y++)
        {
            if (y1 <= y2)
            {
                /* code */
            }
            
        }
        
    }
    

}

#ifdef __cplusplus
}
#endif