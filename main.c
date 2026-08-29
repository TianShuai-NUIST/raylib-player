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
typedef enum HurtState {
  HURT_FLASH,   // 闪白
  HURT_PERSIST,// 疼痛暗红
  HURT_RECOVER, // 恢复颜色
  HURT_IDLE,      // 正常
} HurtState;

typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth   = 600;
const int windowHeight  = 600;
const Vector2 wSize = (Vector2){(float)windowWidth, (float)windowHeight};
const int originFPS     = 60;

void MoveTowardsHSV(Vector3 *value, const Vector3 target, const float k);

// (TYPE, NAME, PLAYER_VALUE, ENEMY_VALUE)
#define ENTITY_ATTRI_TABLE                                     \
XX(int, r, 25, 10)                                             \
XX(Vector2, vel, ((Vector2){150, 150}),                        \
   ((Vector2){120, 120}))                                           \
XX(int, health, 100, 50)                                       \
XX(float, mass, 10, 5)                                       \
XX(Color, color, (GREEN), (RED))                               \
XX(Color, defaultColor, (GREEN), (RED))                               \
XX(Vector2, spawnPoint,                                        \
   ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2}), \
   ((Vector2) {enemy_r, enemy_r}))                                \
XX(Vector2, target, ((Vector2){-1, -1}), player_spawnPoint)    \
XX(Vector2, pos, player_spawnPoint,                            \
   enemy_spawnPoint)                                              \
\
XX(HurtState, hurtState, HURT_IDLE, HURT_IDLE)\
XX(int, stateTimer, 0, 0)\
XX(Color, targetColor, player_color, enemy_color)\

// (Type, Name, Value, Value)
#define ENTITY_FUNC_TABLE                                                             \
XX(toTargetFunc  , toTarget  , ToTarget   , void , (struct Entity*, const Vector2)) \
XX(inTargetFunc  , inTarget  , InTarget   , bool , (struct Entity*))                  \
XX(setTargetFunc , setTarget , SetTarget  , void , (struct Entity*, const Vector2)) \
XX(drawFunc      , draw      , DrawEntity , void , (struct Entity*))                  \

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
  return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

bool ColorEqApprox(Color c1, Color c2, unsigned char tolerance){
  int dr = (int)c1.r - (int)c2.r;
  int dg = (int)c1.g - (int)c2.g;
  int db = (int)c1.b - (int)c2.b;
  int da = (int)c1.a - (int)c2.a;

  return ( abs(dr) <= tolerance && abs(dg) <= tolerance && abs(db) <= tolerance && abs(da) <= tolerance );
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

void ChangeHurtState(Entity *e, HurtState newHurtState){
  e->hurtState = newHurtState;

  if (HURT_IDLE == newHurtState) return;
  switch(newHurtState){
    case HURT_IDLE:
    break;
    case HURT_FLASH:
      e->stateTimer = 3;
    break;
    case HURT_PERSIST:
      e->stateTimer = 20;
      e->targetColor = ColorFromHSV(355.0f, 0.8f, 0.75f);
    break;
    case HURT_RECOVER:
      // e->stateTimer = 20;
      e->targetColor = e->defaultColor;
    break;
  }
}

void UpdateHurtEffect(Entity *e){
  if(e->hurtState == HURT_IDLE) return;

  Vector3 hsv = ColorToHSV(e->color);

  switch(e->hurtState){
    case HURT_FLASH:
      e->color = WHITE;
      e->stateTimer--;
      if(e->stateTimer == 0) ChangeHurtState(e, HURT_PERSIST);
    break;
    case HURT_PERSIST:
      MoveTowardsHSV(&hsv, ColorToHSV(e->targetColor), 0.8f);
      e->color = ColorFromHSV(hsv.x, hsv.y, hsv.z);

      e->stateTimer--;
      if(e->stateTimer == 0) ChangeHurtState(e, HURT_RECOVER);
    break;
    case HURT_RECOVER:
      MoveTowardsHSV(&hsv, ColorToHSV(e->targetColor), 0.15f);
      e->color = ColorFromHSV(hsv.x, hsv.y, hsv.z);

      if(ColorEqApprox(e->color, e->defaultColor, 3)){
        ChangeHurtState(e, HURT_IDLE);
      }
    break;
    case HURT_IDLE:
    break;
  }
}

void SetTarget(Entity *e, const Vector2 target){
  e->target = target;
}

void DrawEntity(Entity *e){
  DrawCircleV(e -> pos, e -> r, e -> color);
}

// entity direction
void ToTarget(Entity *e, const Vector2 v){
  float dt = GetFrameTime();
  if(dt > 0.1f) dt = 0.1f;

  Vector2 cv = Vector2Normalize(v);

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

  p -> base.toTarget(&(p -> base), v);
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

  e -> base.toTarget(&(e->base), v);
}

// ==== game Loop function


void PressedEvent(Player *p){
  if(IsKeyPressed(KEY_SPACE)){
    ChangeHurtState(&(p->base), HURT_FLASH);
  }
}

void ShowInfo(const Player *p){
  if(IsKeyPressed(KEY_TAB)){
    printf("Health :%d\n", p->base.health);
    switch(p->base.hurtState){
      case HURT_IDLE:
        printf("hurt_idle: %d\n", p->base.stateTimer);
        break;
      case HURT_FLASH:
        printf("hurt_flash: %d\n", p->base.stateTimer);
        break;
      case HURT_PERSIST:
        printf("hurt_pesist: %d\n", p->base.stateTimer);
        break;
      case HURT_RECOVER:
        printf("hurt_recover: %d\n", p->base.stateTimer);
        break;
    }
    printf("target Color: %d %d %d, %d\n", p->base.targetColor.r, p->base.targetColor.g, p->base.targetColor.b, p->base.targetColor.a);
  }
}
// 指数趋紧
void MoveTowardsHSV(Vector3 *value, const Vector3 target, const float k){
  float dh = target.x - value->x;
  while(dh > 180.0f) dh -= 360.0f;
  while(dh < -180.0f) dh += 360.0f;
  value->x = fmodf(value->x + dh * k, 360.0f);
  if(value->x < 0.0f) value->x += 360.0f;

  value->y += (target.y - value->y) *k;
  if(fabsf(target.y - value->y) < 0.01f) value->y = target.y;

  value->z += (target.z - value->z) *k;
  if(fabsf(target.z - value->z) < 0.01f) value->z = target.z;

  // float diff = *value - base;
  // if(FloatEqRel(diff, 0.0f)) return;
  //
  // if(diff < 0){
  //   *value += step;
  // }else{
  //   *value -= step;
  // }
  // *value = CLAMP_A(*value, 0.0f, 360.0f);
}

#define ENEMY_NUMBER 2
#define ENEMY_LOOP(x)                  \
for(int i = 0; i < ENEMY_NUMBER; i++){ \
  x;                                   \
}                                      \

#define ENEMY_LOOPS(x)                 \
for(int i = 0; i < ENEMY_NUMBER; i++){ \
  x                                    \
}                                      \

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

  while(!WindowShouldClose()){
    // calc
    if(player.base.health <= 0) break;
    UpdateHurtEffect(&(player.base));
    ENEMY_LOOP(enemy[i].base.setTarget(&((enemy + i)->base),player.base.pos));

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
//TODO: 受伤颜色改变
