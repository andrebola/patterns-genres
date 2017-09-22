package siatec;


public class VectorPointPair implements Comparable<VectorPointPair>{
	private Vector vector;
	private Point point;
	private Integer index;

	public VectorPointPair(Point p1, Point p2, int columnIndex) {
		vector = new Vector(p1,p2);
		point = p1;
		this.index = columnIndex;
	}
	
	public VectorPointPair(Point p1, Point p2) {
		vector = new Vector(p1,p2);
		point = p1;
	}
	
	public VectorPointPair(Vector v, Point p) {
		this.vector = v;
		this.point = p;
	}

	@Override
	public int compareTo(VectorPointPair vp) {
		if (vp == null) return 1;
		if (vp.vector == null) return 1;
		if (vp.point == null) return 1;
		int d = vector.compareTo(vp.vector);
		if (d != 0) return d;
		return (point.compareTo(vp.point));
	}

	public boolean equals(Object o) {
		if (o == null) return false;
		if (!(o instanceof VectorPointPair)) return false;
		return (compareTo((VectorPointPair)o) == 0);
	}
	
	public String toString() {
		return "VPP("+vector+","+point+","+index+")";
	}
	
	public Vector getVector() { return vector; }
	public Point getPoint() { return point; }
	public Integer getIndex() {return index; }
}

