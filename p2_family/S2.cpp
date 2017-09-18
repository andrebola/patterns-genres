#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cassert>

#include "midifile.h"
#include "song.h"
#include "partial.hpp"

struct file {
    std::string wholePath;
    std::string path;
    int score;

    bool operator < (const file& f) const {
        return score < f.score;
    }
};

std::vector<note> patternify(const char* path) {
    song* s = new song;
    midisong* ms = new midisong;

    read_midi_file(path,s,ms,0);

    std::vector<note> ret;

    for(int i = 0; i < s->size; ++i) {
        ret.push_back(note(s->notes[i].strt,s->notes[i].ptch));
        //cout<<"Note "<<i<<" ("<<s->notes[i].strt<<","<<s->notes[i].dur<<")"<<endl;  
    }

    ret.push_back(note(INF,INF));

    return ret;
}

int scorify(const std::vector<scale*>& results) {
    int ret = 0;
	for(int i = 0; i < sz(results); ++i) {
        std::set<int> ind;
		scale* s = results[i];
		while(s != NULL) {
			ind.insert(s->a);
			ind.insert(s->b);
			s = s->y;
		}
        if(sz(ind) > ret) ret = ind.size();
	}
    return ret;
}

int main(int argc, char** argv) {
    file query;
    std::vector<note> qNotes;

    std::ifstream fin("readfiles.txt");
    fin>>query.wholePath>>query.path;
    qNotes = patternify(query.wholePath.c_str());

    std::vector<file> db(1);
    std::vector<note> dNotes;

    while(fin>>db.back().wholePath) {
        fin>>db.back().path;
        dNotes = patternify(db.back().wholePath.c_str());
        std::vector<std::vector<scale> > K;
        PQV Q;
        int W = 10;
        create_K_table(W,qNotes,dNotes,K,Q);
        std::vector<scale*> results = solve(3,sz(qNotes),K,Q);
        db.back().score = scorify(results);
        db.resize(sz(db) + 1);
    }

    db.pop_back();

    std::stable_sort(db.begin(),db.end());
    std::reverse(db.begin(),db.end());

    for(int i = 0; i < 10; ++i) {
        std::cout<<db[i].path<<" ";
    }
    std::cout<<std::endl;

    fin.close();

    return 0;
}
