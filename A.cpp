#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
using namespace std;

class UnionFind
{
public:
    vector<int> parent;
    vector<int> sz;

    UnionFind(int n): parent(n), sz(n)
    {
        for(int i=0;i<n;i++)
        {
            parent[i] = i;
            sz[i] = 1;
        }
    }

    int root(int x)
    {
        return parent[x]==x ? x : parent[x] = root(parent[x]);
    }

    bool same(int x, int y)
    {
        return root(x)==root(y);
    }

    void unite(int x, int y)
    {
        x = root(x);
        y = root(y);
        if (x != y)
        {
            if (sz[x] < sz[y])
            {
                parent[x] = y;
                sz[y] += sz[x];
            }
            else
            {
                parent[y] = x;
                sz[x] += sz[y];
            }
        }
    }

    int size(int x)
    {
        return sz[root(x)];
    }
};

int xor64() {
    static uint64_t x = 88172645463345263ULL;
    x ^= x<<13;
    x ^= x>> 7;
    x ^= x<<17;
    return int(x&0x7fffffff);
}

string to_string(vector<vector<int>> F)
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

//  最大の木のサイズを返す
int get_score1(vector<vector<int>> F)
{
    int N = (int)F.size();

    UnionFind UF(N*N);
    vector<bool> C(N*N);

    for (int y=0; y<N; y++)
        for (int x=0; x<N-1; x++)
            if ((F[y][x]>>2&1) && (F[y][x+1]>>0&1))
            {
                int p1 = y*N+x;
                int p2 = y*N+x+1;
                int r1 = UF.root(p1);
                int r2 = UF.root(p2);
                if (r1==r2)
                    C[r1] = true;
                else
                {
                    UF.unite(p1, p2);
                    C[UF.root(p1)] = C[r1] || C[r2];
                }
            }

    for (int y=0; y<N-1; y++)
        for (int x=0; x<N; x++)
            if ((F[y][x]>>3&1) && (F[y+1][x]>>1&1))
            {
                int p1 = (y+1)*N+x;
                int p2 = y*N+x;
                int r1 = UF.root(p1);
                int r2 = UF.root(p2);
                if (r1==r2)
                    C[r1] = true;
                else
                {
                    UF.unite(p1, p2);
                    C[UF.root(p1)] = C[r1] || C[r2];
                }
            }

    int ans = 0;
    for (int p=0; p<N*N; p++)
        ans = max(ans, UF.size(p));
    return ans;
}

//  Fを並び替えて木を作る
vector<vector<int>> get_tree(vector<vector<int>> F)
{
    int N = (int)F.size();

    //  右下を空白にする
    for (int y=0; y<N; y++)
        for (int x=0; x<N; x++)
            if (!(x==N && y==N) && F[y][x]==0)
                swap(F[y][x], F[N-1][N-1]);

    int T = 100000;
    for (int t=0; t<T; t++)
    {
        int x1 = xor64()%N;
        int y1 = xor64()%N;
        int x2 = xor64()%N;
        int y2 = xor64()%N;

        ////  右下は選択しない
        //if (x1==N-1 && y1==N-1 ||
        //    x2==N-1 && y2==N-1)
        //    continue;

        int s1 = get_score1(F);
        swap(F[y1][x1], F[y2][x2]);
        int s2 = get_score1(F);

        double temp = double(T*9/10-t)/(T*9/10)*N*N;
        if (s2>s1 ||
            t<T*9/10 && double(xor64()%10000)/10000 < exp((s2-s1)/temp))
        {
            //cout<<s2<<" "<<s1<<endl;
        }
        else
            swap(F[y1][x1], F[y2][x2]);
    }
    return F;
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

    int s = 10000;
    for (int y1=0; y1<N; y1++)
        for (int x1=0; x1<N; x1++)
            if (F[y1][x1]==y1*N+x1)
                s += 10000;
            else
            {
                for (int y2=0; y2<N; y2++)
                    for (int x2=0; x2<N; x2++)
                        if (F[y2][x2]==y1*N+x1)
                        {
                            s -= (abs(x1-x2)+abs(y1-y2))*100;
                            goto end;
                        }
            }
end:
    return s;
}

//  F1をF2に並び替えるような動きを返す
string get_moves(vector<vector<int>> F1, vector<vector<int>> F2)
{
    int N = (int)F1.size();

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
                s2.score = get_score2(Ft) + xor64()%100;
                S2.push_back(s2);
            }
        S = S2;
        sort(S.begin(), S.end(), [&](State &s1, State &s2){
            return s1.score>s2.score;
        });
        S.resize(1024);

        cerr<<S[0].score<<endl;

        if (S[0].score==100*N*N)
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

    vector<vector<int>> F2 = get_tree(F);

    cerr<<to_string(F2)<<endl;
    cerr<<get_score1(F2)<<endl;
    return 0;

    string moves = get_moves(F, F2);
    cout<<moves<<endl;
}
