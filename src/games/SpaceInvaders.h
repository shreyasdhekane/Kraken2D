#pragma once
#include "../Engine.h"
#include "../Audio.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class SpaceInvaders : public Scene {
    struct Alien  { float x,y; bool alive; int type; };
    struct Bullet { float x,y,vy; bool alien; };
    struct Shield { float x,y,w,h; int hp; };
    struct Particle { float x,y,vx,vy,life; uint8_t r,g,b; };

    float pX=0, pW=0, pH=0;
    std::vector<Alien>    aliens;
    std::vector<Bullet>   bullets;
    std::vector<Shield>   shields;
    std::vector<Particle> particles;

    float alienDirX=1, alienStepY=0;
    float alienMoveTimer=0, alienMoveInterval=0.5f;
    float alienShootTimer=0;
    int   score=0, lives=3, wave=1;
    bool  dead=false, won=false;
    float shootCooldown=0;
    int   sw=1920, sh=1080;

    void buildWave() {
        aliens.clear(); shields.clear(); bullets.clear(); particles.clear();
        alienDirX=1; alienMoveInterval=std::max(0.08f, 0.5f-wave*0.06f);

        int cols=11, rows=5;
        float aw=sw*0.055f, ah=sh*0.055f;
        float gx=(sw-(cols*aw+(cols-1)*sw*0.01f))*0.5f;
        float gy=sh*0.12f;
        for(int r=0;r<rows;r++)
            for(int c=0;c<cols;c++)
                aliens.push_back({gx+c*(aw+sw*0.01f), gy+r*(ah+sh*0.012f), true, r<1?2:r<3?1:0});

        // 4 shields
        for(int i=0;i<4;i++) {
            float sx=sw*0.12f+i*(sw*0.22f);
            shields.push_back({sx, sh*0.72f, sw*0.1f, sh*0.06f, 8});
        }
    }

    void explode(float x,float y,uint8_t r,uint8_t g,uint8_t b){
        for(int i=0;i<10;i++){
            float ang=(float)i/10*6.28f;
            float spd=60+rand()%120;
            particles.push_back({x,y,std::cos(ang)*spd,std::sin(ang)*spd,0.5f,r,g,b});
        }
    }

public:
    void onEnter() override {
        score=0; lives=3; wave=1; dead=false; won=false;
        pX=0; shootCooldown=0;
        buildWave();
    }

    void update(float dt, const Input& in) override {
        sw=in.screenW; sh=in.screenH;
        pW=sw*0.08f; pH=sh*0.04f;
        if(pX==0) pX=sw*0.5f-pW*0.5f;

        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead||won){if(in.keyJustPressed[SDL_SCANCODE_RETURN])onEnter();return;}

        // player move
        float ps=sw*0.55f;
        if(in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A]) pX-=ps*dt;
        if(in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D]) pX+=ps*dt;
        pX=std::clamp(pX,0.f,(float)sw-pW);

        // player shoot
        shootCooldown-=dt;
        if((in.keys[SDL_SCANCODE_SPACE]||in.keys[SDL_SCANCODE_UP])&&shootCooldown<=0){
            bullets.push_back({pX+pW*0.5f, sh*0.84f-pH, -sh*1.1f, false});
            shootCooldown=0.28f;
            Audio::get().sfxShoot();
        }

        // alien movement
        alienMoveTimer+=dt;
        if(alienMoveTimer>=alienMoveInterval){
            alienMoveTimer=0;
            float step=sw*0.018f;
            bool hitEdge=false;
            for(auto& a:aliens){
                if(!a.alive)continue;
                a.x+=alienDirX*step;
                if(a.x<sw*0.02f||a.x+sw*0.055f>sw*0.98f) hitEdge=true;
            }
            if(hitEdge){
                alienDirX=-alienDirX;
                for(auto& a:aliens) if(a.alive) a.y+=sh*0.04f;
                alienMoveInterval=std::max(0.05f,alienMoveInterval*0.96f);
            }
        }

        // alien shoot
        alienShootTimer-=dt;
        if(alienShootTimer<=0){
            std::vector<int> alive;
            for(int i=0;i<(int)aliens.size();i++) if(aliens[i].alive) alive.push_back(i);
            if(!alive.empty()){
                auto& a=aliens[alive[rand()%alive.size()]];
                bullets.push_back({a.x+sw*0.027f, a.y+sh*0.055f, sh*0.55f, true});
            }
            alienShootTimer=std::max(0.3f,1.8f-wave*0.15f);
        }

        // move bullets
        for(auto& b:bullets){b.y+=b.vy*dt;}
        bullets.erase(std::remove_if(bullets.begin(),bullets.end(),
            [&](auto& b){return b.y<-20||b.y>sh+20;}),bullets.end());

        // bullet vs aliens
        float aw=sw*0.055f,ah=sh*0.055f;
        for(auto& b:bullets){
            if(b.alien)continue;
            for(auto& a:aliens){
                if(!a.alive)continue;
                if(b.x>a.x&&b.x<a.x+aw&&b.y>a.y&&b.y<a.y+ah){
                    a.alive=false; b.y=-100;
                    int pts=a.type==2?30:a.type==1?20:10;
                    score+=pts;
                    Audio::get().sfxExplode();
                    uint8_t cr=a.type==2?255:a.type==1?120:80;
                    uint8_t cg=a.type==2?80:a.type==1?255:200;
                    explode(a.x+aw*0.5f,a.y+ah*0.5f,cr,cg,80);
                }
            }
        }

        // bullet vs shields
        for(auto& b:bullets){
            for(auto& sh2:shields){
                if(sh2.hp<=0)continue;
                if(b.x>sh2.x&&b.x<sh2.x+sh2.w&&b.y>sh2.y&&b.y<sh2.y+sh2.h){
                    sh2.hp--; b.y=-100;
                }
            }
        }

        // alien bullet vs player
        float py=sh*0.84f-pH;
        for(auto& b:bullets){
            if(!b.alien)continue;
            if(b.x>pX&&b.x<pX+pW&&b.y>py&&b.y<py+pH){
                lives--; b.y=-100; explode(pX+pW*0.5f,py,120,220,255);
                Audio::get().sfxHit();
                if(lives<=0){dead=true;Audio::get().sfxDie();}
            }
        }

        // aliens reach player row
        for(auto& a:aliens){
            if(a.alive&&a.y+sh*0.055f>py){dead=true;break;}
        }

        // wave clear
        bool anyAlive=false;
        for(auto& a:aliens) if(a.alive){anyAlive=true;break;}
        if(!anyAlive){wave++;buildWave();Audio::get().sfxLevelUp();}

        // particles
        for(auto& p:particles){p.x+=p.vx*dt;p.y+=p.vy*dt;p.life-=dt;}
        particles.erase(std::remove_if(particles.begin(),particles.end(),
            [](auto& p){return p.life<=0;}),particles.end());
    }

    void drawAlien(SDL_Renderer* r,float x,float y,float w,float h,int type,Uint64 t){
        bool frame=(t/300)%2;
        switch(type){
            case 2: { // top alien
                SDL_SetRenderDrawColor(r,255,100,100,255);
                SDL_FRect top2{x+w*0.25f,y,w*0.5f,h*0.35f}; SDL_RenderFillRect(r,&top2);
                SDL_FRect mid2{x,y+h*0.35f,w,h*0.35f};       SDL_RenderFillRect(r,&mid2);
                SDL_FRect la{x+w*0.1f,y+h*0.7f,w*0.15f,h*0.3f}; SDL_RenderFillRect(r,&la);
                SDL_FRect lb{x+w*0.4f,y+h*0.7f,w*0.15f,h*0.3f}; SDL_RenderFillRect(r,&lb);
                SDL_FRect lc{x+w*0.75f,y+h*0.7f,w*0.15f,h*0.3f};SDL_RenderFillRect(r,&lc);
                break; }
            case 1: { // mid alien
                SDL_SetRenderDrawColor(r,120,255,160,255);
                SDL_FRect body{x+w*0.1f,y+h*0.2f,w*0.8f,h*0.6f}; SDL_RenderFillRect(r,&body);
                SDL_FRect head{x+w*0.3f,y,w*0.4f,h*0.25f};         SDL_RenderFillRect(r,&head);
                SDL_FRect aa{x+(frame?0.f:w*0.05f),y+h*0.3f,w*0.15f,h*0.25f}; SDL_RenderFillRect(r,&aa);
                SDL_FRect ab{x+w*0.85f-(frame?0.f:w*0.05f),y+h*0.3f,w*0.15f,h*0.25f}; SDL_RenderFillRect(r,&ab);
                break; }
            default: { // crab
                SDL_SetRenderDrawColor(r,100,180,255,255);
                SDL_FRect body{x+w*0.1f,y+h*0.25f,w*0.8f,h*0.5f}; SDL_RenderFillRect(r,&body);
                SDL_FRect head{x+w*0.25f,y,w*0.5f,h*0.3f};          SDL_RenderFillRect(r,&head);
                float lo=frame?h*0.15f:0.f;
                SDL_FRect la{x,y+h*0.5f+lo,w*0.12f,h*0.35f};           SDL_RenderFillRect(r,&la);
                SDL_FRect lb{x+w*0.88f,y+h*0.5f+lo,w*0.12f,h*0.35f};   SDL_RenderFillRect(r,&lb);
                SDL_FRect lc{x,y+h*0.3f-lo,w*0.12f,h*0.25f};           SDL_RenderFillRect(r,&lc);
                SDL_FRect ld{x+w*0.88f,y+h*0.3f-lo,w*0.12f,h*0.25f};   SDL_RenderFillRect(r,&ld);
                break; }
        }
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r,4,4,14,255);
        SDL_RenderClear(r);

        // stars
        SDL_SetRenderDrawColor(r,140,140,180,255);
        for(int i=0;i<200;i++){
            int sx=(i*73)%sw, sy=(i*97)%sh;
            SDL_FRect p{(float)sx,(float)sy,2,2};
            SDL_RenderFillRect(r,&p);
        }

        Uint64 t=SDL_GetTicks();
        float aw=sw*0.055f,ah=sh*0.055f;

        // shields
        for(auto& s:shields){
            if(s.hp<=0)continue;
            float alpha=(float)s.hp/8;
            SDL_SetRenderDrawColor(r,(uint8_t)(60*alpha),(uint8_t)(200*alpha),(uint8_t)(80*alpha),255);
            SDL_FRect sr{s.x,s.y,s.w,s.h};
            SDL_RenderFillRect(r,&sr);
        }

        // aliens
        for(auto& a:aliens){
            if(!a.alive)continue;
            drawAlien(r,a.x,a.y,aw,ah,a.type,t);
        }

        // bullets
        for(auto& b:bullets){
            if(b.alien){
                SDL_SetRenderDrawColor(r,255,80,80,255);
                SDL_FRect bf{b.x-2,b.y,5,sh*0.02f};
                SDL_RenderFillRect(r,&bf);
            } else {
                SDL_SetRenderDrawColor(r,120,220,255,255);
                SDL_FRect bf{b.x-2,b.y,4,sh*0.025f};
                SDL_RenderFillRect(r,&bf);
            }
        }

        // player ship
        float py=sh*0.84f-pH;
        SDL_SetRenderDrawColor(r,120,220,255,255);
        SDL_FRect ship{pX,py+pH*0.4f,pW,pH*0.6f};
        SDL_RenderFillRect(r,&ship);
        SDL_FRect cannon{pX+pW*0.43f,py,pW*0.14f,pH*0.5f};
        SDL_RenderFillRect(r,&cannon);
        SDL_FRect dome{pX+pW*0.2f,py+pH*0.2f,pW*0.6f,pH*0.4f};
        SDL_RenderFillRect(r,&dome);

        // ground line
        SDL_SetRenderDrawColor(r,80,180,80,255);
        SDL_RenderLine(r,0,(float)(sh*0.88f),(float)sw,(float)(sh*0.88f));

        // lives as hearts
        Font::drawHearts(r,lives,3,20,(int)(sh*0.91f),(int)(sh*0.014f)+1);

        // particles
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        for(auto& p:particles){
            SDL_SetRenderDrawColor(r,p.r,p.g,p.b,(uint8_t)(p.life*255));
            SDL_FRect pf{p.x-2,p.y-2,5,5};
            SDL_RenderFillRect(r,&pf);
        }
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);

        // HUD
        int ts=std::max(2,sw/480);
        Font::draw(r,"SCORE: "+std::to_string(score),20,20,ts,255,220,120);
        Font::draw(r,"WAVE: "+std::to_string(wave),sw-Font::width("WAVE: "+std::to_string(wave),ts)-20,20,ts,255,160,80);

        std::string h="A/D MOVE   SPACE SHOOT   ESC MENU";
        Font::draw(r,h,sw/2-Font::width(h,1)/2,sh-20,1,100,100,130);

        if(dead){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,180);
            SDL_FRect ov{(float)(sw/2-260),(float)(sh/2-80),520,160};
            SDL_RenderFillRect(r,&ov);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r,255,80,80,255);
            SDL_RenderRect(r,&ov);
            int gs=std::max(3,sw/320);
            Font::draw(r,"GAME OVER",sw/2-Font::width("GAME OVER",gs)/2,sh/2-60,gs,255,80,80);
            std::string s="SCORE "+std::to_string(score)+"   ENTER=RETRY";
            Font::draw(r,s,sw/2-Font::width(s,2)/2,sh/2+10,2,220,220,220);
        }
    }
};
