#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>

const int defaultFieldHeight = 15;
const int defaultFieldWidth  = 30;

#define maxTailLength  35

#define DEFAULT_FPS 5
#define MAX_FPS 35
#define CURRENT_FPS(tl) (((MAX_FPS-DEFAULT_FPS)*1.0/(maxTailLength-2)*((tl) - 2)) + DEFAULT_FPS )


typedef struct position_t{
  int x;
  int y;
}position_t;

typedef struct entity_t{
  char* name;

  position_t position[maxTailLength+1];
  int tailLength;

  position_t lastMove;

  char headSymbol;
  char tailSymbol;

}entity_t;



WINDOW *createNewWin(int height, int width, int starty, int startx);
void printEntity(WINDOW* window, entity_t* entity);

int getCurrentMicroseconds();


bool moveEntity(entity_t* entity, int stepsX, int stepsY);
bool checkForCollision(entity_t* one, entity_t* two);
position_t checkForCollisionAtPosition(entity_t* one, entity_t* two);
int checkForCollisionAtIndex(entity_t* one, entity_t* two);


int main(int argc, char** argv){
  WINDOW* gameWindow = NULL;
  int starty = 0, startx = 0;

  entity_t snake = {NULL, {{3,3},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4},{3,4}}, 8, {0,0}, '$', 'o'};
  //entity_t snake = {NULL, {{3,3},{3,4},{3,5},{3,6},{3,7}, {3,7}, {3,7}, {3,7}, {3,7},{3,7},{3,7},{3,7},{3,7},{3,7}, {3,7}, {3,7}, {3,7}, {3,7}}, 16, {0,0}, 'O', 'o'};
  entity_t apple = {NULL, {-1,-1}, 0, {0,0}, 'X', 'x'};

  int lastTimestamp = 0;

  int lastInputChar = 0;

  //INIT NCURSES
  initscr();
  noecho();
  curs_set(FALSE);
  cbreak();
  keypad(stdscr, TRUE);

  //INT GAME WINDOW
	starty = (LINES - defaultFieldHeight+2) / 2;	/* Calculating for a center placement */
	startx = (COLS - defaultFieldWidth+2) / 2;	/* of the window		*/
	refresh();
	gameWindow = createNewWin(defaultFieldHeight+2, defaultFieldWidth+2, starty, startx);

  //SPAWN FIRST APPLE
  do{
    apple.position[0].x = rand()%defaultFieldHeight;
    apple.position[0].y = rand()%defaultFieldWidth;
  }while(checkForCollision(&snake, &apple));

  int inputChar = 0;

  //MAIN GAME LOOP
  while(inputChar != 'q'){

      inputChar = wgetch(gameWindow);

      switch(inputChar){
        case KEY_UP:
          moveEntity(&snake, -1, 0);
          snake.lastMove.x = -1;
          snake.lastMove.y =0;
          break;
        case KEY_DOWN:
          moveEntity(&snake, 1, 0);
          snake.lastMove.x = 1;
          snake.lastMove.y = 0;
          break;
        case KEY_LEFT:
          moveEntity(&snake, 0, -1);
          snake.lastMove.x = 0;
          snake.lastMove.y = -1;
          break;
        case KEY_RIGHT:
          moveEntity(&snake, 0, +1);
          snake.lastMove.x = 0;
          snake.lastMove.y = 1;
          break;
        default:
          moveEntity(&snake, snake.lastMove.x, snake.lastMove.y);
      }



      if(checkForCollision(&snake, &apple)){
        if(snake.tailLength < maxTailLength){
          snake.tailLength++;
          snake.position[snake.tailLength].x = snake.position[snake.tailLength-1].x;
          snake.position[snake.tailLength].y = snake.position[snake.tailLength-1].y;
        }
        do{
          apple.position[0].x = rand()%defaultFieldHeight;
          apple.position[0].y = rand()%defaultFieldWidth;
        }while(checkForCollision(&snake, &apple));

      }
      int index = 0;
      if((index = checkForCollisionAtIndex(&snake, &snake)) > 0){
        if(index < snake.tailLength){
          snake.tailLength = (snake.tailLength > 5) ? 5 : snake.tailLength - 1;
        }
      }


      for(int i = 0; i < defaultFieldWidth; i++){
        for(int j = 0; j < defaultFieldHeight; j++){
          mvwprintw(gameWindow,j+1,i+1, " ");
        }
      }
      printEntity(gameWindow, &snake);
      printEntity(gameWindow, &apple);

//DebugPrint
//      mvwprintw(gameWindow,1,1, "%d | %d", snake.position[0].x, snake.position[0].y);


      wrefresh(gameWindow);

      //keep snake speed
      int currentTime = getCurrentMicroseconds();
      while((currentTime - lastTimestamp) < 1000000/ CURRENT_FPS(snake.tailLength) && (currentTime - lastTimestamp) > 0){
        usleep( 1000000/ CURRENT_FPS(snake.tailLength) - (currentTime - lastTimestamp));
        currentTime = getCurrentMicroseconds();
      }
      lastTimestamp = currentTime;

  }

  wrefresh(gameWindow);
  sleep(1);

  endwin(); // Restore normal terminal behavior


  return 0;
}

int getCurrentMicroseconds(){
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_usec;
}

bool checkForCollision(entity_t* one, entity_t* two){
  if(one == NULL || two == NULL) return false;
  for(int i = one->tailLength; i >= 0 ; i--){
    if(one->position[i].x == two->position[0].x && one->position[i].y == two->position[0].y) return true;
  }
  return false;
}

position_t checkForCollisionAtPosition(entity_t* one, entity_t* two){
  position_t tmpPosition = {-1, -1};
  if(one == NULL || two == NULL) return tmpPosition;
  for(int i = one->tailLength; i >= 0 ; i--){
    if(one->position[i].x == two->position[0].x && one->position[i].y == two->position[0].y){
      tmpPosition.x = one->position[0].x;
      tmpPosition.y = one->position[0].y;
      return tmpPosition;
    }
  }
  tmpPosition.x = -1;
  tmpPosition.y = -1;
  return tmpPosition;
}

int checkForCollisionAtIndex(entity_t* one, entity_t* two){
  if(one == NULL || two == NULL) return -1;
  for(int i = one->tailLength; i >= 0 ; i--){
    if(one->position[i].x == two->position[0].x && one->position[i].y == two->position[0].y){
      return i;
    }
  }
  return -1;
}

bool moveEntity(entity_t* entity, int stepsX, int stepsY){

  if(entity == NULL)return false;

  for(int i = entity->tailLength; i >= 0 && (stepsX != 0 || stepsY != 0); i--){
    if(i + 1 <= entity->tailLength){
      entity->position[i+1].x = entity->position[i].x;
      entity->position[i+1].y = entity->position[i].y;
    }
  }

  entity->position[0].x += stepsX;
  entity->position[0].y += stepsY;


  if(entity->position[0].y >= defaultFieldWidth)
    entity->position[0].y = entity->position[0].y%defaultFieldWidth;
  else if(entity->position[0].y < 0)
    entity->position[0].y += defaultFieldWidth;

  if(entity->position[0].x >= defaultFieldHeight)
    entity->position[0].x = entity->position[0].x%defaultFieldHeight;
  else if(entity->position[0].x < 0)
    entity->position[0].x += defaultFieldHeight;
}

WINDOW *createNewWin(int height, int width, int starty, int startx){
  WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
  wborder(local_win, '|', '|', '-', '-', '+', '+', '+', '+');
  keypad(local_win, TRUE);
  wtimeout(local_win, 0);

	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void printEntity(WINDOW* window, entity_t* entity){

  if(window == NULL || entity == NULL)return;

  for(int i = 0; i < entity->tailLength; i++){
    if(entity->position[i+1].x >= 0 && entity->position[i+1].x < defaultFieldHeight && entity->position[i+1].y >= 0 && entity->position[i+1].y < defaultFieldWidth)
      mvwprintw(window, entity->position[i+1].x+1, entity->position[i+1].y+1, "%c", entity->tailSymbol);
  }
    if(entity->position[0].x >= 0 && entity->position[0].x < defaultFieldHeight && entity->position[0].y >= 0 && entity->position[0].y < defaultFieldWidth)
      mvwprintw(window, entity->position[0].x+1, entity->position[0].y+1, "%c", entity->headSymbol);

}
