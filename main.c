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

#define CLAMP_A(value, min, max) ({                    \
  typeof(value) _value = (value);                        \
  typeof(max) _max     = (max);                          \
  typeof(min) _min     = (min);                          \
  _value > _max ? _max : (_value < _min ? _min : _value); \
})                                                     \

typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth   = 600;
const int windowHeight  = 600;
const Vector2 wSize = (Vector2){(float)windowWidth, (float)windowHeight};
const int originFPS     = 60;

// (TYPE, NAME, PLAYER_VALUE, ENEMY_VALUE)
#define ENTITY_ATTRI_TABLE                                     \
XX(int, r, 25, 10)                                             \
XX(Vector2, vel, ((Vector2){150, 150}),                        \
   ((Vector2){120, 120}))                                           \
XX(int, health, 100, 50)                                       \
XX(int, iujryBufferTime, 0, 0)                                       \
XX(bool, isIujrying, false, false)                                       \
XX(float, mass, 10, 5)                                       \
XX(Color, color, (GREEN), (RED))                               \
XX(Vector2, spawnPoint,                                        \
   ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2}), \
   ((Vector2) {enemy_r, enemy_r}))                                \
XX(Vector2, target, ((Vector2){-1, -1}), player_spawnPoint)    \
XX(Vector2, pos, player_spawnPoint,                            \
   enemy_spawnPoint)                                              \
XX(int, hurtState, 0, 0)\
XX(int, stateTimer, 0, 0)\
XX(Color, targetColor, player_color, enemy_color)\

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
// 实际上这里的X宏没有任何作用, 只是作为一个标记存在而已
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

void ClampV2(Vector2 *value, const Vector2 min,const Vector2 max){
  float vx = Clamp(value->x,min.x, max.x);
  float vy = Clamp(value->y,min.y, max.y);
  *value = (Vector2){vx, vy};
}

// 浮点数相等判断,
// |a - b| <= max(|a|, |b|) * rel_eps
bool FloatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return true;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= REL_EPS * max_val;
}

bool ColorEq(Color c1, Color c2){
  if(c1.r != c2.r ||
     c1.g != c2.g ||
     c1.b != c2.b ||
     c1.a != c2.a) return false;
  else return true;
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

// entity direction
void ToTarget(Entity *e, const Vector2 *v){
  float dt = GetFrameTime();
  if(dt > 0.1f) dt = 0.1f;

  Vector2 cv = Vector2Normalize(*v);

  e->pos = Vector2Add(e->pos, Vector2Scale(Vector2Multiply(e->vel, cv), dt));

  Vector2 vr = (Vector2){e->r, e->r};
  ClampV2(&(e->pos), vr, Vector2Subtract(wSize, vr));
}

bool InTarget(Entity *e){
  if(Vector2Distance(e->pos, e->target) < REL_EPS) return true;
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
  v = Vector2Subtract(e->base.target, e->base.pos);

  // 如果距离小于步长, 直接到坐标上(防止抖动)
  float remaining = Vector2Length(v);
  if(remaining <= Vector2Length(e->base.vel) * dt){
    e->base.pos = e->base.target;
    return;
  }

  e -> base.toTarget(&(e->base), &v);
}

// ==== game Loop function

#define ENEMY_NUMBER 2
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

void PressedEvent(Player *p){
  if(IsKeyPressed(KEY_SPACE)){
    p->base.color = RED;
    p->base.isIujrying = true;
  }
}

void ShowInfo(const Player *p){
  if(IsKeyPressed(KEY_TAB)){
    printf("Health :%d\n", p->base.health);
    printf("iujryBufferTime:%d\n", p->base.iujryBufferTime);
  }
}
#define COLOR_CHANGE_FACTOR 5
void BindValue(float *value, const float base){
  float diff = *value - base;
  if(diff == 0) return;

  if(diff < 0){
    *value += COLOR_CHANGE_FACTOR;
  }else{
    *value -= COLOR_CHANGE_FACTOR;
  }
  *value = CLAMP_A(*value, 0, 255);
}

void UpdateColor(Player *p){
  //GREEN HSV 132.631577, 1.000000 0.894118
  bool e = ColorEq(GREEN, p->base.color);
  bool inIujrying = p->base.isIujrying;
  int *t = &(p->base.iujryBufferTime);

  if(e){
    *t = 0;
    return;
  }
  if(*t == 0){
    if(!inIujrying){
      *t = 10;
      p->base.color = WHITE;
      return;
    }else{
      Color c = p->base.color;
      Vector3 hsv = ColorToHSV(c);
      BindValue(&(hsv.x), ColorToHSV(GREEN).x);
      p->base.color = ColorFromHSV(hsv.x, hsv.y, hsv.z);
      // BindValue(&(p->base.color.r), 0);
      // BindValue(&(p->base.color.g), 228);
      // BindValue(&(p->base.color.b), 48);
      // BindValue(&(p->base.color.a), 255);
    }
  }else{
    *t -= 1;
    if(*t == 0){
      p->base.isIujrying = false;
    }
  }
}

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

  Entity *entityList[ENEMY_NUMBER + 1];
  entityList[0] = &(player.base);
  for(int i = 1; i < ENEMY_NUMBER + 1; i++){
    entityList[i] = &((enemy + i)->base);
  }

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  printf("\n%f, %f %f\n", ColorToHSV(GREEN).x, ColorToHSV(GREEN).y, ColorToHSV(GREEN).z);

  while(!WindowShouldClose()){
    // calc
    if(player.base.health <= 0) break;
    UpdateColor(&player);
    ENEMY_LOOP(enemy[i].base.target = player.base.pos);

    // event
    ChangeFPS();
    ShowInfo(&player);
    PressedEvent(&player);

    // cotrol
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
//TODO: 受伤颜色改变
// 1. 保持颜色0.5s
// 2. 开始恢复颜色
