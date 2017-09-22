package siatec;

import java.util.ArrayList;
import java.util.TreeSet;

public class PointSetCollectionPair {

	public ArrayList<PointSet> pointSetCollection1 = null;
	public ArrayList<PointSet> pointSetCollection2 = null;

	public PointSetCollectionPair(ArrayList<PointSet> pointSetCollection1, ArrayList<PointSet> pointSetCollection2) {
		this.pointSetCollection1 = pointSetCollection1;
		this.pointSetCollection2 = pointSetCollection2;
	}

	public PointSetCollectionPair(ArrayList<PointSet> pointSetCollection1, PatternOccurrenceListPair patternOccurrenceListPair) {
		this.pointSetCollection1 = pointSetCollection1;
		pointSetCollection2 = new ArrayList<PointSet>();
		for(PointSet occurrence : patternOccurrenceListPair.occurrences)
			pointSetCollection2.add(occurrence);
	}
	
	public PointSetCollectionPair(TEC tec, PatternOccurrenceListPair patternOccurrenceListPair) {
		pointSetCollection1 = new ArrayList<PointSet>();
		TreeSet<Vector> translators = tec.getTranslators().getVectors();
		for(Vector v : translators)
			pointSetCollection1.add(tec.getPattern().translate(v));
		
		pointSetCollection2 = new ArrayList<PointSet>();
//		pointSetCollection2.add(patternOccurrenceListPair.pattern);
		for(PointSet occurrence : patternOccurrenceListPair.occurrences)
			pointSetCollection2.add(occurrence);
	}

}

