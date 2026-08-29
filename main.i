typedef struct Entity Entity;
typedef enum HurtState {
  HURT_FLASH,
  HURT_PERSIST,
  HURT_RECOVER,
  HURT_IDLE,
} HurtState;

typedef struct Player Player;
typedef struct Enemy Enemy;

const int windowWidth = 600;
const int windowHeight = 600;
const Vector2 wSize = (Vector2){(float)windowWidth, (float)windowHeight};
const int originFPS = 60;

void MoveTowards(float *value, const float base, const float step);
void ToTarget (struct Entity*, const Vector2);
_Bool InTarget (struct Entity*);
void SetTarget (struct Entity*, const Vector2);
void DrawEntity (struct Entity*);

typedef void (*toTargetFunc) (struct Entity*, const Vector2);
typedef _Bool (*inTargetFunc) (struct Entity*);
typedef void (*setTargetFunc) (struct Entity*, const Vector2);
typedef void (*drawFunc) (struct Entity*);

const int player_r                   = 25;
const Vector2 player_vel             = ((Vector2){150, 150});
const int player_health              = 100;
const float player_mass              = 10;
const Color player_color             = ((Color){ 0, 228, 48, 255 });
const Color player_defaultColor      = ((Color){ 0, 228, 48, 255 });
const Vector2 player_spawnPoint      = ((Vector2) {(float)windowWidth / 2, (float)windowHeight / 2});
const Vector2 player_target          = ((Vector2){-1, -1});
const Vector2 player_pos             = player_spawnPoint;
const HurtState player_hurtState     = HURT_IDLE;const int player_stateTimer                          = 0;const Color player_targetColor = player_color;;
const toTargetFunc player_toTarget   = ToTarget;
const inTargetFunc player_inTarget   = InTarget;
const setTargetFunc player_setTarget = SetTarget;
const drawFunc player_draw           = DrawEntity;;

const int enemy_r                    = 10;
const Vector2 enemy_vel              = ((Vector2){120, 120});
const int enemy_health               = 50;
const float enemy_mass               = 5;
const Color enemy_color              = ((Color){ 230, 41, 55, 255 });
const Color enemy_defaultColor       = ((Color){ 230, 41, 55, 255 });
const Vector2 enemy_spawnPoint       = ((Vector2) {enemy_r, enemy_r});
const Vector2 enemy_target           = player_spawnPoint;
const Vector2 enemy_pos              = enemy_spawnPoint;
const HurtState enemy_hurtState      = HURT_IDLE;const int enemy_stateTimer                           = 0;const Color enemy_targetColor  = enemy_color;;

const toTargetFunc enemy_toTarget    = ToTarget;
const inTargetFunc enemy_inTarget    = InTarget;
const setTargetFunc enemy_setTarget  = SetTarget;
const drawFunc enemy_draw            = DrawEntity;;


typedef struct Entity{
  int r;
  Vector2 vel;
  int health;
  float mass;
  Color color;
  Color defaultColor;
  Vector2 spawnPoint;
  Vector2 target;
  Vector2 pos;
  HurtState hurtState;
  int stateTimer;
  Color targetColor;
  ;
  toTargetFunc toTarget;
  inTargetFunc inTarget;
  setTargetFunc setTarget;
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
# 138 "main.c"
int RandRange(const int min, const int max){
  ((void) sizeof ((min < max) ? 1 : 0), __extension__ ({ if (min < max) ; else __assert_fail ("min < max", "main.c", 139, __extension__ __PRETTY_FUNCTION__); }));
  if (min == max) return min;
  else return rand() % (max - min) + min;
}

void ClampV2(Vector2 *value, const Vector2 min,const Vector2 max){
  float vx = Clamp(value->x,min.x, max.x);
  float vy = Clamp(value->y,min.y, max.y);
  *value = (Vector2){vx, vy};
}



_Bool FloatEqRel(double a, double b){
  if(a == 0.0 && b == 0.0) return 1;

  double diff = fabs (a - b);
  double max_val = fmax(fabs(a), fabs(b));

  return diff <= 1e-9 * max_val;
}

_Bool ColorEq(Color c1, Color c2){
  return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b && c1.a == c2.a);
}

_Bool ColorEqApprox(Color c1, Color c2, unsigned char tolerance){
  int dr = (int)c1.r - (int)c2.r;
  int dg = (int)c1.g - (int)c2.g;
  int db = (int)c1.b - (int)c2.b;
  int da = (int)c1.a - (int)c2.a;

  return ( dr <= tolerance && dg <= tolerance && db <= tolerance && da <= tolerance );
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
      e->targetColor = ColorFromHSV(350.0f, 0.6f, 0.7f);
    break;
    case HURT_RECOVER:
      e->stateTimer = 20;
      e->targetColor = player_defaultColor;
    break;
  }
}

void UpdateHurtEffect(Entity *e){
  if(e->hurtState == HURT_IDLE) return;

  Vector3 hsv = ColorToHSV(e->color);

  switch(e->hurtState){
    case HURT_FLASH:
      e->color = (Color){ 255, 255, 255, 255 };
      e->stateTimer--;
      if(e->stateTimer == 0) ChangeHurtState(e, HURT_PERSIST);
    break;
    case HURT_PERSIST:
      MoveTowards(&(hsv.x), ColorToHSV(e->targetColor).x, 8.0f);
      e->color = ColorFromHSV(hsv.x, hsv.y, hsv.z);

      e->stateTimer--;
      if(e->stateTimer == 0) ChangeHurtState(e, HURT_RECOVER);
    break;
    case HURT_RECOVER:
      MoveTowards(&(hsv.x), ColorToHSV(e->targetColor).x, 1.5f);
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


void ToTarget(Entity *e, const Vector2 v){
  float dt = GetFrameTime();
  if(dt > 0.1f) dt = 0.1f;

  Vector2 cv = Vector2Normalize(v);

  e->pos = Vector2Add(e->pos, Vector2Scale(Vector2Multiply(e->vel, cv), dt));

  Vector2 vr = (Vector2){e->r, e->r};
  ClampV2(&(e->pos), vr, Vector2Subtract(wSize, vr));
}

_Bool InTarget(Entity *e){
  if(Vector2Distance(e->pos, e->target) < 1e-9) return 1;
  else return 0;
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


  float remaining = Vector2Length(v);
  if(remaining <= Vector2Length(e->base.vel) * dt){
    e->base.pos = e->base.target;
    return;
  }

  e -> base.toTarget(&(e->base), v);
}




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
    printf("target Color: %f %f %f, %f\n", p->base.targetColor.r, p->base.targetColor.g, p->base.targetColor.b, p->base.targetColor.a);
  }
}
void MoveTowards(float *value, const float base, const float step){
  float diff = *value - base;
  if(diff == 0) return;

  if(diff < 0){
    *value += step;
  }else{
    *value -= step;
  }

}
# 348 "main.c"
int main(void){
  srand(time(((void*)0)));


  Player player = (Player){ .base =(Entity){ .r = player_r, .vel = player_vel, .health = player_health, .mass = player_mass, .color = player_color, .defaultColor = player_defaultColor, .spawnPoint = player_spawnPoint, .target = player_target, .pos = player_pos, .hurtState = player_hurtState,.stateTimer = player_stateTimer,.targetColor = player_targetColor, .toTarget = player_toTarget, .inTarget = player_inTarget, .setTarget = player_setTarget, .draw = player_draw, }, .cursor = 0, .move = MovePlayer, };


  Enemy enemy[2];
  for(int i = 0; i < 2; i++){

    enemy[i] = (Enemy){ .base =(Entity){ .r = enemy_r, .vel = enemy_vel, .health = enemy_health, .mass = enemy_mass, .color = enemy_color, .defaultColor = enemy_defaultColor, .spawnPoint = enemy_spawnPoint, .target = enemy_target, .pos = enemy_pos, .hurtState = enemy_hurtState,.stateTimer = enemy_stateTimer,.targetColor = enemy_targetColor, .toTarget = enemy_toTarget, .inTarget = enemy_inTarget, .setTarget = enemy_setTarget, .draw = enemy_draw, }, .move = MoveEnemy, };

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

    if(player.base.health <= 0) break;
    UpdateHurtEffect(&(player.base));
    for(int i = 0; i < 2; i++){ enemy[i].base.setTarget(&((enemy + i)->base),player.base.pos); };


    ChangeFPS();
    ShowInfo(&player);
    PressedEvent(&player);


    player.move(&player);
    for(int i = 0; i < 2; i++){ enemy[i].move(enemy + i); };


    BeginDrawing();
    ClearBackground((Color){ 0, 0, 0, 255 });
    DrawFPS(20, 20);
    player.base.draw(&(player.base));
    for(int i = 0; i < 2; i++){ enemy[i].base.draw(&(enemy[i].base)); };
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
