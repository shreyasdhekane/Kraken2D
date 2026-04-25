#pragma once
#include "../Engine.h"
#include "../Audio.h"
#include "../MiniFont.h"
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

class Tetris : public Scene {
    static constexpr int COLS = 10;
    static constexpr int ROWS = 20;

    // 7 tetrominoes, each 4 rotations, each rotation is 4 {row,col} offsets
    static constexpr int PIECES[7][4][4][2] = {
        // I
        {{{0,0},{0,1},{0,2},{0,3}},{{0,2},{1,2},{2,2},{3,2}},
         {{3,0},{3,1},{3,2},{3,3}},{{0,1},{1,1},{2,1},{3,1}}},
        // O
        {{{0,0},{0,1},{1,0},{1,1}},{{0,0},{0,1},{1,0},{1,1}},
         {{0,0},{0,1},{1,0},{1,1}},{{0,0},{0,1},{1,0},{1,1}}},
        // T
        {{{0,1},{1,0},{1,1},{1,2}},{{0,1},{1,1},{1,2},{2,1}},
         {{1,0},{1,1},{1,2},{2,1}},{{0,1},{1,0},{1,1},{2,1}}},
        // S
        {{{0,1},{0,2},{1,0},{1,1}},{{0,1},{1,1},{1,2},{2,2}},
         {{1,1},{1,2},{2,0},{2,1}},{{0,0},{1,0},{1,1},{2,1}}},
        // Z
        {{{0,0},{0,1},{1,1},{1,2}},{{0,2},{1,1},{1,2},{2,1}},
         {{1,0},{1,1},{2,1},{2,2}},{{0,1},{1,0},{1,1},{2,0}}},
        // J
        {{{0,0},{1,0},{1,1},{1,2}},{{0,1},{0,2},{1,1},{2,1}},
         {{1,0},{1,1},{1,2},{2,2}},{{0,1},{1,1},{2,0},{2,1}}},
        // L
        {{{0,2},{1,0},{1,1},{1,2}},{{0,1},{1,1},{2,1},{2,2}},
         {{1,0},{1,1},{1,2},{2,0}},{{0,0},{0,1},{1,1},{2,1}}}
    };

    // Colors per piece
    uint8_t COLORS[7][3] = {
        {80, 220, 255},  // I - cyan
        {255,220,  0},  // O - yellow
        {180,  0, 255},  // T - purple
        {0,  200,  80},  // S - green
        {255,  60,  60},  // Z - red
        {  0, 100, 255},  // J - blue
        {255, 160,   0},  // L - orange
    };

    // Board: -1 = empty, 0-6 = piece color index
    int board[ROWS][COLS];

    // Current piece
    int pieceType=0, pieceRot=0, pieceRow=0, pieceCol=0;
    int nextType=0;

    // Timing
    float dropTimer=0, dropInterval=0.5f;
    float lockTimer=0;
    float dasTimer=0, dasDelay=0.15f, dasRepeat=0.05f;
    bool  dasLeft=false, dasRight=false;
    int   dasRepeatTimer=0;

    // Input edge detection helpers
    bool prevLeft=false, prevRight=false, prevDown=false;
    bool prevUp=false, prevZ=false;

    // State
    int  score=0, lines=0, level=1;
    bool dead=false;
    int  sw=1920, sh=1080;

    // Cell size derived from screen
    float cellW() const { return sh * 0.042f; }
    float cellH() const { return sh * 0.042f; }
    float boardX() const { return sw*0.5f - COLS*cellW()*0.5f; }
    float boardY() const { return sh*0.05f; }

    void clearBoard() {
        for(int r=0;r<ROWS;r++)
            for(int c=0;c<COLS;c++)
                board[r][c]=-1;
    }

    bool validPos(int type,int rot,int row,int col) {
        for(int i=0;i<4;i++){
            int r=row+PIECES[type][rot][i][0];
            int c=col+PIECES[type][rot][i][1];
            if(r<0||r>=ROWS||c<0||c>=COLS) return false;
            if(board[r][c]!=-1) return false;
        }
        return true;
    }

    void spawnPiece() {
        pieceType=nextType;
        nextType=rand()%7;
        pieceRot=0;
        pieceRow=0;
        pieceCol=COLS/2-2;
        if(!validPos(pieceType,pieceRot,pieceRow,pieceCol)){dead=true;Audio::get().sfxDie();}
    }

    void lockPiece() {
        for(int i=0;i<4;i++){
            int r=pieceRow+PIECES[pieceType][pieceRot][i][0];
            int c=pieceCol+PIECES[pieceType][pieceRot][i][1];
            if(r>=0&&r<ROWS&&c>=0&&c<COLS)
                board[r][c]=pieceType;
        }
        clearLines();
        spawnPiece();
        lockTimer=0;
    }

    void clearLines() {
        int cleared=0;
        for(int r=ROWS-1;r>=0;r--){
            bool full=true;
            for(int c=0;c<COLS;c++) if(board[r][c]==-1){full=false;break;}
            if(full){
                // shift everything down
                for(int rr=r;rr>0;rr--)
                    for(int c=0;c<COLS;c++)
                        board[rr][c]=board[rr-1][c];
                for(int c=0;c<COLS;c++) board[0][c]=-1;
                cleared++; r++; // recheck same row
            }
        }
        if(cleared>0){
            int pts[5]={0,100,300,500,800};
            score+=pts[std::min(cleared,4)]*level;
            lines+=cleared;
            level=lines/10+1;
            dropInterval=std::max(0.05f,0.5f-(level-1)*0.045f);
            Audio::get().sfxLineClear();
        }
    }

    // Ghost piece row
    int ghostRow() {
        int gr=pieceRow;
        while(validPos(pieceType,pieceRot,gr+1,pieceCol)) gr++;
        return gr;
    }

    void reset() {
        clearBoard();
        score=0; lines=0; level=1;
        dead=false; dropTimer=0; lockTimer=0;
        dropInterval=0.5f;
        nextType=rand()%7;
        spawnPiece();
    }

public:
    void onEnter() override { reset(); }

    void update(float dt, const Input& in) override {
        sw=in.screenW; sh=in.screenH;
        if(in.keyJustPressed[SDL_SCANCODE_ESCAPE]){nextScene="menu";return;}
        if(dead){if(in.keyJustPressed[SDL_SCANCODE_RETURN])reset();return;}

        bool kLeft  = in.keys[SDL_SCANCODE_LEFT] ||in.keys[SDL_SCANCODE_A];
        bool kRight = in.keys[SDL_SCANCODE_RIGHT]||in.keys[SDL_SCANCODE_D];
        bool kDown  = in.keys[SDL_SCANCODE_DOWN] ||in.keys[SDL_SCANCODE_S];

        // Rotate
        if(in.keyJustPressed[SDL_SCANCODE_UP]||in.keyJustPressed[SDL_SCANCODE_W]||
           in.keyJustPressed[SDL_SCANCODE_X]){
            int nr=(pieceRot+1)%4;
            // wall kick attempts
            for(int kick : {0,1,-1,2,-2}){
                if(validPos(pieceType,nr,pieceRow,pieceCol+kick)){
                    pieceRot=nr; pieceCol+=kick; lockTimer=0;
                    Audio::get().sfxWall(); break;
                }
            }
        }
        if(in.keyJustPressed[SDL_SCANCODE_Z]){
            int nr=(pieceRot+3)%4;
            for(int kick : {0,1,-1,2,-2}){
                if(validPos(pieceType,nr,pieceRow,pieceCol+kick)){
                    pieceRot=nr; pieceCol+=kick; lockTimer=0; break;
                }
            }
        }

        // Hard drop
        if(in.keyJustPressed[SDL_SCANCODE_SPACE]){
            int gr=ghostRow();
            score+=2*(gr-pieceRow);
            pieceRow=gr;
            Audio::get().sfxHit();
            lockPiece();
            return;
        }

        // DAS (delayed auto-shift) for left/right
        auto doShift = [&](int dir){
            if(validPos(pieceType,pieceRot,pieceRow,pieceCol+dir)){
                pieceCol+=dir; lockTimer=0;
            }
        };

        if(kLeft&&!prevLeft)  { doShift(-1); dasTimer=0; dasLeft=true;  dasRight=false; }
        if(kRight&&!prevRight){ doShift( 1); dasTimer=0; dasRight=true; dasLeft=false;  }
        if(!kLeft)  dasLeft=false;
        if(!kRight) dasRight=false;

        if(dasLeft||dasRight){
            dasTimer+=dt;
            if(dasTimer>=dasDelay){
                dasTimer-=dasRepeat;
                doShift(dasLeft?-1:1);
            }
        }

        prevLeft=kLeft; prevRight=kRight;

        // Soft drop
        float interval = kDown ? std::min(0.05f,dropInterval) : dropInterval;
        dropTimer+=dt;
        if(dropTimer>=interval){
            dropTimer=0;
            if(validPos(pieceType,pieceRot,pieceRow+1,pieceCol)){
                pieceRow++;
                if(kDown) score++;
                lockTimer=0;
            } else {
                // piece landed — start lock delay
                lockTimer+=interval;
                if(lockTimer>=0.5f) lockPiece();
            }
        }
    }

    void drawCell(SDL_Renderer* r, int row, int col, int colorIdx,
                  float alpha=1.f) {
        float cw=cellW(), ch=cellH();
        float x=boardX()+col*cw, y=boardY()+row*ch;
        uint8_t R=COLORS[colorIdx][0];
        uint8_t G=COLORS[colorIdx][1];
        uint8_t B=COLORS[colorIdx][2];
        uint8_t a=(uint8_t)(255*alpha);

        // Main fill
        SDL_SetRenderDrawColor(r,R,G,B,a);
        SDL_FRect cell{x+1,y+1,cw-2,ch-2};
        SDL_RenderFillRect(r,&cell);

        // Top/left highlight
        SDL_SetRenderDrawColor(r,
            (uint8_t)std::min(255,R+80),
            (uint8_t)std::min(255,G+80),
            (uint8_t)std::min(255,B+80),a);
        SDL_FRect hl{x+1,y+1,cw-2,3};
        SDL_RenderFillRect(r,&hl);
        SDL_FRect hl2{x+1,y+1,3,ch-2};
        SDL_RenderFillRect(r,&hl2);

        // Bottom/right shadow
        SDL_SetRenderDrawColor(r,
            (uint8_t)(R*0.5f),(uint8_t)(G*0.5f),(uint8_t)(B*0.5f),a);
        SDL_FRect sh2{x+1,y+ch-4,cw-2,3};
        SDL_RenderFillRect(r,&sh2);
        SDL_FRect sh3{x+cw-4,y+1,3,ch-2};
        SDL_RenderFillRect(r,&sh3);
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r,10,10,22,255);
        SDL_RenderClear(r);

        float cw=cellW(), ch=cellH();
        float bx=boardX(), by=boardY();

        // Board background
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,30,30,50,200);
        SDL_FRect bg{bx,by,COLS*cw,ROWS*ch};
        SDL_RenderFillRect(r,&bg);
        SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);

        // Grid lines
        SDL_SetRenderDrawColor(r,40,40,65,255);
        for(int c=0;c<=COLS;c++)
            SDL_RenderLine(r,bx+c*cw,by,bx+c*cw,by+ROWS*ch);
        for(int rr=0;rr<=ROWS;rr++)
            SDL_RenderLine(r,bx,by+rr*ch,bx+COLS*cw,by+rr*ch);

        // Board border
        SDL_SetRenderDrawColor(r,120,120,180,255);
        SDL_FRect border{bx-2,by-2,COLS*cw+4,ROWS*ch+4};
        SDL_RenderRect(r,&border);

        // Locked cells
        for(int rr=0;rr<ROWS;rr++)
            for(int c=0;c<COLS;c++)
                if(board[rr][c]!=-1)
                    drawCell(r,rr,c,board[rr][c]);

        // Ghost piece
        int gr=ghostRow();
        if(gr!=pieceRow){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            for(int i=0;i<4;i++){
                int rr=gr+PIECES[pieceType][pieceRot][i][0];
                int c=pieceCol+PIECES[pieceType][pieceRot][i][1];
                if(rr>=0&&rr<ROWS&&c>=0&&c<COLS)
                    drawCell(r,rr,c,pieceType,0.25f);
            }
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
        }

        // Active piece
        for(int i=0;i<4;i++){
            int rr=pieceRow+PIECES[pieceType][pieceRot][i][0];
            int c=pieceCol+PIECES[pieceType][pieceRot][i][1];
            if(rr>=0&&rr<ROWS&&c>=0&&c<COLS)
                drawCell(r,rr,c,pieceType);
        }

        // ── Side panel ──
        float px=bx+COLS*cw+cw*1.5f;
        float py=by;
        int ts=std::max(2,sw/520);

        Font::draw(r,"NEXT",  (int)px,(int)py,ts,180,180,220);
        // Next piece preview box
        SDL_SetRenderDrawColor(r,30,30,50,255);
        SDL_FRect npBox{px,py+ts*10.f,cw*5,ch*4};
        SDL_RenderFillRect(r,&npBox);
        SDL_SetRenderDrawColor(r,80,80,120,255);
        SDL_RenderRect(r,&npBox);

        for(int i=0;i<4;i++){
            int nr=PIECES[nextType][0][i][0];
            int nc=PIECES[nextType][0][i][1];
            float nx2=px+nc*cw+cw*0.5f;
            float ny2=py+ts*10.f+nr*ch+ch*0.3f;
            SDL_SetRenderDrawColor(r,COLORS[nextType][0],COLORS[nextType][1],COLORS[nextType][2],255);
            SDL_FRect nc2{nx2,ny2,cw-2,ch-2};
            SDL_RenderFillRect(r,&nc2);
        }

        float sy=py+ch*7;
        Font::draw(r,"SCORE",(int)px,(int)sy,ts,180,180,220);
        Font::draw(r,std::to_string(score),(int)px,(int)(sy+ts*9),ts,255,220,100);
        Font::draw(r,"LINES",(int)px,(int)(sy+ts*20),ts,180,180,220);
        Font::draw(r,std::to_string(lines),(int)px,(int)(sy+ts*29),ts,120,220,255);
        Font::draw(r,"LEVEL",(int)px,(int)(sy+ts*40),ts,180,180,220);
        Font::draw(r,std::to_string(level),(int)px,(int)(sy+ts*49),ts,120,255,160);

        // Controls
        float cpx=bx-cw*8;
        Font::draw(r,"CONTROLS",(int)cpx,(int)by,ts,140,140,180);
        int cts=std::max(1,sw/700);
        Font::draw(r,"LEFT/RIGHT MOVE",(int)cpx,(int)(by+ts*12),cts,180,180,200);
        Font::draw(r,"UP/X  ROTATE CW",(int)cpx,(int)(by+ts*22),cts,180,180,200);
        Font::draw(r,"Z     ROTATE CCW",(int)cpx,(int)(by+ts*32),cts,180,180,200);
        Font::draw(r,"DOWN  SOFT DROP", (int)cpx,(int)(by+ts*42),cts,180,180,200);
        Font::draw(r,"SPACE HARD DROP", (int)cpx,(int)(by+ts*52),cts,180,180,200);
        Font::draw(r,"ESC   MENU",      (int)cpx,(int)(by+ts*62),cts,180,180,200);

        if(dead){
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r,0,0,0,190);
            SDL_FRect ov{bx,by,COLS*cw,ROWS*ch};
            SDL_RenderFillRect(r,&ov);
            SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r,255,80,80,255);
            SDL_RenderRect(r,&ov);
            int gs=std::max(3,sw/320);
            Font::draw(r,"GAME OVER",
                (int)(bx+COLS*cw*0.5f-Font::width("GAME OVER",gs)*0.5f),
                (int)(by+ROWS*ch*0.4f),gs,255,80,80);
            std::string s="ENTER TO RETRY";
            Font::draw(r,s,
                (int)(bx+COLS*cw*0.5f-Font::width(s,2)*0.5f),
                (int)(by+ROWS*ch*0.4f+gs*12),2,220,220,220);
        }
    }
};
