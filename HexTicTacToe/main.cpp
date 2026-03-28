#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include <bit>
#include <random>
#include <optional>
#include <unordered_map>
#include <set>
#include <map>
#include "XY.h"
#include "trace.h"
#include "Timer.h"
#include "SVGBoard.h"

using namespace std;

//random_device g_dev;
//mt19937 g_rng( g_dev() ); // different rng each run
mt19937 g_rng( 12345 ); // same rng each run

namespace
{
   ostream& operator<<( ostream& os, const XY& p )
   {
      return os << "(" << p.x << "," << p.y << ")";
   }

   int64_t rnd( int64_t k ) { return uniform_int_distribution<int64_t>( 0, k - 1 )(g_rng); }
   uint64_t rnd_uint64() { return uniform_int_distribution<uint64_t>( 0, numeric_limits<uint64_t>::max() )(g_rng); }
   vector<XY> g_DIRS = { XY( -1, -1 ), XY( -1, 0 ), XY( 0, 1 ), XY( 1, 1 ), XY( 1, 0 ), XY( 0, -1 ) };
   int hexLen( XY p ) 
   {
      return max( max( abs( p.x ), abs( p.y ) ), abs( p.x - p.y ) );
   }
   XY hexRot000( XY p ) { return p; }
   XY hexRot120( XY p ) { return XY( -p.y, p.x - p.y ); }
   XY hexRot240( XY p ) { return XY( -p.x + p.y, -p.x ); }
   void forEachHex( int radius, const function<void( XY )>& func )
   {
      XY p;
      for ( p.y = -radius; p.y <= radius; p.y++ )
      {
         int x0 = -radius + max( p.y, 0 );
         int x1 = radius + min( p.y, 0 );
         for ( p.x = x0; p.x <= x1; p.x++ )
            func( p );
      }
   }
   string bitsStr( uint64_t x )
   {
      stringstream ss;
      for ( int i = 0; i < 64; i++ )
         ss << (((x >> i) & 1) ? '#' : '.');
      return ss.str();
   }
   void forEachSetBit( uint64_t m, const function<void(int)>& func )
   {
      while ( m )
      {
         int x = countr_zero( m );
         func( x );
         m &= ~(1ull << x);
      }
   }
   string hexGridString( int radius, function<string(XY)> func )
   {
      stringstream ss;
      XY p;
      for ( p.y = -radius; p.y <= radius; p.y++ )
      {
         int x0 = -radius + max( p.y, 0 );
         int x1 = radius + min( p.y, 0 );
         ss << string( abs( p.y ), ' ' );
         for ( p.x = x0; p.x <= x1; p.x++ )
            ss << func( p ) << ' ';
         ss << endl;
      }
      return ss.str();
   }
   int opponentID( int playerID ) { return 1 - playerID; }
   class ZobristTable
   {
   public:
      ZobristTable( XY size )
      {
         m.resize( size.y, vector<uint64_t>( size.x ) );
         for ( XY p : size )
            m[p.y][p.x] = rnd_uint64();
      }
      static XY size()
      {
         return XY( 64, 64 );
      }
      static uint64_t at( XY p )
      {
         static ZobristTable s_zobristTable( size() );
         return s_zobristTable.m[p.y][p.x];
      }
   public:
      vector<vector<uint64_t>> m;
   };
}

//class BitRow
//{
//public:
//   constexpr uint64_t maskAt( int pos ) const { return 1ull << pos; }
//   void setO( int pos ) { mO |= maskAt( pos ); }
//   void setX( int pos ) { mX |= maskAt( pos ); }
//   void removeO( int pos ) { mO &= ~maskAt( pos ); }
//   void removeX( int pos ) { mX &= ~maskAt( pos ); }
//   bool isO( int pos ) const { return (mO & maskAt( pos )) != 0; }
//   bool isX( int pos ) const { return (mX & maskAt( pos )) != 0; }
//   int at( int pos ) const { return isO( pos ) ? 0 : isX( pos ) ? 1 : -1; }
//
//   string str() const
//   {
//      static string chars = ".OX";
//      stringstream ss;
//      for ( int i = 0; i < 64; i++ )
//         ss << chars[at( i ) + 1];
//      return ss.str();
//   }
//
//public:
//   uint64_t mO = 0;
//   uint64_t mX = 0;
//};

class Row
{
public:
   Row( int rowSize )
   {
      m.resize( rowSize, -1 );
      m_countO6.resize( rowSize );
      m_countX6.resize( rowSize );
   }
   int at( int pos ) const { return m[pos]; }
   //void set( int pos, int val ) { set( pos, val, val ? m_countX6 : m_countO6, val ? m_threatsMaskX : m_threatsMaskO ); }
   //void set( int pos, int val, vector<uint8_t>& count6, uint64_t& threatsMask )
   //{
   //   assert( m[pos] == -1 );
   //   m[pos] = val;
   //   for ( int i = 0; i < 6; i++ )
   //   {
   //      ++count6[pos - i];
   //      if ( count6[pos - i] == 4 )
   //         threatsMask |= 1ull << (pos - i);
   //   }
   //}

   void addThreatX( int pos )
   {
      m_threatsMaskX |= 1ull << pos;
   }
   void removeThreatX( int pos )
   {
      m_threatsMaskX &= ~(1ull << pos);
   }
   void addThreatO( int pos )
   {
      m_threatsMaskO |= 1ull << pos;
   }
   void removeThreatO( int pos )
   {
      m_threatsMaskO &= ~(1ull << pos);
   }
   void setO( int pos )
   {
      assert( m[pos] == -1 );
      m[pos] = 0;
      m_maskO |= 1ull << pos;
      for ( int i = 0; i < 6; i++ )
      {
         ++m_countO6[pos - i];
         if ( m_countO6[pos - i] == 4 && m_countX6[pos - i] == 0 )
            addThreatO( pos - i );
         if ( m_countX6[pos - i] >= 4 && m_countO6[pos - i] == 1 )
            removeThreatX( pos - i );
      }
   }
   void setX( int pos )
   {
      assert( m[pos] == -1 );
      m[pos] = 1;
      m_maskX |= 1ull << pos;
      for ( int i = 0; i < 6; i++ )
      {
         ++m_countX6[pos - i];
         if ( m_countX6[pos - i] == 4 && m_countO6[pos - i] == 0 )
            addThreatX( pos - i );
         if ( m_countO6[pos - i] >= 4 && m_countX6[pos - i] == 1 )
            removeThreatO( pos - i );
      }
   }
   void unsetO( int pos )
   {
      assert( m[pos] == 0 );
      m[pos] = -1;
      m_maskO &= ~(1ull << pos);
      for ( int i = 0; i < 6; i++ )
      {
         if ( m_countO6[pos - i] == 4 && m_countX6[pos - i] == 0 )
            removeThreatO( pos - i );
         if ( m_countX6[pos - i] >= 4 && m_countO6[pos - i] == 1 )
            addThreatX( pos - i );
         --m_countO6[pos - i];
      }
   }
   void unsetX( int pos )
   {
      assert( m[pos] == 1 );
      m[pos] = -1;
      m_maskX &= ~(1ull << pos);
      for ( int i = 0; i < 6; i++ )
      {
         if ( m_countX6[pos - i] == 4 && m_countO6[pos - i] == 0 )
            removeThreatX( pos - i );
         if ( m_countO6[pos - i] >= 4 && m_countX6[pos - i] == 1 )
            addThreatO( pos - i );
         --m_countX6[pos - i];
      }
   }
   //void unset( int pos ) { int val = m[pos]; unset( pos, val, val ? m_countX6 : m_countO6, val ? m_threatsMaskX : m_threatsMaskO ); }
   //void unset( int pos, int val, vector<uint8_t>& m_count6, uint64_t& threatsMask )
   //{
   //   assert( val == 0 || val == 1 );
   //   m[pos] = val;
   //   for ( int i = 0; i < 6; i++ )
   //   {
   //      if ( m_count6[pos - i] == 4 )
   //         threatsMask &= ~(1ull << (pos - i));
   //      --m_count6[pos - i];
   //   }
   //}
   string str() const
   {
      static string chars = ".OX";
      stringstream ss;
      for ( int i = 0; i < 64; i++ )
         ss << chars[m[i] + 1];
      return ss.str();
   }
   int numThreatsO() const { return popcount( m_threatsMaskO ); }
   int numThreatsX() const { return popcount( m_threatsMaskX ); }

public:
   vector<int8_t> m; // actual state of cell {-1,0,1}
   vector<uint8_t> m_countO6; // [i] = #O's in { [i+0], [i+1], [i+2], [i+3] }
   vector<uint8_t> m_countX6; // [i] = #X's in { [i+0], [i+1], [i+2], [i+3] }
   uint64_t m_maskO = 0;
   uint64_t m_maskX = 0;
   uint64_t m_threatsMaskO = 0;
   uint64_t m_threatsMaskX = 0;
};

class Board
{
public:
   Board()
   {
      m_rows[0].resize( N * 2, Row( N * 2 ) );
      m_rows[1].resize( N * 2, Row( N * 2 ) );
      m_rows[2].resize( N * 2, Row( N * 2 ) );
      assert( m_rows[0].size() <= ZobristTable::size().y );
   }
   static void parseString( string s, const function<void( XY, char )>& func )
   {
      int r = (int)count( s.begin(), s.end(), '\n' ) / 2;

      Board ret;
      XY pos = XY( 0, 0 );
      for ( char c : s )
      {
         if ( c == '\n' ) { pos = XY( 0, pos.y + 1 ); continue; }
         if ( isalpha( c ) )
            func( XY( (pos.x + pos.y - (r * 3 + 1)) / 2, pos.y - r - 1 ), c );
         pos.x++;
      }
   }
   static vector<XY> parseLetterOptions( string s )
   {
      vector<XY> ret;
      Board::parseString( s, [&]( XY p, char c ) {
         if ( !islower( c ) )
            return;
         int index = c - 'a';
         if ( ret.size() < index + 1 )
            ret.resize( index + 1 );
         ret[index] = p;
      } );
      return ret;
   }
   static Board from( string s )
   {
      Board ret;
      Board::parseString( s, [&]( XY p, char c ) {
         if ( c == 'X' ) ret.setX( p );
         if ( c == 'O' ) ret.setO( p );
      } );
      return ret;
   }
   void set( XY p, int val )
   {
      int deltaThreatsO = 0;
      int deltaThreatsX = 0;
      for ( int r = 0; r < 3; r++, p = hexRot120( p ) )
      {
         Row& row = m_rows[r][p.y + N];
         deltaThreatsO -= row.numThreatsO();
         deltaThreatsX -= row.numThreatsX();
         if ( val == 1 ) row.setX( p.x + N ); else row.setO( p.x + N );
         deltaThreatsO += row.numThreatsO();
         deltaThreatsX += row.numThreatsX();
      }
      m_numThreatsO += deltaThreatsO;
      m_numThreatsX += deltaThreatsX;
      m_zobristHash ^= ZobristTable::at( p + XY( N, N ) ) * (val + 1);
   }
   void setX( XY p )
   {
      set( p, 1 );
   }
   void setO( XY p )
   {
      set( p, 0 );
   }
   void unset( XY p ) 
   {
      int val = m_rows[0][p.y + N].at( p.x + N );
      int deltaThreatsO = 0;
      int deltaThreatsX = 0;
      for ( int r = 0; r < 3; r++, p = hexRot120( p ) )
      {
         Row& row = m_rows[r][p.y + N];
         deltaThreatsO -= row.numThreatsO();
         deltaThreatsX -= row.numThreatsX();
         if ( val == 1 ) row.unsetX( p.x + N ); else row.unsetO( p.x + N );
         deltaThreatsO += row.numThreatsO();
         deltaThreatsX += row.numThreatsX();
      }
      m_numThreatsO += deltaThreatsO;
      m_numThreatsX += deltaThreatsX;
      m_zobristHash ^= ZobristTable::at( p + XY( N, N ) ) * (val + 1);
   }
   int8_t at( XY p ) const 
   { 
      return m_rows[0][p.y + N].at( p.x + N ); 
   }   
   string str() const
   {
      static string chars = ".OX";
      return hexGridString( 7, [&]( XY p ) { return string() + chars[at( p ) + 1]; } );
   }
   int numThreatsO() const { return m_numThreatsO; }
   int numThreatsX() const { return m_numThreatsX; }
   int numThreats( int playerID ) const { return playerID == 0 ? numThreatsO() : numThreatsX(); }
   uint64_t hash() const { return m_zobristHash; }
   vector<XY> allEmptyHexCells( int radius ) const
   {
      vector<XY> ret;
      forEachHex( radius, [&]( XY p ) {
         if ( at( p ) == -1 )
            ret.push_back( p );
      } );
      return ret;
   }

   void forEachOPlacement( int numCellsToPlace, int startIdx, const vector<XY>& candidateCells, const function<void()>& func )
   {
      if ( numCellsToPlace == 0 )
      {
         func();
         return;
      }
      for ( int i = startIdx; i < (int)candidateCells.size(); i++ ) if ( at( candidateCells[i] ) == -1 )
      {
         setO( candidateCells[i] );
         forEachOPlacement( numCellsToPlace - 1, i + 1, candidateCells, func );
         unset( candidateCells[i] );
      }
   }
   void forEachOPlacement( int numCellsToPlace, int radius, const function<void()>& func )
   {
      forEachOPlacement( numCellsToPlace, 0, allEmptyHexCells( radius ), func );
   }
   void forEachXPlacement( int numCellsToPlace, int startIdx, const vector<XY>& candidateCells, const function<void()>& func )
   {
      if ( numCellsToPlace == 0 )
      {
         func();
         return;
      }
      for ( int i = startIdx; i < (int)candidateCells.size(); i++ ) if ( at( candidateCells[i] ) == -1 )
      {
         setX( candidateCells[i] );
         forEachXPlacement( numCellsToPlace - 1, i + 1, candidateCells, func );
         unset( candidateCells[i] );
      }
   }
   void forEachXPlacement( int numCellsToPlace, int radius, const function<void()>& func )
   {
      forEachXPlacement( numCellsToPlace, 0, allEmptyHexCells( radius ), func );
   }
   // all moves that reduce threat (not necessarily eliminate threat)
   vector<XY> defenseCandidatesO( pair<XY, XY> lastMoveX ) const
   {
      uint64_t used[64] = { 0 };
      auto calcRowDefense = [this]( const Row& row )
      {
         uint64_t m = row.m_threatsMaskX;
         m |= m << 1;
         m |= m << 2;
         m |= m << 2; // smear 5 -- now it's a mask covering all threat lines
         m &= ~row.m_maskX; // remove occupied cells
         return m;
      };
      vector<XY> ret;
      auto append = [&]( const XY& p )
      {
         if ( used[p.y + N] & (1ull << (p.x + N)) )
            return; // already appended
         used[p.y + N] |= (1ull << (p.x + N));
         ret.push_back( p );
         //trace << "+" << p << endl;
      };

      auto f = [&]( XY p, int orientationIdx, function<XY( const XY& )> func )
      {
         forEachSetBit( calcRowDefense( m_rows[orientationIdx][p.y + N] ), [&]( int x ) { append( func( XY( x - N, p.y ) ) ); } );
      };

      f( hexRot000( lastMoveX.first ), 0, hexRot000 );
      f( hexRot120( lastMoveX.first ), 1, hexRot240 );
      f( hexRot240( lastMoveX.first ), 2, hexRot120 );
      f( hexRot000( lastMoveX.second ), 0, hexRot000 );
      f( hexRot120( lastMoveX.second ), 1, hexRot240 );
      f( hexRot240( lastMoveX.second ), 2, hexRot120 );      
      return ret;
   }

public:
   int N = 32;
   vector<Row> m_rows[3];
   int m_numThreatsO = 0;
   int m_numThreatsX = 0;
   uint64_t m_zobristHash = 0;
};


vector<XY> singleCellDefenses( Board& board, int playerID, const vector<XY>& candidates, vector<XY>& partialDefenseList )
{
   int origNumThreats = board.numThreats( opponentID( playerID ) );
   assert( origNumThreats > 0 );

   vector<XY> ret;
   for ( XY p : candidates ) if ( board.at( p ) == -1 )
   {
      board.set( p, playerID );
      int newNumThreats = board.numThreats( opponentID( playerID ) );
      if ( newNumThreats == 0 )
         ret.push_back( p );
      if ( newNumThreats < origNumThreats )
         partialDefenseList.push_back( p );
      board.unset( p );
   }
   return ret;
}

vector<pair<XY, XY>> doubleCellDefenses( Board& board, int playerID, const vector<XY>& candidates )
{
   assert( board.numThreats( opponentID( playerID ) ) > 0 );

   vector<pair<XY, XY>> ret;

   for ( int i1 = 0; i1 < (int)candidates.size(); i1++ )
   {
      board.set( candidates[i1], playerID );
      for ( int i0 = 0; i0 < i1; i0++ )
      {
         board.set( candidates[i0], playerID );

         if ( board.numThreats( opponentID( playerID ) ) == 0 )
            ret.push_back( { candidates[i0], candidates[i1] } );

         board.unset( candidates[i0] );
      }
      board.unset( candidates[i1] );
   }
   return ret;
}

void testNumThreats()
{
   Board board;
   board.setX( XY( 0, 0 ) );
   board.setX( XY( 1, 1 ) );
   board.setX( XY( 2, 2 ) );
   assert( board.numThreatsX() == 0 );
   board.setX( XY( 3, 3 ) );
   assert( board.numThreatsX() == 3 );
   board.setX( XY( -2, -2 ) );
   assert( board.numThreatsX() == 4 );
   board.setX( XY( 3, 2 ) );
   assert( board.numThreatsX() == 4 );
   board.setO( XY( -1, 1 ) );
   assert( board.numThreatsX() == 4 );
   board.setO( XY( -1, -1 ) );
   assert( board.numThreatsX() == 1 );
   board.setO( XY( 5, 5 ) );
   assert( board.numThreatsX() == 0 );
}
void testUndo()
{
   Board board;
   assert( board.hash() == 0 );
   board.setX( XY( 0, 0 ) );
   assert( board.hash() != 0 );
   uint64_t hash1 = board.hash();
   string str1 = board.str();
   board.setO( XY( 1, 1 ) );
   board.setX( XY( 2, 2 ) );
   board.unset( XY( 2, 2 ) );
   board.unset( XY( 1, 1 ) );
   assert( board.str() == str1 );
   assert( board.hash() == hash1 );
}

void testSingleCellDefense()
{

   string boardStr = R"(
      . . . . . . .
     . . . X . . . .
    . . . . . . . . .
   . . . X . . . . . .
  . . . X . . . . . . .
 X X X X O . X X X . X .
X X X X O . X X X . X . .
 . . . . . . . . . . . .
  . . . . . . . . . . .
   . . . . . . . . . .
    . . . . . . . . .
     . . . . . . . .
      . . . . . . .     )";

   Board board = Board::from( boardStr );
   //trace << board.m_rows[0][32].str() << endl;

   //uint64_t mx = board.m_rows[0][32].m_threatsMaskX;
   ////trace << mx << endl;
   //trace << bitsStr( mx ) << endl;
   //mx |= mx << 1;
   //mx |= mx << 2;
   //mx |= mx << 2;
   //trace << bitsStr( board.m_rows[0][32].m_maskX ) << endl;
   //trace << bitsStr( board.m_rows[0][32].m_maskO ) << endl;
   //trace << bitsStr( mx ) << endl;
   //mx &= ~board.m_rows[0][32].m_maskX;
   //trace << bitsStr( mx ) << endl;

   vector<XY> partialDefenses;
   vector<XY> defs = singleCellDefenses( board, 0, board.allEmptyHexCells( 8 ), partialDefenses );
   assert( defs.empty() );
   //for ( XY p : partialDefenses )
   //   trace << p << endl;

   vector<XY> defs2 = board.defenseCandidatesO( { XY( 0,-1 ), XY( -3, 0 ) } );

   assert( set<XY>( partialDefenses.begin(), partialDefenses.end() ) == set<XY>( defs2.begin(), defs2.end() ) );
}

void runTests()
{
   testNumThreats();
   testUndo();
   testSingleCellDefense();
}

class ForcedWinSearch
{
public:
   ForcedWinSearch( Board& board ) : m_board( board )
   {
      m_candidateCells = m_board.allEmptyHexCells( 7 );
   }
   bool canODefend( int depth, optional<XY> lastXMove0 = nullopt, optional<XY> lastXMove1 = nullopt )
   {
      if ( m_board.m_numThreatsO > 0 )
         return true; // O actually wins
      if ( m_board.numThreatsX() <= 1 )
         return true; // we already know O only needs one cell to defend

      uint64_t boardHash = m_board.hash();
      auto cacheIt = s_canODefendCache[depth].find( boardHash );
      if ( cacheIt != s_canODefendCache[depth].end() )
         return s_canODefendCache[depth].at( boardHash );

      static int64_t s_nodeCt = 0;
      if ( ++s_nodeCt % 1'000'000 == 0 )
         cout << "nodect = " << s_nodeCt << endl;
      
      vector<XY> defenseCandidates = lastXMove0.has_value() ? m_board.defenseCandidatesO( { *lastXMove0, *lastXMove1 } ) : m_candidateCells;

      vector<XY> partialDefenseList;
      vector<XY> singleCellDefs = singleCellDefenses( m_board, 0, defenseCandidates, partialDefenseList );

      if ( !singleCellDefs.empty() )
         return true; // only one cell is needed for O to defend
            
      //trace << m_board.str() << m_board.numThreatsX() << endl << endl;

      bool ret = false;

      auto doubleCellDefs = doubleCellDefenses( m_board, 0, partialDefenseList );
      for ( const auto& [d0, d1] : doubleCellDefs )
      {
         m_board.setO( d0 );
         m_board.setO( d1 );
         //trace << m_board.str() << endl;
         if ( !isWinForX( depth - 1 ) )
            ret = true;
         m_board.unset( d1 );
         m_board.unset( d0 );

         if ( ret )
            break;
      }
      return s_canODefendCache[depth][boardHash] = ret;
   }
   bool isWinForX( int depth, XY* x0 = nullptr, XY* x1 = nullptr )
   {
      if ( m_board.numThreatsX() > 0 )
         return true;
      if ( depth == 0 )
         return false;

      uint64_t boardHash = m_board.hash();
      auto cacheIt = s_canXWinCache[depth].find(boardHash);
      if ( cacheIt != s_canXWinCache[depth].end() && x0 == nullptr )
         return s_canXWinCache[depth].at( boardHash );

      bool ret = false;
      for ( int i1 = 0; !ret && i1 < (int)m_candidateCells.size(); i1++ ) if ( m_board.at( m_candidateCells[i1] ) == -1 )
      {
         m_board.setX( m_candidateCells[i1] );
         for ( int i0 = 0; !ret && i0 < i1; i0++ ) if ( m_board.at( m_candidateCells[i0] ) == -1 )
         {
            m_board.setX( m_candidateCells[i0] );

            ret = !canODefend( depth, m_candidateCells[i0], m_candidateCells[i1] );
            if ( ret && x0 ) *x0 = m_candidateCells[i0];
            if ( ret && x1 ) *x1 = m_candidateCells[i1];

            m_board.unset( m_candidateCells[i0] );
         }
         m_board.unset( m_candidateCells[i1] );
      }

      return s_canXWinCache[depth][boardHash] = ret;
   }

public:
   Board& m_board;
   vector<XY> m_candidateCells;
   static unordered_map<uint64_t, bool> s_canODefendCache[100];
   static unordered_map<uint64_t, bool> s_canXWinCache[100];
};
unordered_map<uint64_t, bool> ForcedWinSearch::s_canODefendCache[100];
unordered_map<uint64_t, bool> ForcedWinSearch::s_canXWinCache[100];

void mainAnalyzeBonePlus4()
{
   Board board;
   board.setX( XY( 0, 0 ) );
   board.setX( XY( 1, 0 ) );
   board.setX( XY( 1, 1 ) );
   board.setX( XY( -1, 0 ) );
   board.setX( XY( -1, -1 ) );

   trace << board.str() << endl;
   trace << "----" << endl;


   board.forEachOPlacement( 4/*#Os*/, 3/*radius*/, [&]() {
      cout << '.';
      bool canXWin = false;
      for ( int depth = 1; depth <= 10; depth++ )
      {
         //trace << "depth = " << depth << "..." << endl;  
         ForcedWinSearch forcedWinSearch( board );
         canXWin = forcedWinSearch.isWinForX( depth );
         if ( canXWin )
            break;
      }
      if ( !canXWin )
      {
         trace << "Possible DEFENSE:" << endl;
         trace << board.str() << endl;
      }
   } );
}

bool applyExampleDefenseForO( Board& board )
{
   vector<XY> defenseCandidates = board.allEmptyHexCells( 10 );
   vector<XY> partialDefenseList;
   vector<XY> singleCellDefs = singleCellDefenses( board, 0, defenseCandidates, partialDefenseList );
   auto doubleCellDefs = doubleCellDefenses( board, 0, partialDefenseList );
   if ( doubleCellDefs.empty() )
      return false;
   auto scoreFunc = []( pair<XY, XY> a ) { return hexLen( a.first - a.second ); };
   pair<XY, XY> theMove = *min_element( doubleCellDefs.begin(), doubleCellDefs.end(), [&]( pair<XY,XY> a, pair<XY,XY> b ) {
      return scoreFunc( a ) < scoreFunc( b );
   } );
   board.setO( theMove.first );
   board.setO( theMove.second );
   return true;
}

bool analyze_Os_turn( Board board, bool printOutput = true )
{
   if ( printOutput )
   {
      trace << board.str() << endl;
      trace << "----" << endl;
   }

   bool xWinsInAllCases = true;

   board.forEachOPlacement( 2/*#Os*/, 4/*radius*/, [&]() {
      if ( !xWinsInAllCases )
         return;

      if ( printOutput )
      {
         trace << board.str() << endl;
         //cout << '.';
      }
      bool canXWin = false;
      for ( int depth = 1; depth <= 10; depth++ )
      {
         if ( printOutput )
            trace << "depth = " << depth << "..." << endl;   
         ForcedWinSearch forcedWinSearch( board );
         canXWin = forcedWinSearch.isWinForX( depth );
         if ( canXWin )
            break;
      }
      if ( !canXWin )
      {
         xWinsInAllCases = false;
         if ( printOutput )
         {
            trace << "Possible DEFENSE:" << endl;
            trace << board.str() << endl;
         }
      }
      else
      {
         if ( printOutput )
            trace << "..X wins here" << endl;
      }
   } );

   if ( printOutput && xWinsInAllCases )
      trace << "X wins in all cases" << endl;

   return xWinsInAllCases;
}

bool analyze_Xs_turn( Board board, bool printOutput = true, Board* boardWithWinningMove = nullptr )
{
   if ( printOutput )
   {
      trace << board.str() << endl;
      trace << "----" << endl;
   }


   XY x0, x1;
   bool isWinForX = false;
   for ( int depth = 1; !isWinForX && depth <= 10; depth++ )
   {
      if ( printOutput )
         trace << "depth = " << depth << "..." << endl;
      ForcedWinSearch forcedWinSearch( board );
      isWinForX = forcedWinSearch.isWinForX( depth, &x0, &x1 );
      if ( printOutput )
         trace << "..." << (isWinForX ? "WIN FOR X!" : "nothing forced") << endl;
   }
   if ( isWinForX )
   {
      board.setX( x0 );
      board.setX( x1 );
      if ( printOutput )
         trace << board.str() << endl;
      if ( boardWithWinningMove )
         *boardWithWinningMove = board;
      board.unset( x1 );
      board.unset( x0 );
   }
   else
   {
      if ( printOutput )
         trace << "DEFENSE FOR O!" << endl;
   }

   return isWinForX;
}

void mainAnalyze()
{
   string boardStr = R"(
      . . . . . . .
     . . . . . . . .
    . . . . . . . . .
   . . . . . . . . . .
  . . . . . . . . . . .
 . . . . . . X X . . . .
. . . . . O X X . . . . .
 . . . . . . . . . . . .
  . . . . . . . . . . .
   . . . . . . . . . .
    . . . . . . . . .
     . . . . . . . .
      . . . . . . .     )";

   Board board = Board::from( boardStr );

   trace << board.str() << endl;
   trace << "----" << endl;


   XY x0, x1;
   for ( int depth = 1; ; depth++ )
   {
      trace << "depth = " << depth << "..." << endl;
      ForcedWinSearch forcedWinSearch( board );
      bool isWin = forcedWinSearch.isWinForX( depth, &x0, &x1 );
      trace << "..." << (isWin ? "WIN!" : "nothing forced") << endl;
      if ( isWin )
         break;
   }

   trace << x0.x << "," << x0.y << endl;
   trace << x1.x << "," << x1.y << endl;
   board.setX( x0 );
   board.setX( x1 );
   trace << board.str() << endl;
}

void basicAnalyze()
{
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . . . . . .
//  . . . . O O . . . . .
// . . . . . X X O . . . .
//. . . . . . X X . . . . .
// . . . . . X . . . . . .
//  . . . . O . . . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";

//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . . . . . .
//  . . . . a O b . . . .
// . . . . O X X c . . . .
//. . . . . d X X e . . . .
// . . . . . f X . . . . .
//  . . . . . g h . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";
   string boardStr = R"(
      . . . . . . .
     . . . . . . . .
    . . . . . . . . .
   . . . . . . . . . .
  . . . . O X . . . . .
 . . . . . X X O . . . .
. . . . . O X . . . . . .
 . . . . . . . . . . . .
  . . . . . . . . . . .
   . . . . . . . . . .
    . . . . . . . . .
     . . . . . . . .
      . . . . . . .     )";
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . O . . O . . .
//  . . . . X . X . . . .
// . . . O . X X . . . . .
//. . . . . . X . . . . . .
// . . . . O X X . . . . .
//  . . . . O . O . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";

   vector<XY> letterOptions = Board::parseLetterOptions( boardStr );

   analyze_Xs_turn( Board::from( boardStr ), true );
   //analyze_Os_turn( Board::from( boardStr ), true );
}

// return `true` iff X wins
bool drawStepstoSVG( SVGBoardMaker& svg, Board& board, vector<XY> cellsToDraw )
{
   Board futureBoard;
   bool xWins = analyze_Xs_turn( board, true, &futureBoard );

   ////optionally check if non-double-threat moves leads to winning position
   //if ( !xWins ) 
   //{      
   //   board.forEachXPlacement( 2 /*#Os*/, 3/*radius*/, [&]() {
   //      if ( xWins )
   //         return;

   //      trace << board.str() << endl;
   //      xWins = analyze_Os_turn( board, false /*no printing*/ );
   //      //isWinForX = analyze_Os_turn( board );
   //      trace << "..." << (xWins ? "X wins" : "O defends") << endl << endl;
   //   } );

   //   if ( !xWins )
   //   {
   //      trace << board.str() << endl;
   //      trace << "...O defended ALL X moves" << endl;
   //   }
   //}


   trace << "-- STEPS --" << endl;

   vector<Board> steps;

   if ( !xWins )
   {
      for ( XY p : cellsToDraw )
      {
         svg.drawHex( p, board.at( p ) );
      }
      svg.drawNoForcedWinFound();
      return false;
   }

   steps.push_back( board );
   steps.push_back( futureBoard );
   while ( true )
   {
      //trace << futureBoard.str() << endl;
      if ( !applyExampleDefenseForO( futureBoard ) )
         break;
      steps.push_back( futureBoard );
      Board nextBoard;
      xWins = analyze_Xs_turn( futureBoard, false, &nextBoard );
      steps.push_back( nextBoard );
      futureBoard = nextBoard;
   }

   struct CellStatus
   {
      //int val = -1;
      int num = -1;
   };
   map<XY, CellStatus> cellStatuses;

   for ( int i = 1; i < (int)steps.size(); i++ )
   {
      for ( XY p : cellsToDraw )
      {
         if ( steps[i - 1].at( p ) == -1 && steps[i].at( p ) != -1 )
            cellStatuses[p].num = i;
         //cellStatuses[p].val = steps[i].at( p );
      }
   }

   for ( XY p : cellsToDraw )
   {
      svg.drawHex( p, futureBoard.at( p ), cellStatuses[p].num );
      //if ( cellStatuses[p].val >= 0 )
      //{
      //   //string text = cellStatuses[p].num > 0 ? to_string( cellStatuses[p].num ) : cellStatuses[p].val == 0 ? "O" : "X";
      //   svg.drawHex( p, cellStatuses[p].val, cellStatuses[p].num );
      //}
   }

   return true;
}


void drawSteps()
{
   //   string boardStr = R"(
   //      . . . . . . .
   //     . . . . . . . .
   //    . . . . . . . . .
   //   . . . . . . . . . .
   //  . . . . O O . . . . .
   // . . . . . X X O . . . .
   //. . . . . . X X . . . . .
   // . . . . . X . . . . . .
   //  . . . . O . . . . . .
   //   . . . . . . . . . .
   //    . . . . . . . . .
   //     . . . . . . . .
   //      . . . . . . .     )";

   //   string boardStr = R"(
   //      . . . . . . .
   //     . . . . . . . .
   //    . . . . . . . . .
   //   . . . . . . . . . .
   //  . . . . a O b . . . .
   // . . . . O X X c . . . .
   //. . . . . d X X e . . . .
   // . . . . . f X . . . . .
   //  . . . . . g h . . . .
   //   . . . . . . . . . .
   //    . . . . . . . . .
   //     . . . . . . . .
   //      . . . . . . .     )";
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . a b . . .
//  . . . . . . X . . . .
// . . . . . . X X . . . .
//. . . . . O X X . . . . .
// . . . . . . . O . . . .
//  . . . . . . . . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";

   string name = "triangle_standard";
   string boardStr = R"(
      . . . . . . .
     . . . . . . . .
    . . . . . . . . .
   . . . . . a b . . .
  . . . . . i X c . . .
 . . . . . d X X e . . .
. . . . . O X X f . . . .
 . . . . . g h O . . . .
  . . . . . . . . . . .
   . . . . . . . . . .
    . . . . . . . . .
     . . . . . . . .
      . . . . . . .     )";

   vector<XY> letterOptions = Board::parseLetterOptions( boardStr );
   if ( letterOptions.empty() )
   {
      Board board = Board::from( boardStr );
      char nextLetter = 'a';
      forEachHex( 7, [&]( XY p ) 
      { 
         if ( board.at( p ) != -1 )
            return;
         bool nextToX = false;
         for ( XY d : g_DIRS )
            if ( board.at( p + d ) == 1 )
               nextToX = true;
         if ( nextToX )
         {
            letterOptions.push_back( p );
            nextLetter++;
         }
      } );

   }

   auto forEachOMove2 = [&]( const function<void( string, Board& )>& func )
   {
      Board board = Board::from( boardStr );
      for ( int i0 = 0; i0 < (int)letterOptions.size(); i0++ )
         for ( int i1 = i0 + 1; i1 < (int)letterOptions.size(); i1++ )
         {
            board.setO( letterOptions[i0] );
            board.setO( letterOptions[i1] );
            func( string() + char( 'a' + i0 + 0 ) + char( 'a' + i1 + 0 ), board );
            board.unset( letterOptions[i1] );
            board.unset( letterOptions[i0] );
         }
   };
   auto forEachOMove3 = [&]( const function<void( string, Board& )>& func )
   {
      Board board = Board::from( boardStr );
      for ( int i0 = 0; i0 < (int)letterOptions.size(); i0++ )
         for ( int i1 = i0 + 1; i1 < (int)letterOptions.size(); i1++ )
            for ( int i2 = i1 + 1; i2 < (int)letterOptions.size(); i2++ )
            {
               board.setO( letterOptions[i0] );
               board.setO( letterOptions[i1] );
               board.setO( letterOptions[i2] );
               func( string() + char( 'a' + i0 + 0 ) + char( 'a' + i1 + 0 ) + char( 'a' + i2 + 0 ), board );
               board.unset( letterOptions[i2] );
               board.unset( letterOptions[i1] );
               board.unset( letterOptions[i0] );
            }
   };

   vector<XY> cellsToDraw;
   forEachHex( 7, [&]( XY p ) { cellsToDraw.push_back( p ); } );

   SVGBoardMaker svg( name + ".html" );

   // draw template first
   {
      svg.startNewSVG( name, "@@SUMMARY@@" );
      Board board = Board::from( boardStr );
      for ( XY p : cellsToDraw )
         svg.drawHex( p, board.at( p ) );

      char ch = 'a';
      for ( XY p : letterOptions )
         svg.drawText( p, string() + ch++, -1 );
   }

   vector<string> defenseOptions;

   forEachOMove2( [&]( string letters, Board& board ) {
      svg.startNewSVG( letters );

      bool xWins = drawStepstoSVG( svg, board, cellsToDraw );
      if ( !xWins )
         defenseOptions.push_back( letters );

      //for ( XY p : cellsToDraw )
      //{
      //   svg.drawHex( p, board.at( p ) );
      //};

      for ( char letter : letters )
      {
         svg.drawHex( letterOptions[letter - 'a'], 0, -1 );
         svg.drawText( letterOptions[letter - 'a'], string() + letter, 0 );
      }
   } );   

   svg.setDefenseOptions( defenseOptions );
}

void drawSVG()
{
   SVGBoardMaker svg( "out.html" );

   svg.startNewSVG( "heya" );

   string boardStr = R"(
      . . . . . . .
     . . . . a b . .
    . . . . . . c . .
   . . . . . . . . . .
  . . . . O O O . . . .
 . . . . O X X . . . . .
. . . . . . X X . . . . .
 . . . . . . X . . . . .
  . . . . . . . . . . .
   . . . . . . . . . .
    . . . . . . . . .
     . . . . . . . .
      . . . . . . .     )";

   vector<XY> letterOptions = Board::parseLetterOptions( boardStr );
   Board board = Board::from( boardStr );
   forEachHex( 7, [&]( XY p ) {
      svg.drawHex( p, board.at( p ), 0 );
   } );
   char ch = 'a';
   for ( XY p : letterOptions )
      svg.drawText( p, string() + ch++, -1 );
}


//void drawBKE_SVG()
//{
//   SVGBoardMaker svg( "BKE.html" );
//
//   forEachHex( 7, [&]( XY p ) {
//      svg.drawCellBackground( p, -1 );
//   } );
//
//   for ( int r = 1; r < 8; r++ )
//   {
//      XY pos = XY( r, 0 );
//      int idx = 0;
//      for ( XY d : g_DIRS )
//      {
//         for ( int i = 0; i < r; i++ )
//         {
//            svg.drawCellText( pos, string() + (char( 'A' + r - 1 )) + to_string(idx), r & 1 );
//            pos += d;
//            idx++;
//         }
//      }
//   }
//}

#pragma warning(disable:4326)
void main()
{
   //drawBKE_SVG(); exit( 0 );
   runTests();
   //return;

   Timer t;
   //drawSVG();
   drawSteps();
   //basicAnalyze();   
   //mainAnalyzeAkra();
   //mainAnalyzeBoneProposal();
//   return;




//   //setTraceFile( "open3.txt" );
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . . . . . .
//  . . . . . . . . . . .
// . . . . . . . . . . . .
//. . . . . X X X . . . . .
// . . . . . . . . . . . .
//  . . . . . . . . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";
//   Board board = Board::from( boardStr );
//
//   board.forEachOPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//      bool isWinForX = analyze_Xs_turn( board, false /*no printing*/);
//      if ( isWinForX )
//         return;
//      
//      board.forEachXPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//         if ( isWinForX )
//            return;
//
//         //trace << board.str() << endl;
//         //isWinForX = analyze_Os_turn( board, false /*no printing*/ );
//         isWinForX = analyze_Os_turn( board, true );
//         trace << "..." << (isWinForX ? "X wins" : "O defends") << endl << endl;
//      } );
//
//      if ( !isWinForX )
//      {
//         trace << board.str() << endl;
//         trace << "...O defended ALL X moves" << endl;
//      }
//
//   } );


//
//   setTraceFile( "triangle2.txt" );
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . . . . . .
//  . . . . . . . . . . .
// . . . . . . . . . . . .
//. . . . . . X . . . . . .
// . . . . . X X . . . . .
//  . . . . . . . . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";
//   Board board = Board::from( boardStr );
//
//   board.forEachOPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//      bool isWinForX = analyze_Xs_turn( board, false /*no printing*/);
//      if ( isWinForX )
//         return;
//      
//      board.forEachXPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//         if ( isWinForX )
//            return;
//
//         trace << board.str() << endl;
//         isWinForX = analyze_Os_turn( board, false /*no printing*/ );
//         //isWinForX = analyze_Os_turn( board );
//         trace << "..." << (isWinForX ? "X wins" : "O defends") << endl << endl;
//      } );
//
//      if ( !isWinForX )
//      {
//         trace << board.str() << endl;
//         trace << "...O defended ALL X moves" << endl;
//      }
//
//   } );


//
//   string boardStr = R"(
//      . . . . . . .
//     . . . . . . . .
//    . . . . . . . . .
//   . . . . . . . . . .
//  . . . . . . . . . . .
// . . . . . . O . . . . .
//. . . . O X X X . . . . .
// . . . . . . . . . . . .
//  . . . . . . . . . . .
//   . . . . . . . . . .
//    . . . . . . . . .
//     . . . . . . . .
//      . . . . . . .     )";
//   Board board = Board::from( boardStr );
//
//   //board.forEachOPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//   //   bool isWinForX = analyze_Xs_turn( board, false /*no printing*/);
//   //   if ( isWinForX )
//   //      return;
//   bool isWinForX = false;
//      
//      board.forEachXPlacement( 2 /*#Os*/, 2 /*radius*/, [&]() {
//         if ( isWinForX )
//            return;
//
//         trace << board.str() << endl;
//         isWinForX = analyze_Os_turn( board, false /*no printing*/ );
//         //isWinForX = analyze_Os_turn( board );
//         trace << "..." << (isWinForX ? "X wins" : "O defends") << endl << endl;
//      } );
//
//      if ( !isWinForX )
//      {
//         trace << board.str() << endl;
//         trace << "...O defended ALL X moves" << endl;
//      }
//
//   //} );



   trace << "Time = " << t.ElapsedTime() << endl;
}