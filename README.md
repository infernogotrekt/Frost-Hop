# Frost Hop

SIT102 Introduction to Programming project. A side-scrolling platformer built
with C++ and SplashKit, where freezing enemies turns them into platforms.

## The idea

Pip, a small explorer, runs through three snowy levels. Grumbles are walking
snowmen. The Frost Flower lets Pip throw snowballs that freeze a Grumble into
a solid ice block for about five seconds. Pip can stand on it, so an enemy
becomes a step up to a ledge or a bridge over a pit.

## Controls

| Key | Action |
| --- | --- |
| A / D or Left / Right | Run |
| W, Up or Space | Jump (hold for a higher jump) |
| J | Throw a snowball (needs the Frost Flower) |
| Enter | Start, continue, return to title |

## Building and running

Needs SplashKit and the `skm` tool. From the repository root:

    mkdir -p bin
    skm g++ src/*.cpp -o bin/frost_hop
    ./bin/frost_hop

Run it from the repository root, because the game loads `levels/level1.txt` and the others by relative path.

## Repository layout

    src/frost_hop.h       constants, enums, structs and declarations
    src/program.cpp       window and game loop
    src/game.cpp          screen state machine, level flow, HUD
    src/physics.cpp       the shared collision rule (move_and_collide)
    src/level.cpp         level loading, tile queries, drawing tiles
    src/player.cpp        Pip: input, jumping, damage, drawing
    src/camera.cpp        one-way scrolling camera
    src/items.cpp         question blocks, coins, Frost Flower pickup
    src/enemies.cpp       Grumbles: walking, freezing, thawing, stomping
    src/snowballs.cpp     the snowball pool
    levels/               level1.txt to level3.txt

## Level file format

One character per 32-pixel tile, 15 rows tall:

    #  ground            ?  question block (coin)
    f  question block with a Frost Flower
    o  coin              ^  spikes
    F  flag pole         g  Grumble
    P  Pip's start       .  empty
