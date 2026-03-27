#include <string>
#include <Windows.h>
#include <fstream>
#include <iostream>

namespace std
{
   std::ofstream g_traceFile;

   class trace_streambuf : public basic_streambuf<char, char_traits<char>>
   {
   public:
      trace_streambuf() {}
      int_type overflow( int_type x )
      {
         buffer += (char)x;
         if ( x == 10 )
         {
            cout << buffer;
            ::OutputDebugStringA( buffer.c_str() );
            if ( g_traceFile.is_open() )
               g_traceFile << buffer << flush;
            buffer.clear();
         }
         return 0;
      }
   private:
      string buffer;
   };
   std::trace_streambuf trace_buf;
   std::basic_ostream<char, std::char_traits<char>> trace( &trace_buf );

   void setTraceFile( const std::string& filename )
   {
      g_traceFile.open( filename, std::ios::app );
      g_traceFile << "--------------------------------" << endl;
      g_traceFile.flush();
   }
}