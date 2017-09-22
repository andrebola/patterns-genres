package siatec;

import java.util.ArrayList;
import java.util.TreeSet;

public class VectorSet implements Comparable<VectorSet> {
	private TreeSet<Vector> vectors = new TreeSet<Vector>();
	
	public VectorSet(Vector... vectors) {
		for(Vector vector : vectors)
			this.vectors.add(vector);
	}
	
	public void add(Vector vector) {
		vectors.add(vector);
	}
	
	public VectorSet copy() {
		VectorSet newVectorSet = new VectorSet();
		for(Vector vector : vectors)
			newVectorSet.add(vector.copy());
		return newVectorSet;
	}
	
	public TreeSet<Vector> getVectors() {
		return vectors;
	}
	
	public boolean contains(Vector vector) {
		return vectors.contains(vector);
	}
	
	public boolean remove(Vector vector) {
		return vectors.remove(vector);
	}
	
	public int size() {
		return getVectors().size();
	}
	
	public boolean isEmpty() {
		return vectors.isEmpty();
	}
	
	public String toString() {
		if (isEmpty()) return "V()";
		StringBuilder sb = new StringBuilder("V("+vectors.first());
		for(Vector vector : vectors.tailSet(vectors.first(), false))
			sb.append(","+vector);
		sb.append(")");
		return sb.toString();
	}
	
	public Vector get(int i) {
		ArrayList<Vector> array = new ArrayList<Vector>(vectors);
		return array.get(i);
	}

	public VectorSet remove(VectorSet removableSubset) {
		for(Vector vector : removableSubset.getVectors())
			vectors.remove(vector);
		return this.copy();
	}
	
	public VectorSet translate(Vector vector) {
		VectorSet vectorSet = new VectorSet();
		for(Vector v : vectors) vectorSet.add(v.add(vector));
		return vectorSet;
	}

	@Override
	public int compareTo(VectorSet vs) {
		if (vs == null) return 1;
		int d = size() - vs.size();
		if (d != 0) return d;
		//Same size
		for(int i = 0; i < size(); i++) {
			d = get(i).compareTo(vs.get(i));
			if (d != 0) return d;
		}
		return 0;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof VectorSet)) return false;
		return compareTo((VectorSet)obj) == 0;
	}
	
	public static VectorSet getVectorSetFromString(String l) {
		int startIndex = l.indexOf("v(");
		int endIndex = l.indexOf("))",startIndex)+1;
		String vectorSequence = l.substring(startIndex,endIndex);
		VectorSet outputVectorSet = new VectorSet();
		for(int start = 0; start < vectorSequence.length();) {
			int end = vectorSequence.indexOf(")",start)+1;
			String vectorString = vectorSequence.substring(start, end);
			Vector vector = new Vector(vectorString);
			outputVectorSet.add(vector);
			start = end + 1;
		}
		return outputVectorSet;
	}

	public void addAll(VectorSet vectorSet) {
		for(Vector vector : vectorSet.getVectors()) {
			add(vector);
		}
	}
	
//	public static VectorSet getVectorSetFromString(String vectorSetString) {
//		VectorSet vectorSet = new VectorSet();
//		String[] vectorStrings = vectorSetString.substring(vectorSetString.indexOf('v'),vectorSetString.lastIndexOf(')')).split(",");
//		for(String vectorString : vectorStrings)
//			vectorSet.add(new Vector(vectorString));
//		return vectorSet;
//
//	}
}
