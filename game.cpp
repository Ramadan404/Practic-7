// Monster Battle on raylib
#include "raylib.h"
#include "raymath.h"
#include <cmath>
#include <ctime>

// Structures
struct Player {
    Vector2 position;
    int health;
    float speed;
    Texture2D texture;
    float attackCooldown;
};

struct Monster {
    Vector2 position;
    int health;
    float speed;
    Texture2D texture;
    bool isAlive;
};

// Game states
enum GameState {
    MENU,
    GAME,
    GAME_OVER
};

int main() {
    // Window initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Monster Battle");
    InitAudioDevice();

    // Load resources
    Texture2D playerTexture = LoadTexture("resources/player.png");
    Texture2D monsterTexture = LoadTexture("resources/monster.png");
    Texture2D backgroundTexture = LoadTexture("resources/background.png");
    
    Sound attackSound = LoadSound("resources/attack.wav");
    Music backgroundMusic = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(backgroundMusic);

    // Game objects
    Player player = {{100, 360}, 100, 300.0f, playerTexture, 0.0f};
    Monster monster = {{900, 360}, 100, 100.0f, monsterTexture, true};

    GameState currentState = MENU;
    bool gameResult = false; // true - victory, false - defeat

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateMusicStream(backgroundMusic);

        // State management
        switch (currentState) {
        case MENU:
            if (IsKeyPressed(KEY_ENTER)) currentState = GAME;
            break;
        case GAME:
            // Player controls
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed * deltaTime;
            if (IsKeyDown(KEY_LEFT))  player.position.x -= player.speed * deltaTime;
            if (IsKeyDown(KEY_UP))    player.position.y -= player.speed * deltaTime;
            if (IsKeyDown(KEY_DOWN))  player.position.y += player.speed * deltaTime;

            // Keep player within window bounds
            if (player.position.x < 0) player.position.x = 0;
            if (player.position.x > screenWidth - player.texture.width) player.position.x = screenWidth - player.texture.width;
            if (player.position.y < 0) player.position.y = 0;
            if (player.position.y > screenHeight - player.texture.height) player.position.y = screenHeight - player.texture.height;

            // Monster AI
            if (monster.isAlive) {
                Vector2 direction = Vector2Subtract(player.position, monster.position);
                float length = Vector2Length(direction);
                if (length != 0) direction = Vector2Scale(direction, 1.0f/length);
                monster.position = Vector2Add(monster.position, Vector2Scale(direction, monster.speed * deltaTime));
            }

            // Attack cooldown
            if (player.attackCooldown > 0.0f) {
                player.attackCooldown -= deltaTime;
            }

            // Player attack
            if (IsKeyPressed(KEY_SPACE) && player.attackCooldown <= 0.0f && monster.isAlive) {
                if (CheckCollisionRecs(
                        {player.position.x, player.position.y, (float)player.texture.width, (float)player.texture.height},
                        {monster.position.x, monster.position.y, (float)monster.texture.width, (float)monster.texture.height}
                    )) {
                    monster.health -= 20;
                    PlaySound(attackSound);
                    player.attackCooldown = 0.5f;
                    if (monster.health <= 0) {
                        monster.isAlive = false;
                        gameResult = true;
                        currentState = GAME_OVER;
                    }
                }
            }

            // Monster damage
            if (monster.isAlive && CheckCollisionRecs(
                {player.position.x, player.position.y, (float)player.texture.width, (float)player.texture.height},
                {monster.position.x, monster.position.y, (float)monster.texture.width, (float)monster.texture.height}
            )) {
                player.health -= 30 * deltaTime;
                if (player.health <= 0) {
                    gameResult = false;
                    currentState = GAME_OVER;
                }
            }

            break;

        case GAME_OVER:
            if (IsKeyPressed(KEY_ENTER)) {
                // Reset game
                player.position = {100, 360};
                player.health = 100;
                player.attackCooldown = 0.0f;

                monster.position = {900, 360};
                monster.health = 100;
                monster.isAlive = true;

                currentState = MENU;
            }
            break;
        }

        // Drawing
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch (currentState) {
            case MENU:
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawText("MONSTER BATTLE", screenWidth/2 - MeasureText("MONSTER BATTLE", 40)/2, 200, 40, DARKBLUE);
                DrawText("Press [ENTER] to start", screenWidth/2 - MeasureText("Press [ENTER] to start", 20)/2, 300, 20, DARKGRAY);
                break;
            case GAME:
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                DrawTexture(player.texture, (int)player.position.x, (int)player.position.y, WHITE);
                if (monster.isAlive) {
                    DrawTexture(monster.texture, (int)monster.position.x, (int)monster.position.y, WHITE);
                }
                DrawText(TextFormat("Player Health: %d", (int)player.health), 20, 20, 20, RED);
                break;
            case GAME_OVER:
                DrawTexture(backgroundTexture, 0, 0, WHITE);
                if (gameResult)
                    DrawText("VICTORY!", screenWidth/2 - MeasureText("VICTORY!", 50)/2, 200, 50, GREEN);
                else
                    DrawText("DEFEAT!", screenWidth/2 - MeasureText("DEFEAT!", 50)/2, 200, 50, RED);
                DrawText("Press [ENTER] to return to menu", screenWidth/2 - MeasureText("Press [ENTER] to return to menu", 20)/2, 300, 20, DARKGRAY);
                break;
            }

        EndDrawing();
    }

    // Cleanup resources
    UnloadTexture(playerTexture);
    UnloadTexture(monsterTexture);
    UnloadTexture(backgroundTexture);
    UnloadSound(attackSound);
    UnloadMusicStream(backgroundMusic);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}