package siatec;

import java.util.ArrayList;
import java.util.Collection;

public class PatternVectorSetPairList {
	private ArrayList<PatternVectorSetPair> patternVectorSetPairs = null;
	
	public PatternVectorSetPairList(Collection<PatternVectorSetPair> patternVectorSetPairs) {
		this.patternVectorSetPairs = new ArrayList<PatternVectorSetPair>(patternVectorSetPairs);
	}
	
	public ArrayList<PatternVectorSetPair> getPatternVectorSetPairs() {
		return patternVectorSetPairs;
	}
	
	public int size() {
		return patternVectorSetPairs.size();
	}
	
	public PatternVectorSetPair get(int i) {
		return patternVectorSetPairs.get(i);
	}
	
	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		for(PatternVectorSetPair pvsp : patternVectorSetPairs) {
			sb.append(pvsp+"\n");
		}
		return sb.toString();
	}
}
