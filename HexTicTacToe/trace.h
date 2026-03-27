#pragma once

#include <iostream>
#include <string>

namespace std
{
   extern std::basic_ostream<char, std::char_traits<char>> trace;
   void setTraceFile( const std::string& filename );
}