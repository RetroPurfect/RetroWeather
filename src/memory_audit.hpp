#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace memory_audit {
struct Point {unsigned elapsed=0,page=0,builds=0,gdi=0,user=0,modules=0;uint64_t workingKB=0,commitKB=0,trackedKB=0;bool available=false;};
struct History {std::array<Point,12> points;size_t count=0,next=0;void push(const Point&p){points[next]=p;next=(next+1)%points.size();if(count<points.size())++count;}const Point&at(size_t i)const{return points[(next+points.size()-count+i)%points.size()];}};
enum class Event{DiagnosticsBefore,DiagnosticsOpened,DiagnosticsClosed,QuickBefore,QuickReady,QuickWritten,QuickFailed,NoticeClosed};
inline const char* eventName(Event e){switch(e){case Event::DiagnosticsBefore:return "Before diagnostics";case Event::DiagnosticsOpened:return "Diagnostics opened (text populated)";case Event::DiagnosticsClosed:return "Diagnostics closed (modal return)";case Event::QuickBefore:return "Before Quick Export";case Event::QuickReady:return "Quick Export destination ready";case Event::QuickWritten:return "Quick Export file written";case Event::QuickFailed:return "Quick Export failed";case Event::NoticeClosed:return "Export notification closed";}return "Unknown";}
struct EventPoint{Point memory;Event event=Event::DiagnosticsBefore;unsigned sequence=0;};
struct Events{std::array<EventPoint,32> points;size_t count=0,next=0;unsigned sequence=0;void push(Event event,const Point&p){EventPoint item;item.memory=p;item.event=event;item.sequence=++sequence;points[next]=item;next=(next+1)%points.size();if(count<points.size())++count;}const EventPoint&at(size_t i)const{return points[(next+points.size()-count+i)%points.size()];}};
inline int stripHeight(int height){return height<128?height:128;}
}
