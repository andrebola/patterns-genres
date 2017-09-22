package siatec;

import java.util.ArrayList;

public class PatternOccurrenceListPair {
	String patternFileName;
	ArrayList<String> occurrenceFileNames = new ArrayList<String>();
	PointSet pattern;
	ArrayList<PointSet> occurrences = new ArrayList<PointSet>();
	
	public String toString() {
		StringBuilder sb = new StringBuilder(pattern.toString());
		for(PointSet occurrence : occurrences) 
			sb.append("\n  "+occurrence);
		return sb.toString();
	}
}

