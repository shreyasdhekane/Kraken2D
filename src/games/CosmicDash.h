#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class CosmicDash : public Scene {
    struct Platform{float x,y,w,h;};
    struct Coin    {float x,y; bool alive;};
    struct Hazard  {float x,y,r; bool alive;};

    // Player
    float pX=0,pY=0,pVX=0,pVY=0;
    float pW=28,pH=36;
    bool  grounded=false;
    float coyote=0,jBuf=0;
    int   lives=3,score=0;
    bool  dead=false,won=false;

    // Level
    std::vector<Platform> plats;
    std::vector<Coin>     coins;
    std::vector<Hazard>   hazards;
    float levelEnd=0;

    // Camera
    float camX=0;

    // Stars (parallax)
    struct Star{float x,y,spd;};
    std::vector<Star> stars;

    int sw=1920,sh=1080;

    float GRAV()const{return sh*1.8f;}
    float JUMP()const{return sh*0.72f;}
    float SPD() const{return sw*0.22f;}

    void reset(){
        pX=sw*0.1f; pY=sh*0.4f;
        pVX=0;pVY=0;camX=0;
        lives=3;score=0;dead=false;won=false;
        plats.clear();coins.clear();hazards.clear();stars.clear();

        // generate level
        float px=0;
        for(int i=0;i<50;i++){
            float pw2=sw*(0.08f+0.12f*(rand()%100)/100.f);
            float py=sh*(0.5f+0.25f*(rand()%100-50)/100.f);
            if(i>0&&rand()%4==0) px+=sw*0.06f; // gap
            plats.push_back({px+pw2*0.5f,py,pw2,sh*0.04f});
            if(rand()%2==0)
                coins.push_back({px+pw2*0.5f+(rand()%60-30),py-sh*0.1f,true});
            if(i>4&&rand()%4==0)
                hazards.push_back({px+pw2*0.5f,py-sh*0.04f,sw*0.018f,true});
            px+=pw2;
        }
        levelEnd=px;

        for(int i=0;i<200;i++)
            stars.push_back({(float)(rand()%(int)(levelEnd+sw)),
                             (float)(rand()%sh),10.f+(rand()%60)});
    }

    bool rectRect(float ax,float ay,float aw,float ah,
                  float bx,float by,float bw,float bh){
        return ax<bx+bw&&ax+aw>bx&&ay<by+bh&&ay+ah>by;
    }

public:
    void onEnter()override{reset();}

    void update(float dt,const Input& in)override{
        sw=in.screenW;sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead||won){if(in.keyJustPressed[SDL_SCANCODE_RETURN])reset();return;}

        // Input
        pVX=0;
        if(in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A]) pVX=-SPD();
        if(in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D]) pVX= SPD();

        if(grounded) coyote=0.15f; else coyote=std::max(0.f,coyote-dt);
        if(in.keyJustPressed[SDL_SCANCODE_SPACE]||
           in.keyJustPressed[SDL_SCANCODE_UP]||
           in.keyJustPressed[SDL_SCANCODE_W])
            jBuf=0.15f;
        else jBuf=std::max(0.f,jBuf-dt);

        if(jBuf>0&&coyote>0){pVY=-JUMP();jBuf=0;coyote=0;}

        // gravity
        pVY+=GRAV()*dt;
        grounded=false;

        // integrate
        pX+=pVX*dt;
        pY+=pVY*dt;

        // platform collisions
        for(auto& pl:plats){
            float plLeft=pl.x-pl.w*0.5f,plTop=pl.y-pl.h*0.5f;
            if(!rectRect(pX-pW*0.5f,pY-pH*0.5f,pW,pH,plLeft,plTop,pl.w,pl.h))continue;
            // resolve Y
            float prevBottom=pY-pVY*dt+pH*0.5f;
            if(prevBottom<=plTop+2&&pVY>=0){
                pY=plTop-pH*0.5f;
                pVY=0; grounded=true;
            } else if(pVY<0){
                pY=plTop+pl.h+pH*0.5f;pVY=0;
            }
        }

        // coins
        for(auto& c:coins){
            if(!c.alive)continue;
            float dx=c.x-pX,dy=c.y-pY;
            if(std::sqrt(dx*dx+dy*dy)<sw*0.03f){c.alive=false;score+=10;}
        }
        // hazards
        for(auto& h:hazards){
            if(!h.alive)continue;
            float dx=h.x-pX,dy=h.y-pY;
            if(std::sqrt(dx*dx+dy*dy)<h.r+pW*0.4f){
                lives--;h.alive=false;
                if(lives<=0)dead=true;
                pVY=-JUMP()*0.6f;
            }
        }

        // fall
        if(pY>sh+200){
            lives--;
            if(lives<=0){dead=true;return;}
            pX=std::max(sw*0.1f,camX);pY=sh*0.3f;pVY=0;
        }

        // win
        if(pX>=levelEnd-sw*0.05f) won=true;

        // camera
        float targetCam=pX-sw*0.25f;
        camX+=(targetCam-camX)*dt*5.f;
        camX=std::max(0.f,camX);
    }

    float wx(float x)const{return x-camX;}

    void render(SDL_Renderer* r)override{
        // sky gradient
        for(int y=0;y<sh;y++){
            int v=8+y*8/sh;
            SDL_SetRenderDrawColor(r,(uint8_t)v,(uint8_t)v,(uint8_t)(v+12),255);
            SDL_RenderLine(r,0,(float)y,(float)sw,(float)y);
        }

        // parallax stars
        SDL_SetRenderDrawColor(r,180,180,220,255);
        for(auto& s:stars){
            float sx=wx(s.x*0.3f+camX*0.3f);
            if(sx<0||sx>sw)continue;
            SDL_FRect p{sx,(float)((int)s.y),2.f,2.f};
            SDL_RenderFillRect(r,&p);
        }

        // goal flag
        float gx=wx(levelEnd-sw*0.05f),gy=sh*0.3f;
        SDL_SetRenderDrawColor(r,120,255,120,255);
        SDL_FRect goal{gx,gy,sw*0.025f,sh*0.12f};
        SDL_RenderFillRect(r,&goal);

        // platforms
        for(auto& pl:plats){
            float px2=wx(pl.x-pl.w*0.5f);
            if(px2>sw||px2+pl.w<0)continue;
            SDL_SetRenderDrawColor(r,70,90,140,255);
            SDL_FRect pf{px2,pl.y-pl.h*0.5f,pl.w,pl.h};
            SDL_RenderFillRect(r,&pf);
            SDL_SetRenderDrawColor(r,100,130,200,255);
            SDL_FRect top{px2,pl.y-pl.h*0.5f,pl.w,4.f};
            SDL_RenderFillRect(r,&top);
        }

        // coins
        for(auto& c:coins){
            if(!c.alive)continue;
            float cx2=wx(c.x);
            if(cx2<-20||cx2>sw+20)continue;
            // draw circle
            float pulse=2.f+std::sin(SDL_GetTicks()*0.005f)*2.f;
            SDL_SetRenderDrawColor(r,255,220,60,255);
            int ir=(int)(sw*0.012f+pulse);
            for(int dy=-ir;dy<=ir;dy++){
                int dx2=(int)std::sqrt((float)(ir*ir-dy*dy));
                SDL_RenderLine(r,(float)(cx2-dx2),(float)(c.y+dy),
                                 (float)(cx2+dx2),(float)(c.y+dy));
            }
        }

        // hazards
        for(auto& h:hazards){
            if(!h.alive)continue;
            float hx=wx(h.x);
            if(hx<-50||hx>sw+50)continue;
            SDL_SetRenderDrawColor(r,200,60,60,255);
            int ir=(int)h.r;
            for(int dy=-ir;dy<=ir;dy++){
                int dx2=(int)std::sqrt((float)(ir*ir-dy*dy));
                SDL_RenderLine(r,(float)(hx-dx2),(float)(h.y+dy),
                                 (float)(hx+dx2),(float)(h.y+dy));
            }
        }

        // player
        float px2=wx(pX);
        SDL_SetRenderDrawColor(r,255,220,80,255);
        SDL_FRect player{px2-pW*0.5f,pY-pH*0.5f,pW,pH};
        SDL_RenderFillRect(r,&player);
        // visor
        SDL_SetRenderDrawColor(r,100,200,255,255);
        SDL_FRect vis{px2-pW*0.3f,pY-pH*0.35f,pW*0.6f,pH*0.2f};
        SDL_RenderFillRect(r,&vis);

        // HUD
        int ts=std::max(2,sw/480);
        Font::draw(r,"SCORE: "+std::to_string(score),20,20,ts,255,220,80);
        Font::draw(r,"LIVES: "+std::to_string(lives),20,20+ts*10,ts,255,100,100);
        std::string h="WASD/ARROWS MOVE   SPACE/UP JUMP   ESC MENU";
        Font::draw(r,h,sw/2-Font::width(h,1)/2,sh-24,1,100,100,130);

        if(dead||won){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,180);
            SDL_FRect ov{(float)(sw/2-280),(float)(sh/2-80),560.f,160.f};
            SDL_RenderFillRect(r,&ov);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            uint8_t cr=won?120:255,cg=won?255:80,cb=80;
            SDL_SetRenderDrawColor(r,cr,cg,cb,255);
            SDL_RenderRect(r,&ov);
            std::string msg=won?"LEVEL CLEAR!":"GAME OVER";
            int gs=std::max(3,sw/320);
            Font::draw(r,msg,sw/2-Font::width(msg,gs)/2,sh/2-60,gs,cr,cg,cb);
            std::string s2="SCORE "+std::to_string(score)+"   ENTER=RETRY";
            Font::draw(r,s2,sw/2-Font::width(s2,2)/2,sh/2+10,2,220,220,220);
        }
    }
};
