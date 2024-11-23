#include "raylib.h"

#define NUM_SHOOTS 50
#define NUM_MAX_ENEMIES 30 
#define ENEMY_ROWS 3  
#define ENEMIES_PER_ROW 10


typedef struct Player {
    Rectangle rec;
    Vector2 speed;
    Color color;
    int shootCooldown;
    int lives;
} Player;

typedef struct Enemy {
    Rectangle rec;
    bool active;
    Color color;
    float floatOffset;
    int shootCooldown;
    int health;  
} Enemy;

typedef struct Boss {
    Rectangle rec;
    int health;
    Color color;
    int shootCooldown;
    bool active;
    bool specialMoveActive;
    int specialMoveCooldown;
    float moveSpeed;
} Boss;

typedef struct Shoot {
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Shoot;

static const int screenWidth = 450;
static const int screenHeight = 800;

static bool gameOver = false;
static bool pause = false;
static int score = 0;

static Player player = { 0 };
static Enemy enemies[NUM_MAX_ENEMIES] = { 0 };
static Boss boss = { 0 };
static Shoot playerShoots[NUM_SHOOTS] = { 0 };
static Shoot enemyShoots[NUM_SHOOTS] = { 0 };

static int shootRate = 0;

// Function prototypes
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static void SpawnEnemies(void);
static void SpawnBoss(void);
static void HandleSpecialMove(void);
static void BossAttack(void);
static void DrawBossHealthBar(void);  
typedef enum { MAIN_MENU, GAMEPLAY, GAME_OVER } GameState;
GameState currentState = MAIN_MENU;

// Function Prototypes for Menu
void ShowMainMenu(void);
void HandleMainMenu(void);
void ShowGameOver(void);
void HandleGameOver(void);

// Main Menu Functions
void ShowMainMenu(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("Space Invaders", screenWidth / 2 - MeasureText("Space Invaders", 50) / 2, screenHeight / 4 - 50, 50, BLUE);
    DrawText("Press [ENTER] to Start", screenWidth / 2 - MeasureText("Press [ENTER] to Start", 20) / 2, screenHeight / 2, 20, DARKGRAY);
    DrawText("Press [ESC] to Exit", screenWidth / 2 - MeasureText("Press [ESC] to Exit", 20) / 2, screenHeight / 2 + 40, 20, DARKGRAY);
    
    EndDrawing();
}

void HandleMainMenu(void) {
    if (IsKeyPressed(KEY_ENTER)) {
       //game ko start kia hay
        currentState = GAMEPLAY;
        InitGame();  //game ko refresh maara hay
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
     
        CloseWindow(); 
    }
}


void ShowGameOver(void) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 40, 40, RED);
    DrawText("Press [ENTER] to Restart", screenWidth / 2 - MeasureText("Press [ENTER] to Restart", 20) / 2, screenHeight / 2, 20, DARKGRAY);
    DrawText("Press [ESC] to Exit", screenWidth / 2 - MeasureText("Press [ESC] to Exit", 20) / 2, screenHeight / 2 + 40, 20, DARKGRAY);

    EndDrawing();
}

void HandleGameOver(void) {
    if (IsKeyPressed(KEY_ENTER)) {
       
        currentState = GAMEPLAY;
        InitGame();  
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
     
        CloseWindow(); 
    }
}

int main(void) {
    InitWindow(screenWidth, screenHeight, "Space Invaders");

    InitGame();

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }

    UnloadGame();
    CloseWindow();

    return 0;
}

void InitGame(void) {
    shootRate = 0;
    pause = false;
    gameOver = false;
    score = 0;

    // player ko initialize kya
    player.rec = (Rectangle){screenWidth / 2 - 10, screenHeight - 30, 20, 20};
    player.speed = (Vector2){5, 0};
    player.color = BLACK;
    
    player.shootCooldown = 0;
    player.lives = 3;

    // dushmanon ko peda kiya
    SpawnEnemies();

    // boss ki initialization
    boss = (Boss){{screenWidth / 2 - 50, 50, 100, 30}, 20, RED, 120, false, false, 0, 3.0f};

    // gooliyon ki initialization
    for (int i = 0; i < NUM_SHOOTS; i++) {
        playerShoots[i] = (Shoot){{0, 0, 10, 5}, {0, -7}, false, GREEN};
        enemyShoots[i] = (Shoot){{0, 0, 10, 5}, {0, 3}, false, RED};
    }
}

void SpawnEnemies(void) {
    int index = 0;  
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMIES_PER_ROW; col++) {
            if (index < NUM_MAX_ENEMIES) {
                enemies[index] = (Enemy){
                    {col * (screenWidth / ENEMIES_PER_ROW) + 15, 50 + row * 40, 40, 20},
                    true,
                    GRAY,
                    (float)(GetRandomValue(0, 100)) / 100.0f,
                    GetRandomValue(60, 180),
                    2 
                };
                index++;
            }
        }
    }
}

void SpawnBoss(void) {
    boss.active = true;
    boss.rec.x = screenWidth / 2 - boss.rec.width / 2;
    boss.rec.y = 50;
    boss.health = 20; 
    boss.color.a = 0; 
}

void HandleSpecialMove(void) {
    // Boss special move: 10 hearts khoone k baad machine gun ki tarah fire krega
    if (boss.health <= 10 && !boss.specialMoveActive) {
        boss.specialMoveActive = true;  // Activate 
    }
}

void BossAttack(void) {
    if (boss.specialMoveActive) {
        // Boss au zyada tezi se aur ghussey se attack krega
        boss.shootCooldown--;
        if (boss.shootCooldown <= 0) {
            // Shoot multiple bullets 
            for (int i = 0; i < NUM_SHOOTS; i++) {
                if (!enemyShoots[i].active) {
                    // Multiple bullets with different speeds
                    enemyShoots[i].rec.x = boss.rec.x + boss.rec.width / 2 - 5;
                    enemyShoots[i].rec.y = boss.rec.y + boss.rec.height;
                    enemyShoots[i].active = true;
                    enemyShoots[i].speed = (Vector2){(float)GetRandomValue(-2, 2), 5};  // Random angle for the bullets
                    break;
                }
            }
            boss.shootCooldown = 10;  // Faster shooting 
        }
    } else {
        // Regular boss attack
        boss.shootCooldown--;
        if (boss.shootCooldown <= 0) {
            // Shoot a teen  bullets
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < NUM_SHOOTS; j++) {
                    if (!enemyShoots[j].active) {
                        enemyShoots[j].rec.x = boss.rec.x + boss.rec.width / 2 - 5;
                        enemyShoots[j].rec.y = boss.rec.y + boss.rec.height + i * 10;
                        enemyShoots[j].active = true;
                        boss.shootCooldown = 60;  // Regular shooting rate
                        break;
                    }
                }
            }
        }
    }
}

void DrawBossHealthBar(void) {
    // Health bar position 
    float barWidth = 100; // Total width of the health bar 
    float barHeight = 10; // Height of the health bar
    float healthWidth = (float)boss.health / 20 * barWidth; // Width of the health 

    // Position the health bar above the boss
    float healthBarX = boss.rec.x + (boss.rec.width - barWidth) / 2; // Centered above the boss
    float healthBarY = boss.rec.y - barHeight - 5; // Slight gap above the boss

    // Background bar (gray)
    DrawRectangle(healthBarX, healthBarY, barWidth, barHeight, DARKGRAY);

    // Foreground bar (green)
    DrawRectangle(healthBarX, healthBarY, healthWidth, barHeight, GREEN);
}

void UpdateGame(void) {
    if (!gameOver) {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause) {
            // Player ki movement
            if (IsKeyDown(KEY_RIGHT)) player.rec.x += player.speed.x;
            if (IsKeyDown(KEY_LEFT)) player.rec.x -= player.speed.x;
            if (player.rec.x < 0) player.rec.x = 0;
            if (player.rec.x + player.rec.width > screenWidth) player.rec.x = screenWidth - player.rec.width;

            // Player  ki shooting
            if (player.shootCooldown > 0) player.shootCooldown--;
            if (IsKeyDown(KEY_SPACE) && player.shootCooldown <= 0) {
                for (int i = 0; i < NUM_SHOOTS; i++) {
                    if (!playerShoots[i].active) {
                        playerShoots[i].rec.x = player.rec.x + player.rec.width / 4;
                        playerShoots[i].rec.y = player.rec.y;
                        playerShoots[i].active = true;
                        player.shootCooldown = 20;
                        break;
                    }
                }
            }

            // Update player bullets
            for (int i = 0; i < NUM_SHOOTS; i++) {
                if (playerShoots[i].active) {
                    playerShoots[i].rec.y += playerShoots[i].speed.y;

                    // Check collision with enemies
                    for (int j = 0; j < NUM_MAX_ENEMIES; j++) {
                        if (enemies[j].active && CheckCollisionRecs(playerShoots[i].rec, enemies[j].rec)) {
                            playerShoots[i].active = false;
                            enemies[j].health--;

                            if (enemies[j].health <= 0) {
                                enemies[j].active = false;
                                score += 100;
                            }
                        }
                    }

                    // Check collision with boss
                    if (boss.active && CheckCollisionRecs(playerShoots[i].rec, boss.rec)) {
                        playerShoots[i].active = false;
                        boss.health--;

                        if (boss.health <= 0) {
                            boss.active = false;
                            score += 1000;
                        }
                    }

                    if (playerShoots[i].rec.y < 0) playerShoots[i].active = false;
                }
            }

            // Update enemies
            bool enemiesLeft = false;
            for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    enemiesLeft = true;

                    enemies[i].floatOffset += 0.05f;
                    enemies[i].rec.y += 0.5f * sin(enemies[i].floatOffset);

                    // Enemy shooting
                    enemies[i].shootCooldown--;
                    if (enemies[i].shootCooldown <= 0) {
                        for (int j = 0; j < NUM_SHOOTS; j++) {
                            if (!enemyShoots[j].active) {
                                enemyShoots[j].rec.x = enemies[i].rec.x + enemies[i].rec.width / 4;
                                enemyShoots[j].rec.y = enemies[i].rec.y + enemies[i].rec.height;
                                enemyShoots[j].active = true;
                                enemies[i].shootCooldown = GetRandomValue(120, 240);
                                break;
                            }
                        }
                    }
                }
            }

            // Spawn boss agr no enemies left
            if (!enemiesLeft && !boss.active) {
                SpawnBoss();
            }

            // Update boss movement aur blinking effect
            if (boss.active) {
                // Gradually increase boss jb wo medan mai aye
                if (boss.color.a < 255) {
                    boss.color.a += 5;  // Increase opacity
                }

                // Boss moves left and right continuously
                boss.rec.x += boss.moveSpeed;

                if (boss.rec.x <= 0 || boss.rec.x + boss.rec.width >= screenWidth) {
                    boss.moveSpeed = -boss.moveSpeed; // Reverse direction when hitting screen boundaries
                }

                // Handle boss special move if health is low
                HandleSpecialMove();
                BossAttack();
            }

            // Update enemy and boss bullets
            for (int i = 0; i < NUM_SHOOTS; i++) {
                if (enemyShoots[i].active) {
                    enemyShoots[i].rec.y += enemyShoots[i].speed.y;

                    // Check collision with player
                    if (CheckCollisionRecs(enemyShoots[i].rec, player.rec)) {
                        enemyShoots[i].active = false;
                        player.lives--;

                        if (player.lives <= 0) {
                            gameOver = true;
                        }
                    }

                    if (enemyShoots[i].rec.y > screenHeight) enemyShoots[i].active = false;
                }
            }
        }
    } else {
        if (IsKeyPressed(KEY_ENTER)) {
            InitGame();
        }
    }
}

void DrawGame(void) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver) {
        // Draw player
        DrawRectangleRec(player.rec, player.color);

        // Draw lives
        DrawText(TextFormat("Lives: %i", player.lives), 20, 20, 20, RED);

        // Draw enemies
        for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
            if (enemies[i].active) DrawRectangleRec(enemies[i].rec, enemies[i].color);
        }

        // Draw boss
        if (boss.active) {
            DrawRectangleRec(boss.rec, boss.color);
            // Draw boss health bar
            DrawBossHealthBar();
        }

        // Draw bullets
        for (int i = 0; i < NUM_SHOOTS; i++) {
            if (playerShoots[i].active) DrawRectangleRec(playerShoots[i].rec, playerShoots[i].color);
            if (enemyShoots[i].active) DrawRectangleRec(enemyShoots[i].rec, enemyShoots[i].color);
        }

        // Draw score
        DrawText(TextFormat("Score: %04i", score), screenWidth - 120, 20, 20, BLACK);
    } else {
        DrawText("GAME OVER - PRESS [ENTER] TO RESTART", screenWidth / 2 - MeasureText("GAME OVER - PRESS [ENTER] TO RESTART", 20) / 2, screenHeight / 2 - 10, 20, BLACK);
    }

    EndDrawing();
}

void UpdateDrawFrame(void) {
    switch (currentState) {
        case MAIN_MENU:
            ShowMainMenu();
            HandleMainMenu();
            break;
        case GAMEPLAY:
            UpdateGame();
            DrawGame();
            break;
        case GAME_OVER:
            ShowGameOver();
            HandleGameOver();
            break;
    }
}

void UnloadGame(void) {
    
}