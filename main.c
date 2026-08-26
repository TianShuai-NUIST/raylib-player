#include <raylib.h>
#include <stdio.h>
#include <math.h>

const int windowWidth  = 600;
const int windowHeight = 600;
const int originFPS = 60;

const int playerR      = 25;
const int playerSpeed = 200;
const int playerHealth = 100;
const Color playerColor = GREEN;

const int enemyR = 10;
const int enemySpeed = 190;
const int enemyHealth = 50;
const Color enemyColor = RED;

#define PLAYER_INIT (Player) { \
  .e = (Entity){               \
    .pos = (Vector2) {         \
      .x = windowWidth / 2,    \
      .y = windowHeight / 2,   \
    },                         \
    .r = playerR,              \
    .speed = playerSpeed,      \
    .health = playerHealth,    \
    .color = playerColor,      \
    .move = MoveEntity,        \
    .draw = DrawEntity,        \
  }                            \
}

#define ENEMY_INIT (Enemy) {   \
  .e = (Entity){               \
    .pos = (Vector2) {         \
      .x = enemyR,             \
      .y = enemyR,             \
    },                         \
    .r = enemyR,               \
    .speed = enemySpeed,       \
    .health = enemyHealth,     \
    .color = enemyColor,       \
    .move = MoveEntity,        \
    .draw = DrawEntity,        \
  }                            \
}

typedef struct Entity{
  Vector2 pos;
  int speed;
  int r;
  int health;
  Color color;

  void (*move)(struct Entity *e, float);
  void (*draw)(struct Entity *e);
} Entity;

typedef struct Player{
  Entity e;
} Player;

typedef struct Enemy{
  Entity e;
} Enemy;

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

// ===== Entity function

void MoveEntity(Entity *e, float dt){
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

  e -> pos.x += dt * e -> speed * v.x;
  e -> pos.y += dt * e -> speed * v.y;

  clampf(&(e -> pos.x), e -> r, windowWidth - e -> r);
  clampf(&(e -> pos.y), e -> r, windowHeight - e -> r);
}

void DrawEntity(Entity *e){
  DrawCircleV(e -> pos, e -> r, e -> color);
}


int main(void){
  Player player = PLAYER_INIT;
  Enemy enemy = ENEMY_INIT;
  float dt = 0;

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){
    dt = GetFrameTime();

    ChangeFPS();
    MoveEntity(&(player.e), dt);

    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(20, 20);
    player.e.draw(&(player.e));
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
