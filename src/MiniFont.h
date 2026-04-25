#pragma once
#include <SDL3/SDL.h>
#include <string>

namespace Font {

inline const uint8_t* glyph(char c) {
    static const uint8_t e[7]={0,0,0,0,0,0,0};
    static const uint8_t A[7]={0x0E,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const uint8_t B[7]={0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E};
    static const uint8_t C[7]={0x0E,0x11,0x10,0x10,0x10,0x11,0x0E};
    static const uint8_t D[7]={0x1E,0x11,0x11,0x11,0x11,0x11,0x1E};
    static const uint8_t E[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F};
    static const uint8_t F[7]={0x1F,0x10,0x10,0x1E,0x10,0x10,0x10};
    static const uint8_t G[7]={0x0E,0x11,0x10,0x17,0x11,0x11,0x0F};
    static const uint8_t H[7]={0x11,0x11,0x11,0x1F,0x11,0x11,0x11};
    static const uint8_t I[7]={0x0E,0x04,0x04,0x04,0x04,0x04,0x0E};
    static const uint8_t J[7]={0x07,0x02,0x02,0x02,0x02,0x12,0x0C};
    static const uint8_t K[7]={0x11,0x12,0x14,0x18,0x14,0x12,0x11};
    static const uint8_t L[7]={0x10,0x10,0x10,0x10,0x10,0x10,0x1F};
    static const uint8_t M[7]={0x11,0x1B,0x15,0x15,0x11,0x11,0x11};
    static const uint8_t N[7]={0x11,0x11,0x19,0x15,0x13,0x11,0x11};
    static const uint8_t O[7]={0x0E,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const uint8_t P[7]={0x1E,0x11,0x11,0x1E,0x10,0x10,0x10};
    static const uint8_t Q[7]={0x0E,0x11,0x11,0x11,0x15,0x12,0x0D};
    static const uint8_t R[7]={0x1E,0x11,0x11,0x1E,0x14,0x12,0x11};
    static const uint8_t S[7]={0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E};
    static const uint8_t T[7]={0x1F,0x04,0x04,0x04,0x04,0x04,0x04};
    static const uint8_t U[7]={0x11,0x11,0x11,0x11,0x11,0x11,0x0E};
    static const uint8_t V[7]={0x11,0x11,0x11,0x11,0x11,0x0A,0x04};
    static const uint8_t W[7]={0x11,0x11,0x11,0x15,0x15,0x15,0x0A};
    static const uint8_t X[7]={0x11,0x11,0x0A,0x04,0x0A,0x11,0x11};
    static const uint8_t Y[7]={0x11,0x11,0x11,0x0A,0x04,0x04,0x04};
    static const uint8_t Z[7]={0x1F,0x01,0x02,0x04,0x08,0x10,0x1F};
    static const uint8_t N0[7]={0x0E,0x11,0x13,0x15,0x19,0x11,0x0E};
    static const uint8_t N1[7]={0x04,0x0C,0x04,0x04,0x04,0x04,0x0E};
    static const uint8_t N2[7]={0x0E,0x11,0x01,0x02,0x04,0x08,0x1F};
    static const uint8_t N3[7]={0x1F,0x02,0x04,0x02,0x01,0x11,0x0E};
    static const uint8_t N4[7]={0x02,0x06,0x0A,0x12,0x1F,0x02,0x02};
    static const uint8_t N5[7]={0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E};
    static const uint8_t N6[7]={0x06,0x08,0x10,0x1E,0x11,0x11,0x0E};
    static const uint8_t N7[7]={0x1F,0x01,0x02,0x04,0x08,0x08,0x08};
    static const uint8_t N8[7]={0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E};
    static const uint8_t N9[7]={0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C};
    static const uint8_t SP[7]={0,0,0,0,0,0,0};
    static const uint8_t CO[7]={0,0,0x04,0,0,0x04,0};
    static const uint8_t DA[7]={0,0,0,0x1F,0,0,0};
    static const uint8_t SL[7]={0x01,0x02,0x02,0x04,0x08,0x08,0x10};
    static const uint8_t EX[7]={0x04,0x04,0x04,0x04,0x04,0,0x04};
    static const uint8_t DO[7]={0,0,0,0,0,0,0x04};
    if(c>='a'&&c<='z')c=(char)(c-32);
    switch(c){
        case 'A':return A;case 'B':return B;case 'C':return C;case 'D':return D;
        case 'E':return E;case 'F':return F;case 'G':return G;case 'H':return H;
        case 'I':return I;case 'J':return J;case 'K':return K;case 'L':return L;
        case 'M':return M;case 'N':return N;case 'O':return O;case 'P':return P;
        case 'Q':return Q;case 'R':return R;case 'S':return S;case 'T':return T;
        case 'U':return U;case 'V':return V;case 'W':return W;case 'X':return X;
        case 'Y':return Y;case 'Z':return Z;
        case '0':return N0;case '1':return N1;case '2':return N2;case '3':return N3;
        case '4':return N4;case '5':return N5;case '6':return N6;case '7':return N7;
        case '8':return N8;case '9':return N9;
        case ' ':return SP;case ':':return CO;case '-':return DA;
        case '/':return SL;case '!':return EX;case '.':return DO;
        default: return e;
    }
}

inline void draw(SDL_Renderer* r, const std::string& text,
                 int x, int y, int scale,
                 uint8_t R=255, uint8_t G=255, uint8_t B=255) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    int cx = x;
    for (char c : text) {
        const uint8_t* g = glyph(c);
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 5; col++)
                if (g[row] & (1 << (4 - col))) {
                    SDL_FRect px{(float)(cx+col*scale),(float)(y+row*scale),
                                 (float)scale,(float)scale};
                    SDL_RenderFillRect(r, &px);
                }
        cx += 6 * scale;
    }
}

inline int width(const std::string& t, int scale) {
    return (int)t.size() * 6 * scale;
}


// Draw pixel-art hearts for lives display
inline void drawHeart(SDL_Renderer* r, int x, int y, int size,
                      uint8_t R=255, uint8_t G=60, uint8_t B=80) {
    // 7x6 pixel heart bitmap
    static const uint8_t H[6][7] = {
        {0,1,1,0,1,1,0},
        {1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1},
        {0,1,1,1,1,1,0},
        {0,0,1,1,1,0,0},
        {0,0,0,1,0,0,0},
    };
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    for(int row=0;row<6;row++)
        for(int col=0;col<7;col++)
            if(H[row][col]) {
                SDL_FRect p{(float)(x+col*size),(float)(y+row*size),(float)size,(float)size};
                SDL_RenderFillRect(r,&p);
            }
}

inline void drawHearts(SDL_Renderer* r, int lives, int maxLives,
                       int x, int y, int size, int gap=2) {
    int heartW = 7*size + gap;
    for(int i=0;i<maxLives;i++) {
        if(i < lives)
            drawHeart(r, x+i*heartW, y, size, 255, 60, 80);
        else {
            // empty heart outline
            SDL_SetRenderDrawColor(r,80,40,50,255);
            drawHeart(r, x+i*heartW, y, size, 60,30,40);
        }
    }
}
} // namespace Font
