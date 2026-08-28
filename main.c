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

typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth   = 600;
const int windowHeight  = 600;
const int originFPS     = 60;

// (TYPE, NAME, PLAYER_VALUE, ENEMY_VALUE)
#define ENTITY_ATTRI_TABLE                                     \
XX(int, r, 25, 10)                                             \
XX(Vector2, vel, ((Vector2){100, 100}),                        \
   ((Vector2){50, 50}))                                           \
XX(int, health, 100, 50)                                       \
XX(Color, color, (GREEN), (RED))                               \
XX(Vector2, spawnPoint,                                        \
   ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2}), \
   ((Vector2) {enemy_r, enemy_r}))                                \
XX(Vector2, target, ((Vector2){-1, -1}), player_spawnPoint)    \
XX(Vector2, pos, player_spawnPoint,                            \
   enemy_spawnPoint)                                              \

// (Type, Name, Value, Value)
#define ENTITY_FUNC_TABLE                                                          \
XX(toTargetFunc , toTarget , ToTarget  , void , (struct Entity*, const Vector2 *)) \
XX(inTargetFunc , inTarget , InTarget  , bool , (struct Entity*))                  \
XX(drawFunc     , draw     , DrawEntity, void , (struct Entity*))                  \

// function ProtoType
#define XX(type, name, value, return_t, arg) return_t value arg;
ENTITY_FUNC_TABLE
#undef XX

// typedef function pointer
#define XX(type, name, value, return_t, arg) typedef return_t (*type) arg;
ENTITY_FUNC_TABLE
#undef XX

// const value With Player
#define XX(type, name, value, ...) const type player_##name = value;
ENTITY_ATTRI_TABLE;
ENTITY_FUNC_TABLE;
#undef XX

// const value With Enemy
#define XX(type, name, _, value) const type enemy_##name = value;
ENTITY_ATTRI_TABLE;
#undef XX
#define XX(type, name, value, ...) const type enemy_##name = value;
ENTITY_FUNC_TABLE;
#undef XX

typedef struct Entity{
#define XX(type, name, _, ...) type name;
  ENTITY_ATTRI_TABLE;
  ENTITY_FUNC_TABLE;
#undef XX
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

// Init Player Struct
#define XX(type, name, ...) .name = player_##name,
#define PLAYER_INIT (Player){   \
  .base =(Entity){              \
    ENTITY_ATTRI_TABLE          \
    ENTITY_FUNC_TABLE           \
  },                            \
  .cursor = false,              \
  .move = MovePlayer,           \
}
#undef XX

// Init Enemy Struct
#define XX(type, name, ...) .name = enemy_##name,
#define ENEMY_INIT (Enemy){ \
  .base =(Entity){          \
    ENTITY_ATTRI_TABLE      \
    ENTITY_FUNC_TABLE       \
  },                        \
  .move = MoveEnemy,        \
}
#undef XX

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
  int fps = GetFrameTime();
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
  float dt = GetFrameTime();
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

bool InTarget(Entity *e){
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
  float dt = GetFrameTime();
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

// #define ENEMY_LOOPS(x)                 \
// for(int i = 0; i < ENEMY_NUMBER; i++){ \
//   do{                                  \
//     x                                  \
//   }while(0);                           \
// }                                      \

// main loop
int main(void){
  srand(time(NULL));

#define XX(type, name, ...) .name = player_##name,
  Player player = PLAYER_INIT;
#undef XX
  Enemy enemy[ENEMY_NUMBER];
  for(int i = 0; i < ENEMY_NUMBER; i++){
#define XX(type, name, ...) .name = enemy_##name,
    enemy[i] = ENEMY_INIT;
#undef XX
    float r = enemy[i].base.r;
    enemy[i].base.pos.x = RandRange(r, windowWidth  - r);
    enemy[i].base.pos.y = RandRange(r, windowHeight - r);
  }

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){
    // calc
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
