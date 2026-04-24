#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

class Menu : public Scene {
    struct Tile { std::string id,title,sub; uint8_t r,g,b; };
    struct Star { float x,y,spd; uint8_t br; };

    std::vector<Tile> tiles;
    std::vector<Star> stars;
    int   sel  = 0;
    float time = 0;
    int   sw   = 1920, sh = 1080;

public:
    Menu() {
        tiles = {
            {"cosmic_dash",   "TETRIS",         "LINE CLEAR",          120,180,255},
            {"asteroid",      "ASTEROID FIELD", "THRUST + SHOOT",     120,220,255},
            {"brick_breaker", "BRICK BREAKER",  "IMPULSE PHYSICS",    255,100,150},
            {"snake",         "NEON SNAKE",     "GRID GAMEPLAY",      120,255,120},
            {"particle",      "SPACE INVADERS", "DEFEND EARTH",       100,255,160},
            {"pong",          "PONG AI",        "PREDICTIVE AI",      180,140,255},
        };
    }

    void onEnter() override { sel=0; time=0; stars.clear(); }

    void update(float dt, const Input& in) override {
        sw = in.screenW; sh = in.screenH;
        time += dt;

        if(in.keyJustPressed[SDL_SCANCODE_RIGHT]||in.keyJustPressed[SDL_SCANCODE_D])
            sel=(sel+1)%6;
        if(in.keyJustPressed[SDL_SCANCODE_LEFT]||in.keyJustPressed[SDL_SCANCODE_A])
            sel=(sel+5)%6;
        if((in.keyJustPressed[SDL_SCANCODE_DOWN]||in.keyJustPressed[SDL_SCANCODE_S])&&sel+3<6)
            sel+=3;
        if((in.keyJustPressed[SDL_SCANCODE_UP]||in.keyJustPressed[SDL_SCANCODE_W])&&sel-3>=0)
            sel-=3;
        if(in.keyJustPressed[SDL_SCANCODE_RETURN]||in.keyJustPressed[SDL_SCANCODE_SPACE])
            nextScene=tiles[sel].id;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE])
            wantsQuit=true;

        // seed stars once we know screen size
        if(stars.empty())
            for(int i=0;i<280;i++)
                stars.push_back({(float)(rand()%sw),(float)(rand()%sh),
                                 12.f+(rand()%90),(uint8_t)(70+rand()%185)});
        for(auto& s:stars){
            s.x-=s.spd*dt;
            if(s.x<0){s.x=(float)sw;s.y=(float)(rand()%sh);}
        }
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r,6,6,18,255);
        SDL_RenderClear(r);

        // stars
        for(auto& s:stars){
            SDL_SetRenderDrawColor(r,s.br,s.br,(uint8_t)std::min(255,s.br+40),255);
            float sz=s.spd>70?2.f:1.f;
            SDL_FRect p{s.x,s.y,sz,sz};
            SDL_RenderFillRect(r,&p);
        }

        int cx=sw/2;

        // title
        int ts=std::max(4,sw/160);
        std::string t="KRAKEN 2D";
        Font::draw(r,t,cx-Font::width(t,ts)/2,(int)(sh*0.06f),ts,120,220,255);
        std::string sub="ARCADE COLLECTION";
        int ss=std::max(1,sw/550);
        Font::draw(r,sub,cx-Font::width(sub,ss)/2,(int)(sh*0.06f+ts*8+12),ss,160,160,190);

        // tiles
        const int COLS=3;
        int tw=(int)(sw*0.26f), th=(int)(sh*0.22f), gap=(int)(sw*0.025f);
        int gw=COLS*tw+(COLS-1)*gap;
        int sx2=(sw-gw)/2, sy2=(int)(sh*0.22f);

        for(int i=0;i<6;i++){
            int col=i%COLS, row=i/COLS;
            int tx=sx2+col*(tw+gap), ty=sy2+row*(th+gap);
            bool sel2=(i==sel);
            float pulse=sel2?(std::sin(time*6.f)*0.5f+0.5f):0.f;

            // translucent fill — stars show through
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,
                tiles[i].r/4, tiles[i].g/4, tiles[i].b/4,
                sel2?(uint8_t)(55+(int)(pulse*55)):30);
            SDL_FRect tile{(float)tx,(float)ty,(float)tw,(float)th};
            SDL_RenderFillRect(r,&tile);

            // border
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r,tiles[i].r,tiles[i].g,tiles[i].b,
                                   sel2?255:90);
            int bk=sel2?3:1;
            for(int k=0;k<bk;k++){
                SDL_FRect b{(float)(tx-k),(float)(ty-k),
                            (float)(tw+2*k),(float)(th+2*k)};
                SDL_RenderRect(r,&b);
            }

            // corner dots on selected
            if(sel2){
                uint8_t glow=(uint8_t)(160+(int)(pulse*95));
                SDL_SetRenderDrawColor(r,tiles[i].r,tiles[i].g,tiles[i].b,glow);
                float cs=5.f;
                SDL_FRect corners[4]={
                    {(float)tx-cs,       (float)ty-cs,       cs*2,cs*2},
                    {(float)(tx+tw)-cs,  (float)ty-cs,       cs*2,cs*2},
                    {(float)tx-cs,       (float)(ty+th)-cs,  cs*2,cs*2},
                    {(float)(tx+tw)-cs,  (float)(ty+th)-cs,  cs*2,cs*2},
                };
                for(auto& c:corners) SDL_RenderFillRect(r,&c);
            }

            // number
            int ns=std::max(1,tw/100);
            Font::draw(r,std::to_string(i+1),tx+10,ty+10,ns,
                       tiles[i].r,tiles[i].g,tiles[i].b);

            // game title
            int gt=std::max(1,tw/110);
            Font::draw(r,tiles[i].title,
                tx+(tw-Font::width(tiles[i].title,gt))/2,
                ty+th/2-8, gt, 255,255,255);

            // subtitle
            int st2=std::max(1,tw/220);
            Font::draw(r,tiles[i].sub,
                tx+(tw-Font::width(tiles[i].sub,st2))/2,
                ty+th/2+gt*9, st2,
                tiles[i].r,tiles[i].g,tiles[i].b);
        }

        // hint
        std::string hint="ARROWS/WASD: SELECT   ENTER: PLAY   ESC: QUIT";
        Font::draw(r,hint,cx-Font::width(hint,1)/2,sh-28,1,110,110,145);
    }
};
