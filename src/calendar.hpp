#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstdint>
namespace calendar {
struct Date {int year,month,day,hour,minute,second;};
inline bool leap(int y){return y%4==0&&(y%100!=0||y%400==0);}
inline int monthDays(int y,int m){static const int a[]={31,28,31,30,31,30,31,31,30,31,30,31};return a[m-1]+(m==2&&leap(y));}
inline int64_t dayNumber(int y,int m,int d){int64_t n=0;for(int i=1970;i<y;++i)n+=leap(i)?366:365;for(int i=y;i<1970;++i)n-=leap(i)?366:365;for(int i=1;i<m;++i)n+=monthDays(y,i);return n+d-1;}
inline int64_t epoch(int y,int m,int d,int h=0,int min=0,int sec=0){return dayNumber(y,m,d)*86400+h*3600+min*60+sec;}
inline Date fromEpoch(int64_t t){int64_t days=t/86400,rem=t%86400;if(rem<0){rem+=86400;--days;}Date r={1970,1,1,(int)(rem/3600),(int)(rem/60%60),(int)(rem%60)};while(days<0){--r.year;days+=leap(r.year)?366:365;}while(days>=(leap(r.year)?366:365)){days-=leap(r.year)?366:365;++r.year;}while(days>=monthDays(r.year,r.month)){days-=monthDays(r.year,r.month);++r.month;}r.day=(int)days+1;return r;}
inline int weekday(int y,int m,int d){int w=(int)((dayNumber(y,m,d)+4)%7);return w<0?w+7:w;}
inline int nth(int y,int m,int w,int n){return 1+(w-weekday(y,m,1)+7)%7+7*(n-1);}
inline int last(int y,int m,int w){int d=monthDays(y,m);return d-(weekday(y,m,d)-w+7)%7;}
inline std::string dateString(Date d){char s[40];std::sprintf(s,"%04d-%02d-%02d",d.year,d.month,d.day);return s;}
inline std::string longDate(Date d){static const char* names[]={"January","February","March","April","May","June","July","August","September","October","November","December"};return std::string(names[d.month-1])+" "+std::to_string(d.day)+", "+std::to_string(d.year);}
inline std::string weekdayName(Date d){static const char* names[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};return names[weekday(d.year,d.month,d.day)];}
struct Holiday{std::string name;int64_t day;};
inline void add(std::vector<Holiday>& v,int y,int m,int d,const char* name,bool federal=false){int64_t n=dayNumber(y,m,d);v.push_back({name,n});if(federal){int w=weekday(y,m,d);if(w==6)v.push_back({std::string(name)+" (observed)",n-1});if(w==0)v.push_back({std::string(name)+" (observed)",n+1});}}
inline std::vector<Holiday> upcoming(Date today){
 std::vector<Holiday> v;for(int y=today.year-1;y<=today.year+1;++y){
 add(v,y,1,1,"New Year's Day",true);add(v,y,1,nth(y,1,1,3),"Martin Luther King Jr. Day");
 add(v,y,2,14,"Valentine's Day");add(v,y,2,nth(y,2,1,3),"Washington's Birthday");
 int a=y%19,b=y/100,c=y%100,d=b/4,e=b%4,f=(b+8)/25,g=(b-f+1)/3,h=(19*a+b-d-g+15)%30,i=c/4,k=c%4,l=(32+2*e+2*i-h-k)%7,mm=(a+11*h+22*l)/451;
 int em=(h+l-7*mm+114)/31,ed=(h+l-7*mm+114)%31+1;add(v,y,em,ed,"Easter Sunday");
 add(v,y,5,nth(y,5,0,2),"Mother's Day");add(v,y,5,last(y,5,1),"Memorial Day");add(v,y,6,19,"Juneteenth",true);
 add(v,y,6,nth(y,6,0,3),"Father's Day");add(v,y,7,4,"Independence Day",true);add(v,y,9,nth(y,9,1,1),"Labor Day");
 add(v,y,10,nth(y,10,1,2),"Columbus Day");add(v,y,10,31,"Halloween");add(v,y,11,11,"Veterans Day",true);
 add(v,y,11,nth(y,11,4,4),"Thanksgiving");add(v,y,12,25,"Christmas Day",true);
 }int64_t n=dayNumber(today.year,today.month,today.day);v.erase(std::remove_if(v.begin(),v.end(),[n](const Holiday&h){return h.day<n;}),v.end());std::sort(v.begin(),v.end(),[](const Holiday&a,const Holiday&b){return a.day==b.day?a.name<b.name:a.day<b.day;});return v;
}
struct Cycle {
 int page=0,elapsed=0,hold=0;
 void advance(int pause){elapsed=0;if(page==5&&pause>0){page=6;hold=pause;}else page=(page>=5?0:page+1);}
 void tick(int duration,int pause){if(page==6){if(--hold<=0){page=0;elapsed=0;}}else if(++elapsed>=duration)advance(pause);}
 void next(){page=(page>=5?0:page+1);elapsed=0;hold=0;}
};
}
