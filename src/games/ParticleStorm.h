#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class ParticleStorm : public Scene {
    struct Particle{
        float x,y,vx,vy,r;
        uint8_t cr,cg,cb;
    };
    struct Wall{float x,y,w,h;};

    std::vector<Particle> parts;
    std::vector<Wall>     walls;
    int sw=1920,sh=1080;
    float gravX=0,gravY=0;
    int   mode=0; // 0=gravity 1=attract 2=repel 3=zero-g
    float modeTimer=0;

    void addParticles(int n){
        for(int i=0;i<n;i++){
            float x=(float)(rand()%sw),y=(float)(rand()%sh);
            float ang=(rand()%628)/100.f;
            float spd=60.f+rand()%180;
            float r=3.f+rand()%5;
            parts.push_back({x,y,
                std::cos(ang)*spd,std::sin(ang)*spd,r,
                (uint8_t)(100+rand()%155),
                (uint8_t)(50 +rand()%180),
                (uint8_t)(140+rand()%115)});
        }
    }

    void buildWalls(){
        float t=sh*0.02f;
        walls={{0,0,(float)sw,t},{0,(float)(sh-t),(float)sw,t},
               {0,0,t,(float)sh},{(float)(sw-t),0,t,(float)sh}};
        // inner obstacles
        walls.push_back({sw*0.25f,sh*0.3f,sw*0.04f,sh*0.4f});
        walls.push_back({sw*0.5f, sh*0.1f,sw*0.04f,sh*0.4f});
        walls.push_back({sw*0.75f,sh*0.3f,sw*0.04f,sh*0.4f});
    }

public:
    void onEnter()override{
        parts.clear();walls.clear();
        buildWalls();
        addParticles(300);
    }

    void update(float dt,const Input& in)override{
        sw=in.screenW;sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(in.keyJustPressed[SDL_SCANCODE_SPACE]) addParticles(80);
        if(in.keyJustPressed[SDL_SCANCODE_R])     {parts.clear();addParticles(300);}

        // cycle gravity modes
        modeTimer+=dt;
        if(in.keyJustPressed[SDL_SCANCODE_G]) mode=(mode+1)%4;

        float gStrength=sh*1.1f;
        float cx=(float)sw*0.5f,cy=(float)sh*0.5f;

        for(auto& p:parts){
            float ax=0,ay=0;
            switch(mode){
                case 0: ay=gStrength; break; // down gravity
                case 1:{ // attract to center
                    float dx=cx-p.x,dy=cy-p.y;
                    float d=std::sqrt(dx*dx+dy*dy)+1.f;
                    ax=dx/d*gStrength*0.8f;
                    ay=dy/d*gStrength*0.8f;
                    break;
                }
                case 2:{ // repel from center
                    float dx=p.x-cx,dy=p.y-cy;
                    float d=std::sqrt(dx*dx+dy*dy)+1.f;
                    ax=dx/d*gStrength*0.8f;
                    ay=dy/d*gStrength*0.8f;
                    break;
                }
                case 3: // zero-g
                    break;
            }
            // mouse attract/repel
            if(in.mouseDown){
                float dx=in.mouseX-p.x,dy=in.mouseY-p.y;
                float d=std::sqrt(dx*dx+dy*dy)+1.f;
                ax+=dx/d*gStrength*1.2f;
                ay+=dy/d*gStrength*1.2f;
            }

            p.vx+=ax*dt; p.vy+=ay*dt;
            // drag
            p.vx*=(1.f-0.4f*dt); p.vy*=(1.f-0.4f*dt);
            // cap
            float s=std::sqrt(p.vx*p.vx+p.vy*p.vy);
            float ms=sw*0.55f;
            if(s>ms){p.vx=p.vx/s*ms;p.vy=p.vy/s*ms;}

            p.x+=p.vx*dt; p.y+=p.vy*dt;

            // wall collisions
            for(auto& w:walls){
                if(p.x+p.r>w.x&&p.x-p.r<w.x+w.w&&
                   p.y+p.r>w.y&&p.y-p.r<w.y+w.h){
                    // find smallest overlap and bounce
                    float ol=p.x+p.r-w.x, or2=w.x+w.w-p.x+p.r;
                    float ot=p.y+p.r-w.y, ob=w.y+w.h-p.y+p.r;
                    float mn=std::min({ol,or2,ot,ob});
                    if(mn==ol){p.x=w.x-p.r; p.vx=-std::fabs(p.vx)*0.7f;}
                    else if(mn==or2){p.x=w.x+w.w+p.r;p.vx=std::fabs(p.vx)*0.7f;}
                    else if(mn==ot){p.y=w.y-p.r; p.vy=-std::fabs(p.vy)*0.7f;}
                    else{p.y=w.y+w.h+p.r;p.vy=std::fabs(p.vy)*0.7f;}
                }
            }
        }
    }

    void render(SDL_Renderer* r)override{
        // fade trail
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,0,0,0,35);
        SDL_FRect full{0,0,(float)sw,(float)sh};
        SDL_RenderFillRect(r,&full);

        // walls
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
        SDL_SetRenderDrawColor(r,60,70,100,255);
        for(auto& w:walls){
            SDL_FRect wr{w.x,w.y,w.w,w.h};
            SDL_RenderFillRect(r,&wr);
        }

        // particles
        for(auto& p:parts){
            SDL_SetRenderDrawColor(r,p.cr,p.cg,p.cb,255);
            int ir=(int)p.r;
            for(int dy=-ir;dy<=ir;dy++){
                int dx2=(int)std::sqrt((float)(ir*ir-dy*dy));
                SDL_RenderLine(r,(float)(p.x-dx2),(float)(p.y+dy),
                                 (float)(p.x+dx2),(float)(p.y+dy));
            }
        }

        // HUD
        int ts=std::max(2,sw/480);
        std::string modes[4]={"GRAVITY DOWN","ATTRACT CENTER","REPEL CENTER","ZERO-G"};
        Font::draw(r,"PARTICLES: "+std::to_string(parts.size()),20,20,ts,255,220,120);
        Font::draw(r,"MODE: "+modes[mode],20,20+ts*10,ts,120,255,200);
        Font::draw(r,"SPACE=ADD 80   G=GRAVITY MODE   LMB=ATTRACT   R=RESET   ESC=MENU",
                   sw/2-Font::width("SPACE=ADD 80   G=GRAVITY MODE   LMB=ATTRACT   R=RESET   ESC=MENU",1)/2,
                   sh-24,1,100,100,130);
    }
};
