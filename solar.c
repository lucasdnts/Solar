#include "raylib.h" // Inclui a biblioteca raylib para gráficos e áudio

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h> // Inclui a biblioteca emscripten para compilação web
#endif

// Definições de constantes
#define NUM_SHOOTS 50 // Número máximo de tiros
#define NUM_MAX_ENEMIES 50 // Número máximo de inimigos
#define FIRST_WAVE 10 // Primeira onda de inimigos
#define SECOND_WAVE 20 // Segunda onda de inimigos
#define THIRD_WAVE 50 // Terceira onda de inimigos

// Enumerações para o estado do jogo e ondas de inimigos
typedef enum { MENU = 0, COUNTDOWN, PLAYING, GAME_OVER } GameState;
typedef enum { FIRST = 0, SECOND, THIRD } EnemyWave;

// Estrutura para o jogador
typedef struct Player {
    Rectangle rec;
    Vector2 speed;
    Texture2D texture;
    char name[20];
} Player;

// Estrutura para os inimigos
typedef struct Enemy {
    Rectangle rec;
    Vector2 speed;
    bool active;
    Texture2D texture;
} Enemy;

// Estrutura para os tiros
typedef struct Shoot {
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Shoot;

// Variáveis globais
static const int screenWidth = 800;
static const int screenHeight = 450;

static GameState gameState = MENU;
static bool gameOver = false;
static bool pause = false;
static int score = 0;
static bool victory = false;
static int countdown = 300;  // 5 segundos * 60 FPS

static Player player = { 0 };
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS] = { 0 };
static EnemyWave wave = { 0 };

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;

// Declaração das funções
static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(screenWidth, screenHeight, "Solar - Scape from"); // Inicializa a janela do jogo

    InitGame(); // Inicializa o jogo

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1); // Configura o loop principal para a web
#else
    SetTargetFPS(60); // Define a taxa de frames para 60 FPS
    while (!WindowShouldClose())
    {
        UpdateDrawFrame(); // Atualiza e desenha o quadro
    }
#endif
    UnloadGame(); // Descarrega os recursos do jogo
    CloseWindow(); // Fecha a janela do jogo

    return 0;
}

void InitGame(void)
{
    shootRate = 0;
    pause = false;
    gameOver = false;
    victory = false;
    smooth = false;
    wave = FIRST;
    activeEnemies = FIRST_WAVE;
    enemiesKill = 0;
    score = 0;
    alpha = 0;
    countdown = 300;

    // Inicializa o jogador
    player.rec.x =  20;
    player.rec.y = 50;
    player.rec.width = 40;
    player.rec.height = 40;
    player.speed.x = 5;
    player.speed.y = 5;
    player.texture = LoadTexture("player.png");

    // Inicializa os inimigos
    const char* enemyTextures[4] = { "enemy1.png", "enemy2.png", "enemy3.png", "enemy4.png" };
    for (int i = 0; i < NUM_MAX_ENEMIES; i++)
    {
        enemy[i].rec.width = 40;
        enemy[i].rec.height = 40;
        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
        enemy[i].speed.x = 5;
        enemy[i].speed.y = 5;
        enemy[i].active = true;
        enemy[i].texture = LoadTexture(enemyTextures[i % 4]);
    }

    // Inicializa os tiros
    for (int i = 0; i < NUM_SHOOTS; i++)
    {
        shoot[i].rec.x = player.rec.x;
        shoot[i].rec.y = player.rec.y + player.rec.height / 4;
        shoot[i].rec.width = 10;
        shoot[i].rec.height = 5;
        shoot[i].speed.x = 7;
        shoot[i].speed.y = 0;
        shoot[i].active = false;
        shoot[i].color = MAROON;
    }
}

void UpdateGame(void)
{
    static bool canShoot = true;

    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause; // Pausa o jogo quando 'P' é pressionado

        if (!pause)
        {
            // Gerencia as ondas de inimigos
            switch (wave)
            {
                case FIRST:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;
                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies)
                    {
                        enemiesKill = 0;

                        for (int i = 0; i < activeEnemies; i++)
                        {
                            if (!enemy[i].active) enemy[i].active = true;
                        }

                        activeEnemies = SECOND_WAVE;
                        wave = SECOND;
                        smooth = false;
                        alpha = 0.0f;
                    }
                } break;
                case SECOND:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;
                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies)
                    {
                        enemiesKill = 0;

                        for (int i = 0; i < activeEnemies; i++)
                        {
                            if (!enemy[i].active) enemy[i].active = true;
                        }

                        activeEnemies = THIRD_WAVE;
                        wave = THIRD;
                        smooth = false;
                        alpha = 0.0f;
                    }
                } break;
                case THIRD:
                {
                    if (!smooth)
                    {
                        alpha += 0.02f;
                        if (alpha >= 1.0f) smooth = true;
                    }

                    if (smooth) alpha -= 0.02f;

                    if (enemiesKill == activeEnemies) victory = true;

                } break;
                default: break;
            }

            // Movimenta o jogador
            if (IsKeyDown(KEY_RIGHT)) player.rec.x += player.speed.x;
            if (IsKeyDown(KEY_LEFT)) player.rec.x -= player.speed.x;
            if (IsKeyDown(KEY_UP)) player.rec.y -= player.speed.y;
            if (IsKeyDown(KEY_DOWN)) player.rec.y += player.speed.y;

            // Verifica colisão do jogador com os inimigos
            for (int i = 0; i < activeEnemies; i++)
            {
                if (CheckCollisionRecs(player.rec, enemy[i].rec)) {
                    gameOver = true;
                    gameState = GAME_OVER;
                }
            }

            // Movimenta os inimigos
            for (int i = 0; i < activeEnemies; i++)
            {
                if (enemy[i].active)
                {
                    enemy[i].rec.x -= enemy[i].speed.x;

                    if (enemy[i].rec.x < 0)
                    {
                        enemy[i].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                        enemy[i].rec.y = GetRandomValue(0, screenHeight - enemy[i].rec.height);
                    }
                }
            }

            // Limita a posição do jogador dentro da tela
            if (player.rec.x <= 0) player.rec.x = 0;
            if (player.rec.x + player.rec.width >= screenWidth) player.rec.x = screenWidth - player.rec.width;
            if (player.rec.y <= 0) player.rec.y = 0;
            if (player.rec.y + player.rec.height >= screenHeight) player.rec.y = screenHeight - player.rec.height;

            // Atira
            if (IsKeyPressed(KEY_SPACE) && canShoot)
            {
                canShoot = false;
                for (int i = 0; i < NUM_SHOOTS; i++)
                {
                    if (!shoot[i].active)
                    {
                        shoot[i].rec.x = player.rec.x;
                        shoot[i].rec.y = player.rec.y + player.rec.height / 4;
                        shoot[i].active = true;
                        break;
                    }
                }
            }
            if (IsKeyReleased(KEY_SPACE))
            {
                canShoot = true;
            }

            // Movimenta os tiros e verifica colisão com os inimigos
            for (int i = 0; i < NUM_SHOOTS; i++)
            {
                if (shoot[i].active)
                {
                    shoot[i].rec.x += shoot[i].speed.x;

                    for (int j = 0; j < activeEnemies; j++)
                    {
                        if (enemy[j].active)
                        {
                            if (CheckCollisionRecs(shoot[i].rec, enemy[j].rec))
                            {
                                shoot[i].active = false;
                                enemy[j].rec.x = GetRandomValue(screenWidth, screenWidth + 1000);
                                enemy[j].rec.y = GetRandomValue(0, screenHeight - enemy[j].rec.height);
                                shootRate = 0;
                                enemiesKill++;
                                score += 100;
                            }

                            if (shoot[i].rec.x + shoot[i].rec.width >= screenWidth)
                            {
                                shoot[i].active = false;
                                shootRate = 0;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame(); // Reinicializa o jogo
            gameOver = false;
        }
    }
}

void DrawGame(void)
{
    BeginDrawing();

    ClearBackground(BLACK); // Limpa a tela com cor preta

    if (gameState == MENU)
    {
        DrawText("SOLAR - ESCAPE FROM", screenWidth / 2 - MeasureText("SOLAR - ESCAPE FROM", 40) / 2, screenHeight / 4, 40, WHITE);
        DrawText("Digite seu nome:", screenWidth / 2 - MeasureText("Digite seu nome", 20) / 2, screenHeight / 2 - 50, 20, WHITE);
        DrawText(player.name, screenWidth / 2 - MeasureText(player.name, 20) / 2, screenHeight / 2, 20, WHITE);
        DrawText("[ENTER] para começar", screenWidth / 2 - MeasureText("[ENTER] para começar", 20) / 2, screenHeight - 50, 20, WHITE);

        if (IsKeyPressed(KEY_ENTER))
        {
            gameState = COUNTDOWN;
        }
        else
        {
            int key = GetKeyPressed();
            if ((key >= 32) && (key <= 125) && (strlen(player.name) < sizeof(player.name) - 1))
            {
                int len = strlen(player.name);
                player.name[len] = (char)key;
                player.name[len + 1] = '\0';
            }
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                int len = strlen(player.name);
                if (len > 0) player.name[len - 1] = '\0';
            }
        }
    }
    else if (gameState == COUNTDOWN)
    {
        DrawText(TextFormat("Começa em %d...", countdown / 60), screenWidth / 2 - MeasureText("Começa em 5...", 40) / 2, screenHeight / 2 - 20, 40, WHITE);
        countdown--;
        if (countdown <= 0)
        {
            gameState = PLAYING;
        }
    }
    else if (gameState == PLAYING)
    {
        // Desenha estrelas aleatórias no fundo
        for (int i = 0; i < 5; i++)
        {
            int starX = GetRandomValue(0, screenWidth);
            int starY = GetRandomValue(0, screenHeight);
            DrawPixel(starX, starY, WHITE);
        }

        DrawTexture(player.texture, player.rec.x, player.rec.y, RAYWHITE); // Desenha o jogador

        if (wave == FIRST) DrawText("1° CINTURÃO DE ASTERÓIDES", screenWidth / 2 - MeasureText("1° CINTURÃO DE ASTERÓIDES", 40) / 2, screenHeight / 2 - 40, 40, Fade(WHITE, alpha));
        else if (wave == SECOND) DrawText("2° CINTURÃO DE ASTERÓIDES", screenWidth / 2 - MeasureText("2° CINTURÃO DE ASTERÓIDES", 40) / 2, screenHeight / 2 - 40, 40, Fade(WHITE, alpha));
        else if (wave == THIRD) DrawText("3° CINTURÃO DE ASTERÓIDES", screenWidth / 2 - MeasureText("3° CINTURÃO DE ASTERÓIDES", 40) / 2, screenHeight / 2 - 40, 40, Fade(WHITE, alpha));

        // Desenha os inimigos
        for (int i = 0; i < activeEnemies; i++)
        {
            if (enemy[i].active) DrawTexture(enemy[i].texture, enemy[i].rec.x, enemy[i].rec.y, RAYWHITE);
        }

        // Desenha os tiros
        for (int i = 0; i < NUM_SHOOTS; i++)
        {
            if (shoot[i].active) DrawRectangleRec(shoot[i].rec, shoot[i].color);
        }

        DrawText(TextFormat("%04i", score), 20, 20, 40, GRAY); // Desenha a pontuação

        if (victory) DrawText("VOCÊ ESCAPOU", screenWidth / 2 - MeasureText("VOCÊ ESCAPOU", 40) / 2, screenHeight / 2 - 40, 40, WHITE); // Mensagem de vitória

        if (pause) DrawText("JOGO PAUSADO", screenWidth / 2 - MeasureText("JOGO PAUSADO", 40) / 2, screenHeight / 2 - 40, 40, GRAY); // Mensagem de pausa
    }
    else if (gameState == GAME_OVER)
    {
        DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 40) / 2, screenHeight / 2 - 40, 40, WHITE);
        DrawText("[ENTER] PARA JOGAR NOVAMENTE", screenWidth / 2 - MeasureText("[ENTER] PARA JOGAR NOVAMENTE", 20) / 2, screenHeight / 2, 20, WHITE);

        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame(); // Reinicializa o jogo
            gameOver = false;
            gameState = MENU;
        }
    }

    EndDrawing();
}

void UnloadGame(void)
{
    UnloadTexture(player.texture); // Descarrega a textura do jogador
    for (int i = 0; i < NUM_MAX_ENEMIES; i++)
    {
        UnloadTexture(enemy[i].texture); // Descarrega as texturas dos inimigos
    }
}

void UpdateDrawFrame(void)
{
    if (gameState == PLAYING && !gameOver)
    {
        UpdateGame(); // Atualiza o jogo se está em andamento e não está em game over
    }
    DrawGame(); // Desenha o jogo
}
