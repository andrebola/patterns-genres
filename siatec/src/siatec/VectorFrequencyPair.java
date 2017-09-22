package siatec;

public class VectorFrequencyPair implements Comparable<VectorFrequencyPair>{
	Vector vector;
	int frequency;
	
	public Vector getVector() {
		return vector;
	}
	
	public void setVector(Vector vector) {
		this.vector = vector;
	}
	
	public int getFrequency() {
		return frequency;
	}
	
	public void setFrequency(int frequency) {
		this.frequency = frequency;
	}
	
	public VectorFrequencyPair(Vector vector, int frequency) {
		super();
		this.vector = vector;
		this.frequency = frequency;
	}

	@Override
	public int compareTo(VectorFrequencyPair o) {
		if (o == null) return 1;
		int d = o.getFrequency() - getFrequency(); //Sorts descending by frequency
		if (d != 0) return d;
		//Frequencies are the same
		if (getVector() == null) {
			if (o.getVector() == null) return 0;
			return -1;
		}
		//So vector is not null
		if (o.getVector() == null) return 1;
		return getVector().compareTo(o.getVector());
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		//obj is not null
		if (!(obj instanceof VectorFrequencyPair)) return false;
		return compareTo((VectorFrequencyPair)obj) == 0;
	}

	@Override
	public String toString() {
		return "VectorFrequencyPair(" + vector + ", "
				+ frequency + ")";
	}
	
}
