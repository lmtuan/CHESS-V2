/**
 * Chess Game
 * @author lmtuan
 * @version 0.9
 */

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#include "AIPlayer.h"
#include "Board.h"
#include "BoardGUI.h"
#include "GUI.h"
#include "HumanPlayer.h"
#include "Player.h"
#include "RandomPlayer.h"
#include "StartGUI.h"


/************************************
 * Sizes and title of the game window
 ************************************/
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const char* WINDOW_TITLE = "Chess Game by lmtuan";

/**
 * Initialize window and renderer
 * @param window, renderer: window and renderer to be initialized
 * @return true if init successfully
 */
bool initGraphic(SDL_Window* &window, SDL_Renderer* &renderer);

/**
 * Deallocate window and renderer before quitting game
 * @param current window and renderer to be destroyed
 */
void quitGraphic(SDL_Window* window, SDL_Renderer* rederer);

/**
 * Play a chess game, and return the winner
 * @return 0 for white, 1 for black, 2 for draw, and -1 for premature quit (back to start screen or quit game)
 */
int playGame(Board* b, Player** players, BoardGUI* bgui, SDL_Renderer* renderer);


int main(int argc, char* argv[]) {
  SDL_Window* window;
  SDL_Renderer* renderer;

  if( !initGraphic(window, renderer) ) return 0; //quit if cannot initialize graphic


  // Init GUIs and board
  Board b;

  GUI::quit = false;
  StartGUI sgui(renderer);
  BoardGUI bgui(&b, renderer);

  Player** players = new Player*[2];

  sgui.draw(renderer);

  //players[0] = new RandomPlayer(&b);
  //players[1] = new RandomPlayer(&b);
  players[0] = new HumanPlayer(&bgui, &b);
  players[1] = new HumanPlayer(&bgui, &b);
  //players[0] = new AIPlayer(&b, &bgui, 5);
  //players[1] = new AIPlayer(&b, &bgui, 5);

  bgui.draw(renderer);

  playGame(&b, players, &bgui, renderer);
  printf("Done playing");

  int move = 0;
  while (!move) {
    bgui.getInput();
    if (GUI::quit) break;
  }

  quitGraphic(window, renderer);
  return 0;
}

int playGame(Board* b, Player** players, BoardGUI* bgui, SDL_Renderer* renderer) {
  int numUndo[2] = {1, 1};
  int curPlayer, input;
  while (b->getNumMoves() != 0) { // continue as long as the game haven't ended
    // get player's move
    curPlayer = b->getPlayer();
    input = players[curPlayer]->decideMove();

    // make the move
    if (players[curPlayer]->isHuman()) {
    // if player is human, the value returned is a chosen square. Choose the appropriate square.
      if (input == 0) {
      } else if (input >= BoardGUI::INPUT_MIN_SQUARE && input <= BoardGUI::INPUT_MAX_SQUARE) { //olayer chose a square in board
        // if there is a promotion event, cannot make move until player choose a promotion
        if (b->hasPromotion()) {
          //flash the promoted pawn
        } else {
          b->chooseSquare(input - 1);
        }
      } else {//player chose a button from side bar
        int promoteTo;
        switch (input) {
          case BoardGUI::INPUT_UNDO: // undo
            if (numUndo[curPlayer] != 0) {
              b->undoMove();
              b->undoMove();
              numUndo[curPlayer]--;
            }
            break;
          case BoardGUI::INPUT_HOME: return -1; // return home
          case BoardGUI::INPUT_PROMOTE_QUEEN: promoteTo = 0; break; //promote to queen
          case BoardGUI::INPUT_PROMOTE_ROOK: promoteTo = 1; break; //promote to rook
          case BoardGUI::INPUT_PROMOTE_BISHOP: promoteTo = 2; break; //promote to bishop
          case BoardGUI::INPUT_PROMOTE_KNIGHT: promoteTo = 3; break; //promote to knight
        }
        if (b->hasPromotion()) {
          b->promote(promoteTo);
        }
      }
      bgui->updateMovePointers();
    } else {
    //if is AI, the value returned is a move number. Make the move, and process input queue to catch quitting event.
      b->makeMove(input);
      bgui->getInput(); //process all input made during ai's thinking
    }

    // if quit or user chooses to return to start screen
    if (GUI::quit || input == BoardGUI::INPUT_HOME) return -1;

    // draw board
    bgui->draw(renderer);
  }
  printf("Player %i wins!", b->getWinner()+1);
  return (b->getWinner());
}

bool initGraphic(SDL_Window* &window, SDL_Renderer* &renderer) {
  // Init SDL
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    printf("Failed to initialize SDL. SDL Error: %s\n", SDL_GetError());
    return false;
  }

  // Create window
  window = SDL_CreateWindow(WINDOW_TITLE,
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN);
  if (window == NULL) {
    printf("Failed to create window. SDL error: %s\n", SDL_GetError());
    return false;
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(renderer == NULL) {
    printf("Failed to create renderer. SDL error: %s\n", SDL_GetError());
    return false;
  }
  // Init rederer's color to white
  SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF);

  // Init SDL_Image for Image loading
  if( IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0 ) {
    printf("Failed to initialize SDL_image. SDL_image error: %s\n", IMG_GetError());
    return false;
  }
  return true;
}

void quitGraphic(SDL_Window* window, SDL_Renderer* renderer) {
  // destroy window and renderer
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  renderer = NULL;
  window = NULL;

  // quit SDL
  IMG_Quit();
  SDL_Quit();
}
