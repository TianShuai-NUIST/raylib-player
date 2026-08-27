#include <raylib.h>
#include <stdio.h>
#include <math.h>

#define REL_EPS 1e-9

const int windowWidth   = 600;
const int windowHeight  = 600;
const int originFPS     = 60;

const int playerR       = 25;
const int playerSpeed   = 200;
const int playerHealth  = 100;
const Color playerColor = GREEN;
const Vector2 playerSpawnPoint = (Vector2) {windowWidth / 2, windowHeight / 2};

const int enemyR        = 10;
const int enemySpeed    = 150;
const int enemyHealth   = 50;
const Color enemyColor  = RED;
const Vector2 enemySpawnPoint = (Vector2) {enemyR, enemyR};
const Vector2 enemyTarget = playerSpawnPoint;

#define PLAYER_INIT (Player) { \
  .e = (Entity){               \
    .pos = playerSpawnPoint,   \
    .r = playerR,              \
    .speed = playerSpeed,      \
    .health = playerHealth,    \
    .color = playerColor,      \
                               \
    .draw = DrawEntity,        \
  },                           \
    .move = MovePlayer,        \
}

#define ENEMY_INIT (Enemy) {   \
  .e = (Entity){               \
    .pos = enemySpawnPoint,    \
    .r = enemyR,               \
    .speed = enemySpeed,       \
    .health = enemyHealth,     \
    .color = enemyColor,       \
                               \
    .draw = DrawEntity,        \
  },                           \
  .target = enemyTarget,       \
                               \
  .move = navigateTarget,      \
  .in_target = in_target,      \
}

typedef struct Entity{
  Vector2 pos;
  int speed;
  int r;
  int health;
  Color color;

//   void (*move)(struct Entity *e, float dt);
  void (*draw)(struct Entity *e);
} Entity;

typedef struct Player{
  Entity e;

  void (*move)(struct Entity *e, float dt);
} Player;

typedef struct Enemy{
  Entity e;
  Vector2 target;

  void (*move)(struct Enemy *e, float dt);
  bool (*in_target)(struct Enemy *e);
} Enemy;

// ==== Utils
void clampf(float *value, float min, float max){
  if(*value >= max) *value = max;
  if(*value <= min) *value = min;
}

// 浮点数相等判断,
// |a - b| <= max(|a|, |b|) * rel_eps
bool floatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return true;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= REL_EPS * max_val;
}

double Vec2distance(Vector2 start, Vector2 end){
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

void MoveEntity(Entity *e,const Vector2 *v, float dt){
  if(dt > 0.1f) dt = 0.1f;

  Vector2 cv = *v;

  float length = sqrtf(v->x * v->x + v->y * v->y);
  if(length > 0.0f){
    cv.x = v->x / length;
    cv.y = v->y / length;
  }

  e -> pos.x += dt * e -> speed * cv.x;
  e -> pos.y += dt * e -> speed * cv.y;

  clampf(&(e -> pos.x), e -> r, windowWidth - e -> r);
  clampf(&(e -> pos.y), e -> r, windowHeight - e -> r);
}

void MovePlayer(Entity *e, float dt){
  Vector2 v = (Vector2){0, 0};
  if(IsKeyDown(KEY_A)) v.x -= 1.0f;
  if(IsKeyDown(KEY_D)) v.x += 1.0f;
  if(IsKeyDown(KEY_S)) v.y += 1.0f;
  if(IsKeyDown(KEY_W)) v.y -= 1.0f;

  MoveEntity(e, &v, dt);
}

bool in_target(struct Enemy *e){
  // if(floatEqRel(e->e.pos.x, e->target.x) &&
  //    floatEqRel(e->e.pos.x, e->target.x))
  if(Vec2distance(e->e.pos, e->target) < REL_EPS) return true;
  else return false;
}

void navigateTarget(struct Enemy *e, float dt){
  if(e->in_target(e)) return;

  Vector2 v;
  v.x = e->target.x - e->e.pos.x;
  v.y = e->target.y - e->e.pos.y;

  // 如果距离小于步长, 直接到坐标上(防止抖动)
  float remaining = sqrt(v.x * v.x + v.y * v.y);
  if(remaining <= e->e.speed * dt){
    e->e.pos = e->target;
  }

  MoveEntity(&(e->e), &v, dt);
}

// ==== game Loop function

// main loop
int main(void){
  Player player = PLAYER_INIT;
  Enemy enemy = ENEMY_INIT;
  float dt = 0;

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){
    // calc
    dt = GetFrameTime();
    enemy.target = player.e.pos;

    // event
    ChangeFPS();
    player.move(&(player.e), dt);
    enemy.move(&enemy,dt);

    // draw
    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(20, 20);
    player.e.draw(&(player.e));
    enemy.e.draw(&(enemy.e));
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
