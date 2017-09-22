package siatec;

public class Vector implements Comparable<Vector>{
	private long x;
	private int y;
	
	public Vector(long x, int y) {
		setX(x);
		setY(y);
	}
	
	public Vector(int x, int y) {
		setX(x);
		setY(y);
	}
	
	public Vector(Point p, Point q) {
		setX(q.getX()-p.getX());
		setY(q.getY()-p.getY());
	}
	
	/**
	 * Assuming s has the format "v(<x>,<y>)"
	 * where <x> and <y> are integers.
	 * @param s
	 */
	public Vector(String s) {
		int start = s.indexOf('(')+1;
		int end = s.indexOf(",");
		setX(Long.parseLong(s.substring(start,end)));
		start = end + 1;
		end = s.indexOf(")");
		setY(Integer.parseInt(s.substring(start,end)));
	}
	
	private void setX(int x) {
		this.x = x;
	}
	
	private void setX(long x) {
		this.x = x;
	}
	
	private void setY(int y) {
		this.y = y;
	}
	
	public long getX() {
		return x;
	}
	
	public int getY() {
		return y;
	}
	
	public String toString() {
		return "v("+x+","+y+")";
	}
	
	@Override
	public int compareTo(Vector v) {
		if (v == null) return 1;
		long d = getX() - v.getX();
		if (d < 0l) return -1;
		if (d > 0l) return 1;
		return getY() - v.getY();
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof Vector)) return false;
		return compareTo((Vector)obj) == 0;
	}
	
	public Vector copy() {
		return new Vector(x,y);
	}
	
	public Vector inverse() {
		return new Vector(-x,-y);
	}
	
	public Vector add(Vector v) {
		return new Vector(x+v.x,y+v.y);
	}

	public Vector minus(Vector v) {
		return new Vector(x-v.x,y-v.y);
	}
	
	public double getLength() {
		return Math.sqrt((1.0 * getX() * getX()) + (1.0 * getY() * getY()));
	}

}
