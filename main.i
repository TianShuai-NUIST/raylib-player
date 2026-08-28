typedef struct Entity Entity;
typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth = 600;
const int windowHeight = 600;
const int originFPS = 60;

void ToTarget (struct Entity*, const Vector2 *);
_Bool InTarget (struct Entity*);
void DrawEntity (struct Entity*);

typedef void (*toTargetFunc) (struct Entity*, const Vector2 *);
typedef _Bool (*inTargetFunc) (struct Entity*);
typedef void (*drawFunc) (struct Entity*);

const int player_r                 = 25;
const Vector2 player_vel           = ((Vector2){100, 100});
const int player_health            = 100;
const Color player_color           = ((Color){ 0, 228, 48, 255 });
const Vector2 player_spawnPoint    = ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2});
const Vector2 player_target        = ((Vector2){-1, -1});
const Vector2 player_pos           = player_spawnPoint;;
const toTargetFunc player_toTarget = ToTarget;
const inTargetFunc player_inTarget = InTarget;
const drawFunc player_draw         = DrawEntity;;

const int enemy_r                  = 10;
const Vector2 enemy_vel            = ((Vector2){50, 50});
const int enemy_health             = 50;
const Color enemy_color            = ((Color){ 230, 41, 55, 255 });
const Vector2 enemy_spawnPoint     = ((Vector2) {enemy_r, enemy_r});
const Vector2 enemy_target         = player_spawnPoint;
const Vector2 enemy_pos            = enemy_spawnPoint;
;
const toTargetFunc enemy_toTarget  = ToTarget;
const inTargetFunc enemy_inTarget  = InTarget;
const drawFunc enemy_draw          = DrawEntity;
;

typedef struct Entity{
  int r;
  Vector2 vel;
  int health;
  Color color;
  Vector2 spawnPoint;
  Vector2 target;
  Vector2 pos;
  ;
  toTargetFunc toTarget;
  inTargetFunc inTarget;
  drawFunc draw;
  ;
} Entity;

typedef struct Player{
  Entity base;
  _Bool cursor;

  void (*move)(struct Player *p);
} Player;

typedef struct Enemy{
  Entity base;

  void (*move)(struct Enemy *e);
} Enemy;

int RandRange(const int min, const int max){
  ((void) sizeof ((min < max) ? 1 : 0), __extension__ ({ if (min < max) ; else __assert_fail ("min < max", "main.c", 114, __extension__ __PRETTY_FUNCTION__); }));
  if (min == max) return min;
  else return rand() % (max - min) + min;
}

void ClampF(float *value, float min, float max){
  if(*value > max) *value = max;
  else if(*value < min) *value = min;
  else return;
}

_Bool FloatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return 1;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= 1e-9 * max_val;
}

double Vec2Distance(Vector2 start, Vector2 end){
  double dx = start.x - end.x;
  double dy = start.y - end.y;
  return sqrt(dx * dx + dy * dy);
}

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

_Bool InTarget(Entity *e){
  if(Vec2Distance(e->pos, e->target) < 1e-9) return 1;
  else return 0;
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

  float remaining = sqrt(v.x * v.x + v.y * v.y);
  if(remaining <= Vector2Length(e->base.vel) * dt){
    e->base.pos = e->base.target;
    return;
  }

  e -> base.toTarget(&(e->base), &v);
}

int main(void){
  srand(time(((void*)0)));


  Player player = (Player){
    .base = (Entity){
      .r          = player_r,
      .vel        = player_vel,
      .health     = player_health,
      .color      = player_color,
      .spawnPoint = player_spawnPoint,
      .target     = player_target,
      .pos        = player_pos,
      .toTarget   = player_toTarget,
      .inTarget   = player_inTarget,
      .draw       = player_draw,
    },
    .cursor = 0,
    .move = MovePlayer,
  };

  Enemy enemy[1];
  for(int i = 0; i < 1; i++){

    enemy[i] = (Enemy){
      .base =(Entity){
        .r          = enemy_r,
        .vel        = enemy_vel,
        .health     = enemy_health,
        .color      = enemy_color,
        .spawnPoint = enemy_spawnPoint,
        .target     = enemy_target,
        .pos        = enemy_pos,
        .toTarget   = enemy_toTarget,
        .inTarget   = enemy_inTarget,
        .draw       = enemy_draw,
      },
      .move = MoveEnemy,
    };

    float r = enemy[i].base.r;
    enemy[i].base.pos.x = RandRange(r, windowWidth - r);
    enemy[i].base.pos.y = RandRange(r, windowHeight - r);
  }

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){

    for(int i = 0; i < 1; i++){
      do{
        enemy[i].base.target = player.base.pos;
      }while(0);
    };


    ChangeFPS();
    player.move(&player);
    for(int i = 0; i < 1; i++){
      do{
        enemy[i].move(enemy + i);
      }while(0);
    };

    BeginDrawing();
    ClearBackground((Color){ 0, 0, 0, 255 });
    DrawFPS(20, 20);
    player.base.draw(&(player.base));
    for(int i = 0; i < 1; i++){
      do{
        enemy[i].base.draw(&(enemy[i].base));
      }while(0);
    };
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
