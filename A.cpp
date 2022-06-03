#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <chrono>
using namespace std;

int N, T;

int xor64() {
    static uint64_t x = 88172645463345263ULL;
    x ^= x<<13;
    x ^= x>> 7;
    x ^= x<<17;
    return int(x&0x7fffffff);
}

string to_string(const vector<int> &F)
{
    string ret;
    for (int y=0; y<N; y++)
    {
        vector<string> S(3, string(N*6, ' '));
        for (int x=0; x<N; x++)
        {
            S[1][6*x+3] = '+';
            if (F[y*N+x]>>0&1) S[1][6*x+1] = S[1][6*x+2] = '-';
            if (F[y*N+x]>>1&1) S[0][6*x+3] = '|';
            if (F[y*N+x]>>2&1) S[1][6*x+4] = S[1][6*x+5] = '-';
            if (F[y*N+x]>>3&1) S[2][6*x+3] = '|';
        }
        ret += S[0]+"\n";
        ret += S[1]+"\n";
        ret += S[2]+"\n";
    }
    return ret;
}

//  タイルの数がどのくらい一致しているかを返す
int get_score1(const vector<int> &F1, const vector<int> &F2)
{
    int C1[16] = {};
    for (int p=0; p<N*N; p++)
        C1[F1[p]]++;

    int C2[16] = {};
    for (int p=0; p<N*N; p++)
        C2[F2[p]]++;

    int c = N*N;
    for (int i=0; i<16; i++)
        c -= abs(C1[i]-C2[i]);
    return c;
}

//  (x, y), (x+1, y), (x, y+1), (x+1, y+1) の辺数を返す
int count_edge(const vector<int> &F, int x, int y)
{
    return
        ((F[(y  )*N+x  ]>>2&1)&(F[(y  )*N+x+1]>>0&1)) +
        ((F[(y+1)*N+x  ]>>2&1)&(F[(y+1)*N+x+1]>>0&1)) +
        ((F[(y  )*N+x  ]>>3&1)&(F[(y+1)*N+x  ]>>1&1)) +
        ((F[(y  )*N+x+1]>>3&1)&(F[(y+1)*N+x+1]>>1&1));
}

//  (x, y), (x+1, y), (x, y+1), (x+1, y+1) の辺を回転する。
void rotate_edge(vector<int> *F, int x, int y, int r)
{
    r = (r%4+4)%4;
    for (int i=0; i<r; i++)
    {
        int t1 = (*F)[(y  )*N+x  ]>>3&1;
        int t2 = (*F)[(y+1)*N+x  ]>>1&1;

        (*F)[(y  )*N+x  ] &= ~(1<<3);
        (*F)[(y  )*N+x  ] |= ((*F)[(y+1)*N+x  ]>>2&1)<<3;
        (*F)[(y+1)*N+x  ] &= ~(1<<1);
        (*F)[(y+1)*N+x  ] |= ((*F)[(y+1)*N+x+1]>>0&1)<<1;

        (*F)[(y+1)*N+x  ] &= ~(1<<2);
        (*F)[(y+1)*N+x  ] |= ((*F)[(y+1)*N+x+1]>>1&1)<<2;
        (*F)[(y+1)*N+x+1] &= ~(1<<0);
        (*F)[(y+1)*N+x+1] |= ((*F)[(y  )*N+x+1]>>3&1)<<0;

        (*F)[(y+1)*N+x+1] &= ~(1<<1);
        (*F)[(y+1)*N+x+1] |= ((*F)[(y  )*N+x+1]>>0&1)<<1;
        (*F)[(y  )*N+x+1] &= ~(1<<3);
        (*F)[(y  )*N+x+1] |= ((*F)[(y  )*N+x  ]>>2&1)<<3;

        (*F)[(y  )*N+x+1] &= ~(1<<0);
        (*F)[(y  )*N+x+1] |= t1<<0;
        (*F)[(y  )*N+x  ] &= ~(1<<2);
        (*F)[(y  )*N+x  ] |= t2<<2;
    }
}

//  Fを並び替えて木を作る
vector<int> get_tree(vector<int> F)
{
    // +---
    // +---
    // +---
    // +--
    vector<int> F2(N*N);
    F2[0] = 12;
    for (int x=1; x<N-1; x++)
        F2[x] = 5;
    F2[N-1] = 1;
    for (int y=1; y<N-1; y++)
    {
        F2[y*N+0] = 14;
        for (int x=1; x<N-1; x++)
            F2[y*N+x] = 5;
        F2[y*N+N-1] = 1;
    }
    F2[(N-1)*N+0] = 6;
    for (int x=1; x<N-2; x++)
        F2[(N-1)*N+x] = 5;
    F2[(N-1)*N+N-2] = 1;
    F2[(N-1)*N+N-1] = 0;

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

struct State
{
    vector<int> F;
    int sp = 0;
    string moves;
    int score = 0;
};

//  どのくらい揃っているかを返す
int get_score2(const vector<int> &F)
{
    //  揃っている行数
    int compl_ = 0;
    for (int y=0; y<N-2; y++)
    {
        bool ok = true;
        for (int x=0; x<N; x++)
            if (F[y*N+x]!=y*N+x)
                ok = false;
        if (ok)
            compl_++;
        else
            break;
    }

    int s = compl_*(N+10);

    int nt = -1;
    int np = 0;

    if (compl_<N-2)
    {
        int compr = 0;
        for (int x=0; x<N-2; x++)
            if (F[compl_*N+x]==compl_*N+x)
                compr++;
            else
                break;
        s += compr;

        if (compr<N-2)
        {
            nt = compl_*N+compr;
            np = compl_*N+compr;
        }
        else
        {
            //  残り2タイルが入れ替わった状況は詰むので避ける
            if (F[compl_*N+N-2]==compl_*N+N-1 &&
                F[compl_*N+N-1]==compl_*N+N-2)
                ;
            else if (
                F[compl_*N+N-2]==compl_*N+N-2 &&
                F[compl_*N+N-1]==N*N-1 &&
                F[(compl_+1)*N+N-1]==compl_*N+N-1)
                s += 3;
            else if (
                F[compl_*N+N-1]==compl_*N+N-2 &&
                F[(compl_+1)*N+N-1]==compl_*N+N-1)
                s += 2;
            else if (F[compl_*N+N-1]==compl_*N+N-2)
            {
                s++;
                nt = compl_*N+N-1;
                np = (compl_+1)*N+N-1;
            }
            else
            {
                nt = compl_*N+N-2;
                np = compl_*N+N-1;
            }
        }
    }
    else
    {
        int compr = 0;
        for (int x=0; x<N; x++)
            if (F[(N-2)*N+x]==(N-2)*N+x &&
                F[(N-1)*N+x]==(N-1)*N+x)
                compr++;
            else
                break;
        s += compr*10;

        if (compr<N-2)
        {
            //  残り2タイルが入れ替わった状況は詰むので避ける
            if (F[(N-2)*N+compr]==(N-1)*N+compr &&
                F[(N-1)*N+compr]==(N-2)*N+compr)
                ;
            else if (F[(N-2)*N+compr]==(N-2)*N+compr &&
                F[(N-1)*N+compr]==N*N-1 &&
                F[(N-1)*N+compr+1]==(N-1)*N+compr)
                s += 3;
            else if (
                F[(N-1)*N+compr]==(N-2)*N+compr &&
                F[(N-1)*N+compr+1]==(N-1)*N+compr)
                s += 2;
            else if (F[(N-1)*N+compr]==(N-2)*N+compr)
            {
                s++;
                nt = (N-1)*N+compr;
                np = (N-1)*N+compr+1;
            }
            else
            {
                nt = (N-2)*N+compr;
                np = (N-1)*N+compr;
            }
        }
    }

    s *= 100;

    if (nt>=0)
    {
        for (int p=0; p<N*N; p++)
            if (F[p]==nt)
                s += 100 - abs(p/N-np/N) - abs(p%N-np%N);
    }

    return s;
}

vector<int> get_perm(vector<int> F1, vector<int> F2)
{
    //  F1とF2の対応
    //  これを並び替えて、0, 1, 2, ... になれば良い
    vector<int> F(N*N);

    vector<bool> U(N*N);
    int sp = 0;
    for (int p1=0; p1<N*N; p1++)
    {
        if (F1[p1]==0)
            sp = p1;
        for (int p2=0; p2<N*N; p2++)
            if (!U[p2] && F1[p1]==F2[p2])
            {
                U[p2] = true;
                F[p1] = p2;
                break;
            }
    }

    //  距離の合計を最小化
    //  厳密解も求められるが……これでいいだろ
    for (int i=0; i<1000000; i++)
    {
        int p1 = xor64()%(N*N);
        int p2 = xor64()%(N*N);
        if (p1!=p2 && F2[F[p1]]==F2[F[p2]])
        {
            int s1 = abs(F[p1]/N-p1/N)+abs(F[p1]%N-p1%N)+abs(F[p2]/N-p2/N)+abs(F[p2]%N-p2%N);
            int s2 = abs(F[p2]/N-p1/N)+abs(F[p2]%N-p1%N)+abs(F[p1]/N-p2/N)+abs(F[p1]%N-p2%N);
            if (s2<s1)
                swap(F[p1], F[p2]);
        }
    }

    //  パリティチェック
    int par = 0;
    vector<int> Ftemp = F;
    for (int p1=0; p1<N*N; p1++)
    {
        if (Ftemp[p1]!=p1)
            for (int p2=0; p2<N*N; p2++)
                if (Ftemp[p2]==p1)
                {
                    swap(Ftemp[p1], Ftemp[p2]);
                    par ^= 1;
                }
        if (F[p1]==N*N-1)
            par ^= (abs(N-1-p1/N)+abs(N-1-p1%N))%2;
    }

    if (par!=0)
    {
        while (true)
        {
            int p1 = xor64()%(N*N);
            int p2 = xor64()%(N*N);
            if (p1!=p2 && F2[F[p1]]==F2[F[p2]])
            {
                swap(F[p1], F[p2]);
                break;
            }
        }
    }

    return F;
}

//  Fを 0, 1, 2, ... に並び替えるような動きを返す
string get_moves(vector<int> F)
{
    int WIDTH = N<10 ? 256 : 128;

    vector<State> S(1);
    S[0].F = F;
    for (int p=0; p<N*N; p++)
        if (S[0].F[p]==N*N-1)
            S[0].sp = p;
    S[0].score = get_score2(S[0].F)*100;

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

                if (
                    m=='U' && s.sp<N ||
                    m=='D' && s.sp>=N*(N-1) ||
                    m=='L' && s.sp%N==0 ||
                    m=='R' && s.sp%N==N-1)
                    continue;

                State s2 = s;
                switch (m) {
                case 'U':
                    swap(s2.F[s2.sp], s2.F[s2.sp-N]);
                    s2.sp -= N;
                    break;
                case 'D':
                    swap(s2.F[s2.sp], s2.F[s2.sp+N]);
                    s2.sp += N;
                    break;
                case 'L':
                    swap(s2.F[s2.sp], s2.F[s2.sp-1]);
                    s2.sp--;
                    break;
                case 'R':
                    swap(s2.F[s2.sp], s2.F[s2.sp+1]);
                    s2.sp++;
                    break;
                }
                s2.moves += m;
                s2.score = get_score2(s2.F)*100 + xor64()%100;
                S2.push_back(s2);
            }
        S = S2;
        sort(S.begin(), S.end(), [&](State &s1, State &s2){
            return s1.score>s2.score;
        });
        if (S.size()>WIDTH)
            S.resize(WIDTH);

        //cerr<<S[0].score<<endl;

        if (S[0].score/100==((N+10)*(N-2)+10*N)*100)
            break;
    }

    return S[0].moves;
}

int main()
{
    cin>>N>>T;
    vector<int> F(N*N);
    for (int y=0; y<N; y++)
    {
        string t;
        cin>>t;
        for (int x=0; x<N; x++)
            if ('0'<=t[x] && t[x]<='9')
                F[y*N+x] = t[x]-'0';
            else
                F[y*N+x] = t[x]-'a'+10;
    }

    auto start = chrono::system_clock::now();

    vector<int> F2;
    for (int i=0; i<4; i++)
    {
        F2 = get_tree(F);
        if (get_score1(F, F2)==N*N)
            break;
    }

    //cerr<<to_string(F2)<<endl;

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

    vector<int> P = get_perm(F, F2);

    string moves = get_moves(P);
    cout<<moves<<endl;

    end = chrono::system_clock::now();

    cerr<<moves.size()<<"/"<<T<<endl;
    cerr<<chrono::duration_cast<chrono::milliseconds>(end-start).count()<<" ms"<<endl;
    if (moves.size()==T)
        cerr<<"phase 2 NG"<<endl;
    cerr<<endl<<endl;
}
