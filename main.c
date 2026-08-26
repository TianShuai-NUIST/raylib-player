#include <raylib.h>
#include <stdio.h>
#include <math.h>

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

// ==== Utils
void clampf(float *value, float min, float max){
  if(*value >= max) *value = max;
  if(*value <= min) *value = min;
}



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
  if(dt > 0.1f) dt = 0.1f;

  Vector2 v = (Vector2){0, 0};

  if(IsKeyDown(KEY_A)) v.x -= 1.0f;
  if(IsKeyDown(KEY_D)) v.x += 1.0f;
  if(IsKeyDown(KEY_S)) v.y += 1.0f;
  if(IsKeyDown(KEY_W)) v.y -= 1.0f;

  float length = sqrtf(v.x * v.x + v.y * v.y);
  if(length > 0.0f){
    v.x /= length;
    v.y /= length;
  }

  p -> pos.x += dt * playerSpeed * v.x;
  p -> pos.y += dt * playerSpeed * v.y;

  clampf(&(p -> pos.x), p -> r, windowWidth - p -> r);
  clampf(&(p -> pos.y), p -> r, windowHeight - p -> r);
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
