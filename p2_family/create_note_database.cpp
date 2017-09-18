#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <cassert>

#include "midifile.h"
#include "song.h"

using namespace std;

void print(const char* path, long long& a) {
    song* s = new song;
    midisong* ms = new midisong;

    read_midi_file(path,s,ms,0);

    for(int i = 0; i < s->size; ++i) {
        cout<<(s->notes[i].strt + a)<<" "<<(int)s->notes[i].ptch<<endl;
    }
    a += s->notes[s->size - 1].strt + 1;
}

int main(int argc, char** argv) {
    long long a = 0;
    for(int i = 1; i < argc; ++i) 
        print(argv[i],a); 
    return 0;
}
