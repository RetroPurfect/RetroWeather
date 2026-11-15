#pragma once
namespace rendering {
struct Quality {
 int tier=1,ceiling=2,slow=0,fast=0,cooldown=0;double average=-1;
 void configure(bool legacy,unsigned,unsigned long long ramMB){ceiling=legacy?0:ramMB<1024?1:2;tier=legacy?0:1;slow=fast=cooldown=0;average=-1;}
 bool sample(double sceneMs){if(sceneMs<0)return false;average=average<0?sceneMs:average*.9+sceneMs*.1;if(cooldown){--cooldown;return false;}double down=tier==2?8.:18.,up=tier==0?5.:2.;int recovery=tier==0?90:300;
 if(average>down&&tier>0){++slow;fast=0;if(slow>=30){--tier;slow=fast=0;cooldown=90;average=-1;return true;}}
 else if(average<=up&&tier<ceiling){++fast;slow=0;if(fast>=recovery){++tier;slow=fast=0;cooldown=90;average=-1;return true;}}
 else slow=fast=0;return false;}
 int limit()const{const int limits[]={640,1280,1600};return limits[tier];}
 const char* name()const{const char* names[]={"Lightweight","Balanced","Detailed"};return names[tier];}
};
}
