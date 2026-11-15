#pragma once
#include "calendar.hpp"
#include <vector>
namespace page_decoration {
struct Point{double x,y;};
inline std::vector<Point> heart(){const Point curves[4][4]={{{.5,.25},{.72,-.05},{1.05,.11},{.92,.40}},{{.92,.40},{.83,.57},{.60,.83},{.5,.94}},{{.5,.94},{.40,.83},{.17,.57},{.08,.40}},{{.08,.40},{-.05,.11},{.28,-.05},{.5,.25}}};std::vector<Point> points;for(const auto&c:curves)for(int i=0;i<16;++i){double t=i/16.,u=1-t;points.push_back({u*u*u*c[0].x+3*u*u*t*c[1].x+3*u*t*t*c[2].x+t*t*t*c[3].x,u*u*u*c[0].y+3*u*u*t*c[1].y+3*u*t*t*c[2].y+t*t*t*c[3].y});}return points;}

enum Symbol{Star,Tree,Heart,Firework,Leaf};
inline Symbol symbol(const std::string&name){if(name.find("Christmas")!=std::string::npos)return Tree;if(name.find("Valentine")!=std::string::npos||name.find("Mother")!=std::string::npos||name.find("Father")!=std::string::npos)return Heart;if(name.find("New Year")!=std::string::npos||name.find("Independence")!=std::string::npos)return Firework;if(name.find("Thanksgiving")!=std::string::npos||name.find("Halloween")!=std::string::npos)return Leaf;return Star;}
inline const char* month(int m){static const char*n[]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};return m>=1&&m<=12?n[m-1]:"---";}
inline std::string monthYear(calendar::Date d){auto s=calendar::longDate(d);return s.substr(0,s.find(' '))+" "+std::to_string(d.year);}
inline int dayOfYear(calendar::Date d){return (int)(calendar::dayNumber(d.year,d.month,d.day)-calendar::dayNumber(d.year,1,1))+1;}
inline std::string countdown(int64_t days){return days==0?"TODAY":days==1?"TOMORROW":std::to_string(days)+" DAYS";}
}
