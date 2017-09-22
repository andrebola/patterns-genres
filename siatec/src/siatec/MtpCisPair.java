package siatec;

import java.util.ArrayList;

public class MtpCisPair implements Comparable<MtpCisPair>{
	private PointSet mtp;
	private ArrayList<Integer> cis;
	private VectorSet vectorSet;
	
	public MtpCisPair(PointSet mtp, ArrayList<Integer> cis, Vector vector) {
		this.mtp = mtp;
		this.cis = cis;
		addVector(vector);
	}
	
	public MtpCisPair(PointSet mtp, ArrayList<Integer> cis, VectorSet vectorSet) {
		this.mtp = mtp;
		this.cis = cis;
		this.vectorSet = vectorSet;
	}

	public MtpCisPair() {
		mtp = new PointSet();
		cis = new ArrayList<Integer>();
	}
	
	/**
	 * Constructs an MtpCisPair from the pattern, pattern,
	 * and the dataset, dataset. The column index set (CIS)
	 * contains the indices of the points in the pattern
	 * in the lexicographically sorted dataset.
	 * 
	 * @param pattern
	 * @param dataset
	 */
	public MtpCisPair(PointSet pattern, PointSet dataset) {
		mtp = pattern.copy();
		cis = dataset.getIndexSet(pattern);
	}

	public MtpCisPair(PointSet pattern, PointSet dataset, VectorSet vectorSet) {
		this(pattern,dataset);
		this.vectorSet = vectorSet.copy();
	}

	public void add(Point point, Integer index) {
		mtp.add(point);
		cis.add(index);
	}
	
	public PointSet getMtp() {return mtp;}
	public ArrayList<Integer> getCis() {return cis;}
	public VectorSet getVectorSet() {return vectorSet;}
	
	public void addVector(Vector vector) {
		if (vectorSet == null)
			vectorSet = new VectorSet();
		vectorSet.add(vector);
	}
	
	public void addVectorSet(VectorSet vectorSet) {
		if (vectorSet == null) 
			vectorSet = new VectorSet();
		for(Vector vector : vectorSet.getVectors())
			vectorSet.add(vector);
	}
	
	@Override
	public String toString() {
		return "MtpCisPair("+mtp+","+cis+","+vectorSet+")";
	}

	@Override
	public int compareTo(MtpCisPair mcp) {
		if (mcp == null) return 1;
		return getMtp().compareTo(mcp.getMtp());
	}

	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof MtpCisPair)) return false;
		return compareTo((MtpCisPair)obj) == 0;
	}
}
