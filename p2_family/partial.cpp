#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <set>

#include "partial.hpp"
#include "midifile.h"
#include "song.h"
#include "search.h"
#include "geometric_P2.h"

std::ostream& operator << (std::ostream& o, const scale& s) {
	o<<s.a<<" "<<s.b<<" "<<s.c<<" "<<s.w<<" "<<s.s;
	return o;
}

std::vector<note> P; // pattern
std::vector<note> T; // database
std::vector<std::vector<scale> > K;
std::vector<std::priority_queue<scale*,std::vector<scale*>,cmp> > Q;

std::vector<scale*> Kaa;

int main(int argc, char** argv) {

        int only_rhythm = 0;	
        if (argc == 7){
	    only_rhythm = std::stoi(argv[6]);	
        }

 	songcollection* sc = new songcollection;
        sc->songs = (song *) calloc(1, sizeof(song));
        sc->size++;
        // Only rhytm = 1 discards tonic information
	read_midi_file2(argv[1],&sc->songs[0],NULL,0,only_rhythm);
	song s =sc->songs[0];
	
	searchparameters* parameters = new searchparameters;
       
	double similarity = std::stod(argv[5]);
	int length = std::stoi(argv[3]);
	int window = std::stoi(argv[4]);	
	int numb_notes = 100;

        //std::cout << "FILE: " << argv[1];
        //std::cout << "=================="<< std::endl;


	//std::cout << "=================="<< std::endl;
        //std::cout << "size: " << s.size<< std::endl;
	//std::cout << "=================="<< std::endl;

	songcollection* pc = new songcollection;
        matchset* pms = new matchset;
	
	generate_patterns_song(pc, &sc->songs[0], pms, length, window);
        //std::cout << "Genreated patterns: "<< pms->num_matches << std::endl;
	//std::cout << "=================="<< std::endl;
	//std::cout << "Patterns " << pc->size <<std::endl;
	//std::cout << "=================="<< std::endl;


	std::ofstream out(argv[2]);
	out << "[";	
	bool added = false;

	for (int j=0; j<pc->size; ++j) {
		//std::cout << "==>>>>>>>>>>>>>>>"<< std::endl;
		//std::cout << "Size P " << pc->songs[j].size <<std::endl;
		// cut song starting on pms->matches[i].end
	        match m = pms->matches[j];
		//std::cout << "START " << m.start<<std::endl;
		//std::cout << "END " << m.end<<std::endl;

		// Copy song starting from the section to match
		song* newsong = new song;
                newsong->notes = (vector *) malloc(sizeof(vector) * s.size);
                
		int n = 0;
		//std::cout << "M "<<m.end  <<std::endl;
		//std::cout << "SIZE "<<s.size  <<std::endl;
		bool found = false;
		for (int k=0; k<s.size; ++k) {
		    
                    if (found == true ){//&& n<numb_notes){
                        char pitch = s.notes[k].ptch;
                        newsong->notes[n].ptch = pitch;
                        newsong->notes[n].strt = s.notes[k].strt;
                        newsong->notes[n].dur = s.notes[k].dur;
		        n++;
		//std::cout << "N "<<n  <<std::endl;
                    }
		//std::cout << "strt "<<s.notes[k].strt  <<std::endl;
                    if (s.notes[k].strt == m.end){
		        found = true;
		    }
		}
		newsong->size = n;
		newsong->title = s.title;
		newsong->duration = 0;
                sc->songs[0] = *newsong;

		
		//for (int k=0; k<pc->songs[j].size; ++k) {
		
		//std::cout << "Note:  "<< pc->songs[j].notes[k].strt << std::endl;
		//}

                matchset* ms = new matchset;
                ms->size = 0;
	        ms->num_matches = 0;

		if (pc->songs[j].size>2){
		// Call P2 algorithm with song sections
		alg_p2(sc, &pc->songs[j], 2, parameters, ms);
                //free(newsong->notes);
		if (ms->num_matches > 0){
		    //std::cout << "=================="<< std::endl;
		    //std::cout << "Matches: "<< ms->num_matches << std::endl;
		    for (int i=0; i<ms->num_matches; ++i) {
		        if (ms->matches[i].similarity >= similarity){
                            if (added == true){
			        out << ",";
		            }
			    added = true;
		            out << "[";	
		            //std::cout << "Pattern Matched on note: "<< std::endl;
		            //std::cout << ms->matches[i].start<< std::endl;
		            //std::cout << ms->matches[i].similarity<< std::endl;
		            //std::cout << length << std::endl;
		            //std::cout << window << std::endl;
		            //std::cout << "=================="<< std::endl;
			    for (int k=0; k<pc->songs[j].size; ++k) {
				if (k!=0){
				    out << ",";
		                }
			        out << "["<<pc->songs[j].notes[k].strt << "," << (int)pc->songs[j].notes[k].ptch <<"]";
			    }
		            out << "]";	
			}
		    }
		    //std::cout << "<<<<<<<<<<<<<<====="<< std::endl;
		}
		}
		//free(newsong->notes);
		//free(newsong);
		//free(ms->matches);
		//free(ms);
	//free(pc->songs);
	//free(pc);
	//free(pms->matches);
	//free(pms);
	}
        out << "]";	
	out.close();


	return 0;
}

