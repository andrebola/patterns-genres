package siatec;

import java.util.TreeSet;

public class PointFreqSet {

	private TreeSet<PointFreq> pointFreqs = new TreeSet<PointFreq>();
	private TreeSet<PointFreq> multiPoints = new TreeSet<PointFreq>();
	
	public TreeSet<PointFreq> getMultiPoints() {
		return multiPoints;
	}

	public PointFreq addPoint(Point point) {
		PointFreq pf = new PointFreq(point,1);
		PointFreq floorPf = pointFreqs.floor(pf);
		PointFreq ceilPf = pointFreqs.ceiling(pf);
		if (ceilPf != null && ceilPf.equals(floorPf)) {
			ceilPf.incFreq();
			multiPoints.add(ceilPf);
			return ceilPf;
		} else {
			pointFreqs.add(pf);
			return pf; 
		}
	}

	public PointFreq addPoint(Point point, Vector vector) {
		PointFreq pf = new PointFreq(point,1,vector);
		PointFreq floorPf = pointFreqs.floor(pf);
		PointFreq ceilPf = pointFreqs.ceiling(pf);
		if (ceilPf != null && ceilPf.equals(floorPf)) {
			ceilPf.incFreq();
			multiPoints.add(ceilPf);
			ceilPf.addTranslator(vector);
			return ceilPf;
		} else {
			pointFreqs.add(pf);
			return pf;
		}
	}

	public PointFreq get(Point point) {
		PointFreq pointFreq = new PointFreq(point, 1);
		if (pointFreqs.contains(pointFreq))
			return pointFreqs.floor(pointFreq);
		return null;
	}
	
	@Override
	public String toString() {
		return pointFreqs.toString();
	}
	
	public int getFreq(Point p) {
		if (multiPoints.contains(new PointFreq(p,1)))
			return multiPoints.floor(new PointFreq(p,1)).getFreq();
		else if (pointFreqs.contains(new PointFreq(p,1)))
			return 1;
		else
			return 0;
	}
}


