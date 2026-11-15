#include "../src/presentation.hpp"
#include "../src/locations.hpp"
#include <cassert>
#include <iostream>
int main(){
 using namespace locations;
 assert(menuText("Town & Country")=="Town && Country");
 assert(extractZip(" 01234 ")=="01234");assert(extractZip("01234 - Example City, Michigan, United States")=="01234");
 for(auto s:{"1234","123456","12345foo","12345 6","12345-6789","abcde"})assert(extractZip(s).empty());
 assert(acceptPreview(4,4,"01234","01234 - Example City, Michigan, United States"));assert(!acceptPreview(3,4,"01234","01234"));assert(!acceptPreview(4,4,"01234","54321"));
 json provider={{"country","United States"},{"places",json::array({{{"place name","Example City"},{"state","Michigan"},{"state abbreviation","MI"},{"latitude","43.0"},{"longitude","-83.0"}}})}};
 json m=parse("01234",provider);assert(label(m)=="01234 - Example City, Michigan, United States");assert(coordinates(m));assert(fullState("MI")=="Michigan");
 json saved={{"01234",m},{"not-a-zip",m}};json cleaned=sanitize(json::parse(saved.dump()));assert(cleaned.size()==1&&cleaned["01234"]["country"]=="United States");
 provider["places"][0]["latitude"]="999";bool rejected=false;try{parse("01234",provider);}catch(...){rejected=true;}assert(rejected);
 json old={{"zip","01234"},{"city","Example City"},{"state","MI"}};json migrated=fromWeather(old);assert(valid(migrated)&&!coordinates(migrated));assert(migrated["state"]=="Michigan"&&migrated["country"]=="United States");
 json wrong=m;wrong["country"]=4;assert(!valid(wrong));
 int cases=0;
 for(auto size:std::vector<std::pair<int,int>>{{640,480},{800,600},{1280,1024},{1920,1080},{1920,1200},{3440,1440},{3840,2160},{1080,1920}})for(int mode=0;mode<6;++mode)for(int margin:{0,2,5,8}){
  auto v=presentation::aspectFit(size.first,size.second,mode,margin);auto f=presentation::frameFor(v);assert(v.x>=0&&v.y>=0&&v.x+v.width<=size.first&&v.y+v.height<=size.second);assert(f.width>=640&&f.height>=480);
  double sx=v.width/(double)f.width,sy=v.height/(double)f.height;assert(std::abs(sx/sy-1)<.004);
  int x,y;assert(!f.map(v,v.x-1,v.y,x,y));assert(f.map(v,v.x,v.y,x,y)&&x==0&&y==0);
  int buttonX=f.width-130,buttonY=f.height-25;int px=v.x+(int)((buttonX+.5)*v.width/f.width),py=v.y+(int)((buttonY+.5)*v.height/f.height);assert(f.map(v,px,py,x,y));
  auto col=presentation::forecastColumns(f.width);for(int i=0;i<4;++i)assert(col[i+1]-col[i]-10>40);
  assert(104+4*57+51+f.bodyOffset()<f.height-67);assert(104+f.bodyOffset()>=80);
  ++cases;
 }
 auto wide=presentation::aspectFit(1920,1080,2,0);assert(wide.width==1920&&wide.height==1080);auto classic=presentation::aspectFit(1920,1080,1,0);assert(classic.width==1440&&classic.x==240);
 auto autoView=presentation::aspectFit(1918,986,0,0);assert(autoView.width==1918&&autoView.height==986);
 std::cout<<"PASS: location parsing/labels/persistence/legacy migration, rejected coordinates, stale lookup replies; "<<cases<<" aspect/resolution/margin combinations, uniform scaling, non-overlapping field bounds and footer coordinate mapping.\n";
}
