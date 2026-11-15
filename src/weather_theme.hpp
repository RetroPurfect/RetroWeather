#pragma once
#include <string>
namespace weather_theme {
enum Condition {Clear,Partly,Cloudy,Rain,Snow,Storm,Fog,Unknown};
inline Condition classify(int c){if(c==0)return Clear;if(c==1||c==2)return Partly;if(c==3)return Cloudy;if(c==45||c==48)return Fog;if((c>=51&&c<=57&&c%2==1)||c==56||(c>=61&&c<=67&&c%2==1)||c==66||(c>=80&&c<=82))return Rain;if(c==71||c==73||c==75||c==77||c==85||c==86)return Snow;if(c==95||c==96||c==99)return Storm;return Unknown;}
inline int minutes(const std::string&s){if(s.size()<16||s[10]!='T'||s[13]!=':'||s[11]<'0'||s[11]>'2'||s[12]<'0'||s[12]>'9'||s[14]<'0'||s[14]>'5'||s[15]<'0'||s[15]>'9')return -1;int h=(s[11]-'0')*10+s[12]-'0',m=(s[14]-'0')*10+s[15]-'0';return h<24?h*60+m:-1;}
inline bool daylight(int now,int rise,int set){return rise<=set?now>=rise&&now<set:now>=rise||now<set;}
inline const char* track(Condition c,bool day){switch(c){case Clear:return day?"01_Clear_Day.wav":"02_Clear_Night.wav";case Partly:case Cloudy:return "03_Cloudy.wav";case Rain:return "04_Rain.wav";case Snow:return "05_Snow.wav";case Storm:return "06_Thunderstorm.wav";case Fog:return "07_Fog.wav";default:return "";}}
inline const char* name(Condition c){static const char* n[]={"Clear","Partly cloudy","Cloudy","Rain","Snow","Thunderstorm","Fog","Waiting for weather"};return n[c];}
}
