#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class BrickBreaker : public Scene {
    struct Brick{float x,y,w,h; bool alive; uint8_t r,g,b;};

    float padX=0,padW=0,padH=0;
    float ballX=0,ballY=0,ballVX=0,ballVY=0,ballR=0;
    std::vector<Brick> bricks;
    int   score=0, level=1;
    bool  launched=false,dead=false,won=false;
    int   sw=1920,sh=1080;

    void reset(){
        padW=sw*0.14f; padH=sh*0.022f;
        padX=sw*0.5f-padW*0.5f;
        ballR=sw*0.008f;
        ballX=sw*0.5f; ballY=sh*0.82f;
        ballVX=0; ballVY=0;
        launched=false; dead=false; won=false; score=0; level=1;
        bricks.clear();

        int cols=12, rows=6;
        float gap=sw*0.006f;
        float bw=(sw*0.88f-(cols-1)*gap)/cols;
        float bh=sh*0.038f;
        float startX=sw*0.06f, startY=sh*0.12f;
        uint8_t pal[6][3]={{255,80,80},{255,160,60},{255,220,60},
                           {100,220,100},{80,180,255},{180,100,255}};
        for(int r=0;r<rows;r++)
            for(int c=0;c<cols;c++)
                bricks.push_back({
                    startX+c*(bw+gap), startY+r*(bh+gap),
                    bw, bh, true,
                    pal[r][0],pal[r][1],pal[r][2]
                });
    }

public:
    void onEnter()override{reset();}

    void update(float dt,const Input& in)override{
        sw=in.screenW;sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead){if(in.keyJustPressed[SDL_SCANCODE_RETURN])reset();return;}

        float ps=sw*0.55f;
        if(in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A]) padX-=ps*dt;
        if(in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D]) padX+=ps*dt;
        padX=std::clamp(padX,0.f,(float)sw-padW);

        if(!launched){
            ballX=padX+padW*0.5f;
            ballY=sh*0.82f-ballR*2;
            if(in.keyJustPressed[SDL_SCANCODE_SPACE]){
                launched=true;
                float spd=sh*0.68f;
                ballVX=spd*0.5f*(rand()%2?1:-1);
                ballVY=-spd;
            }
            return;
        }

        ballX+=ballVX*dt;
        ballY+=ballVY*dt;

        // wall bounces
        if(ballX-ballR<0)      {ballX=ballR;       ballVX=std::fabs(ballVX);}
        if(ballX+ballR>(float)sw){ballX=(float)sw-ballR;ballVX=-std::fabs(ballVX);}
        if(ballY-ballR<0)      {ballY=ballR;       ballVY=std::fabs(ballVY);}

        // paddle hit
        float padY=(float)sh*0.85f;
        if(ballY+ballR>=padY&&ballY+ballR<=padY+padH*2&&
           ballX>=padX-ballR&&ballX<=padX+padW+ballR&&ballVY>0){
            ballVY=-std::fabs(ballVY);
            float off=(ballX-(padX+padW*0.5f))/(padW*0.5f);
            ballVX+=off*sh*0.22f;
            // clamp speed
            float s=std::sqrt(ballVX*ballVX+ballVY*ballVY);
            float ms=sh*0.85f;
            if(s>ms){ballVX=ballVX/s*ms;ballVY=ballVY/s*ms;}
            ballY=padY-ballR-1;
        }

        // brick collisions
        int alive=0;
        for(auto& b:bricks){
            if(!b.alive){continue;}
            alive++;
            float left=b.x,right=b.x+b.w,top=b.y,bot=b.y+b.h;
            if(ballX+ballR<left||ballX-ballR>right||
               ballY+ballR<top ||ballY-ballR>bot)  continue;
            // which face?
            float overL=ballX+ballR-left, overR=right-ballX+ballR;
            float overT=ballY+ballR-top,  overB=bot-ballY+ballR;
            float minOver=std::min({overL,overR,overT,overB});
            if(minOver==overT||minOver==overB) ballVY=-ballVY;
            else                               ballVX=-ballVX;
            b.alive=false; score+=50; alive--;
        }
        if(alive==0){
            level++;
            // random new brick layout
            bricks.clear();
            int cols=8+rand()%5, rows=3+rand()%4;
            float gap=sw*0.006f;
            float bw=(sw*0.88f-(cols-1)*gap)/cols;
            float bh=sh*0.038f;
            float startX=sw*0.06f, startY=sh*0.10f;
            uint8_t pal[7][3]={{255,80,80},{255,160,60},{255,220,60},
                               {100,220,100},{80,180,255},{180,100,255},{255,120,180}};
            for(int r=0;r<rows;r++)
                for(int c=0;c<cols;c++){
                    int pi=rand()%7;
                    bricks.push_back({
                        startX+c*(bw+gap), startY+r*(bh+sh*0.012f),
                        bw, bh, true,
                        pal[pi][0],pal[pi][1],pal[pi][2]
                    });
                }
            // re-launch ball
            launched=false;
            ballVX=0; ballVY=0;
        }

        // lost
        if(ballY-ballR>(float)sh) dead=true;
    }

    void render(SDL_Renderer* r)override{
        SDL_SetRenderDrawColor(r,12,14,28,255);
        SDL_RenderClear(r);

        // bricks
        for(auto& b:bricks){
            if(!b.alive)continue;
            SDL_SetRenderDrawColor(r,b.r,b.g,b.b,255);
            SDL_FRect br{b.x,b.y,b.w,b.h};
            SDL_RenderFillRect(r,&br);
            // shine
            SDL_SetRenderDrawColor(r,255,255,255,60);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_FRect sh2{b.x+2,b.y+2,b.w-4,b.h*0.35f};
            SDL_RenderFillRect(r,&sh2);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
        }

        // paddle
        float padY=(float)sh*0.85f;
        SDL_SetRenderDrawColor(r,140,200,255,255);
        SDL_FRect pad{padX,padY,padW,padH};
        SDL_RenderFillRect(r,&pad);

        // ball
        SDL_SetRenderDrawColor(r,255,240,160,255);
        // draw filled circle for ball
        int ir=(int)ballR;
        for(int dy2=-ir;dy2<=ir;dy2++){
            int dx2=(int)std::sqrt((float)(ir*ir-dy2*dy2));
            SDL_RenderLine(r,(float)(ballX-dx2),(float)(ballY+dy2),
                             (float)(ballX+dx2),(float)(ballY+dy2));
        }

        if(!launched){
            std::string s="SPACE TO LAUNCH";
            Font::draw(r,s,sw/2-Font::width(s,2)/2,(int)(sh*0.5f),2,255,255,180);
        }

        int ts=std::max(2,sw/480);
        Font::draw(r,"LEVEL: "+std::to_string(level),20,20,ts,120,200,255);
        Font::draw(r,"SCORE: "+std::to_string(score),sw/2-Font::width("SCORE: "+std::to_string(score),ts)/2,
                   20,ts,255,220,120);
        std::string h="A/D OR ARROWS MOVE   SPACE LAUNCH   ESC MENU";
        Font::draw(r,h,sw/2-Font::width(h,1)/2,sh-24,1,100,100,130);

        if(dead){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,180);
            SDL_FRect ov{(float)(sw/2-280),(float)(sh/2-80),560.f,160.f};
            SDL_RenderFillRect(r,&ov);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            uint8_t cr=255,cg=80,cb=80;
            SDL_SetRenderDrawColor(r,cr,cg,cb,255);
            SDL_RenderRect(r,&ov);
            std::string msg="GAME OVER";
            int gs=std::max(3,sw/320);
            Font::draw(r,msg,sw/2-Font::width(msg,gs)/2,(int)(sh/2)-60,gs,cr,cg,cb);
            std::string s2="SCORE "+std::to_string(score)+"   ENTER=RETRY";
            Font::draw(r,s2,sw/2-Font::width(s2,2)/2,(int)(sh/2)+10,2,220,220,220);
        }
    }
};
