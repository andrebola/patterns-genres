package siatec;

import java.util.TreeSet;


public class PointFreq implements Comparable<PointFreq> {

	private Point point;
	private int freq;
	private boolean isMaxPoint = false;
	private TreeSet<Vector> translators = new TreeSet<Vector>();

	public PointFreq(Point point, int freq) {
		super();
		setPoint(point);
		setFreq(freq);
	}

	public PointFreq(Point point, int freq, Vector vector) {
		super();
		setPoint(point);
		setFreq(freq);
		translators.add(vector);
	}

	public boolean isMaxPoint() {
		return isMaxPoint;
	}
	
	public void setMaxPoint(boolean isMaxPoint) {
		this.isMaxPoint = isMaxPoint;
	}
	
	public TreeSet<Vector> getTranslators() {
		return translators;
	}
	
	public void addTranslator(Vector translator) {
		translators.add(translator);
	}
	
	public int compareTo(PointFreq pf) {
		return getPoint().compareTo(pf.getPoint());
	};

	public boolean equals(Object obj) {
		if (!(obj instanceof PointFreq)) return false;
		return compareTo((PointFreq) obj) == 0;
	}

	public Point getPoint() {
		return point;
	}

	public void setPoint(Point point) {
		this.point = point;
	}

	public int getFreq() {
		return freq;
	}

	public void setFreq(int freq) {
		this.freq = freq;
	}
	
	public void incFreq() {
		this.freq++;
	}

	@Override
	public String toString() {
		return "PointFreq("+point+","+freq+")";
	}
}

