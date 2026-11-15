#pragma once
#include <algorithm>
namespace rendering {
struct Size{int width,height;};
inline Size fit(int w,int h,int limit){double ratio=std::min(1.,limit/(double)std::max(1,std::max(w,h)));return {std::max(1,(int)(w*ratio)),std::max(1,(int)(h*ratio))};}
inline Size foreground(int vw,int vh,int fw,int fh,bool crt){return fit(crt?fw:vw,crt?fh:vh,4096);}
inline Size scenery(Size front,int level){return level?fit(front.width,front.height,level==1?960:1440):front;}
inline int interval(int level){return level==1?100:50;}
}
