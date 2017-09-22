package siatec;

public class PatternPair implements Comparable<PatternPair>{
	private PointSet pattern1, pattern2;
	
	public PatternPair(PointSet pattern1, PointSet pattern2) {
		this.pattern1 = pattern1;
		this.pattern2 = pattern2;
	}
	
	public PointSet getPattern1() {return pattern1;}
	public PointSet getPattern2() {return pattern2;}
	
	@Override
	public int compareTo(PatternPair pp) {
		int d = pattern1.compareTo(pp.getPattern1());
		if (d != 0) return d;
		return pattern2.compareTo(pp.getPattern2());
	}
	
	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof PatternPair)) return false;
		return compareTo((PatternPair)obj) == 0;
	}
	
	@Override
	public String toString() {
		return "PatternPair("+pattern1+","+pattern2+")";
	}
}
