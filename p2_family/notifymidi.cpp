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

void print(const char* path) {
    song* s = new song;
    midisong* ms = new midisong;

    read_midi_file(path,s,ms,0);

    for(int i = 0; i < s->size; ++i) {
        cout<<s->notes[i].strt<<" "<<(int)s->notes[i].ptch<<endl;
    }
}

int main(int argc, char** argv) {
    print(argv[1]); 
    return 0;
}
