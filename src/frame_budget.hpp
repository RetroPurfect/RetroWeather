#pragma once
#include <algorithm>
#include <cstdint>
namespace rendering {
inline int budgetDelay(int cost,bool legacy){cost=std::max(0,cost);int period=legacy?200:67;return std::max(period-cost,std::min(10000,cost*9));}
struct FrameMeter{uint32_t started=0,frames=0;double fps=0;bool ready=false;
 void reset(){started=frames=0;fps=0;ready=false;}
 void frame(uint32_t now){if(!frames){started=now;frames=1;return;}++frames;uint32_t elapsed=now-started;if(elapsed>=2000){fps=(frames-1)*1000./elapsed;ready=true;started=now;frames=1;}}
};
}
