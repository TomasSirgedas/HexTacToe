#include "SVGBoard.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

namespace
{
   const double PI = acos( 0. ) * 2;
}

class SVGWriter
{
public:
   SVGWriter( XY size ) : _Size( size )
   {
      ss << "<!DOCTYPE html><html><body style='background-color:#222222;'>" << endl << endl;
      ss << "<a href='index.html'><h2>Home</h2></a>" << endl << endl;
      std::string commonText = R"(
<svg style="display:none;">
<style>
   h1 { color: #66aacc; background-color:#ffffff22; font-family: Consolas; }
   h2 { color: #4488aa; font-family: Consolas; }
   .hexOutline { stroke: #25233D; stroke-width: 2; }
   .xBackground { fill: #5E2F1D; }
   .xForeground { fill: #E96302; }
   .oBackground { fill: #163B65; }
   .oForeground { fill: #0288E9; }
   ._Background { fill: #1F1729; }
   .hexText { dominant-baseline: middle; text-anchor: middle; font-weight: bold; font-family: Arial; }
</style>
<defs>
<g id='hex'><polygon class='hexOutline _Background' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/></g>
<g id='hexX_'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/></g>
<g id='hexO_'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/></g>
<g id='hexX'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="30">X</text></g>
<g id='hexO'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="30">O</text></g>    
<g id='hex1'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">1</text></g>
<g id='hex2'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">2</text></g>
<g id='hex3'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">3</text></g>
<g id='hex4'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">4</text></g>
<g id='hex5'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">5</text></g>
<g id='hex6'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">6</text></g>
<g id='hex7'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">7</text></g>
<g id='hex8'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">8</text></g>
<g id='hex9'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">9</text></g>
<g id='hex10'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">10</text></g>
<g id='hex11'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">11</text></g>
<g id='hex12'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">12</text></g>
<g id='hex13'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">13</text></g>
<g id='hex14'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">14</text></g>
<g id='hex15'><polygon class='hexOutline xBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText xForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">15</text></g>
<g id='hex16'><polygon class='hexOutline oBackground' points = '0,20 -17.3205,10 -17.3205,-10 -2.44929e-15,-20 17.3205,-10 17.3205,10'/><text class='hexText oForeground' x="0" y="2" font-size="NUMBER_FONTSIZE">16</text></g>
</defs>
</svg>
)"; 
      commonText = regex_replace( commonText, std::regex( "NUMBER_FONTSIZE" ), std::to_string( 20 ) );


      ss << commonText << endl;
   }
   void closeSVGIfNecessary()
   {
      if ( !_SVGSectionOpen )
         return;
      ss << "</svg>\n\n" << endl;
      _SVGSectionOpen = false;
   }
   void startNewSVG( XY size, string title, string txt )
   {
      closeSVGIfNecessary();

      ss << endl << "<h1 id='" << title << "'>" << title << "</h1>" << endl << endl;
      if ( !txt.empty() )
         ss << endl << "<h2>" << txt << "</h2>" << endl << endl;

      string text = R"(
<svg width='@@WIDTH@@' height='@@HEIGHT@@' viewBox='@@LEFT@@ @@TOP@@ @@WIDTH@@ @@HEIGHT@@' xmlns='http://www.w3.org/2000/svg'>
)";
      text = std::regex_replace( text, std::regex( "@@LEFT@@" ), std::to_string( _Size.x * -.5 ) );
      text = std::regex_replace( text, std::regex( "@@TOP@@" ), std::to_string( _Size.y * -.5 ) );
      text = std::regex_replace( text, std::regex( "@@WIDTH@@" ), std::to_string( _Size.x ) );
      text = std::regex_replace( text, std::regex( "@@HEIGHT@@" ), std::to_string( _Size.y ) );
      ss << text << endl;
      _SVGSectionOpen = true;
   }
   XYf map( const XYf& a ) const
   {
      if ( !m )
         return a;
      return m( a );
   }
   void drawLine( const XYf& a_, const XYf& b_, double width = 2, const std::string& color = "red", double opacity = 1 )
   {
      XYf a = map( a_ );
      XYf b = map( b_ );
      std::string text = R"(<line x1="AX" y1="AY" x2="BX" y2="BY" style="stroke:COLOR;stroke-width:STROKEWIDTH;stroke-opacity:OPACITY;stroke-linecap:round"/>)";
      text = std::regex_replace( text, std::regex( "AX" ), std::to_string( a.x ) );
      text = std::regex_replace( text, std::regex( "AY" ), std::to_string( a.y ) );
      text = std::regex_replace( text, std::regex( "BX" ), std::to_string( b.x ) );
      text = std::regex_replace( text, std::regex( "BY" ), std::to_string( b.y ) );
      text = std::regex_replace( text, std::regex( "COLOR" ), color );
      text = std::regex_replace( text, std::regex( "STROKEWIDTH" ), std::to_string( width ) );
      text = std::regex_replace( text, std::regex( "OPACITY" ), std::to_string( opacity ) );
      ss << text << endl;
   }
   void drawLine2( const XYf& a, const XYf& b, double width = 2, const std::string& color = "red", double opacity = 1 )
   {
      drawLine( a, b, width, color, opacity );
      XYf n = (b - a).rot90();
      drawLine( a + (b - a) * .9, a + (b - a) * .8 + n * .1, width, color, opacity );
      drawLine( a + (b - a) * .9, a + (b - a) * .8 - n * .1, width, color, opacity );
   }
   void drawRect( const XYf& pt0_, const XYf& pt1_, const std::string& fill, double opacity = 1 )
   {
      XYf pt0 = map( pt0_ );
      XYf pt1 = map( pt1_ );
      string text = R"(<rect width = "WIDTH" height = "HEIGHT" x = "X0" y = "Y0" fill = "FILL" opacity="OPACITY" />)";
      //string text = R"(<line x1="AX" y1="AY" x2="BX" y2="BY" style="stroke:COLOR;stroke-width:STROKEWIDTH;stroke-opacity:OPACITY" />)";
      text = std::regex_replace( text, std::regex( "X0" ), std::to_string( pt0.x ) );
      text = std::regex_replace( text, std::regex( "Y0" ), std::to_string( pt0.y ) );
      text = std::regex_replace( text, std::regex( "WIDTH" ), std::to_string( pt1.x - pt0.x ) );
      text = std::regex_replace( text, std::regex( "HEIGHT" ), std::to_string( pt1.y - pt0.y ) );
      text = std::regex_replace( text, std::regex( "FILL" ), fill );
      text = std::regex_replace( text, std::regex( "STROKEWIDTH" ), std::to_string( 0 ) );
      text = std::regex_replace( text, std::regex( "OPACITY" ), std::to_string( opacity ) );
      ss << text << endl;
   }
   void drawPoly( const std::vector<XYf>& v, double width = 2, const std::string& stroke = "black", const std::string& fill = "gray" )
   {
      // <polygon points = "100,100 150,25 150,75 200,0" fill = "none" stroke = "black" / >
      std::string text = R"(<polygon points = "POLY" fill = "FILL" stroke = "STROKE" stroke-width = "WIDTH"/>)";
      std::stringstream polyss;
      for ( XYf a : v )
      {
         a = map( a );
         polyss << a.x << "," << a.y << " ";
      }
      text = std::regex_replace( text, std::regex( "POLY" ), polyss.str() );
      text = std::regex_replace( text, std::regex( "FILL" ), fill );
      text = std::regex_replace( text, std::regex( "STROKE" ), stroke );
      text = std::regex_replace( text, std::regex( "WIDTH" ), std::to_string( width ) );
      ss << text << endl;
   }
   void drawCircle( const XYf& center_, double radius_, const std::string& fill = "transparent", const std::string& stroke = "black", double strokeWidth = 2 )
   {
      XYf center = map( center_ );
      double radius = map( center_ + XYf( radius_, 0 ) ).dist( center );
      std::string text = R"(<circle cx="CX" cy="CY" r="RADIUS" fill = "FILL" stroke = "STROKE" stroke-width = "WIDTH"/>)";
      text = std::regex_replace( text, std::regex( "CX" ), std::to_string( center.x ) );
      text = std::regex_replace( text, std::regex( "CY" ), std::to_string( center.y ) );
      text = std::regex_replace( text, std::regex( "RADIUS" ), std::to_string( radius ) );
      text = std::regex_replace( text, std::regex( "STROKE" ), stroke );
      text = std::regex_replace( text, std::regex( "FILL" ), fill );
      text = std::regex_replace( text, std::regex( "WIDTH" ), std::to_string( strokeWidth ) );
      ss << text << endl;
   }
   void drawText( const XYf& pos_, const std::string& myText, const std::string& weight = "thin", const std::string& color = "black" )
   {
      XYf pos = map( pos_ );
      pos.y += 2;
      //std::string text = R"(<text x="PX" y="PY" font-weight="thin" font-family:"Consolas">TEXT</text>)";
      std::string text = R"(<text x="PX" y="PY" dominant-baseline="middle" text-anchor="middle" font-weight="WEIGHT" font-size="30" font-family="Arial" fill="FILL">TEXT</text>)";
      text = std::regex_replace( text, std::regex( "PX" ), std::to_string( pos.x ) );
      text = std::regex_replace( text, std::regex( "PY" ), std::to_string( pos.y ) );
      text = std::regex_replace( text, std::regex( "TEXT" ), myText );
      text = std::regex_replace( text, std::regex( "WEIGHT" ), weight );
      text = std::regex_replace( text, std::regex( "FILL" ), color );
      ss << text << endl;
   }
   void drawHex( const XYf& pos_, const string& href )
   {
      XYf pos = map( pos_ );
      std::string text = R"(<use href='#HREF' x='PX' y='PY'/>)";
      text = std::regex_replace( text, std::regex( "HREF" ), href );
      text = std::regex_replace( text, std::regex( "PX" ), std::to_string( pos.x ) );
      text = std::regex_replace( text, std::regex( "PY" ), std::to_string( pos.y ) );
      ss << text << endl;
   }
   void drawHexText( const XYf& pos_, string myText, int val )
   {
      XYf pos = map( pos_ );
      std::string text = R"(<text class='hexTextCLS' x='PX' y='PY' font-size='FONTSIZE' fill='#aaaaaa'>TEXT</text>)";
      text = std::regex_replace( text, std::regex( "TEXT" ), myText );
      text = std::regex_replace( text, std::regex( "PX" ), std::to_string( pos.x ) );
      text = std::regex_replace( text, std::regex( "PY" ), std::to_string( pos.y + 2 ) );
      text = std::regex_replace( text, std::regex( "CLS" ), val == 1 ? " xForeground" : val == 0 ? " oForeground" : "" );
      text = std::regex_replace( text, std::regex( "FONTSIZE" ), to_string( val >= 0 ? 20 : 20 ) );

      ss << text << endl;
   }

   void setDefenseOptions( std::vector<std::string> defenseOptions )
   {
      m_defenseOptions = defenseOptions;
   }

   std::string summaryText()
   {
      stringstream ss;
      ss << "Summary: O can defend with <span style='color: #cccccc;'>";
      for ( auto s : m_defenseOptions )
         ss << " " << s;
      ss << (m_defenseOptions.empty() ? " nothing</span>! (X wins)" : "</span>");
      ss << ".";
      return ss.str();
   }

   std::string str()
   {
      closeSVGIfNecessary();

      std::string text = ss.str();
      text = std::regex_replace( text, std::regex( "@@SUMMARY@@" ), summaryText() );

      return text + "</body></html>\n";
   }


public:
   std::function<XYf( XYf )> m;
   //auto toSVG = [&]( const XYf& p ) { return p * svg._Size.y * .2 + XYf( svg._Size / 2 ); };
   std::stringstream ss;
   XY _Size;
   bool _SVGSectionOpen = false;
   std::vector<std::string> m_defenseOptions;
};



SVGBoardMaker::SVGBoardMaker( std::string filename )
   : m_filename( filename )
{
   m_svgWriter = std::make_unique<SVGWriter>( XY( 500, 400 ) );
   m_svgWriter->m = [&]( XYf p ) { return p * 20; };
   //drawCellBackground( XY( 0, 0 ), 0 );
   //drawCellBackground( XY( 1, 0 ), 0 );
   //drawCellBackground( XY( 0, 1 ), 0 );
   //drawCellBackground( XY( 2, 0 ), 1 );
   //drawCellBackground( XY( 3, 0 ), -1 );
   //drawCellText( XY( 1, 0 ), "X", 0 );
   //drawCellText( XY( 2, 0 ), "O", 1 );
}

SVGBoardMaker::~SVGBoardMaker()
{
   std::ofstream f( m_filename );
   f << m_svgWriter->str() << std::endl;
}

void SVGBoardMaker::startNewSVG( string title, string text )
{
   m_svgWriter->startNewSVG( m_svgWriter->_Size, title, text );
}

XYf SVGBoardMaker::cellCenter( XY pos ) const
{
   return  XYf( sqrt( 3 ), 0 ) * pos.x + XYf( -sqrt( 3 ) / 2, 1.5 ) * pos.y;
}

void SVGBoardMaker::drawHex( XY pos, int val, int num )
{
   string href;
   if ( num > 0 )
      href = "hex" + to_string( num );
   else
      href = vector<string>{ "hex", "hexO", "hexX" } [val + 1] ;

   m_svgWriter->drawHex( cellCenter( pos ), href );
}

void SVGBoardMaker::drawText( XY pos, string text, int val )
{
   if ( val == 0 || val == 1 ) 
      m_svgWriter->drawHex( cellCenter( pos ), val ? "hexX_" : "hexO_" );
   m_svgWriter->drawHexText( cellCenter( pos ), text, val );
}

void SVGBoardMaker::drawNoForcedWinFound()
{
   m_svgWriter->drawText( XYf( 0, -7 ), "X runs out of double-threats", "bold", "#FFFFFF80" );
}

void SVGBoardMaker::setDefenseOptions( std::vector<std::string> defenseOptions )
{
   m_svgWriter->setDefenseOptions( defenseOptions );
}
