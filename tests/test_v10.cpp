#include "../src/saver.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace saver;
assert(parse("",false).mode==Player);assert(parse("",true).mode==Configure);assert(parse(" /S ",true).mode==Run);assert(parse("-s",true).mode==Run);assert(parse("/c:1234",true).mode==Configure&&parse("/c:1234",true).parent==1234);assert(parse("/P 456",true).mode==Preview&&parse("/P 456",true).parent==456);assert(parse("\t/p:789",true).parent==789);
for(auto s:{"/p","/p 0","/p -1","/p ABC","/p 23xyz","/s 2","/surprise","/x","/p 999999999999999999999999999999999"})assert(parse(s,true).mode==Invalid);
assert(!moved(14,10,10,10));assert(moved(15,10,10,10));assert(moved(10,4,10,10));std::cout<<"PASS: screensaver/player mode routing, upper/lowercase and colon/space HWND arguments, missing/invalid/overflow HWND rejection, mouse jitter threshold.\n";
}
