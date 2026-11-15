#pragma once
#include <string>
#include <sstream>
#include <iomanip>
namespace diagnostic_export {
inline std::wstring filename(unsigned year,unsigned month,unsigned day,unsigned hour,unsigned minute,unsigned second,unsigned millisecond,unsigned collision,bool fullBenchmark=false){
 std::wostringstream s;s<<(fullBenchmark?L"RetroWeather_Benchmark_":L"RetroWeather_Diagnostics_")<<std::setfill(L'0')<<std::setw(4)<<year<<std::setw(2)<<month<<std::setw(2)<<day<<L"-"<<std::setw(2)<<hour<<std::setw(2)<<minute<<std::setw(2)<<second<<L"-"<<std::setw(3)<<millisecond;
 if(collision)s<<L"_"<<collision;
 return s.str()+L".txt";
}
}
