#pragma once

#include "XY.h"
#include <string>
#include <vector>
#include <memory>

class SVGWriter;

class SVGBoardMaker
{
public:
   SVGBoardMaker( std::string filename );
   ~SVGBoardMaker();

   void startNewSVG( std::string title, std::string text = std::string() );

   XYf cellCenter( XY pos ) const;
   void drawHex( XY pos, int val, int num=0 );
   void drawText( XY pos, std::string text, int val );
   //void drawCellText( XY pos, std::string text, int val );
   void drawNoForcedWinFound();
   void setDefenseOptions( std::vector<std::string> defenseOptions );

public:
   std::unique_ptr<SVGWriter> m_svgWriter;
   std::string m_filename;
};