#include "../src/diagnostic_export.hpp"
#include <cassert>
#include <set>
int main(){
 using diagnostic_export::filename;
 assert(filename(2026,1,2,3,4,5,6,0)==L"RetroWeather_Diagnostics_20260102-030405-006.txt");
 assert(filename(2026,12,31,23,59,59,999,1)==L"RetroWeather_Diagnostics_20261231-235959-999_1.txt");
 assert(filename(2026,1,2,3,4,5,6,2,true)==L"RetroWeather_Benchmark_20260102-030405-006_2.txt");
 std::set<std::wstring> names;
 for(unsigned i=0;i<1000;++i)assert(names.insert(filename(2026,1,2,3,4,5,6,i)).second);
}
