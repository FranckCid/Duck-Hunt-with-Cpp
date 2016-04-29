#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <unistd.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

//Classes declaration
class GameObject;
class Game;
class Duck;
class Player;

//Interface ducks types
enum duck_apparence{
    ALIVE=0,
    DEAD=1,
    ACTUAL=2
};

//GAME INFO
const int SCREEN_W = 650, SCREEN_H = 600;
const char GAME_NAME[255] = "Duck Hunt";
int ENEMY_W, ENEMY_H;
int ducks_interface[] = {ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE, ALIVE};
SDL_Surface *s_ducks_interface;
SDL_Rect r_duck_interface[3];

//Fonts
TTF_Font *font;
SDL_Rect r_ammo;
SDL_Rect r_score;
SDL_Rect r_hit;
SDL_Rect r_round;
SDL_Surface *s_ammo=NULL;
SDL_Surface *s_score=NULL;
SDL_Surface *s_round;
SDL_Surface *s_static_text[4];

//SDL VARIABLES
SDL_Event event;
SDL_Surface *screen=NULL;

//SPRITES
SDL_Surface *background=NULL;
SDL_Surface *duck_skins[3]={NULL,NULL,NULL};
SDL_Rect clip[10];

//Game Characters
std::vector<Duck> ducks;

//Classes
class GameObject{
    public:
        GameObject();
        bool isDead = false;
        int VEL=1, SPEED = 50;
        int x, y, w, h, dirx, diry, timer;
        SDL_Surface *sprite;
};
class Player{
    public:
        static unsigned int ammo, score;
        Player();
};
unsigned int Player::ammo=3, Player::score=0;
class Duck : public GameObject{
    public:
        Duck(int s);
        void Move();
        void Spawn();
        bool Intersects(int cx, int cy);
        void Die();
};
class Game{
    public:
        static unsigned int round, counter, match;
        static bool hasStart;
        Game();
        static void Start();
        static void SpawnEnemy();
        static void IncreaseCounter();
        static void Loose();
        static void Win();
        static void NextMatch();
};
unsigned int Game::round=1, Game::counter=0, Game::match=0;
bool Game::hasStart=false;

GameObject::GameObject(){

}

void Game::Start(){
    if(match < 10){
        ducks.clear();
        sleep(2);
        Game::SpawnEnemy();
        hasStart = true;
        Player::ammo = 3;
        counter = 0;
        match++;
        char aux1[10];
        sprintf(aux1, "%d", Player::ammo);
        std::cout << aux1 << std::endl;
        s_ammo = TTF_RenderText_Shaded(font, aux1, {255,255,255}, {0,0,0});
        for(int i=0; i<10; i++){
            if(ducks_interface[i] == ACTUAL){
                ducks_interface[i] = ALIVE;
            }
        }
        ducks_interface[match-1] = ACTUAL;
    }else{
        NextMatch();
    }
}

void Game::SpawnEnemy(){
    Duck d(rand() % 3 + 3);
    d.sprite = duck_skins[0];
    ducks.push_back(d);
    ducks.push_back(d);
}

void Game::IncreaseCounter(){
    counter++;
    if(counter > 2000){
        Loose();
    }else if(ducks.size() <= 0){
        Win();
    }
}

void Game::Loose(){
    std::cout << "You loose!" << std::endl;
    hasStart = false;
    Start();
}

void Game::Win(){
    std::cout << "You win!" << std::endl;
    hasStart = false;
    Start();
}

void Game::NextMatch(){
    match = 0;
    round++;
    for(int i=0; i<10; i++){
        ducks_interface[i] = ALIVE;
    }
    char aux[20];
    sprintf(aux, "%d", Game::round);
    s_round = TTF_RenderText_Solid(font, aux, {255,255,255});
    Start();
}

Duck::Duck(int s){
    SPEED = s;
    timer = 0;
    Spawn();
    dirx = VEL;
    diry = -VEL;
    if(rand() % 5 + 1 > 2){
        dirx *= -1;
    }
}

void Duck::Move(){
    timer++;
    if(timer > SPEED/5){

        if(rand() % 300 + 1 > 299){
            dirx *= -1;
        }

        if(x <= 0){
            dirx *= -1;
            x = 1;
        }else if(x >= SCREEN_W){
            dirx *= -1;
            x = SCREEN_W - 1;
        }
        if(y <= 0){
            diry *= -1;
            y = 1;
        }else if(y >= SCREEN_H - (SCREEN_H / 3)){
            diry *= -1;
            y = SCREEN_H - (SCREEN_H / 3) - 1;
        }

        x+=dirx;
        y+=diry;

        timer = 0;
    }
}

void Duck::Spawn(){
    x = 20;
    y = SCREEN_H - (SCREEN_H/3);
    w = ENEMY_W;
    h = ENEMY_H;
}

bool Duck::Intersects(int cx, int cy){
    if((x < cx && x + w > cx) &&
       (y < cy && y + h > cy)){
        return true;
    }
    return false;
}

void Duck::Die(){
    isDead = true;
    for(int i=0; i<ducks.size(); i++){
        if(ducks[i].isDead){
            ducks.erase(ducks.begin() + i);
            Player::score += 500;
            char aux[20];
            sprintf(aux, "%d", Player::score);
            s_score = TTF_RenderText_Shaded(font, aux, {255,255,255}, {0,0,0});
            ducks_interface[Game::match-1] = DEAD;
        }
    }
}

//Engine Methods
void LoadGame();
void Logic();
void DrawScreen();
void Quit();

void LoadGame(){

    //Fundamentals

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);

    srand(static_cast<unsigned int>(time(0)));

    font = TTF_OpenFont("Munro.ttf", 24);

    SDL_WM_SetCaption(GAME_NAME, NULL);
    screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);

    //Interface Text
    SDL_Color white = {255,255,255};

    unsigned int interface_y = SCREEN_H - (SCREEN_H/8.5);
    r_ammo = {SCREEN_W / 10, interface_y, 0, 0};
    r_score = {SCREEN_W - (SCREEN_W / 4.5), interface_y, 0, 0};
    r_round = {SCREEN_W/10, SCREEN_H - SCREEN_H/5, 0, 0};
    s_ammo = TTF_RenderText_Shaded(font, "3", white, {0,0,0});
    s_score = TTF_RenderText_Shaded(font, "0", white, {0,0,0});
    s_round = TTF_RenderText_Solid(font, "1", white);
    s_static_text[0] = TTF_RenderText_Solid(font, "R = ", white);
    s_static_text[1] = TTF_RenderText_Solid(font, "Hit", white);
    s_static_text[2] = TTF_RenderText_Solid(font, "Score", white);
    s_static_text[3] = TTF_RenderText_Solid(font, "Ammo", white);

    //Load Sprites
    r_hit = {SCREEN_W/3, interface_y, 0, 0};

    std::string sprites[3] = {"sprites/background.gif", "sprites/duck.gif", "sprites/interface.gif"};
    background = IMG_Load(sprites[0].c_str());
    duck_skins[0] = IMG_Load(sprites[1].c_str());
    s_ducks_interface = IMG_Load(sprites[2].c_str());

    const int SPRITE_W = 77;
    const int SPRITE_H = 77;

    clip[0] = {0*0, 0, SPRITE_W, SPRITE_H};
    clip[1] = {SPRITE_W*1, 0, SPRITE_W, SPRITE_H};
    clip[2] = {SPRITE_W*2, 0, SPRITE_W, SPRITE_H};

    r_duck_interface[ALIVE] = {0, 20, 18, 20};
    r_duck_interface[DEAD] = {0, 68, 18, 20};
    r_duck_interface[ACTUAL] = {0, 106, 18, 20};

    //SET ENEMY SIZE
    ENEMY_W = clip[0].w;
    ENEMY_H = clip[0].h;

}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* c){
    SDL_Rect offset;

    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, c, destination, &offset);
}

void Shoot(int cx, int cy){
    for(int i=0; i<ducks.size(); i++){
        if(ducks[i].Intersects(cx, cy)){
            ducks[i].Die();
        }
    }
    Player::ammo--;
    char aux1[10];
    sprintf(aux1, "%d", Player::ammo);
    s_ammo = TTF_RenderText_Shaded(font, aux1, {255,255,255}, {0,0,0});
}

bool isLocked = false;
void Logic(){
    for(int i=0; i<ducks.size(); i++){
        ducks[i].Move();
    }

    if(event.type == SDL_MOUSEBUTTONDOWN && !isLocked){
        isLocked = true;
        int cx = event.motion.x;
        int cy = event.motion.y;
        if(Player::ammo > 0){
            Shoot(cx, cy);
        }else
            std::cout << "Out of ammo." << std::endl;
    }else if(event.type == SDL_MOUSEBUTTONUP){
        isLocked = false;
    }

}

int frame_timer=0;
int anim_frame = 0;
void AnimateDucks(std::vector<Duck>& gb, SDL_Surface* destination, SDL_Rect* tiles, int sprites){
    for(int i=0; i<gb.size(); i++){
        if(frame_timer>50){
            anim_frame = rand()%sprites;
            frame_timer=0;
        }
        apply_surface(gb[i].x, gb[i].y, gb[i].sprite, screen, &tiles[anim_frame]);
    }
    frame_timer++;
    std::cout << frame_timer << std::endl;
}

void DrawScreen(){

    apply_surface(0, 0, background, screen, NULL);

    AnimateDucks(ducks, screen, clip, 3);

    //INTERFACE

    apply_surface(r_ammo.x, r_ammo.y, s_ammo, screen, NULL);
    apply_surface(r_score.x+20, r_score.y, s_score, screen, NULL);
    apply_surface(r_round.x+40, r_round.y+9, s_round, screen, NULL);

    for(int i=0; i<10; i++){
        apply_surface((r_hit.x/10)*i+SCREEN_W/3, r_hit.y, s_ducks_interface, screen, &r_duck_interface[ducks_interface[i]]);
    }

    apply_surface(r_round.x, r_round.y+9, s_static_text[0], screen, NULL);
    apply_surface(r_hit.x - 50, r_hit.y, s_static_text[1], screen, NULL);
    apply_surface(r_score.x + 22, r_score.y + 20, s_static_text[2], screen, NULL);
    apply_surface(r_ammo.x - 2, r_ammo.y + 20, s_static_text[3], screen, NULL);

    SDL_Flip(screen);

}

void Quit(){
    SDL_FreeSurface(screen);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

//-------------

int main ( int argc, char** argv ){

    LoadGame();
    Game::Start();

    bool running = true;

    while(running){

        SDL_PollEvent(&event);

        if(event.type == SDL_QUIT){
            running = false;
        }

        if(Game::hasStart){
            Game::IncreaseCounter();
        }
        Logic();
        DrawScreen();


    }

    Quit();

    return 0;
}
