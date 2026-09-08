typedef struct Entity Entity;

typedef enum HurtState {
  HURT_FLASH,
  HURT_PERSIST,
  HURT_RECOVER,
  HURT_IDLE,
} HurtState;

typedef struct EntityVtbl{
  void (*move)(Entity *self, float dt);
  void (*draw)(Entity *self);
} EntityVtbl;

typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth = 600;
const int windowHeight = 600;
const Vector2 wSize = (Vector2){(float)windowWidth, (float)windowHeight};
const int originFPS = 60;

void MoveTowardsHSV(Vector3 *value, const Vector3 target, const float k);
void ToTarget (struct Entity*, const Vector2);
bool InTarget (struct Entity*);
void SetTarget (struct Entity*, const Vector2);
void DrawEntity (struct Entity*);




typedef void (*toTargetFunc) (struct Entity*, const Vector2);
typedef bool (*inTargetFunc) (struct Entity*);
typedef void (*setTargetFunc) (struct Entity*, const Vector2);
typedef void (*drawFunc) (struct Entity*);




const Vector2 player_vel             = ((Vector2){150, 150});
const Vector2 player_maxVel          = ((Vector2){200, 200});
const Vector2 player_accel           = ((Vector2){1, 1});
const float player_mass              = 10;
const int player_r                   = 25;
const Color player_color             = ( (Color){ 0, 228, 48, 255 });
const Color player_defaultColor      = ( (Color){ 0, 228, 48, 255 });
const int player_health              = 100;
const Vector2 player_spawnPoint      = ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2});
const Vector2 player_target          = ((Vector2){-1, -1});
const Vector2 player_pos             = player_spawnPoint;
const Vector2 player_renderPos       = player_pos;
const HurtState player_hurtState     = HURT_IDLE;
const int player_stateTimer          = 0;
const Color player_targetColor       = player_color;;
const toTargetFunc player_toTarget   = ToTarget;
const inTargetFunc player_inTarget   = InTarget;
const setTargetFunc player_setTarget = SetTarget;
const drawFunc player_draw           = DrawEntity;;




const Vector2 enemy_vel         = ((Vector2){120, 120});
const Vector2 enemy_maxVel      = ((Vector2){170, 170});
const Vector2 enemy_accel       = ((Vector2){1, 1});
const float enemy_mass          = 5;
const int enemy_r               = 10;
const Color enemy_color         = ( (Color){ 230, 41, 55, 255 });
const Color enemy_defaultColor  = ( (Color){ 230, 41, 55, 255 });
const int enemy_health          = 50;
const Vector2 enemy_spawnPoint  = ((Vector2) {enemy_r, enemy_r});
const Vector2 enemy_target      = player_spawnPoint;
const Vector2 enemy_pos         = enemy_spawnPoint;
const Vector2 enemy_renderPos   = enemy_pos;
const HurtState enemy_hurtState = HURT_IDLE;
const int enemy_stateTimer      = 0;
const Color enemy_targetColor   = enemy_color;;


const toTargetFunc enemy_toTarget = ToTarget;
const inTargetFunc enemy_inTarget = InTarget;
const setTargetFunc enemy_setTarget = SetTarget;
const drawFunc enemy_draw = DrawEntity;;


typedef struct Entity{
  const EntityVtbl *vtbl;

  Vector2 vel;
  Vector2 maxVel;
  Vector2 accel;
  float mass;
  int r;
  Color color;
  Color defaultColor;
  int health;
  Vector2 spawnPoint;
  Vector2 target;
  Vector2 pos;
  Vector2 renderPos;
  HurtState hurtState;
  int stateTimer;
  Color targetColor;;
  toTargetFunc toTarget;
  inTargetFunc inTarget;
  setTargetFunc setTarget;
  drawFunc draw;;
} Entity;

typedef struct Player{
  Entity base;
  bool cursor;
} Player;

typedef struct Enemy{
  Entity base;
} Enemy;

int RandRange(const int min, const int max){
  ((void) sizeof (__assert_single_arg (
      min < max
      )), __extension__ ({ if (
        min < max
        ) ; else __assert_fail (
          "min < max"
          , "main.c", 160, __extension__ __PRETTY_FUNCTION__); }))
    if (min == max) return min;
    else return rand() % (max - min) + min;
}

void ClampV2(Vector2 *value, const Vector2 min,const Vector2 max){
  float vx = Clamp(value->x,min.x, max.x);
  float vy = Clamp(value->y,min.y, max.y);
  *value = (Vector2){vx, vy};
}



bool FloatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return true;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= 1e-9 * max_val;
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

      e->targetColor = e->defaultColor;
    break;
  }
}

void UpdateHurtEffect(Entity *e){
  if(e->hurtState == HURT_IDLE) return;

  Vector3 hsv = ColorToHSV(e->color);

  switch(e->hurtState){
    case HURT_FLASH:
      e->color = 
# 239 "main.c" 3
                (Color){ 255, 255, 255, 255 }
# 239 "main.c"
                     ;
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


void ToTarget(Entity *e, const Vector2 direction){
  float dt = GetFrameTime();
  if(dt > 0.1f) dt = 0.1f;
  Vector2 nd = Vector2Normalize(direction);
# 284 "main.c"
  e->pos.x += e->vel.x * nd.x * dt;
  e->pos.y += e->vel.y * nd.y * dt;

  Vector2 vr = (Vector2){e->r, e->r};
  ClampV2(&(e->pos), vr, Vector2Subtract(wSize, vr));
}

bool InTarget(Entity *e){
  if(Vector2Distance(e->pos, e->target) < 1e-9) return true;
  else return false;
}

void MovePlayer(Entity *e, float dt){
  (void)dt;
  Vector2 v = (Vector2){0, 0};
  if(IsKeyDown(KEY_A)) v.x -= 1.0f;
  if(IsKeyDown(KEY_D)) v.x += 1.0f;
  if(IsKeyDown(KEY_S)) v.y += 1.0f;
  if(IsKeyDown(KEY_W)) v.y -= 1.0f;

  e->toTarget(e, v);
}

void MoveEnemy(Entity *e, float dt){
  if(e->inTarget(e)) return;

  Vector2 v;
  v = Vector2Subtract(e->target, e->pos);


  float remaining = Vector2Length(v);
  if(remaining <= Vector2Length(e->vel) * dt){
    e->pos = e->target;
    return;
  }

  e->toTarget(e, v);
}

const EntityVtbl PLAYER_VTBL = (EntityVtbl){ .move = MovePlayer, .draw = DrawEntity, };
const EntityVtbl ENEYM_VTBL = (EntityVtbl){ .move = MovePlayer, .draw = DrawEntity, };




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
# 378 "main.c"
}
# 392 "main.c"
int main(void){
  srand(time(
# 393 "main.c" 3
            ((void *)0)
# 393 "main.c"
                ));


  Player player = (Player){ .base =(Entity){ .vtbl = &PLAYER_VTBL .vel = player_vel, .maxVel = player_maxVel, .accel = player_accel, .mass = player_mass, .r = player_r, .color = player_color, .defaultColor = player_defaultColor, .health = player_health, .spawnPoint = player_spawnPoint, .target = player_target, .pos = player_pos, .renderPos = player_renderPos, .hurtState = player_hurtState, .stateTimer = player_stateTimer, .targetColor = player_targetColor, .toTarget = player_toTarget, .inTarget = player_inTarget, .setTarget = player_setTarget, .draw = player_draw, }, .cursor = false, };


  Enemy enemy[2];
  for(int i = 0; i < 2; i++){

    enemy[i] = (Enemy){ .base =(Entity){ .vtbl = &ENEMY_VTBL .vel = enemy_vel, .maxVel = enemy_maxVel, .accel = enemy_accel, .mass = enemy_mass, .r = enemy_r, .color = enemy_color, .defaultColor = enemy_defaultColor, .health = enemy_health, .spawnPoint = enemy_spawnPoint, .target = enemy_target, .pos = enemy_pos, .renderPos = enemy_renderPos, .hurtState = enemy_hurtState, .stateTimer = enemy_stateTimer, .targetColor = enemy_targetColor, .toTarget = enemy_toTarget, .inTarget = enemy_inTarget, .setTarget = enemy_setTarget, .draw = enemy_draw, }, };

    float r = enemy[i].base.r;
    enemy[i].base.pos.x = RandRange(r, windowWidth - r);
    enemy[i].base.pos.y = RandRange(r, windowHeight - r);
  }

  Entity *entityList[2 + 1];
  entityList[0] = &(player.base);
  for(int i = 1; i < 2 + 1; i++){
    entityList[i] = &((enemy + i)->base);
  }

  InitWindow(windowWidth, windowHeight, "Moving..");
  SetTargetFPS(60);

  while(!WindowShouldClose()){

    float dt = GetFrameTime();


    if(player.base.health <= 0) break;
    UpdateHurtEffect(&(player.base));
    for(int i = 0; i < 2; i++){ enemy[i].base.setTarget(&((enemy + i)->base),player.base.pos); };


    ChangeFPS();
    ShowInfo(&player);
    PressedEvent(&player);


    player.move(&(player.base), dt);
    for(int i = 0; i < 2; i++){ enemy[i].move(&((enemy + i)->base), dt); };


    BeginDrawing();
    ClearBackground(
# 438 "main.c" 3
                   (Color){ 0, 0, 0, 255 }
# 438 "main.c"
                        );
    DrawFPS(20, 20);
    player.base.draw(&(player.base));
    for(int i = 0; i < 2; i++){ enemy[i].base.draw(&(enemy[i].base)); };
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
