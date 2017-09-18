#ifndef PARTIAL_HPP
#define PARTIAL_HPP

#include <vector>
#include <queue>

#define sz(x) ((int)x.size())
const int INF = (1<<30);

struct note {
	int x,y;
	note() {}
	note(int x, int y) : x(x), y(y) {}
	bool operator < (const note& n) const {
		if(x == n.x) return y < n.y;
		return x < n.x;
	}
	bool operator == (const note& n) {
		return x == n.x && y == n.y;
	}
	bool operator != (const note& n) {
		return !(*this == n);
	}
};

struct scale {
	int a,b,w;
	int c,z;
	scale* y;
	double s;

	bool operator < (const scale& s) const {
		if(a == s.a) return this->s < s.s;
		return a < s.a;
	}

};

struct cmp {
	bool operator () (scale* s1, scale* s2) {
		if(s1->b == s2->b) return s1->s > s2->s;
		return s1->b > s2->b;
	}
};

std::ostream& operator << (std::ostream& o, const scale& s);
typedef std::vector<std::priority_queue<scale*,std::vector<scale*>,cmp> > PQV;

void create_K_table(int W, const std::vector<note>& P, const std::vector<note>& T, 
        std::vector<std::vector<scale> >& K, PQV& Q);
void reportOccurences(const std::vector<note>& T, const std::vector<scale*>& Kaa);
std::vector<scale*> solve(int alpha, int m, std::vector<std::vector<scale> >& K, PQV& Q);

#endif
