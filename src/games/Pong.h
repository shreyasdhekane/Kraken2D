#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <cmath>
#include <algorithm>
#include <vector>

class Pong : public Scene {
    float lY=0,rY=0,bX=0,bY=0,bVX=0,bVY=0;
    int   lScore=0,rScore=0;
    int   sw=1920,sh=1080;

    struct Pt{float x,y;};
    std::vector<Pt> trail;

    float pW()  const{return sw*0.012f;}
    float pH()  const{return sh*0.18f;}
    float bSz() const{return sw*0.013f;}
    float lX()  const{return sw*0.045f;}
    float rX()  const{return sw-lX()-pW();}
    float maxSpd()const{return sh*0.72f;}

    void resetBall(){
        bX=sw*0.5f; bY=sh*0.5f;
        float spd=sh*0.42f;
        float ang=((rand()%100)/100.f-0.5f)*0.9f;
        float dir=(rand()%2)?1.f:-1.f;
        bVX=std::cos(ang)*spd*dir;
        bVY=std::sin(ang)*spd;
        lY=sh*0.5f-pH()*0.5f;
        rY=sh*0.5f-pH()*0.5f;
        trail.clear();
    }

    void clamp(){
        float s=std::sqrt(bVX*bVX+bVY*bVY);
        float ms=maxSpd();
        if(s>ms){bVX=bVX/s*ms;bVY=bVY/s*ms;}
        float mn=sh*0.25f;
        if(std::fabs(bVX)<mn) bVX=(bVX<0?-1:1)*mn;
    }

public:
    void onEnter()override{lScore=0;rScore=0;resetBall();}

    void update(float dt,const Input& in)override{
        sw=in.screenW; sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}

        float ps=sh*0.55f;
        if(in.keys[SDL_SCANCODE_W]||in.keys[SDL_SCANCODE_UP])   lY-=ps*dt;
        if(in.keys[SDL_SCANCODE_S]||in.keys[SDL_SCANCODE_DOWN]) lY+=ps*dt;
        lY=std::clamp(lY,0.f,(float)(sh)-pH());

        // predictive AI
        float predY=bY;
        if(bVX>0){
            float t=(rX()-bX)/bVX;
            predY=bY+bVY*t;
            float fh=(float)sh;
            while(predY<0||predY>fh){
                if(predY<0)  predY=-predY;
                if(predY>fh) predY=2.f*fh-predY;
            }
        }
        float tgt=predY-pH()*0.5f;
        float diff=tgt-rY;
        float ais=sh*0.48f;
        rY+=std::clamp(diff,-ais*dt,ais*dt);
        rY=std::clamp(rY,0.f,(float)(sh)-pH());

        bX+=bVX*dt; bY+=bVY*dt;

        if(bY<0)      {bY=0;      bVY= std::fabs(bVY);}
        if(bY>(float)sh){bY=(float)sh;bVY=-std::fabs(bVY);}

        float bsz=bSz(),ph2=pH(),pw2=pW();
        // left paddle
        if(bX-bsz*0.5f<=lX()+pw2&&bX-bsz*0.5f>=lX()&&bVX<0&&
           bY>=lY-bsz&&bY<=lY+ph2+bsz){
            bVX=std::fabs(bVX);
            bVY+=(bY-(lY+ph2*0.5f))/(ph2*0.5f)*sh*0.18f;
            clamp(); bX=lX()+pw2+bsz*0.5f+1;
        }
        // right paddle
        if(bX+bsz*0.5f>=rX()&&bX+bsz*0.5f<=rX()+pw2&&bVX>0&&
           bY>=rY-bsz&&bY<=rY+ph2+bsz){
            bVX=-std::fabs(bVX);
            bVY+=(bY-(rY+ph2*0.5f))/(ph2*0.5f)*sh*0.18f;
            clamp(); bX=rX()-bsz*0.5f-1;
        }

        if(bX<-bsz){rScore++;resetBall();}
        if(bX>(float)sw+bsz){lScore++;resetBall();}

        trail.push_back({bX,bY});
        if((int)trail.size()>16) trail.erase(trail.begin());
    }

    void render(SDL_Renderer* r)override{
        SDL_SetRenderDrawColor(r,8,10,20,255);
        SDL_RenderClear(r);

        float cx2=sw*0.5f;
        // dashed center line
        SDL_SetRenderDrawColor(r,45,50,75,255);
        float dh=sh*0.03f, dg=sh*0.015f;
        for(float y=0;y<sh;y+=dh+dg){
            SDL_FRect d{cx2-2.f,y,4.f,dh};
            SDL_RenderFillRect(r,&d);
        }

        // trail
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        for(int i=0;i<(int)trail.size();i++){
            float frac=(float)i/trail.size();
            SDL_SetRenderDrawColor(r,255,255,160,(uint8_t)(frac*frac*180));
            float sz=bSz()*(0.3f+0.7f*frac);
            SDL_FRect tr{trail[i].x-sz*0.5f,trail[i].y-sz*0.5f,sz,sz};
            SDL_RenderFillRect(r,&tr);
        }
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);

        // paddles
        SDL_SetRenderDrawColor(r,140,180,255,255);
        SDL_FRect lp{lX(),lY,pW(),pH()};
        SDL_RenderFillRect(r,&lp);
        SDL_SetRenderDrawColor(r,255,140,160,255);
        SDL_FRect rp{rX(),rY,pW(),pH()};
        SDL_RenderFillRect(r,&rp);

        // ball
        SDL_SetRenderDrawColor(r,255,255,200,255);
        float bsz=bSz();
        SDL_FRect ball{bX-bsz*0.5f,bY-bsz*0.5f,bsz,bsz};
        SDL_RenderFillRect(r,&ball);

        // scores
        int sc=std::max(4,sw/160);
        auto ls=std::to_string(lScore), rs=std::to_string(rScore);
        Font::draw(r,ls,(int)(cx2*0.55f),(int)(sh*0.06f),sc,140,180,255);
        Font::draw(r,rs,(int)(cx2+cx2*0.35f),(int)(sh*0.06f),sc,255,140,160);

        int sc2=std::max(2,sw/350);
        Font::draw(r,"YOU",(int)(cx2*0.5f)-Font::width("YOU",sc2)/2,(int)(sh*0.04f),sc2,140,180,255);
        Font::draw(r,"CPU",(int)(cx2+cx2*0.4f)-Font::width("CPU",sc2)/2,(int)(sh*0.04f),sc2,255,140,160);

        std::string h="W/S OR UP/DOWN TO MOVE   ESC = MENU";
        Font::draw(r,h,(int)cx2-Font::width(h,1)/2,sh-28,1,100,100,130);
    }
};
