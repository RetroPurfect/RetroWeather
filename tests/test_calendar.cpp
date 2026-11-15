#include "../src/calendar.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace calendar;
 assert(dayNumber(1970,1,1)==0);assert(weekday(1970,1,1)==4);
 for(int y=1970;y<=2051;++y)for(int m=1;m<=12;++m)for(int d=1;d<=monthDays(y,m);++d){Date x=fromEpoch(epoch(y,m,d,23,59,59));assert(x.year==y&&x.month==m&&x.day==d&&x.hour==23&&x.minute==59&&x.second==59);}
 assert(!leap(2100)&&leap(2000));assert(nth(2026,11,4,4)==26);assert(last(2026,5,1)==25);
 auto holidays=upcoming({2026,12,30,0,0,0});assert(holidays.front().name=="New Year's Day");assert(fromEpoch(holidays.front().day*86400).year==2027);
 holidays=upcoming({2027,12,30,0,0,0});bool observed=false;for(auto h:holidays)if(h.name=="New Year's Day (observed)"&&dateString(fromEpoch(h.day*86400))=="2027-12-31")observed=true;assert(observed);
 holidays=upcoming({2026,1,1,0,0,0});bool easter=false;for(auto h:holidays)if(h.name=="Easter Sunday"&&dateString(fromEpoch(h.day*86400))=="2026-04-05")easter=true;assert(easter);
 Cycle c;for(int i=0;i<11;++i)c.tick(12,0);assert(c.page==0);c.tick(12,0);assert(c.page==1);for(int i=0;i<60;++i)c.tick(12,0);assert(c.page==0);
 c.page=5;c.elapsed=0;c.tick(1,3);assert(c.page==6&&c.hold==3);c.tick(1,3);c.tick(1,3);assert(c.page==6);c.tick(1,3);assert(c.page==0);c.page=6;c.next();assert(c.page==0);
 std::cout<<"PASS: all date roundtrips 1970-2051, leap years, holiday rules, observed New Year across years, Easter 2026, cycle timings and pauses.\n";
}
