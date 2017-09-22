package siatec;

public class VectorVectorPair implements Comparable<VectorVectorPair> {
	private Vector differenceVector, originVector;

	public Vector getDifferenceVector() {return differenceVector; }
	public void setDifferenceVector(Vector differenceVector) {this.differenceVector = differenceVector; }
	public Vector getOriginVector() {return originVector; }
	public void setOriginVector(Vector originVector) {this.originVector = originVector; }

	public VectorVectorPair(Vector differenceVector, Vector originVector) {
		setDifferenceVector(differenceVector);
		setOriginVector(originVector);
	}
	
	public int compareTo(VectorVectorPair otherVVPair) {
		if (otherVVPair == null) return 1;
		int d = getDifferenceVector().compareTo(otherVVPair.getDifferenceVector());
		if (d != 0) return d;
		return getOriginVector().compareTo(otherVVPair.getOriginVector());
	}
	
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof VectorVectorPair)) return false;
		return compareTo((VectorVectorPair)obj) == 0;
	}

	public String toString() {return "VectorVectorPair("+differenceVector+","+originVector+")"; }

}

