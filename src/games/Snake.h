#pragma once
#include "../Engine.h"
#include "../Audio.h"
#include "../MiniFont.h"
#include <deque>
#include <cstdlib>
#include <cmath>

class Snake : public Scene {
    struct P{int x,y;};

    static constexpr int GW=32, GH=22;

    std::deque<P> snake;
    P    food{0,0};
    int  dx=1,dy=0,ndx=1,ndy=0;
    float acc=0;
    int  score=0, best=0, lives=3;
    bool dead=false;
    int  sw=1920,sh=1080;

    int cellW()const{return sw/GW;}
    int cellH()const{return sh/GH;}
    int offX() const{return (sw-GW*cellW())/2;}
    int offY() const{return (sh-GH*cellH())/2;}

    void spawnFood(){
        while(true){
            int x=rand()%GW, y=rand()%GH;
            bool ok=true;
            for(auto& s:snake) if(s.x==x&&s.y==y){ok=false;break;}
            if(ok){food={x,y};break;}
        }
    }

    void reset(){
        snake.clear();
        snake.push_back({GW/2,GH/2});
        snake.push_back({GW/2-1,GH/2});
        snake.push_back({GW/2-2,GH/2});
        dx=1;dy=0;ndx=1;ndy=0;
        score=0;dead=false;acc=0;
        spawnFood();
    }

public:
    void onEnter()override{reset();}

    void update(float dt,const Input& in)override{
        sw=in.screenW;sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead){if(in.keyJustPressed[SDL_SCANCODE_RETURN])reset();return;} // handled below

        if((in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A])&&dx!=1) {ndx=-1;ndy=0;}
        if((in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D])&&dx!=-1){ndx=1; ndy=0;}
        if((in.keys[SDL_SCANCODE_UP]   ||in.keys[SDL_SCANCODE_W])&&dy!=1) {ndx=0; ndy=-1;}
        if((in.keys[SDL_SCANCODE_DOWN] ||in.keys[SDL_SCANCODE_S])&&dy!=-1){ndx=0; ndy=1;}

        float step=std::max(0.05f,0.13f-score*0.0018f);
        acc+=dt;
        while(acc>=step){
            acc-=step;
            dx=ndx;dy=ndy;
            P nh={snake.front().x+dx,snake.front().y+dy};
            if(nh.x<0||nh.x>=GW||nh.y<0||nh.y>=GH){ if(score>best)best=score; dead=true;Audio::get().sfxDie();return;}
            for(auto& s:snake) if(s.x==nh.x&&s.y==nh.y){ if(score>best)best=score; dead=true;Audio::get().sfxDie();return;}
            snake.push_front(nh);
            if(nh.x==food.x&&nh.y==food.y){score+=10;spawnFood();Audio::get().sfxCoin();}
            else snake.pop_back();
        }
    }

    void render(SDL_Renderer* r)override{
        SDL_SetRenderDrawColor(r,8,14,24,255);
        SDL_RenderClear(r);

        int cw=cellW(),ch=cellH(),ox=offX(),oy=offY();

        // grid bg
        SDL_SetRenderDrawColor(r,18,28,42,255);
        SDL_FRect bg{(float)ox,(float)oy,(float)(GW*cw),(float)(GH*ch)};
        SDL_RenderFillRect(r,&bg);

        // grid lines
        SDL_SetRenderDrawColor(r,24,38,55,255);
        for(int x=0;x<=GW;x++)
            SDL_RenderLine(r,(float)(ox+x*cw),(float)oy,(float)(ox+x*cw),(float)(oy+GH*ch));
        for(int y=0;y<=GH;y++)
            SDL_RenderLine(r,(float)ox,(float)(oy+y*ch),(float)(ox+GW*cw),(float)(oy+y*ch));

        // border
        SDL_SetRenderDrawColor(r,60,120,100,255);
        SDL_FRect border{(float)(ox-2),(float)(oy-2),(float)(GW*cw+4),(float)(GH*ch+4)};
        SDL_RenderRect(r,&border);

        // food pulse
        float pulse=(float)(SDL_GetTicks()%800)/800.f;
        float ps=2.f+std::sin(pulse*6.28f)*2.f;
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,255,80,130,60);
        SDL_FRect fg{(float)(ox+food.x*cw)-ps,(float)(oy+food.y*ch)-ps,
                     (float)cw+ps*2,(float)ch+ps*2};
        SDL_RenderFillRect(r,&fg);
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(r,255,100,150,255);
        SDL_FRect fd{(float)(ox+food.x*cw+2),(float)(oy+food.y*ch+2),
                     (float)(cw-4),(float)(ch-4)};
        SDL_RenderFillRect(r,&fd);

        // snake
        for(int i=0;i<(int)snake.size();i++){
            float frac=1.f-(float)i/snake.size();
            uint8_t g=(uint8_t)(60+frac*195);
            SDL_SetRenderDrawColor(r,40,g,100,255);
            SDL_FRect seg{(float)(ox+snake[i].x*cw+2),(float)(oy+snake[i].y*ch+2),
                          (float)(cw-4),(float)(ch-4)};
            SDL_RenderFillRect(r,&seg);
            // head highlight
            if(i==0){
                SDL_SetRenderDrawColor(r,160,255,180,255);
                SDL_FRect hl{(float)(ox+snake[i].x*cw+2),(float)(oy+snake[i].y*ch+2),
                             (float)(cw-4),(float)3};
                SDL_RenderFillRect(r,&hl);
            }
        }

        // HUD
        int ts=std::max(2,sw/480);
        Font::draw(r,"SCORE: "+std::to_string(score),ox,oy-ts*10,ts,255,220,120);
        Font::draw(r,"BEST: "+std::to_string(best),ox+GW*cw/2,oy-ts*10,ts,120,220,255);
        Font::draw(r,"LENGTH: "+std::to_string(snake.size()),
                   ox+GW*cw-Font::width("LENGTH: "+std::to_string(snake.size()),ts),
                   oy-ts*10,ts,120,200,255);

        std::string h="ARROWS/WASD MOVE   ESC MENU";
        Font::draw(r,h,sw/2-Font::width(h,1)/2,sh-24,1,100,100,130);

        if(dead){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,180);
            SDL_FRect ov{(float)(sw/2-280),(float)(sh/2-80),560.f,160.f};
            SDL_RenderFillRect(r,&ov);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r,255,80,80,255);
            SDL_RenderRect(r,&ov);
            int gs=std::max(3,sw/320);
            Font::draw(r,"GAME OVER",sw/2-Font::width("GAME OVER",gs)/2,sh/2-60,gs,255,80,80);
            std::string s2="SCORE "+std::to_string(score)+"   ENTER=RETRY";
            Font::draw(r,s2,sw/2-Font::width(s2,2)/2,sh/2+10,2,220,220,220);
        }
    }
};
