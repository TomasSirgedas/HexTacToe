#pragma once

#include "XY.h"
#include <string>
#include <memory>

class SVGWriter;

class SVGBoardMaker
{
public:
   SVGBoardMaker( std::string filename );
   ~SVGBoardMaker();

   void startNewSVG( std::string title );

   XYf cellCenter( XY pos ) const;
   void drawHex( XY pos, int val, int num=0 );
   void drawText( XY pos, std::string text, int val );
   //void drawCellText( XY pos, std::string text, int val );

public:
   std::unique_ptr<SVGWriter> m_svgWriter;
   std::string m_filename;
};