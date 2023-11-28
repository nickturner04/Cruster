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

#ifdef __cplusplus
}
#endif