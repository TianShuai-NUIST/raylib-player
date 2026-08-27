# Info

基于raylib

# 动画问题

1. 写死: 受帧率影响
  解决方案: 移动与帧率挂钩
2. 斜方向速度1.4倍

# 架构

```mermaid
classDiagram
    class Entity{
        + Pos
        + r
        + speed
        + health
        + color
        + draw() void
    }

    class Enemy{
        + move(float dt) void
        + in_target() bool
    }
    class Player{
        + move(float dt) void
    }
    Entity <|-- Enemy
    Entity <|-- Player
```
