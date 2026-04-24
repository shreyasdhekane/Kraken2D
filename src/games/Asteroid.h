#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Asteroid : public Scene {
    struct Rock{float x,y,vx,vy,rot,rotSpd,r; bool alive;};
    struct Bullet{float x,y,vx,vy; float life;};
    struct Particle{float x,y,vx,vy,life,maxLife; uint8_t r,g,b;};

    float shX=0,shY=0,shRot=0,shVX=0,shVY=0;
    bool  dead=false;
    int   score=0;
    float spawnT=0,gameT=0,shootT=0;
    int   sw=1920,sh=1080;

    std::vector<Rock>     rocks;
    std::vector<Bullet>   bullets;
    std::vector<Particle> particles;

    void reset(){
        shX=sw*0.5f;shY=sh*0.5f;
        shRot=-3.14f/2;shVX=0;shVY=0;
        dead=false;score=0;spawnT=0;gameT=0;shootT=0;
        rocks.clear();bullets.clear();particles.clear();
    }

    void spawnRock(float spd=0){
        int edge=rand()%4;
        float x,y;
        switch(edge){
            case 0:x=(float)(rand()%sw);y=-30;break;
            case 1:x=(float)(rand()%sw);y=(float)sh+30;break;
            case 2:x=-30;y=(float)(rand()%sh);break;
            default:x=(float)sw+30;y=(float)(rand()%sh);break;
        }
        float dx=sw*0.5f+(rand()%200-100)-x;
        float dy=sh*0.5f+(rand()%200-100)-y;
        float l=std::sqrt(dx*dx+dy*dy)+0.001f;
        float s=spd>0?spd:(80.f+rand()%120);
        float r2=(float)(14+rand()%20);
        rocks.push_back({x,y,dx/l*s,dy/l*s,0,(float)((rand()%200-100)/100.f),r2,true});
    }

    void explode(float x,float y,uint8_t r,uint8_t g,uint8_t b,int n=12){
        for(int i=0;i<n;i++){
            float ang=(float)i/n*6.28f+(rand()%100)/100.f;
            float spd=40.f+rand()%120;
            particles.push_back({x,y,
                std::cos(ang)*spd,std::sin(ang)*spd,
                0.6f+rand()%100/200.f, 0.8f, r,g,b});
        }
    }

public:
    void onEnter()override{reset();}

    void update(float dt,const Input& in)override{
        sw=in.screenW;sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead){if(in.keyJustPressed[SDL_SCANCODE_RETURN])reset();return;}

        gameT+=dt;
        float spawnInterval=std::max(0.25f,1.4f-gameT*0.018f);
        spawnT-=dt;
        if(spawnT<=0){spawnRock();spawnT=spawnInterval;}

        // ship controls
        if(in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A]) shRot-=3.8f*dt;
        if(in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D]) shRot+=3.8f*dt;
        if(in.keys[SDL_SCANCODE_UP]   ||in.keys[SDL_SCANCODE_W]){
            shVX+=std::cos(shRot)*sw*0.28f*dt;
            shVY+=std::sin(shRot)*sw*0.28f*dt;
        }
        // drag
        shVX*=(1.f-0.65f*dt); shVY*=(1.f-0.65f*dt);
        float ms=sw*0.38f;
        float sv=std::sqrt(shVX*shVX+shVY*shVY);
        if(sv>ms){shVX=shVX/sv*ms;shVY=shVY/sv*ms;}

        // shoot
        shootT-=dt;
        if(in.keys[SDL_SCANCODE_SPACE]&&shootT<=0){
            float spd=sw*0.65f;
            bullets.push_back({shX+std::cos(shRot)*sw*0.018f,
                                shY+std::sin(shRot)*sw*0.018f,
                                std::cos(shRot)*spd,std::sin(shRot)*spd,1.4f});
            shootT=0.16f;
        }

        // move ship + wrap
        shX+=shVX*dt; shY+=shVY*dt;
        if(shX<0){shX+=(float)sw;} if(shX>(float)sw){shX-=(float)sw;}
        if(shY<0){shY+=(float)sh;} if(shY>(float)sh){shY-=(float)sh;}

        // move rocks + wrap
        for(auto& rock:rocks){
            rock.x+=rock.vx*dt; rock.y+=rock.vy*dt;
            rock.rot+=rock.rotSpd*dt;
            if(rock.x<-80)rock.x+=(float)sw+160;
            if(rock.x>sw+80)rock.x-=(float)sw+160;
            if(rock.y<-80)rock.y+=(float)sh+160;
            if(rock.y>sh+80)rock.y-=(float)sh+160;
        }

        // move bullets
        for(auto& b:bullets){b.x+=b.vx*dt;b.y+=b.vy*dt;b.life-=dt;}
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),
            [&](auto& b){return b.life<=0||b.x<-50||b.x>sw+50||b.y<-50||b.y>sh+50;}),
            bullets.end());

        // bullet vs rock
        for(auto& b:bullets){
            for(auto& rock:rocks){
                if(!rock.alive)continue;
                float dx=b.x-rock.x,dy=b.y-rock.y;
                if(dx*dx+dy*dy<rock.r*rock.r){
                    explode(rock.x,rock.y,200,120,60);
                    rock.alive=false; b.life=0; score+=100;
                    // split big rocks
                    if(rock.r>sw*0.015f){
                        for(int i=0;i<2;i++){
                            float spd=rock.r*3.f;
                            rocks.push_back({rock.x,rock.y,
                                (float)((rand()%200-100)/100.f*spd),
                                (float)((rand()%200-100)/100.f*spd),
                                0,(float)((rand()%200-100)/100.f),
                                rock.r*0.55f,true});
                        }
                    }
                    break;
                }
            }
        }
        rocks.erase(std::remove_if(rocks.begin(),rocks.end(),[](auto& r){return !r.alive;}),rocks.end());

        // ship vs rock
        for(auto& rock:rocks){
            float dx=rock.x-shX,dy=rock.y-shY;
            float minDist=rock.r+sw*0.014f;
            if(dx*dx+dy*dy<minDist*minDist){
                explode(shX,shY,120,220,255,20);
                dead=true;
            }
        }

        // particles
        for(auto& p:particles){
            p.x+=p.vx*dt;p.y+=p.vy*dt;
            p.vx*=(1.f-dt); p.vy*=(1.f-dt);
            p.life-=dt;
        }
        particles.erase(std::remove_if(particles.begin(),particles.end(),
            [](auto& p){return p.life<=0;}),particles.end());
    }

    // draw filled poly for ship and rocks
    void drawShip(SDL_Renderer* r,float x,float y,float rot,float sz){
        auto pt=[&](float px,float py,float& ox,float& oy){
            ox=x+px*std::cos(rot)-py*std::sin(rot);
            oy=y+px*std::sin(rot)+py*std::cos(rot);
        };
        float x0,y0,x1,y1,x2,y2,x3,y3;
        pt(sz,0,x0,y0);
        pt(-sz*0.6f,-sz*0.6f,x1,y1);
        pt(-sz*0.3f,0,x2,y2);
        pt(-sz*0.6f,sz*0.6f,x3,y3);
        SDL_RenderLine(r,x0,y0,x1,y1);
        SDL_RenderLine(r,x1,y1,x2,y2);
        SDL_RenderLine(r,x2,y2,x3,y3);
        SDL_RenderLine(r,x3,y3,x0,y0);
    }

    void drawRock(SDL_Renderer* r,float x,float y,float rot,float radius){
        const int SIDES=8;
        float verts[SIDES][2];
        for(int i=0;i<SIDES;i++){
            float ang=rot+(float)i/SIDES*6.28f;
            float jit=radius*(0.75f+0.25f*((i*137)%100)/100.f);
            verts[i][0]=x+std::cos(ang)*jit;
            verts[i][1]=y+std::sin(ang)*jit;
        }
        for(int i=0;i<SIDES;i++){
            int j=(i+1)%SIDES;
            SDL_RenderLine(r,verts[i][0],verts[i][1],verts[j][0],verts[j][1]);
        }
    }

    void render(SDL_Renderer* r)override{
        SDL_SetRenderDrawColor(r,4,4,12,255);
        SDL_RenderClear(r);

        // stars
        SDL_SetRenderDrawColor(r,160,160,200,255);
        for(int i=0;i<300;i++){
            int sx=(i*53)%sw,sy=(i*71)%sh;
            SDL_FRect p{(float)sx,(float)sy,2.f,2.f};
            SDL_RenderFillRect(r,&p);
        }

        // particles
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        for(auto& p:particles){
            float frac=p.life/p.maxLife;
            SDL_SetRenderDrawColor(r,p.r,p.g,p.b,(uint8_t)(frac*220));
            SDL_FRect pf{p.x-2,p.y-2,4,4};
            SDL_RenderFillRect(r,&pf);
        }
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);

        // rocks
        SDL_SetRenderDrawColor(r,180,120,80,255);
        for(auto& rock:rocks) drawRock(r,rock.x,rock.y,rock.rot,rock.r);

        // bullets
        SDL_SetRenderDrawColor(r,255,255,120,255);
        for(auto& b:bullets){
            SDL_FRect bf{b.x-3,b.y-3,6,6};
            SDL_RenderFillRect(r,&bf);
        }

        // ship
        if(!dead){
            SDL_SetRenderDrawColor(r,120,220,255,255);
            drawShip(r,shX,shY,shRot,sw*0.018f);
            // thrust flame
            if(SDL_GetTicks()%200<120){
                SDL_SetRenderDrawColor(r,255,160,60,200);
                float fx=shX-std::cos(shRot)*sw*0.025f;
                float fy=shY-std::sin(shRot)*sw*0.025f;
                SDL_FRect fl{fx-4,fy-4,8,8};
                SDL_RenderFillRect(r,&fl);
            }
        }

        // HUD
        int ts=std::max(2,sw/480);
        Font::draw(r,"SCORE: "+std::to_string(score),20,20,ts,120,255,255);
        Font::draw(r,"TIME: "+std::to_string((int)gameT)+"s",20,20+ts*10,ts,255,220,120);
        Font::draw(r,"ROCKS: "+std::to_string(rocks.size()),20,20+ts*20,ts,200,140,80);
        std::string h="WASD/ARROWS ROTATE+THRUST   SPACE SHOOT   ESC MENU";
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
            Font::draw(r,"SHIP DESTROYED",sw/2-Font::width("SHIP DESTROYED",gs)/2,sh/2-60,gs,255,80,80);
            std::string s2="SCORE "+std::to_string(score)+"   ENTER=RETRY";
            Font::draw(r,s2,sw/2-Font::width(s2,2)/2,sh/2+10,2,220,220,220);
        }
    }
};
