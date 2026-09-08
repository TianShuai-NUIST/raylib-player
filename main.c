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
  HURT_PERSIST, // 疼痛暗红
  HURT_RECOVER, // 恢复颜色
  HURT_IDLE,    // 正常
} HurtState;

typedef struct EntityVtbl{
  void (*move)(Entity *self, float dt);
  void (*draw)(Entity *self);
} EntityVtbl;

typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth  = 600;
const int windowHeight = 600;
const Vector2 wSize    = (Vector2){(float)windowWidth, (float)windowHeight};
const int originFPS    = 60;

void MoveTowardsHSV(Vector3 *value, const Vector3 target, const float k);

// (TYPE, NAME, PLAYER_VALUE, ENEMY_VALUE)
#define ENTITY_ATTRI_TABLE                                     \
XX(Vector2, vel, ((Vector2){0, 0}), ((Vector2){0, 0}))         \
XX(Vector2, velRange, ((Vector2){0, 300}), ((Vector2){0, 280}))\
XX(Vector2, accel, ((Vector2){10, 10}), ((Vector2){95, 95})) \
XX(Vector2, friction, ((Vector2){200, 200}), ((Vector2){50, 50}))\
XX(float, mass, 10, 5)                                         \
                                                               \
XX(int, r, 25, 10)                                             \
                                                               \
XX(Color, color, (GREEN), (RED))                               \
XX(Color, defaultColor, (GREEN), (RED))                        \
                                                               \
XX(int, health, 100, 50)                                       \
XX(Vector2, spawnPoint,                                        \
((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2}), \
((Vector2) {enemy_r, enemy_r}))                                \
XX(Vector2, target, ((Vector2){-1, -1}), player_spawnPoint)    \
XX(Vector2, pos, player_spawnPoint,                            \
enemy_spawnPoint)                                              \
XX(Vector2, renderPos, player_pos,                             \
enemy_pos)                                                     \
                                                               \
XX(HurtState, hurtState, HURT_IDLE, HURT_IDLE)                 \
XX(int, stateTimer, 0, 0)                                      \
XX(Color, targetColor, player_color, enemy_color)              \
/*
 * pos: p
 * vel: p/s -> dp = vel % dt
 * accel: p/s^2 -> dv = accel * dt
 * */

// const value With Player
#define XX(type, name, value, ...) const type player_##name = value;
ENTITY_ATTRI_TABLE;
#undef XX

// const value With Enemy
#define XX(type, name, _, value) const type enemy_##name = value;
ENTITY_ATTRI_TABLE;
#undef XX

typedef struct Entity{
  const EntityVtbl *vtbl;
#define XX(type, name, _, ...) type name;
  ENTITY_ATTRI_TABLE;
#undef XX
} Entity;

typedef struct Player{
  Entity base;
  bool cursor;
} Player;

typedef struct Enemy{
  Entity base;
} Enemy;

#define REL_EPS 1e-9

// Init Player Struct
// 实际上这里的X宏没有任何作用, 只是作为一个标记存在而已
#define XX(type, name, ...) .name = player_##name,
#define PLAYER_INIT (Player){   \
  .base =(Entity){              \
    .vtbl = &PLAYER_VTBL,        \
    ENTITY_ATTRI_TABLE          \
  },                            \
  .cursor = false,              \
}
#undef XX

// Init Enemy Struct
#define XX(type, name, ...) .name = enemy_##name,
#define ENEMY_INIT (Enemy){ \
  .base =(Entity){          \
    .vtbl = &ENEMY_VTBL,    \
    ENTITY_ATTRI_TABLE      \
  },                        \
}
#undef XX

// ==== Utils

int RandRange(const int min, const int max){
  assert(min < max);
  if (min == max) return min;
  else return rand() % (max - min) + min;
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

// argument: entity direction+distance dt
void ToTarget(Entity *e, const Vector2 direction, float dt){
  if(dt > 0.1f) dt = 0.1f;

  // friction & accel 是外部改变的参数, 由我们设定, 与状态无关, 如果不使用新的局部变量, 那么行为就不会被我们控制
  if(Vector2Length(direction) <= Vector2Length(Vector2Scale(e->vel, dt))){
    Vector2 friction = Vector2Scale(Vector2Normalize(e->vel), Vector2Length(e->friction));
    e->vel = Vector2Subtract(e->vel, Vector2Scale(friction, dt));
    if(Vector2Length(e->vel) <= Vector2Length(Vector2Scale(friction, dt))){
      e->vel = Vector2Zero();
    }
  } else {
    Vector2 accel = Vector2Scale(Vector2Normalize(direction), Vector2Length(e->accel));
    e->vel = Vector2Add(e->vel, Vector2Scale(accel, dt));
  }

  e->vel = Vector2ClampValue(e->vel, e->velRange.x, e->velRange.y);
  e->pos = Vector2Add(e->pos,Vector2Scale(e->vel, dt));

  Vector2 vr = (Vector2){e->r, e->r};
  e->pos = Vector2Clamp(e->pos, vr, Vector2Subtract(wSize, vr));
}

bool InTarget(Entity *e){
  if(Vector2Distance(e->pos, e->target) < REL_EPS) return true;
  else return false;
}

void MovePlayer(Entity *e, float dt){
  Vector2 v = (Vector2){0, 0};
  if(IsKeyDown(KEY_A)) v.x -= 1.0f;
  if(IsKeyDown(KEY_D)) v.x += 1.0f;
  if(IsKeyDown(KEY_S)) v.y += 1.0f;
  if(IsKeyDown(KEY_W)) v.y -= 1.0f;

  // 设置假想目标, 在单位1距离处.
  v = Vector2Normalize(v);

  ToTarget(e, v, dt);
}

void MoveEnemy(Entity *e, float dt){
  if(InTarget(e)) return;

  Vector2 v;
  v = Vector2Subtract(e->target, e->pos);

  // 如果距离小于步长, 直接到坐标上(防止抖动)
  // float remaining = Vector2Length(v);
  // if(remaining <= Vector2Length(e->vel) * dt){
  //   e->pos = e->target;
  //   return;
  // }

  ToTarget(e, v, dt);
}

const EntityVtbl PLAYER_VTBL = (EntityVtbl){ .move = MovePlayer, .draw = DrawEntity, };
const EntityVtbl ENEMY_VTBL  = (EntityVtbl){ .move = MoveEnemy, .draw = DrawEntity, };

// ==== game Loop function

void PressedEvent(Player *p){
  if(IsKeyPressed(KEY_SPACE)){
    ChangeHurtState(&(p->base), HURT_FLASH);
  }
}

void ShowInfo(const Player *p){
  if(IsKeyPressed(KEY_TAB)){
    printf("Health :%d\n", p->base.health);
    printf("target Color: %d %d %d, %d\n", p->base.targetColor.r, p->base.targetColor.g, p->base.targetColor.b, p->base.targetColor.a);
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
  for(int i = 0; i < ENEMY_NUMBER; i++){
    entityList[i + 1] = &(enemy[i].base);
  }

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){

    float dt = GetFrameTime();

    // calc
    if(player.base.health <= 0) break;
    UpdateHurtEffect(&(player.base));
    for(size_t i = 1; i < ENEMY_NUMBER + 1; i++){
      SetTarget(entityList[i], player.base.pos);
    }

    // event
    ChangeFPS();
    ShowInfo(&player);
    PressedEvent(&player);

    // cotrol
    for(size_t i = 0; i < ENEMY_NUMBER + 1; i++){
      entityList[i]->vtbl->move(entityList[i], dt);
    }

    // draw
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(20, 20);
    for(size_t i = 0; i < ENEMY_NUMBER + 1; i++){
      entityList[i]->vtbl->draw(entityList[i]);
    }
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

//TODO: 添加碰撞
//Done: 受伤颜色改变
