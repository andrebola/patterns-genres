package siatec;

public class VectorPointSetPair implements Comparable<VectorPointSetPair> {
	
	private Vector vector;
	private PointSet pointSet = new PointSet();
	
	public VectorPointSetPair(Vector vector, PointSet pointSet) {
		setVector(vector);
		setPointSet(pointSet);
	}
	
	public Vector getVector() {
		return vector;
	}
	
	public void setVector(Vector vector) {
		this.vector = vector;
	}
	
	public PointSet getPointSet() {
		return pointSet;
	}
	
	public void setPointSet(PointSet pointSet) {
		this.pointSet = pointSet;
	}

	@Override
	public int compareTo(VectorPointSetPair v) {
		if (v == null) return 1;
		int d = v.getPointSet().size() - getPointSet().size();
		if (d != 0) return d;
		d = v.getPointSet().compareTo(getPointSet());
		if (d != 0) return d;
		return v.getVector().compareTo(getVector());
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof VectorPointSetPair)) return false;
		return compareTo((VectorPointSetPair)obj) == 0;
	}
	
	public void addPoint(Point point) {
		getPointSet().add(point);
	}
	
}
