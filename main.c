#include <raylib.h>
#include <raymath.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

// FunctioinName : 大驼峰
// valueName     : 小驼峰
// ClassName     : 大驼峰

const int windowWidth   = 600;
const int windowHeight  = 600;
const int originFPS     = 60;

const int playerR       = 25;
const Vector2 playerVel   = (Vector2){100, 100};
const int playerHealth  = 100;
const Color playerColor = GREEN;
const Vector2 playerSpawnPoint = (Vector2) {windowWidth / 2, windowHeight / 2};
const Vector2 playerTarget = (Vector2) {-1, -1};

const int enemyR        = 10;
const Vector2 enemyVel    = (Vector2){50, 50};
const int enemyHealth   = 50;
const Color enemyColor  = RED;
const Vector2 enemySpawnPoint = (Vector2) {enemyR, enemyR};
const Vector2 enemyTarget = playerSpawnPoint;

typedef struct Entity{
  Vector2 pos;
  Vector2 vel;
  int r;
  int health;
  Color color;
  // player 光标操作
  Vector2 target;

  void (*toTarget)(struct Entity *e, const Vector2 *target);
  bool (*inTarget)(struct Entity *e);
  void (*draw)(struct Entity *e);
} Entity;

typedef struct Player{
  Entity base;
  bool cursor;

  void (*move)(struct Player *p);
} Player;

typedef struct Enemy{
  Entity base;

  void (*move)(struct Enemy *e);
} Enemy;

#define REL_EPS 1e-9
#define PLAYER_INIT (Player) { \
  .base = (Entity){            \
    .pos = playerSpawnPoint,   \
    .r = playerR,              \
    .vel = playerVel,      \
    .health = playerHealth,    \
    .color = playerColor,      \
    .target = playerTarget,    \
    \
    .toTarget = ToTarget,      \
    .inTarget = InTarget,      \
    .draw = DrawEntity,        \
  },                           \
  .cursor = false,             \
  \
  .move = MovePlayer,          \
}

#define ENEMY_INIT (Enemy) {   \
  .base = (Entity){            \
    .pos = enemySpawnPoint,    \
    .r = enemyR,               \
    .vel = enemyVel,       \
    .health = enemyHealth,     \
    .color = enemyColor,       \
    .target = enemyTarget,     \
    \
    .toTarget = ToTarget,      \
    .inTarget = InTarget,      \
    .draw = DrawEntity,        \
  },                           \
  \
  .move = MoveEnemy,      \
}

// ==== Utils

int RandRange(const int min, const int max){
  assert(min < max);
  if (min == max) return min;
  else return rand() % (max - min) + min;
}

void ClampF(float *value, float min, float max){
  if(*value > max) *value = max;
  else if(*value < min) *value = min;
  else return;
}

// 浮点数相等判断,
// |a - b| <= max(|a|, |b|) * rel_eps
bool FloatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return true;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= REL_EPS * max_val;
}

double Vec2Distance(Vector2 start, Vector2 end){
  double dx = start.x - end.x;
  double dy = start.y - end.y;
  return sqrt(dx * dx + dy * dy);
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

void DrawEntity(Entity *e){
  DrawCircleV(e -> pos, e -> r, e -> color);
}

void ToTarget(Entity *e, const Vector2 *v){
  float dt = GetFPS();
  if(dt > 0.1f) dt = 0.1f;

  Vector2 cv = *v;

  float length = sqrtf(v->x * v->x + v->y * v->y);
  if(length > 0.0f){
    cv.x = v->x / length;
    cv.y = v->y / length;
  }

  e -> pos.x += dt * e -> vel.x * cv.x;
  e -> pos.y += dt * e -> vel.y * cv.y;

  ClampF(&(e -> pos.x), e -> r, windowWidth - e -> r);
  ClampF(&(e -> pos.y), e -> r, windowHeight - e -> r);
}

bool InTarget(struct Entity *e){
  if(Vec2Distance(e->pos, e->target) < REL_EPS) return true;
  else return false;
}

void MovePlayer(Player *p){
  Vector2 v = (Vector2){0, 0};
  if(IsKeyDown(KEY_A)) v.x -= 1.0f;
  if(IsKeyDown(KEY_D)) v.x += 1.0f;
  if(IsKeyDown(KEY_S)) v.y += 1.0f;
  if(IsKeyDown(KEY_W)) v.y -= 1.0f;

  p -> base.toTarget(&(p -> base), &v);
}

void MoveEnemy(struct Enemy *e){
  float dt = GetFPS();
  if(e->base.inTarget(&(e->base))) return;

  Vector2 v;
  v.x = e->base.target.x - e->base.pos.x;
  v.y = e->base.target.y - e->base.pos.y;

  // 如果距离小于步长, 直接到坐标上(防止抖动)
  float remaining = sqrt(v.x * v.x + v.y * v.y);
  if(remaining <= Vector2Length(e->base.vel) * dt){
    e->base.pos = e->base.target;
    return;
  }

  e -> base.toTarget(&(e->base), &v);
}

// ==== game Loop function

#define ENEMY_NUMBER 1
#define ENEMY_LOOP(x)                  \
for(int i = 0; i < ENEMY_NUMBER; i++){ \
  do{                                  \
    x;                                 \
  }while(0);                           \
}                                      \

#define ENEMY_LOOPS(x)                 \
for(int i = 0; i < ENEMY_NUMBER; i++){ \
  do{                                  \
    x                                  \
  }while(0);                           \
}                                      \

// main loop
int main(void){
  srand(time(NULL));

  Player player = PLAYER_INIT;
  Enemy enemy[ENEMY_NUMBER];
  for(int i = 0; i < ENEMY_NUMBER; i++){
    enemy[i] = ENEMY_INIT;
    enemy[i].base.pos.x = RandRange(enemyR, windowWidth - enemyR);
    enemy[i].base.pos.y = RandRange(enemyR, windowHeight - enemyR);
  }
  float dt = 0;

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){
    // calc
    dt = GetFrameTime();
    ENEMY_LOOP(enemy[i].base.target = player.base.pos);

    // event
    ChangeFPS();
    player.move(&player);
    ENEMY_LOOP(enemy[i].move(enemy + i));
    // draw
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(20, 20);
    player.base.draw(&(player.base));
    ENEMY_LOOP(enemy[i].base.draw(&(enemy[i].base)));
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

//TODO: 添加碰撞
// 1. 增加Vector2 vel
