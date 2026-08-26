#include <raylib.h>
#include <stdio.h>

const int windowWidth  = 600;
const int windowHeight = 600;
const int originFPS = 60;
const int playerR      = 25;
const int playerSpeed = 200;
const Color playerColor = GREEN;


typedef struct Player{
  Vector2 pos;
  int r;
} Player;

// -> or ^ to up fps
// <- or v to down fps
// r to reset
void ChangeFPS(void){
  int fps = GetFPS();
  if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP)){
    SetTargetFPS(fps + 1);
  } else if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN)){
    SetTargetFPS(fps - 1);
  } else if(IsKeyPressed(KEY_R)){
    SetTargetFPS(originFPS);
  }
}

void MovePlayer(Player *p, float dt){

  if(p -> pos.x < p -> r) p -> pos.x = p -> r;
  if(p -> pos.y < p -> r) p -> pos.y = p -> r;
  if(p -> pos.x > windowWidth - p -> r) p -> pos.x = windowWidth - p -> r;
  if(p -> pos.y > windowHeight - p -> r) p -> pos.y = windowHeight - p -> r;
  if(dt > 0.1f) dt = 0.1f;

  if(IsKeyDown(KEY_A)){
    if(p -> pos.x >= p -> r && p -> pos.x <= windowWidth - p -> r){
      p -> pos.x -= 1 * playerSpeed * dt;
    }else{
      p -> pos.x = p -> r;
    }
  }
  if(IsKeyDown(KEY_S)){
    if(p -> pos.y >= p -> r && p -> pos.y <= windowHeight - p -> r){
      p -> pos.y += 1 * playerSpeed * dt;
    }else {
      p -> pos.y = windowHeight - p -> r;
    }
  }
  if(IsKeyDown(KEY_D)){
    if(p -> pos.x >= p -> r && p -> pos.x <= windowWidth - p -> r){
      p -> pos.x += 1 * playerSpeed * dt;
    }else{
      p -> pos.x = windowWidth - p -> r;
    }
  }
  if(IsKeyDown(KEY_W)){
    if(p -> pos.y >= p -> r && p -> pos.y <= windowHeight - p -> r){
      p -> pos.y -= 1 * playerSpeed * dt;
    }else{
      p -> pos.y = p -> r;
    }
  }
}


int main(void){
  Player player = (Player) {
    .pos = (Vector2) {
      .x = windowWidth / 2,
      .y = windowHeight / 2,
    }, 
    .r = playerR
  };

  float dt = 0;

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){
    
    dt = GetFrameTime();
    ChangeFPS();
    MovePlayer(&player, dt);
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(20, 20);
    DrawCircleV(player.pos, player.r, playerColor);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
