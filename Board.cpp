#include "Board.h"
#include <PieceType.h>
#include <stdio.h> // TO DO: remove after debug



Board::Board()
{
  initBoard();
}
Board::~Board() {};

const int Board::dir[8][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0},
                               {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
const int Board::dirKnight[8][2] = { {1, 2}, {1, -2}, {-1, 2}, {-1, -2},
                                     {2, 1}, {2, -1}, {-2, 1}, {-2, -1} };

void Board::initBoard()
{
  player = 1; // white player go first
  chosenSquare = -1; // no chosen square yet

  kingSquares[0] = 4;
  kingSquares[1] = 60;
  enPassantCols[0] = -1;
  enPassantCols[1] = -1;
  castlingFlags[0] = true;
  castlingFlags[1] = true;
  castlingFlags[2] = true;
  castlingFlags[3] = true;
  castlingFlags[4] = true;
  castlingFlags[5] = true;

  moveList.clear();
  moveList.reserve(100); //around 30 moves
  checkingPieces[0] = -1;
  checkingPieces[1] = -1;
  pinPieces.clear();
  pinPieces.reserve(8); // 4 pairs of pinned and pinning pieces

  // Fill board
  // Fill all squares with -1 (empty)
  for(int i = 16; i < 48; i++)
  {
    board[i/8][i%8] = -1;
  }
  board[0][0] = WR;
  board[0][1] = WN;
  board[0][2] = WB;
  board[0][3] = WQ;
  board[0][4] = WK;
  board[0][5] = WB;
  board[0][6] = WN;
  board[0][7] = WR;

  board[7][0] = BR;
  board[7][1] = BN;
  board[7][2] = BB;
  board[7][3] = BQ;
  board[7][4] = BK;
  board[7][5] = BB;
  board[7][6] = BN;
  board[7][7] = BR;

  for (int j = 0; j < 8; j++)
  {
    board[1][j] = WP;
    board[6][j] = BP;
  }
  updateMoveList();
}

int Board::getPiece(int square)
{
  int piece = board[square/8][square%8];
  if (square == chosenSquare)
  {
    return piece + 12;
  }
  return piece;
}

void Board::printBoard()
{
  printf("Printing Board\n");
  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      printf(" %i", board[i][j]);
    }
    printf("\n");
  }
}

void Board::printMoveList()
{
  for (int i = 0; i < moveList.size(); i+=3)
  {
    char x1 = 'a' + moveList[i]%8;
    char y1 = '1' + moveList[i]/8;
    char x2 = 'a' + moveList[i+1]%8;
    char y2 = '1' + moveList[i+1]/8;
    printf("%i) %c%c %c%c %i\n", (i/3 + 1), x1, y1, x2, y2, moveList[i+2]);
  }
}

void Board::chooseSquare(int square)
{
  // if choose the same square, unchoose the square
  if (square == chosenSquare || square < 0 || square > 63)
  {
    chosenSquare = -1;
  }
  else
  {
    if(chosenSquare == -1) // if currently no square is selected
    {
      int chosenPiece = board[square/8][square%8];
      if ( (chosenPiece >= 0) && ((chosenPiece > 5) == (player == 1)) ) //if the piece chosen is player's piece
      {
        chosenSquare = square;
      }
    }
    else
    {
      makeMove(chosenSquare, square);
      chosenSquare = -1;
    }
    printf("Board chosen square %i\n", chosenSquare);
  }
}

void Board::makeMove(int square1,int square2)
{
  int moveType = -1;
  for (int i = 0; i < moveList.size(); i += 3)
  {
    if (moveList[i] == square1 && moveList[i+1] == square2)
    {
      moveType = moveList[i+2];
      break;
    }
  }

  if (moveType == -1) return;

  /*
   * Make the move
   */
  int x1 = square1 / 8;
  int y1 = square1 % 8;
  int x2 = square2 / 8;
  int y2 = square2 % 8;
  board[x2][y2] = board[x1][y1];
  board[x1][y1] = -1;

  if (moveType == MOVE_PAWN_EN_PASSANT)
  {
    board[x1][y2] = -1;
  }
  else if (moveType == MOVE_CASTLING)
  {

  }
  else if (moveType == MOVE_PAWN_PROMOTION)
  {

  }

  /*
   * Update some variables
   */
  // Castling
  if (square1 == kingSquares[player - 1])
  {
    kingSquares[player - 1] = square2;
  }
  switch (square1)
  {
    case 0: castlingFlags[2] = false; break;
    case 4: castlingFlags[0] = false; break;
    case 7: castlingFlags[4] = false; break;
    case 56: castlingFlags[3] = false; break;
    case 60: castlingFlags[1] = false; break;
    case 63: castlingFlags[5] = false; break;
  }
  // En passant
  enPassantCols[2 - player] = (moveType == MOVE_PAWN_DOUBLE_JUMP)? y2 : -1;

  player = 3 - player;
  updateMoveList();
}

void Board::updateMoveList()
{
  moveList.clear();
  findPinAndCheck();
  // loop through all squares, and update player's pieces in those squares
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      // if square is empty, or has opponent's piece, do not update
      if (board[i][j] < 0 || ((player == 1) == (board[i][j] < 6))) continue;
      // if square has friendly piece, update
      int pType = board[i][j] %6; // don't care about color
      switch (pType)
      {
        case BP: updatePawnMoves(i, j); break;
        case BQ:
        case BR:
        case BB: updateRayMoves(i, j); break;
        case BN: updateKnightMoves(i, j); break;
        case BK: updateKingMoves(i, j);
      }
    }
  }
  printf("Updating move list\n");
  printMoveList();
}

void Board::findPinAndCheck()
{
  // Position of King
  int r = kingSquares[player-1] / 8;
  int c = kingSquares[player-1] % 8;
  int i,j;

  // Clear checking pieces and pin pieces
  checkingPieces[0] = -1;
  checkingPieces[1] = -1;
  pinPieces.clear();

  // Check 8 basic direction to see if there are ray pieces
  for(int d = 0; d <= 8; d++)
  {
    int pinnedSquare = -1;
    i = r;
    j = c;
    while(true)
    {
      // advance 1 square in the direction d
      i += dir[d][0];
      j += dir[d][1];
      // if out of bound
      if (i < 0 || i > 7 || j < 0 || j > 7) break;
      // if empty square, advance 1 more square
      if (board[i][j] == -1) continue;
      if (pinnedSquare == -1) // the first obstacle
      {
        if ( (player == 1) == (board[i][j] > 5) ) // if it's current player's piece
        {
          pinnedSquare = i * 8 + j;
        }
        else // if it's opponent's piece
        {
          // Check if it's checking current king
          int curPieceType = board[i][j]%6; // do not care about color
          if (curPieceType == BQ || (d < 4 && curPieceType == BR) || (d > 3 && curPieceType == BB))
          {
            int checkIndex = (checkingPieces[0] == -1)? 0 : 1;
            checkingPieces[checkIndex] = i * 8 + j;
          }
          break;
        }
      }
      else // the second obstacle
      {
        if ( (player == 1) != (board[i][j] > 5) ) // if it's opponent's piece
        {
          int curPieceType = board[i][j]%6; // do not care about color
          if (curPieceType == BQ || (d < 4 && curPieceType == BR) || (d > 3 && curPieceType == BB)) // if it's opponent's ray piece
          {
            pinPieces.push_back(pinnedSquare);
            pinPieces.push_back(i* 8 + j);
          }
        }
        break;
      }
    }
  }

  if (checkingPieces[1] != -1) return;

  // Check 8 knight's direction to see if there are opponent's knight
  for (int d = 0; d < 8; d++)
  {
    i = r + dirKnight[d][0];
    j = c + dirKnight[d][1];
    // if out of bound, check other directions
    if ( i < 0 || i > 7 || j < 0 || j > 7) continue;

    if (board[i][j] == (player == 1? BN : WN) ) // if has opponent's knight
    {
      int t = (checkingPieces[0] == -1)? 0 : 1;
      checkingPieces[t] = i * 8 + j;
      // if there are 2 checking pieces, that's max
      if (t == 1) return;
    }
  }

  // Check 2 squares diagonally to the left and right to see if there are opponent pawns
  i = (player == 1)? (r + 1) : (r - 1);
  j = c - 1; // The left column
  if (i >= 0 && i < 8 && j >= 0 && j < 8)
  {
    if ( board[i][j] == (player == 1? BP:WP) )
    {
      int t = (checkingPieces[0] == -1)? 0 : 1;
      checkingPieces[t] = i * 8 + j;
      if (t == 1) return;
    }
  }
  j = c + 1; // The right column
  if (i >= 0 && i < 8 && j >= 0 && j < 8)
  {
    if (board[i][j] == (player == 1? BP:WP))
    {
      int t = (checkingPieces[0] == -1)? 0 : 1;
      checkingPieces[t] = i * 8 + j;
    }
  }
}

void Board::updateKingMoves(int r, int c)
{
  // king's position
  int i, j;
  /*
   * 8 squares around king
   */
  for (int d = 0; d < 8; d++)
  {
    i = r + dir[d][0];
    j = c + dir[d][1];
    if (i < 0 || i > 7 || j < 0 || j > 7) continue; // out of bound

    if ((board[i][j] != -1) && (player == 1) == (board[i][j] > 5)) continue; // square has a friendly piece

    if (isSquareControlled(i, j)) continue; //square is controlled by opponent

    // if the king is between a square and opponent's ray piece, king cannot move to that square
    if (checkingPieces[0] != -1
        && isInRay(checkingPieces[0]/8, checkingPieces[0]%8, r, c, i, j) )
    {
      continue;
    }
    if (checkingPieces[1] != -1
        && isInRay(checkingPieces[1]/8, checkingPieces[1]%8, r, c, i, j) )
    {
      continue;
    }

    // If passes all tests above, the square is a legal move. Add to moveList
    moveList.push_back(r * 8 + c); //starting square
    moveList.push_back(i * 8 + j); //ending square
    moveList.push_back(MOVE_NORMAL); // move type
  }

  /*
   * Castling
   */
  if (castlingFlags[player - 1] && checkingPieces[0] == -1) //king has not moved and is not checked
  {
    // left rook has not moved, and the 2 squares left of king are empty and not controlled by the opponent
    if (castlingFlags[player + 1] && (board[r][2] == -1) && (board[r][3] == -1) && !isSquareControlled(r, 2) && !isSquareControlled(r, 3))
    {
      moveList.push_back(r * 8 + c); //starting square
      moveList.push_back(r * 8 + 2); //ending square
      moveList.push_back(MOVE_CASTLING); // move type
    }
    // right rook has not moved, and the 2 squares right of king are empty and not controlled by the opponent
    if (castlingFlags[player + 3] && (board[r][5] == -1) && (board[r][6] == -1) && !isSquareControlled(r, 5) && !isSquareControlled(r, 6))
    {
      moveList.push_back(r * 8 + c); //starting square
      moveList.push_back(r * 8 + 6); //ending square
      moveList.push_back(MOVE_CASTLING); // move type
    }
  }
}

void Board::updateRayMoves(int r, int c)
{
  /*
   * Find if the king is checked or the piece is pinned
   */
  // if 2 pieces are checking the king, the ray piece cannot move
  if (checkingPieces[1] != -1) return;

  int checkingSquare = -1; // the square of the piece that is checking the king or pinning the ray piece
  int raySquare = r * 8 + c;

  if (checkingPieces[0] != -1) checkingSquare = checkingPieces[0];

  for (int i = 0; i < pinPieces.size(); i += 2)
  {
    if (pinPieces[i] == raySquare)
    {
      if (checkingSquare != -1) return; //if ray is pinned and king is checked, ray cannot move
      checkingSquare = pinPieces[i+1];
      break;
    }
  }

  /*
   * Update ray's move
   * If there is a piece checking king or pinning ray, ray can only move between the checking piece and the king
   */
  int i,j;

  /*
   * Choose direction (queen: 8 dirs, rook: first 4 dirs, bishop: 4 last dirs)
   */
  int minDir, maxDir;
  switch(board[r][c])
  {
    case WR: case BR:
      minDir = 0; maxDir = 3; break;
    case WB: case BB:
      minDir = 4; maxDir = 7; break;
    case WQ: case BQ:
      minDir = 0; maxDir = 7; break;
  }
  // Look into all directions
  for(int d = minDir; d <= maxDir; d++)
  {
    i = r;
    j = c;
    while(true)
    {
      // advance 1 square in the direction d
      i += dir[d][0];
      j += dir[d][1];

      // if out of bound, look in another direction
      if (i < 0 || i > 7 || j < 0 || j > 7) break;
      // if moving causes king danger, advance 1 more square in direction d
      if ( (checkingSquare != -1)
           && !isInRay(checkingSquare/8, checkingSquare%8, i, j, kingSquares[player-1]/8, kingSquares[player-1]%8) ) continue;

      // if square is not empty, stop looking in this direction
      if (board[i][j] != -1)
      {
        // if has opponent piece, it's a legal move
        if ((player == 1) != (board[i][j] > 5))
        {
          moveList.push_back(raySquare);
          moveList.push_back(i * 8 + j);
          moveList.push_back(MOVE_NORMAL);
        }
        break;
      }
      else
      {
        // if square is empty, it;s a legal move
        moveList.push_back(raySquare);
        moveList.push_back(i * 8 + j);
        moveList.push_back(MOVE_NORMAL);
      }
    }
  }
}

void Board::updateKnightMoves(int r, int c)
{
  /*
   * Find if the king is checked or the pawn is pinned
   */
  // if 2 pieces are checking king, knight cannot move
  if (checkingPieces[1] != -1) return;

  int knightSquare = r * 8 + c;
  for (int i = 0; i < pinPieces.size(); i += 2)
  {
    if (pinPieces[i] == knightSquare) return; // if knight is pinned by a ray piece, it cannot move (because it cannot return to the same ray)
  }

  /*
   * Update knight's move
   * If there is a piece checking king, knight can only move between the checking piece and the king
   */
  int i,j;
  //Look in 8 knight directions and add legal move
  for (int d = 0; d < 8; d++)
  {
    i = r + dirKnight[d][0];
    j = c + dirKnight[d][1];
    // if out of bound, check other directions
    if ( i < 0 || i > 7 || j < 0 || j > 7) continue;
    // if moving causes king danger (not going between king and the checking piece), check other directions
    if ((checkingPieces[0] != -1) && !isInRay(checkingPieces[0]/8, checkingPieces[0]%8, i, j, kingSquares[player-1]/8, kingSquares[player-1]%8)) continue;

    if( board[i][j] == -1 || ((player == 1) != (board[i][j] > 5)) ) //if square empty or has opponent, legal move
    {
      moveList.push_back(knightSquare);
      moveList.push_back(i * 8 + j);
      moveList.push_back(MOVE_NORMAL);
    }
  }
}

void Board::updatePawnMoves(int r, int c)
{
  /*
   * Find if the king is checked or the pawn is pinned
   */
  // if 2 pieces are checking king, pawn cannot move
  if (checkingPieces[1] != -1) return;

  int checkingSquare = -1; // the square of the piece that is checking king or pinning pawn
  int pawnSquare = r * 8 + c;

  if (checkingPieces[0] != -1) checkingSquare = checkingPieces[0];

  for (int i = 0; i < pinPieces.size(); i += 2)
  {
    if (pinPieces[i] == pawnSquare)
    {
      if (checkingSquare != -1) return; //if both pawn is pinned and king is checked, pawn cannot move
      checkingSquare = pinPieces[i+1];
      break;
    }
  }

  /*
   * Update pawn's move
   * If there is a piece checking king or pinning pawn, pawn can only move between the checking piece and the king
   */
  int i,j;
  int moveForward = (player == 1)? 1: -1; // white pawn moves up, black pawn moves down

  /*
   * Move straight
   */
  // One square ahead
  i = r + moveForward;
  j = c;

  bool canPromote = (i == 0) || (i == 7); // is in the correct row for promotion
  bool canDoubleJump = (i == 2 || i == 5); // is in the correct row for double jump
  bool canEnPassant = (r == ((player == 1)? 4 : 3)); // is in the correct row for en passant

  if (board[i][j] == -1 // empty square in front
      && ((checkingSquare == -1) || isInRay(checkingSquare/8, checkingSquare%8, i, j, kingSquares[player-1]/8, kingSquares[player-1]%8))) // no king danger if move there
  {
    // can jump 1 square forward
    moveList.push_back(pawnSquare);
    moveList.push_back(i * 8 + j);
    moveList.push_back(MOVE_NORMAL);
  }
  else //cannot double jump if has piece in front or king is in danger if pawn move forward
  {
    canDoubleJump = false;
  }

  // Two squares ahead
  i += moveForward;
   //if in the correct row and the 1st square ahead is empty, and the 2nd square ahead is empty, can jump 2 squares ahead
  if (canDoubleJump && board[i][j] == -1)
  {
    moveList.push_back(pawnSquare);
    moveList.push_back(i * 8 + j);
    moveList.push_back(MOVE_PAWN_DOUBLE_JUMP);
  }

  /*
   * Capture diagonally and en passant
   */
  // Diagonally to the left
  i = r + moveForward;
  j = c-1;
  if (j >= 0 && j < 8  //not outside board
      && ((checkingSquare == -1) || isInRay(checkingSquare/8, checkingSquare%8, i, j, kingSquares[player-1]/8, kingSquares[player-1]%8))) //no king danger if move there
  {
    if (canEnPassant && j == enPassantCols[player-1]) // if in the correct row and correct col, can en passant
    {
      moveList.push_back(pawnSquare);
      moveList.push_back(i * 8 + j);
      moveList.push_back(MOVE_PAWN_EN_PASSANT);
    }
    else if (board[i][j] != -1 && (player == 1) != (board[i][j] > 5)) // if has opponent, can capture
    {
      moveList.push_back(pawnSquare);
      moveList.push_back(i * 8 + j);
      moveList.push_back( canPromote? MOVE_PAWN_PROMOTION : MOVE_NORMAL );
    }
  }
  // Diagonally to the right
  j = c+1;
  if (j >= 0 && j < 8  //not outside board
      && ((checkingSquare == -1) || isInRay(checkingSquare/8, checkingSquare%8, i, j, kingSquares[player-1]/8, kingSquares[player-1]%8))) //no king danger if move there
  {
    if (canEnPassant && j == enPassantCols[player-1]) // if in the correct row and correct col, can en passant
    {
      moveList.push_back(pawnSquare);
      moveList.push_back(i * 8 + j);
      moveList.push_back(MOVE_PAWN_EN_PASSANT);
    }
    else if (board[i][j] != -1 && (player == 1) != (board[i][j] > 5)) // if has opponent, can capture
    {
      moveList.push_back(pawnSquare);
      moveList.push_back(i * 8 + j);
      moveList.push_back( canPromote? MOVE_PAWN_PROMOTION : MOVE_NORMAL );
    }
  }
}

bool Board::isSquareControlled(int r, int c)
{
  int i,j;

  /*
   * Check 8 knight's move
   */
  for (int d = 0; d < 8; d++)
  {
    i = r + dirKnight[d][0];
    j = c + dirKnight[d][1];
    // if out of bound, check other directions
    if ( i < 0 || i > 7 || j < 0 || j > 7) continue;
    if (board[i][j] == (player == 1? BN : WN) ) return true;
  }

  /*
   * Check 8 ray directions
   */
  for(int d = 0; d <= 8; d++)
  {
    i = r;
    j = c;
    while(true)
    {
      // advance 1 square in the direction d
      i += dir[d][0];
      j += dir[d][1];
      // if out of bound
      if (i < 0 || i > 7 || j < 0 || j > 7) break;
      // if empty square, advance 1 more square
      if (board[i][j] == -1) continue;
      // if it is opponent's ray piece
      if ( (player == 1) != (board[i][j] > 5) )
      {
        int curPiece = board[i][j];
        if (curPiece < 6) curPiece += 6; //turn into whiteint curPiece = board[i][j];
        if (curPiece == WQ || (d < 4 && curPiece == WR) || (d > 3 && curPiece == WB)) return true;
      }
      break;
    }
  }
  /*
   * Check pawn's move
   */
  i = (player == 1)? (r + 1) : (r - 1);
  j = c - 1; // left column
  if (i >= 0 && i < 8 && j >= 0 && j < 8)
  {
    if (board[i][j] == (player == 1? BP : WP)) return true;
  }
  j = c + 1;
  if (i >= 0 && i < 8 && j >= 0 && j < 8)
  {
    if (board[i][j] == (player == 1? BP : WP)) return true;
  }

  /*
   * Check if controlled by opponent's king
   */
  for (int d = 0; d < 8; d++)
  {
    i = r + dir[d][0];
    j = c + dir[d][1];
    if (i >= 0 && i < 8 && j >= 0 && j < 8)
    {
      if (board[i][j] == (player == 1? BK : WK)) return true;
    }
  }

  // if pass all tests, square is not controlled by opponent
  return false;
}

bool Board::isInRay(int x1, int y1, int x2, int y2, int x3, int y3)
{
  return ((x1 - x2) * (y1 - y3) == (x1 - x3) * (y1 - y2)) // 3 squares are in a line
         && ((x2 >= x1 && x2 < x3) || (x2 <= x1 && x2 > x3)); // square 2 is in the middle (or square 1 is square 2)
}
