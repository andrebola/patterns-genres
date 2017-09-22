package siatec;

public class PatternVectorSetPair implements Comparable<PatternVectorSetPair> {
	private PointSet mtp = new PointSet();
	private VectorSet vectorSet = new VectorSet();
	
	public PatternVectorSetPair(PointSet mtp, VectorSet vectorSet) {
		setMtp(mtp);
		setVectorSet(vectorSet);
	}
	
	public PointSet getMtp() {
		return mtp;
	}
	
	public VectorSet getVectorSet() {
		return vectorSet;
	}
	
	public void setMtp(PointSet mtp) {
		this.mtp = mtp;
	}
	
	public void setVectorSet(VectorSet vectorSet) {
		this.vectorSet = vectorSet;
	}
	
	@Override
	public int compareTo(PatternVectorSetPair o) {
		if (o == null) return 1;
		PatternVectorSetPair mvsp = (PatternVectorSetPair)o;
		int d = getMtp().compareTo(mvsp.getMtp());
		if (d != 0) return d;
		return getVectorSet().compareTo(mvsp.getVectorSet());
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof PatternVectorSetPair)) return false;
		return compareTo((PatternVectorSetPair)obj) == 0;
	}
	
	@Override
	public String toString() {
		return "PV("+getMtp()+","+getVectorSet()+")";
	}
}
