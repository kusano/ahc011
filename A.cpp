#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <chrono>
using namespace std;

int xor64() {
    static uint64_t x = 88172645463345263ULL;
    x ^= x<<13;
    x ^= x>> 7;
    x ^= x<<17;
    return int(x&0x7fffffff);
}

string to_string(const vector<vector<int>> &F)
{
    int N = (int)F.size();

    string ret;
    for (int y=0; y<N; y++)
    {
        vector<string> S(3, string(N*6, ' '));
        for (int x=0; x<N; x++)
        {
            S[1][6*x+3] = '+';
            if (F[y][x]>>0&1) S[1][6*x+1] = S[1][6*x+2] = '-';
            if (F[y][x]>>1&1) S[0][6*x+3] = '|';
            if (F[y][x]>>2&1) S[1][6*x+4] = S[1][6*x+5] = '-';
            if (F[y][x]>>3&1) S[2][6*x+3] = '|';
        }
        ret += S[0]+"\n";
        ret += S[1]+"\n";
        ret += S[2]+"\n";
    }
    return ret;
}

//  タイルの数がどのくらい一致しているかを返す
int get_score1(const vector<vector<int>> &F1, const vector<vector<int>> &F2)
{
    int N = (int)F1.size();

    int C1[16] = {};
    for (int y=0; y<N; y++)
        for (int x=0; x<N; x++)
            C1[F1[y][x]]++;

    int C2[16] = {};
    for (int y=0; y<N; y++)
        for (int x=0; x<N; x++)
            C2[F2[y][x]]++;

    int c = N*N;
    for (int i=0; i<16; i++)
        c -= abs(C1[i]-C2[i]);
    return c;
}

//  (x, y), (x+1, y), (x, y+1), (x+1, y+1) の辺数を返す
int count_edge(const vector<vector<int>> &F, int x, int y)
{
    int N = (int)F.size();

    return
        ((F[y  ][x  ]>>2&1)&(F[y  ][x+1]>>0&1)) +
        ((F[y+1][x  ]>>2&1)&(F[y+1][x+1]>>0&1)) +
        ((F[y  ][x  ]>>3&1)&(F[y+1][x  ]>>1&1)) +
        ((F[y  ][x+1]>>3&1)&(F[y+1][x+1]>>1&1));
}

//  (x, y), (x+1, y), (x, y+1), (x+1, y+1) の辺を回転する。
void rotate_edge(vector<vector<int>> *F, int x, int y, int r)
{
    r = (r%4+4)%4;
    for (int i=0; i<r; i++)
    {
        int t1 = (*F)[y  ][x  ]>>3&1;
        int t2 = (*F)[y+1][x  ]>>1&1;

        (*F)[y  ][x  ] &= ~(1<<3);
        (*F)[y  ][x  ] |= ((*F)[y+1][x  ]>>2&1)<<3;
        (*F)[y+1][x  ] &= ~(1<<1);
        (*F)[y+1][x  ] |= ((*F)[y+1][x+1]>>0&1)<<1;

        (*F)[y+1][x  ] &= ~(1<<2);
        (*F)[y+1][x  ] |= ((*F)[y+1][x+1]>>1&1)<<2;
        (*F)[y+1][x+1] &= ~(1<<0);
        (*F)[y+1][x+1] |= ((*F)[y  ][x+1]>>3&1)<<0;

        (*F)[y+1][x+1] &= ~(1<<1);
        (*F)[y+1][x+1] |= ((*F)[y  ][x+1]>>0&1)<<1;
        (*F)[y  ][x+1] &= ~(1<<3);
        (*F)[y  ][x+1] |= ((*F)[y  ][x  ]>>2&1)<<3;

        (*F)[y  ][x+1] &= ~(1<<0);
        (*F)[y  ][x+1] |= t1<<0;
        (*F)[y  ][x  ] &= ~(1<<2);
        (*F)[y  ][x  ] |= t2<<2;
    }
}

//  Fを並び替えて木を作る
vector<vector<int>> get_tree(vector<vector<int>> F)
{
    int N = (int)F.size();

    // +---
    // +---
    // +---
    // +--
    vector<vector<int>> F2(N, vector<int>(N));
    F2[0][0] = 12;
    for (int x=1; x<N-1; x++)
        F2[0][x] = 5;
    F2[0][N-1] = 1;
    for (int y=1; y<N-1; y++)
    {
        F2[y][0] = 14;
        for (int x=1; x<N-1; x++)
            F2[y][x] = 5;
        F2[y][N-1] = 1;
    }
    F2[N-1][0] = 6;
    for (int x=1; x<N-2; x++)
        F2[N-1][x] = 5;
    F2[N-1][N-2] = 1;
    F2[N-1][N-1] = 0;

    int s = get_score1(F, F2);

    int T = 1000000;
    for (int t=0; t<T; t++)
    {
        int x = xor64()%(N-1);
        int y = xor64()%(N-1);
        if (count_edge(F2, x, y)==3)
        {
            int r = xor64()%3+1;
            rotate_edge(&F2, x, y, r);

            int s2 = get_score1(F, F2);
            double temp = (double)(T-t)/T*N;
            if (s2>=s ||
                double(xor64()%0x10000)/0x10000 < exp((s2-s)/temp))
            {
                s = s2;
            }
            else
            {
                rotate_edge(&F2, x, y, -r);
            }
        }
    }

    return F2;
}

//  Fにmovesを適用する
vector<vector<int>> apply_moves(vector<vector<int>> F, int sx, int sy, string moves)
{
    int N = (int)F.size();

    for (char m: moves)
        switch (m)
        {
        case 'U':
            if (sy-1<0)
            {
                F[0][0] = -1;
                return F;
            }
            swap(F[sy][sx], F[sy-1][sx]);
            sy--;
            break;
        case 'D':
            if (N<=sy+1)
            {
                F[0][0] = -1;
                return F;
            }
            swap(F[sy][sx], F[sy+1][sx]);
            sy++;
            break;
        case 'L':
            if (sx-1<0)
            {
                F[0][0] = -1;
                return F;
            }
            swap(F[sy][sx], F[sy][sx-1]);
            sx--;
            break;
        case 'R':
            if (N<=sx+1)
            {
                F[0][0] = -1;
                return F;
            }
            swap(F[sy][sx], F[sy][sx+1]);
            sx++;
            break;
        }
    return F;
}

struct State
{
    string moves;
    int score = 0;
};

//  どのくらい揃っているかを返す
int get_score2(vector<vector<int>> F)
{
    int N = (int)F.size();

    if (F[0][0]==-1)
        return -99999999;

    //  揃っている行数
    int compl_ = 0;
    for (int y=0; y<N-2; y++)
    {
        bool ok = true;
        for (int x=0; x<N; x++)
            if (F[y][x]!=y*N+x)
                ok = false;
        if (ok)
            compl_++;
        else
            break;
    }

    int s = compl_*(N+10);

    int nt = -1;
    int nx = 0;
    int ny = 0;

    if (compl_<N-2)
    {
        int compr = 0;
        for (int x=0; x<N-2; x++)
            if (F[compl_][x]==compl_*N+x)
                compr++;
            else
                break;
        s += compr;

        if (compr<N-2)
        {
            nt = compl_*N+compr;
            nx = compr;
            ny = compl_;
        }
        else
        {
            if (F[compl_][N-2]==compl_*N+N-2 &&
                F[compl_][N-1]==N*N-1 &&
                F[compl_+1][N-1]==compl_*N+N-1)
                s += 3;
            else if (
                F[compl_][N-1]==compl_*N+N-2 &&
                F[compl_+1][N-1]==compl_*N+N-1)
                s += 2;
            else if (F[compl_][N-1]==compl_*N+N-2)
            {
                s++;
                nt = compl_*N+N-1;
                nx = N-1;
                ny = compl_+1;
            }
            else
            {
                nt = compl_*N+N-2;
                nx = N-1;
                ny = compl_;
            }
        }
    }
    else
    {
        int compr = 0;
        for (int x=0; x<N; x++)
            if (F[N-2][x]==(N-2)*N+x &&
                F[N-1][x]==(N-1)*N+x)
                compr++;
            else
                break;
        s += compr*10;

        if (compr<N-2)
        {
            if (F[N-2][compr]==(N-2)*N+compr &&
                F[N-1][compr]==N*N-1 &&
                F[N-1][compr+1]==(N-1)*N+compr)
                s += 3;
            else if (
                F[N-1][compr]==(N-2)*N+compr &&
                F[N-1][compr+1]==(N-1)*N+compr)
                s += 2;
            else if (F[N-1][compr]==(N-2)*N+compr)
            {
                s++;
                nt = (N-1)*N+compr;
                nx = compr+1;
                ny = N-1;
            }
            else
            {
                nt = (N-2)*N+compr;
                nx = compr;
                ny = N-1;
            }
        }
    }

    s *= 100;

    if (nt>=0)
    {
        for (int y=0; y<N; y++)
            for (int x=0; x<N; x++)
                if (F[y][x]==nt)
                    s += 100 - abs(x-nx) - abs(y-ny);
    }

    return s;
}

//  F1をF2に並び替えるような動きを返す
string get_moves(vector<vector<int>> F1, vector<vector<int>> F2)
{
    int N = (int)F1.size();

    int WIDTH = 128;

    //  F1とF2の対応
    //  これを並び替えて、0, 1, 2, ... になれば良い
    vector<vector<int>> F(N, vector<int>(N));

    vector<vector<bool>> U(N, vector<bool>(N));
    int sx = 0;
    int sy = 0;
    for (int y1=0; y1<N; y1++)
        for (int x1=0; x1<N; x1++)
        {
            if (F1[y1][x1]==0)
            {
                sx = x1;
                sy = y1;
            }
            for (int y2=0; y2<N; y2++)
                for (int x2=0; x2<N; x2++)
                    if (!U[y2][x2] && F1[y1][x1]==F2[y2][x2])
                    {
                        U[y2][x2] = true;
                        F[y1][x1] = y2*N+x2;
                        goto end;
                    }
        end:;
        }

    //  パリティチェック
    int p = 0;
    vector<vector<int>> Ftemp = F;
    for (int y1=0; y1<N; y1++)
        for (int x1=0; x1<N; x1++)
        {
            if (Ftemp[y1][x1]!=y1*N+x1)
                for (int y2=0; y2<N; y2++)
                    for (int x2=0; x2<N; x2++)
                        if (Ftemp[y2][x2]==y1*N+x1)
                        {
                            swap(Ftemp[y1][x1], Ftemp[y2][x2]);
                            p ^= 1;
                        }
            if (F[y1][x1]==N*N-1)
                p ^= (abs(N-1-x1)+abs(N-1-y1))%2;
        }

    if (p!=0)
    {
        while (true)
        {
            int x1 = xor64()%N;
            int y1 = xor64()%N;
            int x2 = xor64()%N;
            int y2 = xor64()%N;
            if (x1!=x2 || y1!=y2)
            {
                int f1 = F[y1][x1];
                int f2 = F[y2][x2];
                if (F2[f1/N][f1%N]==F2[f2/N][f2%N])
                {
                    swap(F[y1][x1], F[y2][x2]);
                    break;
                }
            }
        }
    }

    int T = 2*N*N*N;
    vector<State> S(1);

    for (int t=0; t<T; t++)
    {
        vector<State> S2;
        for (State &s: S)
            for (char m: string("UDLR"))
            {
                char p = '?';
                if (!s.moves.empty())
                    p = s.moves.back();
                if (
                    m=='U' && p=='D' ||
                    m=='D' && p=='U' ||
                    m=='L' && p=='R' ||
                    m=='R' && p=='L')
                    continue;

                State s2 = s;
                s2.moves += m;
                vector<vector<int>> Ft = apply_moves(F, sx, sy, s2.moves);
                s2.score = get_score2(Ft)*100 + xor64()%100;
                S2.push_back(s2);
            }
        S = S2;
        sort(S.begin(), S.end(), [&](State &s1, State &s2){
            return s1.score>s2.score;
        });
        if (S.size()>WIDTH)
            S.resize(WIDTH);

        if (S[0].score/100==((N+10)*(N-2)+10*N)*100)
            break;
    }

    return S[0].moves;
}

int main()
{
    int N, T;
    cin>>N>>T;
    vector<vector<int>> F(N, vector<int>(N));
    for (int y=0; y<N; y++)
    {
        string t;
        cin>>t;
        for (int x=0; x<N; x++)
            if ('0'<=t[x] && t[x]<='9')
                F[y][x] = t[x]-'0';
            else
                F[y][x] = t[x]-'a'+10;
    }

    auto start = chrono::system_clock::now();

    vector<vector<int>> F2 = get_tree(F);

    auto end = chrono::system_clock::now();

    cerr<<get_score1(F, F2)<<"/"<<N*N<<endl;
    cerr<<chrono::duration_cast<chrono::milliseconds>(end-start).count()<<" ms"<<endl;

    if (get_score1(F, F2)<N*N)
    {
        cerr<<"phase 1 NG"<<endl;
        cerr<<endl;

        cout<<endl;
        return 0;
    }

    string moves = get_moves(F, F2);
    cout<<moves<<endl;

    end = chrono::system_clock::now();

    cerr<<moves.size()<<"/"<<T<<endl;
    cerr<<chrono::duration_cast<chrono::milliseconds>(end-start).count()<<" ms"<<endl;
    if (moves.size()==T)
        cerr<<"phase 2 NG"<<endl;
    cerr<<endl<<endl;
}
