/*********************************************************
 * INSTITUTO TECNOLOGICO DE COSTA RICA 
 * CURSO: DISEÑO DE SISTEMAS DIGITALES
 * AUTOR: ANGIE KARINA 
 *********************************************************/

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

//DEFINICIÓN DE CONSTANTES
#define SCREEN_WIDTH 512 //ANCHO
#define SCREEN_HEIGHT 416 //ALTO
#define MOVEMENT_DELTA 2
#define MAX_ENEMIES_ON_SCREEN 3
#define MAX_ENEMIES_LEVEL 20


//DIRECCION TANQUE
#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3

#define MAX_BLOCKS 1000  // Número máximo de bloques permitidos en el juego

#define MAX_BULLETS 5  // Número máximo de balas activas
#define BULLET_SPEED 4
#define MAX_ENEMY_BULLETS 10


//ESTRUTURA QUE MANEJA CONDICIONES DE JUEGO
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_WIN,
    STATE_TRANSITION 
} GameState;

GameState game_state = STATE_MENU;


//MANEJO DE ENEMIGOS POR NIVEL

// Nivel 1
int enemy_order_level_1[MAX_ENEMIES_LEVEL] = { 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,0 , 2, 2 };
int enemy_count_level_1 = 20;
int current_level = 1; //( dice en qué nivel estás, y con eso decidís qué mapa, enemigos, música, HUD o reglas cargar)

// Nivel 2 (solo ejemplo)
int enemy_order_level_2[MAX_ENEMIES_LEVEL] = { 0, 0, 3, 2, 0, 5, 1, 0, 0, 2, 0, 0, 3, 0, 1, 5, 0,1, 0, 5};
int enemy_count_level_2 = 20;

// Nivel 3
int enemy_order_level_3[MAX_ENEMIES_LEVEL] = { 0, 2, 0, 5, 0, 0, 2 , 0, 1, 0, 5, 0, 1, 0, 3, 0, 1, 0, 3, 0 };
int enemy_count_level_3 = 20;


// Nivel 4
int enemy_order_level_4[MAX_ENEMIES_LEVEL] = { 4, 0, 4, 2, 0, 4, 5, 4, 3, 4, 2, 4, 1, 4, 3, 2, 4, 5, 4 ,3  };

int enemy_count_level_4 = 20;

// Nivel 5
int enemy_order_level_5[MAX_ENEMIES_LEVEL] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 ,3  };
int enemy_count_level_5 = 20;


//VIDAS DE JUGADOR 
int player_lives = 5;  // Comienza con 2 vidas
SDL_Texture* hud_texture = NULL; // Para la imagen Vidaspantalla.bmp


//Estas variables se configuran en init_SDL()

SDL_Texture* menu_texture = NULL;
SDL_Texture* gameover_texture = NULL;
SDL_Texture* win_texture = NULL;

SDL_Window* window = NULL; //Es un puntero a una estructura SDL_Window, que representa la ventana del juego
SDL_Renderer* renderer = NULL; // Es un puntero a una estructura SDL_Renderer, que se usa para dibujar gráficos en la ventana
SDL_Texture* block_texture = NULL; // 
SDL_Texture* bullet_texture = NULL;  // Textura de la bala
SDL_Texture* tank_texture_base = NULL;
SDL_Texture* tank_texture_star1 = NULL;
SDL_Texture* tank_texture_star2 = NULL;
SDL_Texture* tank_texture_star3 = NULL;
SDL_Texture* enemy2_texture = NULL;



SDL_Texture* enemy1_gray_texture = NULL;
SDL_Texture* enemy1_red_texture = NULL;

SDL_Texture* numeros_texture = NULL;
SDL_Texture* enemigo_icon_texture = NULL;
SDL_Texture* shield_texture = NULL;
SDL_Texture* powerup_texture = NULL;


// Blindado (cambia de color por vida)
SDL_Texture* tank_blindado_4 = NULL;  // gris
SDL_Texture* tank_blindado_3 = NULL;  // amarillo
SDL_Texture* tank_blindado_2 = NULL;  // verde
SDL_Texture* tank_blindado_1 = NULL;  // rojo

// Power-Up enemy (alternante)
SDL_Texture* tank_powerup_gray = NULL;
SDL_Texture* tank_powerup_red = NULL;

SDL_Texture* enemy2_alt_texture = NULL;

// VARIABLES GLOBALES PARA ENEMIGOS

const int spawn_points[3] = {32, 256-32, 416}; 
// Puntos X donde pueden aparecer enemigos (3 posiciones fijas arriba del mapa)

int active_enemies = 0;       
// Número actual de enemigos vivos en pantalla

int remaining_enemies = 20;   
// Enemigos que aún faltan por aparecer en este nivel

int* current_enemy_order = NULL;
// Apunta al array que indica el tipo de enemigos a aparecer en el nivel actual

int current_enemy_count = 0;
// Número total de enemigos definidos para este nivel (tamaño de current_enemy_order)

int next_enemy_index = 0;
// Índice del próximo enemigo que se debe generar a partir de current_enemy_order

Uint32 last_spawn_time = 0;
// Tiempo (en ms) en el que se activó el último destello de aparición

const Uint32 spawn_delay = 3000; 
// Espera mínima de 3 segundos entre cada aparición de enemigos

int spawn_index = 0;
// Controla qué punto de aparición usar (entre los 3 disponibles); se incrementa secuencialmente

int player_start_x;
int player_start_y;
// Posiciones iniciales del jugador al cargar un nivel (se usan para reiniciar)

int score = 0;
// Puntaje acumulado del jugador

void reiniciar_juego_completo();


void spawn_powerup(int x, int y, int type);
// Función que genera un nuevo power-up en la posición (x, y) con el tipo indicado

Uint32 last_powerup_time = 0;
// Marca el tiempo en que apareció el último power-up

const Uint32 powerup_interval = 15000; 
// Tiempo mínimo entre apariciones automáticas de power-ups (15 s)

const Uint32 POWERUP_DURATION = 10000; 
// Duración activa de un power-up como el escudo o el congelamiento (10 s)


int freeze_enemies = 0;
// Indica si los enemigos están congelados (1) o no (0)

Uint32 freeze_start_time = 0;
// Momento en que se activó el congelamiento

const Uint32 FREEZE_DURATION = 5000; 
// Duración del efecto freeze: enemigos paralizados por 5 s


int metal_active = 0;
// Indica si la base está protegida con bloques de metal

Uint32 metal_start_time = 0;
// Tiempo en que se activó la protección metálica

const Uint32 METAL_DURATION = 10000; 
// Duración del blindaje metálico: 10 segundos

Uint32 last_enemy_spawn_time = 0;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        ESTRUCTURAS PARA JUEGO
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

///////////////////////////////////
// ESTRUCTURA PARA JUGADOR TANQUE
///////////////////////////////////
typedef struct PlayerTank_t {
    int x, y;
    int w, h;
    int direction;
    SDL_Texture* texture;

    // ESCUDO DE PROTECCIÓN
    int shield_active;
    Uint32 shield_start_time;
    int shield_frame;
    int star_level;          // Nivel de estrella (0 a 3)
    Uint32 last_shot_time;  // Último disparo (para cooldown)

} PlayerTank_t;

PlayerTank_t player;
int WalkPlayer = 0;
int pause = 0;

//////////////////////////////
// ESTRUCTURA PARA BLOQUES 
/////////////////////////////
typedef struct Block_t {
    int x, y;             // Posición del bloque
    int w, h;             // Tamaño del bloque
    int type;             // Tipo del bloque (0 = destructible, 1 = indestructible)
    SDL_Texture* texture; // Textura del bloque
    SDL_Rect srcRect;     // Parte del spritesheet a extraer
} Block_t;

//ALMACENAMIENTO DE BLOQUES
Block_t blocks[MAX_BLOCKS];  // Lista donde guardaremos los bloques
int num_blocks = 0;  // Número de bloques en la pantalla




///////////////////////////
// ESTRUCTURA DE LA BALA //
///////////////////////////
typedef struct Bullet_t {
    int x, y;      // Posición de la bala
    int w, h;      // Tamaño de la bala
    int direction; // Dirección en la que viaja
    int active;    // Si la bala está activa o no
} Bullet_t;

// Arreglo de balas (máximo 3 activas)
Bullet_t bullets[MAX_BULLETS] = {0};

Bullet_t enemy_bullets[MAX_ENEMY_BULLETS] = {0};


//////////////////////////
// ESTRUCTURA PARA ENEMIGOS
//////////////////////////
typedef struct EnemyTank {
    int x, y;             // Posición del enemigo
    int w, h;             // Tamaño del tanque enemigo
    int direction;        // Dirección en la que se mueve
    int speed;            // Velocidad del enemigo
    int type;             // Tipo de enemigo (0 = normal, 1 = alternante, 2 = rápido)
    int color_toggle;     // Alternar color (para enemigos tipo 1)
    int health;           // Vida del enemigo (1 impacto = destrucción)
    int frame;            // Para animación del tanque (como el jugador)
    SDL_Texture* texture; // Textura del enemigo
    struct EnemyTank* next; // Apuntador al siguiente enemigo en la lista (para lista enlazada)
} EnemyTank_t;



//LISTA PARA ENEMIGOS
EnemyTank_t* enemy_list = NULL; // Lista enlazada para gestionar enemigos

///////////////////////////////
// ESTRUCTURA PARA EFECTO SPAWN
///////////////////////////////


typedef struct SpawnEffect {
    int x, y;               // Posición
    Uint32 start_time;      // Momento en que comenzó la animación
    int frame;              // Frame actual (0 a 3)
    struct SpawnEffect* next;
} SpawnEffect_t;

SpawnEffect_t* spawn_list = NULL;
SpawnEffect_t* player_spawn_effect = NULL;

SDL_Texture* spawn_texture = NULL;



int count_active_spawn_effects() {
    int count = 0;
    SpawnEffect_t* current = spawn_list;
    while (current) {
        count++;
        current = current->next;
    }
    return count;
}



//////////////////////////
// ESTRUCTURA PARA AGUILA 
/////////////////////////
typedef struct Base_t {
    int x, y;
    int w, h;
    int destroyed;
    SDL_Texture* texture;
} Base_t;

Base_t base;



///////////////////////////
//ESTRUCTURA PARA POWER UP
///////////////////////////

typedef struct PowerUp {
    int x, y;
    int type; // 0=helmet, 1=timer, 2=shovel, 3=star, 4=grenade, 5=tank
    Uint32 spawn_time;
    int active;
    struct PowerUp* next;
} PowerUp_t;

PowerUp_t* powerup_list = NULL;

///////////////////////////////////////
// CARGAR IMAGEN COMO `SDL_Texture*`//
//////////////////////////////////////
/*La función load_texture(const char* path) 
se encarga de cargar una imagen desde un archivo 
BMP y convertirla en una textura (SDL_Texture*) 
que SDL puede usar para renderizar en pantalla*/

SDL_Texture* load_texture(const char* path) {
    SDL_Surface* tempSurface = SDL_LoadBMP(path);
    if (!tempSurface) {
        printf("Error al cargar imagen %s: %s\n", path, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}


//ESTE ME PERMITE QUITAR FONDO NEGRO

SDL_Texture* load_texture_with_transparency(const char* path, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Surface* tempSurface = SDL_LoadBMP(path);
    if (!tempSurface) {
        printf("Error al cargar imagen %s: %s\n", path, SDL_GetError());
        return NULL;
    }

    // Hacer transparente el color especificado
    SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, r, g, b));

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}



/////////////////////  Se encarga de inicializar la biblioteca SDL
// Inicializar SDL///  y configurar la ventana y el renderizador para 
/////////////////////  el juego (PANTALLA)

int init_SDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    
    window = SDL_CreateWindow("❤ Battle City By Angie ❤ ",
     SDL_WINDOWPOS_CENTERED,
     SDL_WINDOWPOS_CENTERED, 
     SCREEN_WIDTH, SCREEN_HEIGHT, 
     SDL_WINDOW_SHOWN);
    if (!window) return -1;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return -1;

    return 0;
}



///////////////////////
//CARGAS DE IMAGENES //
//////////////////////

// Cargar la imagen del tanque
int load_tank_sprite() {
   player.texture = load_texture_with_transparency("TankPlayer.bmp", 0, 0, 0);

    return (player.texture) ? 0 : -1;
}




/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        DECLARACIÓN DE FUNCIONES 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


/////////////////////////
//FUNCIONES QUE DIBUJAN
/////////////////////////


//DIBUJA MARCO DE JUEGO GRIS
void draw_frame(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 96, 96, 96, 255); // Color gris

    // Marco superior (Barra horizontal arriba)
    SDL_Rect topWall = {0, 0, 768, 16}; 

    // Marco inferior (Barra horizontal abajo)
    SDL_Rect bottomWall = {0, SCREEN_HEIGHT - 16, 768, 16}; 

    // Marco izquierdo (Barra vertical izquierda)
    SDL_Rect leftWall = {0, 0, 32, 576};  

    // Marco derecho (Barra vertical derecha)
    SDL_Rect rightWall = {SCREEN_WIDTH - (64), 0, 64, 576}; 

    // Dibujar los rectángulos
    SDL_RenderFillRect(renderer, &topWall);
    SDL_RenderFillRect(renderer, &bottomWall);
    SDL_RenderFillRect(renderer, &leftWall);
    SDL_RenderFillRect(renderer, &rightWall);
}


//DIBUJA BLOQUES 
void draw_blocks() {
    for (int i = 0; i < num_blocks; i++) {
        SDL_Rect dstRect = {blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h};

        if (blocks[i].texture == NULL) {
            printf("Error: La textura del bloque %d es NULL.\n", i);
            continue;
        }

        // Renderiza solo una parte de la imagen usando srcRect
        SDL_RenderCopy(renderer, blocks[i].texture, &blocks[i].srcRect, &dstRect);
    }
}

//DIBUJO TANQUE JUGADOR
void draw_tank(PlayerTank_t* player) {
    SDL_Rect srcRect = {
        player->direction * (16* 2) + WalkPlayer *16,  // Mantener tamaño original del sprite
        0, 
        16, // Ancho del sprite original
        16  // Alto del sprite original
    }; 

    SDL_Rect dstRect = {player->x, player->y, player->w, player->h}; // Tamaño escalado en pantalla

    SDL_RenderCopyEx(renderer, player->texture, &srcRect, &dstRect, 0, NULL, SDL_FLIP_NONE);
}


// DIBUJAR LAS BALAS
void draw_bullets() {
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);  // Color gris

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = {bullets[i].x, bullets[i].y, 6, 6};  // Tamaño 10x10
            SDL_RenderFillRect(renderer, &bulletRect);  // Dibujar la bala
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Restaurar color negro de fondo
}

//BALAS ENEMIGAS
void draw_enemy_bullets() {
    SDL_SetRenderDrawColor(renderer, 96, 96, 96, 255);  // Gris para distinguir

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active) {
            SDL_Rect bulletRect = {enemy_bullets[i].x, enemy_bullets[i].y, 6, 6};
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Restaurar color fondo
}


//DIUJA ENEMIGOS
void draw_enemies() {
    EnemyTank_t* current = enemy_list;
    while (current) {
        SDL_Rect srcRect = {
            (current->direction * 32) + (current->frame * 16),  //FRAME ANIMACIÓN DE MOVIMIENTO 
            0,
            16,
            16
        };

        SDL_Rect dstRect = {
            current->x,
            current->y,
            current->w,
            current->h
        };

        SDL_RenderCopy(renderer, current->texture, &srcRect, &dstRect);

        current = current->next;
    }
}

//CHECK COLLISION PARTE GRIS

void check_collision_with_frame(PlayerTank_t* player) {
    if (player->x < 32) player->x = 32;  // Límite izquierdo
    if (player->x + player->w > SCREEN_WIDTH - 128/2) player->x = SCREEN_WIDTH - 128/2 - player->w;  // Límite derecho
    if (player->y < 16) player->y = 16;  // Límite superior
    if (player->y + player->h > SCREEN_HEIGHT - 16) player->y = SCREEN_HEIGHT - 16 - player->h;  // Límite inferior
}

//CHECK COLLISION BLOQUES
int check_collision_with_blocks(int x, int y, int w, int h) {
    // Colisión con bloques normales
    for (int i = 0; i < num_blocks; i++) {
        int block_left = blocks[i].x;
        int block_right = blocks[i].x + blocks[i].w;
        int block_top = blocks[i].y;
        int block_bottom = blocks[i].y + blocks[i].h;
        if (blocks[i].type == 2){
            continue; 
        }
        if (x + w > block_left && x < block_right && y + h > block_top && y < block_bottom){
            return 1;  //  Colisión con bloque
        }
    }

    // Colisión con la base (águila)
    SDL_Rect entity = {x, y, w, h};
    SDL_Rect eagle = {base.x, base.y, base.w, base.h};
    if (SDL_HasIntersection(&entity, &eagle)) {
        return 1;  // Colisión con la base
    }

    return 0;  // No hay colisión
}




//AGREGA ENEMIGO
void add_enemy(int type, int spawn_x) {
    if (active_enemies >= MAX_ENEMIES_ON_SCREEN || remaining_enemies <= 0) return;

// Se reserva dinámicamente memoria para un nuevo enemigo del tipo EnemyTank_t
// malloc devuelve un puntero tipo void*, por eso se convierte (cast) a EnemyTank_t*
// sizeof(EnemyTank_t) indica cuántos bytes necesita reservar para guardar todos los datos de la estructura
// El puntero new_enemy apunta a esa nueva estructura en memoria, que se usará para inicializar y manejar el enemigo

    EnemyTank_t* new_enemy = (EnemyTank_t*)malloc(sizeof(EnemyTank_t));
    if (!new_enemy) {
        printf(" Error: No se pudo asignar\n");
        return;
    }

    new_enemy->x = spawn_x;
    new_enemy->y = 20;
    new_enemy->w = 30;
    new_enemy->h = 30;
    new_enemy->direction = rand() % 4;
    new_enemy->type = type;
    new_enemy->color_toggle = 0;
    new_enemy->health = 1;
    new_enemy->frame = 0;

    // Asignar velocidad según tipo
    if (type == 0 || type == 1) {
        new_enemy->speed = 1;
    } else if (type == 2 || type == 5) {  // Tipo 5 también es rápido
        new_enemy->speed = 1.7;
    } else if (type == 3 || type == 4) {
        new_enemy->speed = 1;
    }

    // Asignar textura según tipo
    if (type == 0) {
        new_enemy->texture = load_texture("Enemigo1.bmp");
    } else if (type == 1) {
        new_enemy->texture = enemy1_gray_texture;
    } else if (type == 2) {
        new_enemy->texture = enemy2_texture;
    } else if (type == 3) {
        new_enemy->texture = tank_blindado_4;
        new_enemy->health = 4;  // Blindado empieza con 4 vidas
    } else if (type == 4) {
        new_enemy->texture = tank_powerup_gray;
    } else if (type == 5) {
        new_enemy->texture = enemy2_texture;  // Comienza gris también
    }

    if (!new_enemy->texture) {
        printf("Error al cargar la textura del enemigo tipo %d.\n", type);
        free(new_enemy);
        return;
    }
    //LISTA DE ENEMIGOS
    new_enemy->next = enemy_list;
    enemy_list = new_enemy;
    active_enemies++;
    remaining_enemies--;

    printf("Enemigo en (%d, %d), tipo: %d, velocidad: %d\n",
           new_enemy->x, new_enemy->y, new_enemy->type, new_enemy->speed);
}

//MUEVEE ENEMIGOS


int will_enemy_collide_with_player(int x, int y, int w, int h, PlayerTank_t* player) {
    SDL_Rect futureEnemy = {x, y, w, h};
    SDL_Rect playerRect = {player->x, player->y, player->w, player->h};

    if (SDL_HasIntersection(&futureEnemy, &playerRect)) {
        return 1; // Colisión detectada
    }
    return 0;
}



// Verifica si un enemigo colisionaría con otro enemigo
int will_collide_with_other_enemy(int x, int y, int w, int h, EnemyTank_t* self) {
    EnemyTank_t* current = enemy_list;
    SDL_Rect futureRect = {x, y, w, h};

    while (current) {
        if (current != self) {
            SDL_Rect otherRect = {current->x, current->y, current->w, current->h};

            int dx = abs(futureRect.x - otherRect.x);
            int dy = abs(futureRect.y - otherRect.y);

            // Permitir una pequeña superposición (ajustá este valor si querés más libertad)
            if (dx < w - 4 && dy < h - 4) {
                return 1; // Aún se considera colisión
            }
        }
        current = current->next;
    }
    return 0;
}


int will_collide_with_spawn_effect(int x, int y, int w, int h) {
    SDL_Rect futureEnemy = {x, y, w, h};
    SpawnEffect_t* current = spawn_list;

    while (current) {
        SDL_Rect spawnRect = {current->x + 8, current->y, 30, 30}; // Tamaño del destello
        if (SDL_HasIntersection(&futureEnemy, &spawnRect)) {
            return 1; // Colisión detectada
        }
        current = current->next;
    }
    return 0; // No hay colisión
}



void move_enemies() 
{

    if (freeze_enemies) return; //Si los enemigos están congelados (por  power-up), no se mueven

    EnemyTank_t* current = enemy_list; //Usa un puntero temporal current para recorrer la lista enlazada de enemigos activos
    while (current) {           
        int prev_x = current->x;  // Guardamos la posición actual por si hay que retroceder luego
        int prev_y = current->y;

        // Mover al enemigo en su dirección actual
     int next_x = current->x;   
int next_y = current->y;

switch (current->direction) {   //Según la dirección del enemigo, calcula su próxima posición (next_x, next_y)
    case UP:    next_y -= current->speed; break;
    case DOWN:  next_y += current->speed; break;
    case LEFT:  next_x -= current->speed; break;
    case RIGHT: next_x += current->speed; break;
}

// Verificar colisión con jugador y enemigos antes de mover (si no choca con el jugador, otro enemigo o un efecto de aparición, se actualiza la posición del enemigo)
if (!will_enemy_collide_with_player(next_x, next_y, current->w, current->h, &player) &&
    !will_collide_with_other_enemy(next_x, next_y, current->w, current->h, current) &&
    !will_collide_with_spawn_effect(next_x, next_y, current->w, current->h)) {
    
    current->x = next_x;
    current->y = next_y;
}

// DETECTAR COLISIÓN CON BLOQUES 
// Usamos una colisión suavizada
//Recorro todos los bloques (menos tipo 2) y detecta si el enemigo colisiona con alguno, usando una colisión suavizada (permite un margen de error visual)
int colision = 0;
for (int i = 0; i < num_blocks; i++) {
    if (blocks[i].type != 2) {  // Ignorar tipo 2 si usás bloques especiales
        SDL_Rect blockRect = {blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h};
        SDL_Rect enemyRect = {current->x, current->y, current->w, current->h};

        int dx = abs(enemyRect.x - blockRect.x);
        int dy = abs(enemyRect.y - blockRect.y);

        if (dx < current->w - 2 && dy < current->h - 2 &&
            SDL_HasIntersection(&enemyRect, &blockRect)) {
            colision = 1;
            break;
        }
    }
}

if (colision) {
    // Retroceder antes de cambiar dirección
    //Si hay colisión con un bloque, deshace el movimiento y 
    //le da una nueva dirección aleatoria para que intente moverse hacia otro lado
    switch (current->direction) {
        case UP: current->y += current->speed; break;
        case DOWN: current->y -= current->speed; break;
        case LEFT: current->x += current->speed; break;
        case RIGHT: current->x -= current->speed; break;
    }

    current->direction = rand() % 4; //dirención en 4 posiciones diferente de forma random
}



        // DETECTAR COLISIÓN CON LOS BORDES 
        if (current->x < 32) {  
            current->x = 32;  
            current->direction = RIGHT;  
        } 
        if (current->x + current->w > SCREEN_WIDTH - 32-16-16) {  
            current->x = SCREEN_WIDTH - 32-16-16 - current->w;  
            current->direction = LEFT;  
        } 
        if (current->y < 16) {  
            current->y = 16;  
            current->direction = DOWN;  
        } 
        if (current->y + current->h > SCREEN_HEIGHT - 16) {  
            current->y = SCREEN_HEIGHT - 32 - current->h;  
            current->direction = UP;  
        }
        // Cambiar de dirección aleatoriamente cada cierto tiempo
if (rand() % 100 < 3) { // 3% de probabilidad en cada frame
    current->direction = rand() % 4;
}


     // Si el enemigo realmente se movió, alternar animación
        if (prev_x != current->x || prev_y != current->y) {
            current->frame ^= 1;  // Alternar entre 0 y 1 para animación
        }
        current = current->next;
   }
}



void draw_hud() {
    SDL_Rect hudRect = {
        SCREEN_WIDTH - 128+32+32, // A la derecha del área jugable
        0,                  // Desde arriba
        32+16+16,                // Ancho del HUD (ajustalo si tu imagen es más pequeña)
        384+32                 // Alto total del área jugable
    };
    SDL_RenderCopy(renderer, hud_texture, NULL, &hudRect);
}


void draw_player_lives() {
    for (int i = 0; i < player_lives; i++) {
        SDL_Rect lifeRect = {
            SCREEN_WIDTH - 128 + 40 + i * 12,  // Posición relativa a "IP"
            SCREEN_HEIGHT - 20,               // Altura alineada con HUD
            8, 8
        };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo
        SDL_RenderFillRect(renderer, &lifeRect);
    }
}

void draw_enemy_counter() {
    int enemies_to_draw = remaining_enemies;
    int start_x = SCREEN_WIDTH - 24;
    int start_y = 40;

    for (int i = 0; i < enemies_to_draw; i++) {
        int row = i / 2;
        int col = i % 2;

        SDL_Rect rect = {
            start_x - col * 10,
            start_y + row * 10,
            8, 8
        };

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo
        SDL_RenderFillRect(renderer, &rect);
    }
}

void draw_lives_number() {
    SDL_Rect src = {
        player_lives * 8,  // X: posición del número (0 al 9)
        0,                 // Y: los números están en una sola fila
        8, 8
    };

    SDL_Rect dst = {
        SCREEN_WIDTH - 128+64+32+2+2 ,   // ancho Coordenadas donde irá el número (ajustá si hace falta)
        SCREEN_HEIGHT - 128-32-2-2,    //alto 
        12, 12
    };

    SDL_RenderCopy(renderer, numeros_texture, &src, &dst);
}


void draw_enemy_icons() {
    int count = remaining_enemies;
    int start_x = SCREEN_WIDTH - 24-10; //POSICIÓN EN X (horizontal)
    int start_y = 40;                //POSICIÓN EN Y (vertical)

    for (int i = 0; i < count; i++) {
        int row = i / 2;
        int col = i % 2;

        SDL_Rect dst = {
            start_x - col * 16,  //Separación horizontal
            start_y + row * 16,  //Separación vertical 
            16, 16                     //TAMAÑO del ícono en pantalla
        };

        SDL_Rect src = {0, 0, 8, 8};  // Solo hay un ícono (Toma todo el sprite de 8x8)

        SDL_RenderCopy(renderer, enemigo_icon_texture, &src, &dst);
    }
}


//NUMERO DE NIVEL INDICADOR

void draw_level_number() {
    SDL_Rect src = {
        current_level * 8,  // Cada número es de 8x8 en el sprite
        0,
        8, 8
    };

    SDL_Rect dst = {
        SCREEN_WIDTH - 128 + 48+32+8,  // Posición mismo X que el número de vidas
        SCREEN_HEIGHT - 32-32,  //  Un poco más abajo que el número de vidas
        12, 12                      // Tamaño visible del número
    };

    SDL_RenderCopy(renderer, numeros_texture, &src, &dst);
}


//AGREGA BLOQUES

void add_block(int x, int y, int w, int h, int type, SDL_Texture* texture) {
    if (num_blocks >= MAX_BLOCKS) {
      
        return;
    }

    blocks[num_blocks].x = x;
    blocks[num_blocks].y = y;
    blocks[num_blocks].w = w;
    blocks[num_blocks].h = h;
    blocks[num_blocks].type = type;  // Asigna correctamente el tipo
    blocks[num_blocks].texture = texture;

    blocks[num_blocks].srcRect.x = type * 8;  
    blocks[num_blocks].srcRect.y = 0;          
    blocks[num_blocks].srcRect.w = 8;        
    blocks[num_blocks].srcRect.h = 8;


    num_blocks++;  
}

///////////////////////////////////
//FUNCION DE MOVIMIENTO DEL TANQUE
//////////////////////////////////
// COLISIÓN ENTRE EL TANQUE JUGADOR Y ENEMIGOS
// EVITA QUE EL JUGADOR ATRAVIESE ENEMIGOS
int will_collide_with_enemy(int x, int y, int w, int h) {
    EnemyTank_t* current = enemy_list;
    SDL_Rect futurePlayer = {x + 2, y + 2, w - 4, h - 4};  // Margen de tolerancia

    while (current) {
        SDL_Rect enemyRect = {current->x, current->y, current->w, current->h};
        if (SDL_HasIntersection(&futurePlayer, &enemyRect)) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}



// Alternar animación del tanque
void WalkTank() {
    WalkPlayer ^= 1;
}

void move_tank(const Uint8* keystate, PlayerTank_t* player) {
    int new_x = player->x;
    int new_y = player->y;

    float speed = MOVEMENT_DELTA;
    if (player->star_level >= 2) {
        speed = 3;  // Aumenta la velocidad si tiene 2 estrellas o más
    }


    if (keystate[SDL_SCANCODE_W] && !keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_D]) {
        player->direction = UP;
        new_y -= speed;
    } 
    else if (keystate[SDL_SCANCODE_S] && !keystate[SDL_SCANCODE_W] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_D]) {
        player->direction = DOWN;
        new_y += speed;
    } 
    else if (keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_W] && !keystate[SDL_SCANCODE_S]) {
        player->direction = LEFT;
        new_x -= speed;
    } 
    else if (keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_W] && !keystate[SDL_SCANCODE_S]) {
        player->direction = RIGHT;
        new_x += speed;
    }

    // Verifica colisión con el marco gris
    check_collision_with_frame(player);

    // Simula el movimiento y verifica colisión con bloques antes de mover el tanque
    int prev_x = player->x;
    int prev_y = player->y;

// Solo mover si no hay colisión con bloques NI enemigos
if (!check_collision_with_blocks(new_x + 2, new_y + 2, player->w - 4, player->h - 4) &&
    !will_collide_with_enemy(new_x + 2, new_y + 2, player->w - 4, player->h - 4)) {


    player->x = new_x;
    player->y = new_y;

    WalkTank();  // Alternar animación solo si se movió
}

}


// FUNCIÓN PARA DISPARAR JUGADOR
void shoot_bullet() {
    int max_bullets = 1;
    if (player.star_level >= 1) {
        max_bullets = 2; // Doble disparo si tiene al menos 1 estrella
    }

    // Contar balas activas
    int active_count = 0;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) active_count++;
    }

    if (active_count >= max_bullets) return; // No puede disparar más

    // Crear nueva bala
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x + player.w / 2 - 4;
            bullets[i].y = player.y + player.h / 2 - 4;
            bullets[i].w = 6;
            bullets[i].h = 6;
            bullets[i].direction = player.direction;
            bullets[i].active = 1;
            break;
        }
    }
}


//FUNCIÓN DISPARO ENEMIGO
void enemy_shoot_bullet(EnemyTank_t* enemy) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].active) {
            enemy_bullets[i].x = enemy->x + enemy->w / 2 - 3;
            enemy_bullets[i].y = enemy->y + enemy->h / 2 - 3;
            enemy_bullets[i].w = 6;
            enemy_bullets[i].h = 6;
            enemy_bullets[i].direction = enemy->direction;
            enemy_bullets[i].active = 1;
            break;
        }
    }
}



// MOVIMIENTO DE BALAS
void update_bullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            // Mover según la dirección
            switch (bullets[i].direction) {
    case UP: bullets[i].y -= BULLET_SPEED; break;
    case DOWN: bullets[i].y += BULLET_SPEED; break;
    case LEFT: bullets[i].x -= BULLET_SPEED; break;
    case RIGHT: bullets[i].x += BULLET_SPEED; break;
}

            // Si la bala sale de la pantalla, desactivarla
            if (bullets[i].x < 0 || bullets[i].x > SCREEN_WIDTH ||
                bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT) {
                bullets[i].active = 0;
               }
              }
             }
            }
//BALAS ENEMIGAS

void update_enemy_bullets() {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active) {
            switch (enemy_bullets[i].direction) {
                case UP: enemy_bullets[i].y -= BULLET_SPEED; break;
                case DOWN: enemy_bullets[i].y += BULLET_SPEED; break;
                case LEFT: enemy_bullets[i].x -= BULLET_SPEED; break;
                case RIGHT: enemy_bullets[i].x += BULLET_SPEED; break;
            }

            if (enemy_bullets[i].x < 0 || enemy_bullets[i].x > SCREEN_WIDTH ||
                enemy_bullets[i].y < 0 || enemy_bullets[i].y > SCREEN_HEIGHT) {
                enemy_bullets[i].active = 0;
            }
        }
    }
}


//
void update_enemies(Uint32 current_time) {
    if (freeze_enemies) return;

    static Uint32 last_toggle_time = 0;
    const Uint32 toggle_interval = 500;

    if (current_time - last_toggle_time >= toggle_interval) {
        last_toggle_time = current_time;

        EnemyTank_t* current = enemy_list;
        while (current) {
            // Alternar color para tipos con cambio visual
            if (current->type == 1 || current->type == 2 || current->type == 4 || current->type == 5) {
                current->color_toggle = !current->color_toggle;

                if (current->type == 1) {
                    current->texture = current->color_toggle ? enemy1_red_texture : enemy1_gray_texture;
                } else if (current->type == 2 || current->type == 5) {
                    current->texture = current->color_toggle ? enemy2_alt_texture : enemy2_texture;
                } else if (current->type == 4) {
                    current->texture = current->color_toggle ? tank_powerup_red : tank_powerup_gray;
                }
            }

            // Posible disparo
            if (rand() % 3 < 1) {
                enemy_shoot_bullet(current);
            }

            current = current->next;
        }
    }
}


//////////////////////////////
// COLISIÓN DE BALAS CON BLOQUES
//////////////////////////////
void check_bullet_collision() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            for (int j = 0; j < num_blocks; j++) {
               
                
                // Si la bala impacta un bloque de tipo 0, lo destruye
                if (blocks[j].type == 0 &&
                    bullets[i].x + bullets[i].w > blocks[j].x &&
                    bullets[i].x < blocks[j].x + blocks[j].w &&
                    bullets[i].y + bullets[i].h > blocks[j].y &&
                    bullets[i].y < blocks[j].y + blocks[j].h) {

                
                    // Destruir el bloque y la bala
                    blocks[j] = blocks[num_blocks - 1]; // Mueve el último bloque a la posición actual
                    num_blocks--; 
                    bullets[i].active = 0; // Desactivar la bala
                    break; 
                }

                // Si la bala impacta un bloque de tipo 1, simplemente se detiene
                if (blocks[j].type == 1 &&
                bullets[i].x + bullets[i].w > blocks[j].x &&
                bullets[i].x < blocks[j].x + blocks[j].w &&
                bullets[i].y + bullets[i].h > blocks[j].y &&
                bullets[i].y < blocks[j].y + blocks[j].h) {

                if (player.star_level >= 3) {
                    // Destruir bloque metálico si tiene 3 estrellas
                    blocks[j] = blocks[num_blocks - 1];
                    num_blocks--;
                }

                bullets[i].active = 0; // Bala siempre se detiene
                break;
            }

            }


// Colisión con el águila (base)
SDL_Rect bulletRect = { bullets[i].x, bullets[i].y, bullets[i].w, bullets[i].h };
SDL_Rect eagleRect = { base.x, base.y, base.w, base.h };

if (!base.destroyed && SDL_HasIntersection(&bulletRect, &eagleRect)) {
    bullets[i].active = 0;
    base.destroyed = 1;
game_state = STATE_GAME_OVER;

printf("Águila destruida! GAME OVER\n");



 //reiniciar_juego_completo();

}

}
}
}



//FUNCIÓN QUE DETECTA COLISÓN CON BALA ENEMIGA
void check_bullet_enemy_collisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = {
                bullets[i].x,
                bullets[i].y,
                bullets[i].w,
                bullets[i].h
            };

            EnemyTank_t** current = &enemy_list;

            while (*current) {
                SDL_Rect enemyRect = {
                    (*current)->x,
                    (*current)->y,
                    (*current)->w,
                    (*current)->h
                };

                if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
                    bullets[i].active = 0;  // Desactiva la bala
                    (*current)->health--;

                    // Si es blindado (tipo 3), actualizar textura según la vida
                    if ((*current)->type == 3) {
                        if ((*current)->health == 3) {
                            (*current)->texture = tank_blindado_3;
                        } else if ((*current)->health == 2) {
                            (*current)->texture = tank_blindado_2;
                        } else if ((*current)->health == 1) {
                            (*current)->texture = tank_blindado_1;
                        }
                    }

                    // Si la vida llegó a 0, eliminar enemigo y aumentar puntaje
                    if ((*current)->health <= 0) {
                        // Aumentar puntaje según tipo
                        if ((*current)->type == 0 || (*current)->type == 1) {
                            score += 100;
                        } else if ((*current)->type == 2) {
                            score += 200;
                        } else if ((*current)->type == 3) {
                            score += 400;
                        } else if ((*current)->type == 4) {
                            score += 300;

                            // Generar power-up aleatorio
                            int spawn_x, spawn_y;
                            int attempts = 0;
                            int valid = 0;

                            while (!valid && attempts < 100) {
                                spawn_x = 32 + rand() % (SCREEN_WIDTH - 128 - 16);
                                spawn_y = 16 + rand() % (SCREEN_HEIGHT - 32 - 16);

                                SDL_Rect test_rect = {spawn_x, spawn_y, 16, 16};

                                valid = 1;
                                for (int i = 0; i < num_blocks; i++) {
                                    SDL_Rect block_rect = {blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h};
                                    if (SDL_HasIntersection(&test_rect, &block_rect)) {
                                        valid = 0;
                                        break;
                                    }
                                }
                            }

                            if (valid) {
                                int type = rand() % 6;
                                spawn_powerup(spawn_x, spawn_y, type);
                                printf("Power-up activado por enemigo tipo 4 en (%d, %d)\n", spawn_x, spawn_y);
                            }
                        } else if ((*current)->type == 5) {
                            score += 250;  // Tipo 5: 250 puntos
                        }

                        // Eliminar enemigo
                        EnemyTank_t* to_delete = *current;
                        *current = (*current)->next;
                        free(to_delete);
                        active_enemies--;
                        printf(" Enemigo destruido jajaj\n");
                    } else {
                        current = &(*current)->next;
                    }

                    break; // Solo puede golpear un enemigo
                } else {
                    current = &(*current)->next;
                }
            }
        }
    }
}

//FUNCIÓN QUE DETECTA COLISIÓN DE BALA CON BALA (ENMIGO/JUGADOR)
void check_bullet_vs_bullet_collisions() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = {
                bullets[i].x, bullets[i].y,
                bullets[i].w, bullets[i].h
            };

            for (int j = 0; j < MAX_ENEMY_BULLETS; j++) {
                if (enemy_bullets[j].active) {
                    SDL_Rect enemyBulletRect = {
                        enemy_bullets[j].x, enemy_bullets[j].y,
                        enemy_bullets[j].w, enemy_bullets[j].h
                    };

                    if (SDL_HasIntersection(&bulletRect, &enemyBulletRect)) {
                        // Ambas balas se destruyen
                        bullets[i].active = 0;
                        enemy_bullets[j].active = 0;
                        printf("Bala del jugador y del enemigo se destruyeron mutuamente\n");
                        break;  // Romper el inner loop si la bala del jugador ya fue destruida
                    }
                }
            }
        }
    }
}


//BULLET COLLSION ENEMIGO

void check_enemy_bullet_collisions() {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].active) {

            // Colisión con bloques
            for (int j = 0; j < num_blocks; j++) {
                if (blocks[j].type == 0 &&
                    enemy_bullets[i].x + enemy_bullets[i].w > blocks[j].x &&
                    enemy_bullets[i].x < blocks[j].x + blocks[j].w &&
                    enemy_bullets[i].y + enemy_bullets[i].h > blocks[j].y &&
                    enemy_bullets[i].y < blocks[j].y + blocks[j].h) {
                    
                    blocks[j] = blocks[num_blocks - 1]; // Eliminar bloque
                    num_blocks--;
                    enemy_bullets[i].active = 0;
                    break;
                }

                if (blocks[j].type == 1 &&
                    enemy_bullets[i].x + enemy_bullets[i].w > blocks[j].x &&
                    enemy_bullets[i].x < blocks[j].x + blocks[j].w &&
                    enemy_bullets[i].y + enemy_bullets[i].h > blocks[j].y &&
                    enemy_bullets[i].y < blocks[j].y + blocks[j].h) {

                    enemy_bullets[i].active = 0;
                    break;
                }
            }

            // Colisión con el jugador
            SDL_Rect bulletRect = {enemy_bullets[i].x, enemy_bullets[i].y, enemy_bullets[i].w, enemy_bullets[i].h};
            SDL_Rect playerRect = {player.x, player.y, player.w, player.h};

  if (!player.shield_active && SDL_HasIntersection(&bulletRect, &playerRect)) {
    enemy_bullets[i].active = 0;
    player_lives--;
    printf("Jugador impactado. Vidas restantes: %d\n", player_lives);
// Reiniciar mejoras de estrella al ser golpeado
player.star_level = 0;
player.texture = load_texture("TankPlayer.bmp");  // Volver al sprite base
printf("Tanque golpeado, mejoras perdidas.\n");

  if (player_lives <= 0) {
    printf("GAME OVER\n");
    game_state = STATE_GAME_OVER;

      reiniciar_juego_completo();
}


    else {
        // Reiniciar posición del jugador
        player.x = player_start_x;
        player.y = player_start_y;
        player.direction = UP;


        // iniciar un escudo aquí

        player.shield_active = 1;
        player.shield_start_time = SDL_GetTicks();
        player.shield_frame = 0;

    }
}

// Colisión con el águila (base)
SDL_Rect eagleRect = { base.x, base.y, base.w, base.h };
if (!base.destroyed && SDL_HasIntersection(&bulletRect, &eagleRect)) {
    enemy_bullets[i].active = 0;
    base.destroyed = 1;
    pause = 1;
    printf("Águila destruida por el enemigo :( GAME OVER\n");
}


}
}

}
int is_enemy_on_spawn(int spawn_x) {
    SDL_Rect spawn_area = { spawn_x + 8, 20, 30, 30 };  // Mismo área que el destello
    EnemyTank_t* current = enemy_list;

    while (current) {
        SDL_Rect enemy_rect = { current->x, current->y, current->w, current->h };
        if (SDL_HasIntersection(&spawn_area, &enemy_rect)) {
            return 1;  // Hay un enemigo ocupando el área de spawn
        }
        current = current->next;
    }
    return 0;
}


//FUNCIÓN  PARA EFECTO DE DESTELLO

void start_spawn_effect(int x, int y) {
    SpawnEffect_t* new_effect = (SpawnEffect_t*)malloc(sizeof(SpawnEffect_t));
    if (!new_effect) return;

    new_effect->x = x;
    new_effect->y = y;
    new_effect->start_time = SDL_GetTicks();
    new_effect->frame = 0;
    new_effect->next = spawn_list;
    spawn_list = new_effect;
}


//FUNCIÓN PARA DESTELLO DEL JUGADOR
void start_player_spawn_effect(int x, int y) {
    player_spawn_effect = (SpawnEffect_t*)malloc(sizeof(SpawnEffect_t));
    if (!player_spawn_effect) return;

    player_spawn_effect->x = x;
    player_spawn_effect->y = y;
    player_spawn_effect->start_time = SDL_GetTicks();
    player_spawn_effect->frame = 0;
    player_spawn_effect->next = NULL;
}

void update_player_spawn_effect() {
    if (!player_spawn_effect) return;

    Uint32 elapsed = SDL_GetTicks() - player_spawn_effect->start_time;
    player_spawn_effect->frame = (elapsed / 150) % 4;

    if (elapsed >= 1000) {
        free(player_spawn_effect);
        player_spawn_effect = NULL;

        // Activamos el escudo justo cuando termina el destello:
        player.shield_active = 1;
        player.shield_start_time = SDL_GetTicks();
        player.shield_frame = 0;
    }
}


//FUNCIÓN QUE ACTUALIZA LOS DESTELLOS 
void update_spawn_effects() {
    SpawnEffect_t** current = &spawn_list;
    Uint32 now = SDL_GetTicks();

    while (*current) {
        Uint32 elapsed = now - (*current)->start_time;

        // Avanzar frames cada 150ms
        (*current)->frame = (elapsed / 150) % 4;

        if (elapsed >= 800) {  // Después de 600ms, intentar crear enemigo
            int spawn_x = (*current)->x;

            // Si hay un enemigo justo encima, esperar más tiempo
            if (is_enemy_on_spawn(spawn_x)) {
                current = &((*current)->next);
                continue;  // Saltar sin eliminar el destello
            }

            if (active_enemies < MAX_ENEMIES_ON_SCREEN && remaining_enemies > 0) {
                if (next_enemy_index < current_enemy_count) {
                    int type = current_enemy_order[next_enemy_index];
                    next_enemy_index++;
                    add_enemy(type, spawn_x);

                    if (next_enemy_index >= current_enemy_count) {
                        remaining_enemies = 0;
                    }
                }
            }

            // Eliminar el efecto (solo si el lugar estaba libre y se intentó generar el enemigo)
            SpawnEffect_t* to_delete = *current;
            *current = (*current)->next;
            free(to_delete);
        } else {
            current = &((*current)->next);
        }
    }
}

//FUNCIÓN QUE DIBUJA LOS DESTELLOS DE INICIO
void draw_spawn_effects() {
    SpawnEffect_t* current = spawn_list;
    while (current) {
        SDL_Rect src = {current->frame * 16, 0, 16, 16};
        SDL_Rect dst = {current->x + 8, current->y, 30, 30};  // Lo escalamos si querés

        SDL_RenderCopy(renderer, spawn_texture, &src, &dst);
        current = current->next;
    }
}

void try_spawn_enemy() {
    if (active_enemies >= MAX_ENEMIES_ON_SCREEN || remaining_enemies <= 0) return;

    int slot = rand() % 3;
    printf(" Intentando crear destello en x=%d\n", spawn_points[slot]);
    start_spawn_effect(spawn_points[slot], 20);
}


void draw_player_spawn_effect() {
    if (!player_spawn_effect) return;

    SDL_Rect src = { player_spawn_effect->frame * 16, 0, 16, 16 };
    SDL_Rect dst = { player.x, player.y, player.w, player.h };

    SDL_RenderCopy(renderer, spawn_texture, &src, &dst);
}


//FUNCIÓN QUE DIBUJA EL AGULA Y LA BANDERA
void draw_base() {
    SDL_Rect src;

    if (base.destroyed) {
        src.x = 16;  // Sprite de la base destruida (bandera)
    } else {
        src.x = 0;   // Sprite de la base viva (águila)
    }

    src.y = 0;
    src.w = 16;
    src.h = 16;

    SDL_Rect dst = {base.x, base.y, base.w, base.h};
    SDL_RenderCopy(renderer, base.texture, &src, &dst);
}

//DIBUJA EL PUNTAJE 

void draw_score() {
    int x = SCREEN_WIDTH - 128 + 40+20+16;  // Posición X del primer número
    int y = 100+200-10;                      // Posición Y del score (ajustalo como gustes)

    int temp_score = score;
    int digits[10];
    int num_digits = 0;

    if (temp_score == 0) {
        digits[0] = 0;
        num_digits = 1;
    } else {
        while (temp_score > 0 && num_digits < 10) {
            digits[num_digits++] = temp_score % 10;
            temp_score /= 10;
        }
    }

    for (int i = num_digits - 1; i >= 0; i--) {
        SDL_Rect src = { digits[i] * 8, 0, 8, 8 };
        SDL_Rect dst = { x, y, 12, 12 };  // Tamaño visual
        SDL_RenderCopy(renderer, numeros_texture, &src, &dst);
        x += 12; // Avanza para el siguiente número
    }
}

//FUNCIÓN QUE GENERA UN POWER UP
void spawn_powerup(int x, int y, int type) {
    PowerUp_t* new_powerup = (PowerUp_t*)malloc(sizeof(PowerUp_t)); //malloc me permite hacer un enemigo en tiempo real cuando lo necesito
    if (!new_powerup) return;

    new_powerup->x = x;
    new_powerup->y = y;
    new_powerup->type = type;
    new_powerup->spawn_time = SDL_GetTicks();
    new_powerup->active = 1;
    new_powerup->next = powerup_list;
    powerup_list = new_powerup;
}

//FUNCIÓN QUE DIBUJA LOS POWER UPS
void draw_powerups() {
    Uint32 now = SDL_GetTicks();  // Tiempo actual
    PowerUp_t** current = &powerup_list;

    while (*current) {
        PowerUp_t* pu = *current;

        // Verificar si el power-up lleva mucho tiempo activo
        if (pu->active && (now - pu->spawn_time > POWERUP_DURATION)) {
            printf("Power-up tipo %d desapareció automáticamente\n", pu->type);
            PowerUp_t* to_delete = pu;
            *current = pu->next;
            free(to_delete);
            continue; // No avanzar current, ya está actualizado
        }

        if (pu->active) {
            SDL_Rect src = { pu->type * 16, 0, 16, 16 };
            SDL_Rect dst = { pu->x, pu->y, 26, 26 };
            SDL_RenderCopy(renderer, powerup_texture, &src, &dst);
        }

        current = &(*current)->next;
    }
}

//FUNCIÓN QUE COLOCA BLOQUES METALICOS DE PU
void blindar_aguila() {
    int bx = base.x;
    int by = base.y;

    int offsets[12][2] = {
        { -16, -16 }, { 0, -16 }, { 16, -16 }, { 32, -16 },
        { -16, 0   },                     { 32, 0 },
        { -16, 16  },                     { 32, 16 },
        { -16, 32  }, { 0, 32 }, { 16, 32 }, { 32, 32 }
    };

    for (int i = 0; i < 12; i++) {
        int x = bx + offsets[i][0];
        int y = by + offsets[i][1];

        SDL_Rect area_a_proteger = {x, y, 16, 16};

        // Eliminar cualquier bloque que colisione con esta área
        for (int j = 0; j < num_blocks;) {
            SDL_Rect bloque = {blocks[j].x, blocks[j].y, blocks[j].w, blocks[j].h};

            if (SDL_HasIntersection(&area_a_proteger, &bloque)) {
                blocks[j] = blocks[num_blocks - 1];
                num_blocks--;
            } else {
                j++;
            }
        }

        // Agregar bloque metálico exacto
        add_block(x, y, 16, 16, 1, block_texture);
    }

    printf("Águila blindada metal\n");
    metal_active = 1;
   metal_start_time = SDL_GetTicks();

}


//FUNCIÓN PARA QUITAR LOS BLOQUES METALICOS DEL PU
void desblindar_aguila() {
    int bx = base.x;
    int by = base.y;

    int offsets[12][2] = {
        { -16, -16 }, { 0, -16 }, { 16, -16 }, { 32, -16 },
        { -16, 0   },                     { 32, 0  },
        { -16, 16  },                     { 32, 16 },
        { -16, 32  }, { 0, 32 }, { 16, 32 }, { 32, 32 }
    };

    for (int i = 0; i < 12; i++) {
        int x = bx + offsets[i][0];
        int y = by + offsets[i][1];

        for (int j = 0; j < num_blocks; j++) {
            if (blocks[j].x == x && blocks[j].y == y && blocks[j].type == 1) {
                blocks[j].type = 0;
                blocks[j].srcRect.x = 0 * 8;  // Sprite ladrillo
                break;
            }
        }
    }

    metal_active = 0;
    printf("Blindaje metálico desactivado\n");
}


//COLISIONES Y FUNCIÓN DE POWER UPS


void check_powerup_collisions() {
    PowerUp_t** current = &powerup_list;

    while (*current) {
        PowerUp_t* pu = *current;

        SDL_Rect playerRect = { player.x, player.y, player.w, player.h };
        SDL_Rect powerupRect = { pu->x, pu->y, 16, 16 };

        if (pu->active && SDL_HasIntersection(&playerRect, &powerupRect)) {
            printf("Power-up recogido: tipo %d\n", pu->type);

        // --- Aplica efecto según tipo ---
        switch (pu->type) {
            case 0: // Helmet = escudo temporal
                player.shield_active = 1;
                player.shield_start_time = SDL_GetTicks();
                player.shield_frame = 0;
                break;

            case 1: // Timer = congelar enemigos
                freeze_enemies = 1;
                freeze_start_time = SDL_GetTicks();
                printf("Enemigos congelados paralizados\n");
                break;


            case 2: // Shovel = blindar águila 
                   blindar_aguila();
                break;
            case 3: // Star = mejora progresiva del tanque
                if (player.star_level < 3) {
                    player.star_level++;
                    printf("Tanque mejorado a nivel %d\n", player.star_level);
                }
                break;


            case 4: // Grenade = eliminar todos los enemigos
                while (enemy_list) {
                    EnemyTank_t* e = enemy_list;
                    score += (e->type == 2) ? 200 : 100;
                    enemy_list = enemy_list->next;
                    free(e);
                    active_enemies--;
                }
                break;

            case 5: // Tank = vida extra
                player_lives++;
                break;
        }

        // Eliminar power-up tras recogerlo
        PowerUp_t* to_delete = pu;
        *current = pu->next;
        free(to_delete);
    } else {
        current = &((*current)->next);
    }
}
}



//FUNCION PARA CAMNBIO DE BLOQUES A METAL POWER UP

////////////////////////////////
// FUNCIONES DE NIVELES BLOCK
////////////////////////////////


//NIVEL 1 MAPA
void load_level_1(SDL_Texture* block_texture) {
    
    num_blocks=0;

    // LINEA DE BLOQUES EN VERTICAL 1
    for (int y = 48; y < 176; y += 16) {
        add_block(64, y, 16, 16, 0, block_texture);
    }
    for (int y = 48; y < 176; y += 16) {
        add_block(80, y, 16, 16, 0, block_texture);
    }


    // LINEA DE BLOQUES EN VERTICAL 2
    for (int y = 48; y < 176; y += 16) {
        add_block(128, y, 16, 16, 0, block_texture);
    }
    for (int y = 48; y < 176; y += 16) {
        add_block(144, y, 16, 16, 0, block_texture);
    }

   // LINEA DE BLOQUES EN VERTICAL 3
    for (int y = 48; y < 128+16; y += 16) {
        add_block(192, y, 16, 16, 0, block_texture);
    }
    for (int y = 48; y < 128+16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }


       // LINEA DE BLOQUES EN VERTICAL 4
    for (int y = 48; y < 128+16; y += 16) {
        add_block(208+32+16, y, 16, 16, 0, block_texture);
    }

    for (int y = 48; y < 128+16; y += 16) {
        add_block(208+32+16+16, y, 16, 16, 0, block_texture);
    }

      // LINEA DE BLOQUES EN VERTICAL 5
    for (int y = 48; y < 176; y += 16) {
        add_block(320, y, 16, 16, 0, block_texture);
    }

    for (int y = 48; y < 176; y += 16) {
        add_block(320+16, y, 16, 16, 0, block_texture);
    }

  // LINEA DE BLOQUES EN VERTICAL 6
    for (int y = 48; y < 176; y += 16) {
        add_block(384, y, 16, 16, 0, block_texture);
    }

   for (int y = 48; y < 176; y += 16) {
        add_block(400, y, 16, 16, 0, block_texture);
    }


    // LINEA DE BLOQUES EN VERTICAL 7
    for (int y = 255+16; y < 384-32; y += 16) {
        add_block(64, y, 16, 16, 0, block_texture);
    }
    for (int y = 255+16; y < 384-32; y += 16) {
        add_block(80, y, 16, 16, 0, block_texture);
    }



    // LINEA DE BLOQUES EN VERTICAL 8
   for (int y = 255+16; y < 352; y += 16) {
        add_block(128, y, 16, 16, 0, block_texture);
    }
    for (int y = 255+16; y < 352; y += 16) {
        add_block(128+16, y, 16, 16, 0, block_texture);
    }

    // LINEA DE CENTRO LADRILLO (VERTICAL)
    for (int y = 239; y < 304; y += 16) {
        add_block(192, y, 16, 16, 0, block_texture);
    }
    for (int y = 239; y < 304; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }
  

    // LINEA DE CENTRO LADRILLO
    for (int y = 239; y < 304; y += 16) {
        add_block(256, y, 16, 16, 0, block_texture);
    }
    for (int y = 239; y < 304; y += 16) {
        add_block(272, y, 16, 16, 0, block_texture);
    }
  

 // LINEA DE CENTRO LADRILLO
    for (int y = 239+16; y < 304-32; y += 16) {
        add_block(208+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 239+16; y < 304-32; y += 16) {
        add_block(208+16+16, y, 16, 16, 0, block_texture);
    }
  

    //CUADRITOS DEL CENTRO

     // LINEA DE BLOQUES EN VERTICAL 3
    for (int y = 48*4-16; y < 224-16; y += 16) {
        add_block(192, y, 16, 16, 0, block_texture);
    }
    for (int y = 48*4-16; y < 224-16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }


     // LINEA DE BLOQUES EN VERTICAL 3
    for (int y = 48*4-16; y < 224-16; y += 16) {
        add_block(192+32+32, y, 16, 16, 0, block_texture);
    }
    for (int y = 48*4-16; y < 224-16; y += 16) {
        add_block(208+32+32, y, 16, 16, 0, block_texture);
    }


    //BASE DE AGUILA

   for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208+32+16, y, 16, 16, 0, block_texture);
    }


    //BLOQUE DE METAL
    for (int y = 96; y < 128; y += 16) {
        add_block(192+16+16, y, 16, 16, 1, block_texture);
    }
    for (int y = 96; y < 128; y += 16) {
        add_block(192+16+16+16, y, 16, 16, 1, block_texture);
    }

   // primer LINEA DE BLOQUES AGREGO DE FORMA HORIZONTAL
    for (int x = 32; x < 48+16; x += 16) {
    add_block(x, 176+32, 16, 16, 0, block_texture);
    }

  for (int x = 32; x < 48+16; x += 16) {
    add_block(x, 176+32+16, 16, 16, 1, block_texture);
    }



    //SEGUDAN LINEA DE BLOQUES DE FORMA HORAIZONTAL 2 TIPOS

   for (int x = 384+32; x < 384+32+32; x += 16) {
    add_block(x, 176+32, 16, 16, 0, block_texture);
    }

   for (int x = 384+32; x < 384+32+32; x += 16) {
    add_block(x, 176+32+16, 16, 16, 1, block_texture);
    }


   //LINEA HORIZONTAL BLOQUES DE LADRILLO
   for (int x = 320; x <320+32+16+16; x += 16) {
    add_block(x, 176+32, 16, 16, 0, block_texture);
    }
  for (int x = 320; x <320+32+16+16; x += 16) {
    add_block(x, 176+32+16, 16, 16, 0, block_texture);
    }

  // LINEA DE BLOQUES AGREGO DE FORMA HORIZONTAL
    for (int x = 64+16+16; x < 160; x += 16) {
    add_block(x, 176+32, 16, 16, 0, block_texture);
    }

  for (int x = 64+32; x < 160; x += 16) {
    add_block(x, 176+32+16, 16, 16, 0, block_texture);
    }

    // LINEA VERTICAL 9
    for (int y = 271; y < 356; y += 16) {
        add_block(320, y, 16, 16, 0, block_texture);
    }
    for (int y = 271; y < 356; y += 16) {
        add_block(336, y, 16, 16, 0, block_texture);
    }
  

  
    // LINEA VERTICAL 10
    for (int y = 271; y < 356; y += 16) {
        add_block(368+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 271; y < 356; y += 16) {
        add_block(384+16, y, 16, 16, 0, block_texture);
    }
  

}


//MAPA 2


void load_level_2(SDL_Texture* block_texture) {
    
    num_blocks=0; 

    //BASE DE AGUILA

   for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208+32+16, y, 16, 16, 0, block_texture);
    }



    for (int y = 9; y <= 12; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 17; y <= 18; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 3; y <= 6; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 11; y <= 12; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 4; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 17; y <= 24; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 1; y <= 4; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 7; y <= 10; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <= 18; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 22; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 16; y += 1) {
        for (int x = 10; x <= 11; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 11; y <= 12; y += 1) {
        for (int x = 12; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 12; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 15; y <= 20; y += 1) {
        for (int x = 12; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 10; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 1; y <= 2; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 3; y <= 6; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 15; y <= 20; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 11; y <= 12; y += 1) {
        for (int x = 18; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 3; y <= 6; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <= 8; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 9; y <= 10; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <= 16; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 22; y <= 24; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 22; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 9; y <= 14; y += 1) {
        for (int x = 22; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 22; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 24; y <= 24; y += 1) {
        for (int x = 22; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 3; y <= 6; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 10; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 20; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 22; y <= 24; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 10; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
}

void load_level_3(SDL_Texture* block_texture) {
    num_blocks=0; 



    //BASE DE AGUILA

       for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
            add_block(208+16, y, 16, 16, 0, block_texture);
        }
        for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
            add_block(208+16+16, y, 16, 16, 0, block_texture);
        }
        for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
            add_block(208, y, 16, 16, 0, block_texture);
        }
        for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
            add_block(208+32+16, y, 16, 16, 0, block_texture);
        }



    for (int y = 5; y <= 6; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <= 12; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 17; y <= 18; y += 1) {
        for (int x = 2; x <= 4; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 21; y <= 24; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 3; y <= 14; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 16; y <= 16; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 23; y <= 24; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 3; y <= 12; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 17; y <= 18; y += 1) {
        for (int x = 7; x <= 10; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 16; y <= 16; y += 1) {
        for (int x = 8; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 21; y <= 24; y += 1) {
        for (int x = 8; x <= 8; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 1; y <= 4; y += 1) {
        for (int x = 10; x <= 11; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 9; y += 1) {
        for (int x = 10; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 10; y <= 10; y += 1) {
        for (int x = 10; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 17; y <= 20; y += 1) {
        for (int x = 13; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 12; x <= 12; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 12; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 14; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 16; y <= 16; y += 1) {
        for (int x = 14; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <= 9; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 16; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 1; y <= 2; y += 1) {
        for (int x = 18; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <= 10; y += 1) {
        for (int x = 20; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <= 20; y += 1) {
        for (int x = 20; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 21; y <= 24; y += 1) {
        for (int x = 20; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 4; y <= 4; y += 1) {
        for (int x = 22; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 7; y <= 8; y += 1) {
        for (int x = 22; x <= 26; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 12; y += 1) {
        for (int x = 25; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 25; x <= 26; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
}


void load_level_4(SDL_Texture* block_texture) {
    
    num_blocks=0; 



    //BASE DE AGUILA

   for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208+32+16, y, 16, 16, 0, block_texture);
    }



    for (int y = 3; y <= 6; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 1; y <= 4; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 1; y <= 2; y += 1) {
        for (int x = 6; x <= 7; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 7; y <= 7; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 11; y <= 12; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 21; y <= 22; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 23; y <= 24; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 23; y <= 24; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 1; y <= 2; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 3; y <= 4; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 5; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 21; y <= 24; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 19; y <= 22; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 23; y <= 24; y += 1) {
        for (int x = 22; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 23; y <= 24; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 12; y <= 13; y += 1) {
        for (int x = 25; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 3; y <= 3; y += 1) {
        for (int x = 10; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 4; y <= 4; y += 1) {
        for (int x = 9; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 5; y <= 5; y += 1) {
        for (int x = 9; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 6; y <= 7; y += 1) {
        for (int x = 8; x <= 24; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 8; y <= 8; y += 1) {
        for (int x = 7; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 8; y <= 8; y += 1) {
        for (int x = 16; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 8; y <= 9; y += 1) {
        for (int x = 24; x <= 24; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 13; y += 1) {
        for (int x = 7; x <= 7; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <= 13; y += 1) {
        for (int x = 18; x <= 20; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 10; y <= 11; y += 1) {
        for (int x = 10; x <= 10; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 10; y <= 11; y += 1) {
        for (int x = 14; x <= 14; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 13; y <= 13; y += 1) {
        for (int x = 10; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 12; y <= 13; y += 1) {
        for (int x = 6; x <= 6; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 14; y <= 14; y += 1) {
        for (int x = 6; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <= 16; y += 1) {
        for (int x = 5; x <= 22; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 17; y <= 17; y += 1) {
        for (int x = 4; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 18; y <= 18; y += 1) {
        for (int x = 8; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 19; y += 1) {
        for (int x = 10; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 20; y <= 20; y += 1) {
        for (int x = 12; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 19; y += 1) {
        for (int x = 4; x <= 7; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 20; y <= 20; y += 1) {
        for (int x = 4; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 21; y <= 21; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 19; y <= 19; y += 1) {
        for (int x = 20; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 20; y <= 20; y += 1) {
        for (int x = 18; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 21; y <= 21; y += 1) {
        for (int x = 18; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
}





void load_level_5(SDL_Texture* block_texture) {
    
    num_blocks=0;
     for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16; y += 16) {
        add_block(208+16+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208, y, 16, 16, 0, block_texture);
    }
    for (int y = 272+32+32+16; y < 320+32+16+16+16; y += 16) {
        add_block(208+32+16, y, 16, 16, 0, block_texture);
    }
    for (int y = 7; y <= 12; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 15; y <= 16; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 21; y <= 22; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 7; y <= 12; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 15; y <= 16; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 21; y <= 22; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 10; x <= 11; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 18; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 5; y <= 6; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 7; y <= 8; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 7; y <= 8; y += 1) {
        for (int x = 20; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 21; y <= 22; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 21; y <= 22; y += 1) {
        for (int x = 20; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 10; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 19; y <= 20; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 2, block_texture);
        }
    }
    for (int y = 13; y <= 14; y += 1) {
        for (int x = 2; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 17; y <= 20; y += 1) {
        for (int x = 2; x <= 3; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 17; y <= 20; y += 1) {
        for (int x = 26; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 11; y <=12; y += 1) {
        for (int x = 4; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 11; y <=12; y += 1) {
        for (int x = 20; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 13; y <=14; y += 1) {
        for (int x = 24; x <= 27; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 15; y <=16; y += 1) {
        for (int x = 10; x <= 11; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 17; y <=18; y += 1) {
        for (int x = 8; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 19; y <=20; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 15; y <=16; y += 1) {
        for (int x = 18; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 17; y <=18; y += 1) {
        for (int x = 16; x <= 21; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 19; y <=20; y += 1) {
        for (int x = 20; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 3, block_texture);
        }
    }
    for (int y = 9; y <=10; y += 1) {
        for (int x = 12; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 9; y <=10; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 1, block_texture);
        }
    }
    for (int y = 3; y <=8; y += 1) {
        for (int x = 12; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 3; y <=8; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 11; y <=16; y += 1) {
        for (int x = 12; x <= 13; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 11; y <=16; y += 1) {
        for (int x = 16; x <= 17; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <=14; y += 1) {
        for (int x = 10; x <= 11; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <=18; y += 1) {
        for (int x = 14; x <= 15; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <=14; y += 1) {
        for (int x = 18; x <= 19; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <=8; y += 1) {
        for (int x = 4; x <= 5; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 7; y <=8; y += 1) {
        for (int x = 24; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <=10; y += 1) {
        for (int x = 4; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 9; y <=10; y += 1) {
        for (int x = 20; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <=14; y += 1) {
        for (int x = 6; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <=16; y += 1) {
        for (int x = 4; x <= 9; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 17; y <=18; y += 1) {
        for (int x = 4; x <= 7; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 13; y <=14; y += 1) {
        for (int x = 20; x <= 23; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 15; y <=16; y += 1) {
        for (int x = 20; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
    for (int y = 17; y <=18; y += 1) {
        for (int x = 22; x <= 25; x += 1) {add_block(16*x,16*y , 16, 16, 0, block_texture);
        }
    }
}

//FUNCIÓN QUE CARGA LOS NIVELES (ENEMIGOS Y BLOQUES)

void load_level(int nivel, SDL_Texture* block_texture) {
    switch (nivel) {
        case 1:
            load_level_1(block_texture);
            current_enemy_order = enemy_order_level_1;
            current_enemy_count = enemy_count_level_1;
            break;
        case 2:
            // aquí va load_level_2 
            load_level_2(block_texture);
            current_enemy_order = enemy_order_level_2;
            current_enemy_count = enemy_count_level_2;
             

             break;
       case 3:
            load_level_3(block_texture);
            current_enemy_order = enemy_order_level_3;
            current_enemy_count = enemy_count_level_3;
        break;
    case 4:
            load_level_4(block_texture);
            current_enemy_order = enemy_order_level_4;
            current_enemy_count = enemy_count_level_4;
        break; 

      
      case 5:
            load_level_5(block_texture);
            current_enemy_order = enemy_order_level_5;
            current_enemy_count = enemy_count_level_5;
        break; 




        

        default:
            printf("Nivel no válido\n");
            return;
    }

    remaining_enemies = current_enemy_count;
    next_enemy_index = 0;
}

///////////////////////
// FUNCION QUE LIBERA
//////////////////////


void free_enemy_list() {
    EnemyTank_t* current = enemy_list;
    while (current) {
        EnemyTank_t* next = current->next;
        free(current);
        current = next;
    }
    enemy_list = NULL;
}

void free_spawn_list() {
    SpawnEffect_t* current = spawn_list;
    while (current) {
        SpawnEffect_t* next = current->next;
        free(current);
        current = next;
    }
    spawn_list = NULL;
}



void reiniciar_jugador() {
    player.x = player_start_x;
    player.y = player_start_y;
    player.direction = UP;
    player.star_level = 0;
    player.texture = load_texture("TankPlayer.bmp");
    player.shield_active = 1;
    player.shield_start_time = SDL_GetTicks();
    player.shield_frame = 0;
    start_player_spawn_effect(player.x, player.y);
}





Uint32 transition_start_time = 0;


//FUNCIONES  EL GAME LOOP (PARA MAS ORDEN)

void manejar_eventos(SDL_Event* event, int* running) {
    while (SDL_PollEvent(event)) {
        
        if (event->type == SDL_QUIT) {
            *running = 0;
        }
        if (game_state == STATE_MENU && event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
            current_level = 1;
            player_lives = 5;
            score = 0;
            player.star_level = 0;

            free_enemy_list(); // Limpieza por si algo quedó
            free_spawn_list();

            load_level(current_level, block_texture);
            reiniciar_jugador();
            game_state = STATE_PLAYING;
        }


        if ((game_state == STATE_GAME_OVER || game_state == STATE_WIN) &&
            event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
            game_state = STATE_MENU;
          reiniciar_juego_completo();
        }

        if (event->type == SDL_QUIT) *running = 0;
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.scancode == SDL_SCANCODE_ESCAPE) *running = 0;
            if (event->key.keysym.scancode == SDL_SCANCODE_P) pause ^= 1;
            if (event->key.keysym.scancode == SDL_SCANCODE_SPACE && !pause) shoot_bullet();
        }
    }
}

void actualizar_juego(Uint32 now, const Uint8* keystate) {
    // Descongelar enemigos si se acaba el efecto
    if (freeze_enemies && (now - freeze_start_time >= FREEZE_DURATION)) {
        freeze_enemies = 0;
        printf("Enemigos descongelados\n");
    }

    // Finaliza el efecto del escudo metálico
    if (metal_active && (now - metal_start_time >= METAL_DURATION)) {
        desblindar_aguila();
    }

    // Mover jugador solo si ya apareció
    if (!player_spawn_effect) {
        move_tank(keystate, &player);
    }

    move_enemies();
    update_bullets();
    update_enemy_bullets();

    check_bullet_collision();
    check_enemy_bullet_collisions();
    check_bullet_enemy_collisions();
    check_bullet_vs_bullet_collisions();
    check_powerup_collisions();

    // Actualizar sprite del tanque según la estrella
    if (player.star_level == 1) {
        player.texture = tank_texture_star1;
    } else if (player.star_level == 2) {
        player.texture = tank_texture_star2;
    } else if (player.star_level >= 3) {
        player.texture = tank_texture_star3;
    }

    update_enemies(now);
    update_spawn_effects();
    update_player_spawn_effect();

    // --- CAMBIO DE NIVEL ---
    if (remaining_enemies == 0 && active_enemies == 0 && spawn_list == NULL) {
        current_level++;
        if (current_level <= 5) {
            printf("Nivel %d completado. Transición...\n", current_level - 1);

        free_enemy_list();
        free_spawn_list();

            game_state = STATE_TRANSITION;
            transition_start_time = now;
        } else {
            printf("Has ganado todos los niveles\n");
            game_state = STATE_WIN;
            pause = 1;
        }
    }

    // --- Power-Up automático ---
    if (now - last_powerup_time >= powerup_interval) {
        int spawn_x, spawn_y;
        int attempts = 0;
        int valid_position = 0;

        while (!valid_position && attempts < 100) {
            spawn_x = 32 + rand() % (SCREEN_WIDTH - 128 - 16);
            spawn_y = 16 + rand() % (SCREEN_HEIGHT - 32 - 16);

            SDL_Rect test_rect = {spawn_x, spawn_y, 16, 16};
            int blocked = 0;

            for (int i = 0; i < num_blocks; i++) {
                SDL_Rect block_rect = {blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h};
                if (SDL_HasIntersection(&test_rect, &block_rect)) {
                    blocked = 1;
                    break;
                }
            }

            PowerUp_t* existing = powerup_list;
            while (existing) {
                SDL_Rect existing_rect = {existing->x, existing->y, 16, 16};
                if (SDL_HasIntersection(&test_rect, &existing_rect)) {
                    blocked = 1;
                    break;
                }
                existing = existing->next;
            }

            if (!blocked && spawn_x < SCREEN_WIDTH - 128) {
                valid_position = 1;
            }

            attempts++;
        }

        if (valid_position) {
            int type = rand() % 6;
            spawn_powerup(spawn_x, spawn_y, type);
            last_powerup_time = now;
            printf("Se generÓ power up automáticamente en (%d, %d), tipo %d!\n", spawn_x, spawn_y, type);
        }
    }

    // --- Control de aparición de enemigos ---
    int active_spawns = count_active_spawn_effects();
    int total_incoming = active_enemies + active_spawns;

    if (total_incoming < MAX_ENEMIES_ON_SCREEN && remaining_enemies > 0) {
        if (now - last_spawn_time >= spawn_delay) {
        int tried = 0;
        while (tried < 3) {
            int i = spawn_index % 3;
            int spawn_x = spawn_points[i];

            int already_spawning = 0;
            SpawnEffect_t* temp = spawn_list;
            while (temp) {
                if (temp->x == spawn_x) {
                    already_spawning = 1;
                    break;
                }
                temp = temp->next;
            }

            int blocked_by_enemy = is_enemy_on_spawn(spawn_x);

            if (!already_spawning && !blocked_by_enemy) {
                start_spawn_effect(spawn_x, 20);
                last_spawn_time = now;
                printf("Iniciando aparición en punto %d (x=%d)\n", i, spawn_x);
                spawn_index++;
                break;
            } else {
                spawn_index++;
                tried++;
                }
            }
        }
    }
}


void manejar_transicion(Uint32 now) {
    Uint32 elapsed = now - transition_start_time;

    // Fondo gris de transición
    SDL_SetRenderDrawColor(renderer, 96, 96, 96, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer); // Mostrar pantalla gris temporalmente

    if (elapsed >= 3000) {
        game_state = STATE_PLAYING;

        // Limpiar enemigos, efectos y power-ups
        free_enemy_list();
        free_spawn_list();
        powerup_list = NULL;

        // Reiniciar jugador
        reiniciar_jugador();
        start_player_spawn_effect(player.x, player.y);

        // Cargar nuevo nivel
        load_level(current_level, block_texture);
    }
}



void dibujar_todo() {
    // Limpiar la pantalla (negro)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Dibujar jugador
    draw_tank(&player);

    // Dibujar escudo si está activo y ya apareció
    if (!player_spawn_effect && player.shield_active) {
        SDL_Rect src = { player.shield_frame * 16, 0, 16, 16 };
        SDL_Rect dst = {
            player.x - 3,
            player.y - 3,
            player.w + 6,
            player.h + 6
        };
        SDL_RenderCopy(renderer, shield_texture, &src, &dst);
    }

    // Resto de elementos
    draw_bullets();
    draw_enemies();
    draw_spawn_effects();
    draw_player_spawn_effect();
    draw_base();
    draw_enemy_bullets();
    draw_blocks();
    draw_powerups();
    draw_frame(renderer);
    draw_hud();
    draw_lives_number();
    draw_enemy_icons();
    draw_score();
    draw_level_number();

    // Detectar colisión con marco
    check_collision_with_frame(&player);

    // Animación del escudo
    if (player.shield_active) {
        Uint32 elapsed = SDL_GetTicks() - player.shield_start_time;
        player.shield_frame = ((elapsed / 200) % 2 == 0) ? 0 : 1;

        if (elapsed > 5000) {
            player.shield_active = 0;
        }
    }

    // Pantallas especiales
    if (game_state == STATE_MENU) {
        SDL_RenderCopy(renderer, menu_texture, NULL, NULL);
    }
    else if (game_state == STATE_GAME_OVER) {
        SDL_RenderCopy(renderer, gameover_texture, NULL, NULL);
    }
    else if (game_state == STATE_WIN) {
        SDL_RenderCopy(renderer, win_texture, NULL, NULL);
    }
}



void free_powerup_list() {
    PowerUp_t* current = powerup_list;
    while (current) {
        PowerUp_t* temp = current;
        current = current->next;
        free(temp);
    }
    powerup_list = NULL;
}




void liberar_juego() {
    // 1. Enemigos
    free_enemy_list();

    // 2. Efectos de aparición
    free_spawn_list();

    // 3. Power-ups
    PowerUp_t* current = powerup_list;
    while (current) {
        PowerUp_t* temp = current;
        current = current->next;
        free(temp);
    }
    powerup_list = NULL;

    // 4. Balas jugador
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    // 5. Balas enemigos
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        enemy_bullets[i].active = 0;
    }

    // 6. Bloques
    num_blocks = 0;
}

void game_loop(void);


void iniciar_juego() {
    // Resetear variables globales
    game_state = STATE_PLAYING;
    current_level = 1;
    player_lives = 3;
    score = 0;
    player.star_level = 0;


    // Resetear jugador
    player.x = (SCREEN_WIDTH / 2) - 16 - 32 - 16 - 16;
    player.y = SCREEN_HEIGHT - 48;
    player.direction = UP;
    player.w = 30;
    player.h = 30;
    player.shield_active = 1;
    player.shield_start_time = SDL_GetTicks();
    player.shield_frame = 0;

    player_start_x = player.x;
    player_start_y = player.y;

    // Resetear base
    base.x = 240 - 16;
    base.y = SCREEN_HEIGHT - 50;
    base.w = 32;
    base.h = 32;
    base.destroyed = 0;

    // Limpiar enemigos, efectos y power-ups
    free_enemy_list();
    free_spawn_list();
    free_powerup_list();  // Asegurate de tener esta función también

    // Resetear balas
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemy_bullets[i].active = 0;

    // Resetear nivel
    num_blocks = 0;
    load_level(current_level, block_texture);

    // Efecto de aparición del jugador
    start_player_spawn_effect(player.x, player.y);

    // Reiniciar contadores de enemigos
    active_enemies = 0;
    remaining_enemies = MAX_ENEMIES_LEVEL;

    // Resetear temporizadores si usás alguno
    last_enemy_spawn_time = SDL_GetTicks();

    // Iniciar bucle principal
    game_loop();
}



void reiniciar_juego_completo() {
    // Volver al menú
    game_state = STATE_MENU;

    // Reiniciar nivel y score
    current_level = 1;
    score = 0;
    player_lives = 3;

    // Resetear jugador
    player.star_level = 0;
    player.x = player_start_x;
    player.y = player_start_y;
    player.direction = UP;
    player.shield_active = 1;
    player.shield_start_time = SDL_GetTicks();
    player.texture = load_texture("TankPlayer.bmp");

    // Limpiar enemigos, balas, efectos, powerups
    free_enemy_list();
    free_spawn_list();
    free_powerup_list();

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) enemy_bullets[i].active = 0;

    // Reiniciar bloques
    num_blocks = 0;

    // Reiniciar base
    base.destroyed = 0;

    // Reiniciar timers
    last_spawn_time = SDL_GetTicks();
    last_powerup_time = SDL_GetTicks();

    // Reiniciar efecto de aparición
    start_player_spawn_effect(player.x, player.y);
}



//////////////////
//BUCLE DEL JUEGO
/////////////////



void game_loop() {
    int running = 1;
    SDL_Event event;
while (running) {
    Uint32 now = SDL_GetTicks();
    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    manejar_eventos(&event, &running);

    if (game_state == STATE_PLAYING && !pause) {
        actualizar_juego(now, keystate);
        dibujar_todo();
    }
    else if (game_state == STATE_TRANSITION) {
        manejar_transicion(now);
        // NO llamar a dibujar_todo() aquí
    }
    else {
        dibujar_todo(); // Menú, win, game over
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}
}









///////////////////////
// Liberar recursos //
//////////////////////
void close_SDL() {
    if (player.texture) SDL_DestroyTexture(player.texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    if (block_texture) SDL_DestroyTexture(block_texture);
    if (bullet_texture) SDL_DestroyTexture(bullet_texture);
    if (enemy1_gray_texture) SDL_DestroyTexture(enemy1_gray_texture);
    if (enemy1_red_texture) SDL_DestroyTexture(enemy1_red_texture);
    if (spawn_texture) SDL_DestroyTexture(spawn_texture);
    if (hud_texture) SDL_DestroyTexture(hud_texture);
    if (numeros_texture) SDL_DestroyTexture(numeros_texture);
    if (enemigo_icon_texture) SDL_DestroyTexture(enemigo_icon_texture);
    

free_enemy_list();
free_spawn_list();

    SDL_Quit();
}
///////////////
// Main      //
//////////////



int main() {
    // === 1. Inicializar SDL ===
    if (init_SDL() != 0) return -1;

    // === 2. Cargar sprites y texturas ===
    if (load_tank_sprite() != 0) {
        printf("Error al cargar el tanque.\n");
        close_SDL();
        return -1;
    }

    block_texture = load_texture("Paredes8.bmp");
    spawn_texture = load_texture("Aparicion16.bmp");
    enemy1_gray_texture = load_texture_with_transparency("Enemigo1.bmp", 0, 0, 0);
    enemy1_red_texture = load_texture_with_transparency("Enemigo1Alternado.bmp", 0, 0, 0);
    enemy2_texture = load_texture_with_transparency("Enemigo2.bmp", 0, 0, 0);
    enemy2_alt_texture = load_texture_with_transparency("Enemigo2A.bmp", 0, 0, 0);

    SDL_Texture* aguila_texture = load_texture("Aguila16.bmp");
    hud_texture = load_texture("Vidaspantalla.bmp");
    numeros_texture = load_texture("Numeros8.bmp");
    enemigo_icon_texture = load_texture("ContadorEnemigo8.bmp");
    shield_texture = load_texture_with_transparency("Escudo16.bmp", 0, 0, 0);
    powerup_texture = load_texture("Powerup16.bmp");

    menu_texture = load_texture("inicio.bmp");
    gameover_texture = load_texture("gameover.bmp");
    win_texture = load_texture("win.bmp");

    // Sprites del tanque con estrellas
    tank_texture_star1 = load_texture("TanquesE16.bmp");
    tank_texture_star2 = load_texture("TanquesE216.bmp");
    tank_texture_star3 = load_texture("TanquesE316.bmp");


    // Tanques especiales
    tank_blindado_4 = load_texture_with_transparency("Tanqueblindado16.bmp", 0, 0, 0);
    tank_blindado_3 = load_texture_with_transparency("TanqueblindadoV1.bmp", 0, 0, 0);
    tank_blindado_2 = load_texture_with_transparency("TanqueblindadoV2.bmp", 0, 0, 0);
    tank_blindado_1 = load_texture_with_transparency("TanqueblindadoV3.bmp", 0, 0, 0);
    tank_powerup_gray = load_texture_with_transparency("Tanquepower-up16.bmp", 0, 0, 0);
    tank_powerup_red  = load_texture_with_transparency("TanquepowerupV.bmp", 0, 0, 0);

    // === 3. Validar que se cargó todo correctamente ===
    if (!block_texture || !spawn_texture || !aguila_texture || !hud_texture || !numeros_texture ||
        !enemigo_icon_texture || !shield_texture || !powerup_texture || !menu_texture ||
        !gameover_texture || !win_texture || !tank_blindado_4 || !tank_blindado_3 ||
        !tank_blindado_2 || !tank_blindado_1 || !tank_powerup_gray || !tank_powerup_red) {
        printf("Error al cargar recursos.\n");
        close_SDL();
        return -1;
    }

    // === 4. Inicializar base (águila) ===
    base.x = 240 - 16;
    base.y = SCREEN_HEIGHT - 50;
    base.w = 32;
    base.h = 32;
    base.destroyed = 0;
    base.texture = aguila_texture;

    // === 5. Inicializar jugador ===
    player.x = (SCREEN_WIDTH / 2) - 16 - 32 - 16 - 16;
    player.y = SCREEN_HEIGHT - 48;
    player.direction = UP;
    player.w = 30;
    player.h = 30;
    player.shield_active = 0;
    player.shield_start_time = 0;
    player.shield_frame = 0;

    player_start_x = player.x;
    player_start_y = player.y;

    start_player_spawn_effect(player.x, player.y);

    // === 6. Cargar primer nivel ===
    load_level(1, block_texture);
    
    // === 7. Iniciar el bucle principal del juego ===
    game_loop();
    

    // === 8. Cerrar SDL y liberar recursos ===
    close_SDL();
    return 0;
}


