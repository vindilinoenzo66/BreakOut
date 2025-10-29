/**
 * @file main.c
 * @author Prof. Dr. David Buzatto
 * @brief Main function and logic for the game. Simplified template for game
 * development in C using Raylib (https://www.raylib.com/).
 * 
 * @copyright Copyright (c) 2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> // para deixar as bolinhas aleatorias
/*---------------------------------------------
 * Library headers.
 *-------------------------------------------*/
#include "raylib/raylib.h"
//#include "raylib/raymath.h"

/*---------------------------------------------
 * Project headers.
 *-------------------------------------------*/


/*---------------------------------------------
 * Macros. 
 *-------------------------------------------*/


/*--------------------------------------------
 * Constants. 
 *------------------------------------------*/


/*---------------------------------------------
 * Custom types (enums, structs, unions, etc.)
 *-------------------------------------------*/
typedef struct Bolinha {
    Vector2 pos;
    Vector2 vel;
    float raio;
    Color cor;
} Bolinha;

typedef struct Jogador {
    Vector2 pos;
    Vector2 dim;
    float vel;
    int pontos;
    Color cor;
} Jogador;

typedef struct Retangulo {
    Vector2 pos;
    Vector2 dim;
    Color cor;
    bool ativo;
} Retangulo;

/*---------------------------------------------
 * Global variables.
 *-------------------------------------------*/
Jogador jogador;
Bolinha bola;
Retangulo retangulo[8][8];
int totalDeRetangulos = 56;
int pontosPorLinha[] = {40, 30, 25, 20, 15, 10, 5, 1};
int vidas = 3;
int vidasPerdidas = 0;
float tempoDecorrido = 0;
float fatorAumento = 1.0f;
bool iniciarJogo = false;
bool venceu = false;
bool perdeu = false;
bool perdeuVida = false;
bool movimentar = false;
Sound somColisao;
Sound somPerdeuVida;
Sound somGameOver;
Sound somGameWin;

/*---------------------------------------------
 * Function prototypes. 
 *-------------------------------------------*/
/**
 * @brief Reads user input and updates the state of the game.
 */
void update( float delta );

/**
 * @brief Draws the state of the game.
 */
void draw( void );
void mexerJogador( Jogador *jogador, int teclaCima, int teclaBaixo, float delta );
void mexerBolinha(Bolinha *bolinha, float delta);
void desenharJogador(Jogador *jogador);
void desenharBolinha(Bolinha *bolinha);
void colisaoJogador(Bolinha *bolinha, Jogador *jogador);
void colisaoObstaculos(Bolinha *bolinha);
void inserirObstaculos();
void desenharObstaculos();

/**
 * @brief Game entry point.
 */
int main( void ) {

    // local variables and initial user input

    // antialiasing
    SetConfigFlags( FLAG_MSAA_4X_HINT );
    srand(time(NULL)); //inicializa o gerador de bolinhas aleatorias
    // creates a new window 800 pixels wide and 450 pixels high
    InitWindow( 600, 800, "Breakout" );
   
    // init audio device only if your game uses sounds
    InitAudioDevice();
    somColisao = LoadSound("sound/blip.wav");
    somGameOver = LoadSound("sound/perdeuvida.wav");
    somPerdeuVida = LoadSound("sound/gameover.wav");
    somGameWin = LoadSound("sound/gamewin.wav");
    // FPS: frames per second
    SetTargetFPS( 60 );    
    inserirObstaculos();
    // you must load game resources here
    jogador = (Jogador) {
        .pos = {
            .x = GetScreenWidth() / 2 - 55,
            .y = GetScreenHeight() - 60
        },
        .dim = {
            .x = 110,
            .y = 25
        }, 
        .vel = 200,
        .pontos = 0,
        .cor = WHITE 
    };

    bola = (Bolinha) {
        .pos = {
            .x = GetScreenWidth() / 2,
            .y = GetScreenHeight() - 90
        },
        .vel = {
            .x = 200,
            .y = 200
        },
        .raio = 12,
        .cor = WHITE
    };

    // game loop
    while ( !WindowShouldClose() ) {
        update( GetFrameTime() );

        draw();
    }

    // you should unload game resources here

    
    // close audio device only if your game uses sounds
    UnloadSound(somColisao);
    UnloadSound(somGameOver);
    UnloadSound(somGameWin);
    UnloadSound(somPerdeuVida);
    CloseAudioDevice();
    CloseWindow();

    return 0;

}

void update(float delta) {

    if (perdeu) {
        if (IsKeyPressed(KEY_SPACE)) {
            jogador.pos.x = GetScreenWidth() / 2 - 55;
            jogador.pos.y = GetScreenHeight() - 60;
            jogador.pontos = 0;
            vidas = 3;
            iniciarJogo = false;
            venceu = false;
            perdeu = false;
            movimentar = false;
            perdeuVida = false;
            totalDeRetangulos = 56;
            fatorAumento = 0.5f;
            tempoDecorrido = 0;
            inserirObstaculos();
        }
        return;
    }

    if (!iniciarJogo) {
        if (IsKeyPressed(KEY_SPACE)) {
            iniciarJogo = true;
            movimentar = true;
        }
    } else {

        tempoDecorrido += delta;
        if (tempoDecorrido >= 10.0f) {  // cada 10seg
            fatorAumento *= 1.1f;
            tempoDecorrido = 0;
        }

        if (movimentar) {
            jogador.vel = 200 * fatorAumento;
            
            //só aumenta a velocidade a direção é a mesma
            if (bola.vel.x > 0) {
                bola.vel.x = 200 * fatorAumento;
            } else {
                bola.vel.x = -200 * fatorAumento;
            }

            if (bola.vel.y > 0) {
                bola.vel.y = 200 * fatorAumento;
            } else {
                bola.vel.y = -200 * fatorAumento;
            }
            mexerJogador(&jogador, KEY_RIGHT, KEY_LEFT, delta);
            mexerBolinha(&bola, delta);        
        }

        colisaoJogador(&bola, &jogador);
        colisaoObstaculos(&bola);

        if (totalDeRetangulos == 0) {
            venceu = true;
            movimentar = false;
            bola.pos.x = GetScreenWidth() / 2;
            bola.pos.y = GetScreenHeight() - 90;
            fatorAumento = 1.0f;
            tempoDecorrido = 0;
            if (IsKeyPressed(KEY_SPACE)) {
                jogador.pontos = 0;
                vidas = 3;
                iniciarJogo = false;
                venceu = false;
                movimentar = false;
                totalDeRetangulos = 56;
                inserirObstaculos();
            }
        }

        if (perdeuVida) {
            movimentar = false;
            if (IsKeyPressed(KEY_SPACE)) {
                movimentar = true;
                perdeuVida = false;
            }
        }

        if (vidas == 0) {
            perdeu = true;
            movimentar = false;
            PlaySound(somGameOver);
        }
    }
}


void draw(void) {
    BeginDrawing();
    ClearBackground(BLACK);

    if(!iniciarJogo) {

        const char *title = "Breakout";
        DrawText(title, 117, 40, 80, RED);

        const char *msg = "<Pressione ESPACO para comecar>";
        DrawText(msg, 100, GetScreenHeight() / 2, 25, WHITE);

        const char *nomes = "Desenvolvido por Enzo Vindilino e Luis Felipe.";
        DrawText(nomes, GetScreenWidth() / 2 - 50, 750, 16, WHITE);
    } else if(venceu) {

        if (!IsSoundPlaying(somGameWin)) {
            PlaySound(somGameWin);
        }
        const char *msg = "<Voce GANHOU ! !>";
        int textWidth = MeasureText(msg, 25);
        DrawText(msg, 176, (GetScreenHeight() - textWidth) / 2, 30, GREEN);

        const char *msg1 = "<Pressione ESPACO para comecar>";
        DrawText(msg1, 100, GetScreenHeight() / 2, 25, WHITE);

        char str[20];
        sprintf(str, "%d", jogador.pontos);
        DrawText(str, GetScreenWidth() - 90, 25, 40, GREEN);

        for (int i = 0; i < vidas; i++) {
        DrawCircle(20 + i * 30, 40, 13, GREEN);
        }
        desenharJogador(&jogador);
        desenharBolinha(&bola);

    } else if(perdeuVida) {

        const char *msg = "<Voce PERDEU uma VIDA ! !>";
        int textWidth = MeasureText(msg, 25);
        DrawText(msg, (GetScreenWidth() - textWidth) / 2, GetScreenHeight() / 2 - 35, 25, RED);

        const char *msg1 = "<Pressione ESPACO para continuar>";
        DrawText(msg1, 100, GetScreenHeight() / 2 , 25, WHITE);
        
        desenharObstaculos();
        desenharJogador(&jogador);
        desenharBolinha(&bola);

        for (int i = 0; i < vidas; i++) {
        DrawCircle(20 + i * 30, 40, 13, WHITE);
        }

        char str[20];
        sprintf(str, "%d", jogador.pontos);
        DrawText(str, GetScreenWidth() - 90, 25, 40, WHITE);

    }else if (perdeu){
        const char *msg = "<Voce PERDEU ! !>";
        int textWidth = MeasureText(msg, 25);
        DrawText(msg, 176, (GetScreenHeight() - textWidth) / 2, 30, RED);

        const char *msg1 = "<Pressione ESPACO para comecar>";
        DrawText(msg1, 100, GetScreenHeight() / 2, 25, WHITE);
        char str[20];
        sprintf(str, "%d", jogador.pontos);
        DrawText(str, GetScreenWidth() - 90, 25, 40, RED);

        //desenharJogador(&jogador);
        //desenharBolinha(&bola);
    }else {
        desenharObstaculos();
        desenharJogador(&jogador);
        desenharBolinha(&bola);

        for (int i = 0; i < vidas; i++) {
        DrawCircle(20 + i * 30, 40, 13, WHITE);
        }

        char str[20];
        sprintf(str, "%d", jogador.pontos);
        DrawText(str, GetScreenWidth() - 80, 25, 40, WHITE);
    }

    EndDrawing();
}

void inserirObstaculos() {
    int espaco = 5;
    float larguraRet = (GetScreenWidth() - 7 * espaco) / 7;
    float alturaRet = 30;

    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 7; j++) {

            retangulo[i][j].dim = (Vector2){larguraRet - espaco, alturaRet - espaco}; 
            retangulo[i][j].pos = (Vector2){3 + espaco + j *(larguraRet + espaco), i * alturaRet + 80};
            retangulo[i][j].ativo = true;

            switch (i) {
                case 0:
                    retangulo[i][j].cor = RED;
                    break;
                case 1:
                    retangulo[i][j].cor = ORANGE;
                    break;
                case 2:
                    retangulo[i][j].cor = YELLOW;
                    break;
                case 3:
                    retangulo[i][j].cor = GREEN;
                    break;
                case 4:
                    retangulo[i][j].cor = PINK;
                    break;
                case 5:
                    retangulo[i][j].cor = PURPLE;
                    break;
                case 6:
                    retangulo[i][j].cor = BLUE;
                    break;
                case 7:
                    retangulo[i][j].cor = DARKBLUE;
                    break;
                default:
                    retangulo[i][j].cor = DARKGREEN;
                    break;
            }
        }
    }
}

void mexerJogador( Jogador *jogador, int teclaDireita, int teclaEsquerda, float delta ) {
    if(IsKeyDown(teclaDireita)) {
        jogador -> pos.x += jogador -> vel * delta;
    }

    if(IsKeyDown(teclaEsquerda)) {
        jogador -> pos.x -= jogador -> vel * delta;
    }

    if(jogador->pos.x < 0) {
        jogador->pos.x = 0;
    } else if(jogador->pos.x + jogador->dim.x > GetScreenWidth()) {
        jogador->pos.x = GetScreenWidth() - jogador->dim.x;
    }
}

void mexerBolinha(Bolinha *bolinha, float delta) {
    bolinha->pos.x += bolinha->vel.x * delta;
    bolinha->pos.y += bolinha->vel.y * delta;

    // Colisão com as laterais
    if (bolinha->pos.x + bolinha->raio > GetScreenWidth()) {
        bolinha->pos.x = GetScreenWidth() - bolinha->raio;
        bolinha->vel.x *= -1;
        PlaySound(somColisao);  // TOCA O SOM NA PAREDE
    } else if (bolinha->pos.x - bolinha->raio < 0) {
        bolinha->pos.x = bolinha->raio;
        bolinha->vel.x *= -1;
        PlaySound(somColisao);  // TOCA O SOM NA PAREDE
    }

    // Colisão com a parte de baixo (perdeu uma vida)
    if ( bolinha->pos.y + bolinha->raio > GetScreenHeight() ) {
        
        bolinha->pos.x = GetScreenWidth() / 2;
        bolinha->pos.y = GetScreenHeight() - 90;

        bolinha->vel.y = -200; // Sempre sobe depois de perder vida

        // Velocidade X aleatória entre -150 e 150, para nao ficar reto
        float velX = (rand() % 301) - 150; 
        if (velX == 0) {
            velX = 50; //GARANTE QUE NAO FIQUE PARADO
        }
        bolinha->vel.x = velX;

        vidas--;

        if (vidas > 0) {
            perdeuVida = true;
            PlaySound(somPerdeuVida);
        }
    } 
    // Colisão com o topo
    else if ( bolinha->pos.y - bolinha->raio < 0 ) {
        bolinha->pos.y = bolinha->raio;
        bolinha->vel.y = -bolinha->vel.y;
        PlaySound(somColisao);  // TOCA O SOM NA PAREDE
    }
}


void colisaoJogador(Bolinha *bolinha, Jogador *jogador) {
    bool colidiu = CheckCollisionCircleRec(
        bolinha -> pos,
        bolinha -> raio,
        (Rectangle) {
            .x = jogador -> pos.x,
            .y = jogador -> pos.y,
            .width = jogador -> dim.x,
            .height = jogador -> dim.y
        }
    );

    if (colidiu) {
        bolinha->pos.y = jogador->pos.y - bolinha->raio;
        bolinha->vel.y = -bolinha->vel.y;
        PlaySound(somColisao);  // TOCA O SOM AO COLIDIR COM O JOGADOR
    }  
}

void colisaoObstaculos(Bolinha *bolinha) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if(retangulo[i][j].ativo && CheckCollisionCircleRec(
                        bolinha->pos, 
                        bolinha->raio, 
                        (Rectangle){
                            .x = retangulo[i][j].pos.x,
                            .y = retangulo[i][j].pos.y,
                            .width = retangulo[i][j].dim.x, 
                            .height = retangulo[i][j].dim.y
                        })) {
                    retangulo[i][j].ativo = false;
                    bolinha->vel.y *= -1;
                    totalDeRetangulos--;

                    switch (i) {
                        case 0:
                            jogador.pontos += pontosPorLinha[0];
                            break;
                        case 1:
                            jogador.pontos += pontosPorLinha[1];
                            break;
                        case 2:
                            jogador.pontos += pontosPorLinha[2];
                            break;
                        case 3:
                            jogador.pontos += pontosPorLinha[3];
                            break;
                        case 4:
                            jogador.pontos += pontosPorLinha[4];
                            break;
                        case 5:
                            jogador.pontos += pontosPorLinha[5];
                            break;
                        case 6:
                            jogador.pontos += pontosPorLinha[6];
                            break;
                        case 7:
                            jogador.pontos += pontosPorLinha[7];
                            break;
                        default:
                            jogador.pontos += 0;
                            break;
                    }
                PlaySound(somColisao);
            }  
        }
    }
}

void desenharObstaculos() {
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            if(retangulo[i][j].ativo){
                DrawRectangleV(
                    retangulo[i][j].pos,
                    retangulo[i][j].dim,
                    retangulo[i][j].cor
                );
            }          
        }
    }
}

void desenharJogador(Jogador *jogador) {
    if(perdeuVida) {
        jogador->pos.x = GetScreenWidth() / 2 - 55;
        jogador->pos.y = GetScreenHeight() - 60;
    }

    if(venceu) {
        jogador->pos.x = GetScreenWidth() / 2 - 55;
        jogador->pos.y = GetScreenHeight() - 60;
    }

    DrawRectangle(
        jogador->pos.x,
        jogador->pos.y,
        jogador->dim.x,
        jogador->dim.y,
        jogador->cor
    );
}

void desenharBolinha(Bolinha *bolinha) {
    DrawCircle(
        bolinha->pos.x,
        bolinha->pos.y,
        bolinha->raio,
        bolinha->cor
    );
}