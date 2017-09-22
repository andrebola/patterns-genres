package siatec;

import java.util.ArrayList;
import java.util.TreeSet;

public class TEC implements Comparable<TEC>{
	private PointSet pattern = null;
	private ArrayList<Integer> cis = null;
	private VectorSet translators = new VectorSet();
	private Double compressionRatio = null;
	private Integer coverage = null;
	private PointSet coveredPoints = null;
	private Double compactness = null;
	private Integer numPointsInBB = null;
	private PointSet dataset = null;
	private boolean isDual = false;

	public TEC() {}

	public TEC(PointSet pattern, PointSet dataset) {
		this.pattern = pattern;
		this.dataset = dataset;
		translators.add(new Vector(0,0));
	}

	public TEC(PointSet pointSet, ArrayList<Integer> cis, PointSet dataSet) {
		this.pattern = pointSet;
		this.cis = cis;
		this.dataset = dataSet;
	}

	public TEC(PointSet pattern, VectorSet translators, PointSet dataset) {
		this.pattern = pattern;
		this.translators = translators;
		this.dataset = dataset;
	}

	/**
	 * Parses a string representing a TEC.
	 * The String must have the format
	 * 
	 * T(P(p1,p2,...),V(v1,v2,...))
	 * 
	 * where pk = p(x,y) and vk = v(x,y)
	 * where x and y are integers.
	 * @param l
	 */
	public TEC(String l) {
		PointSet pointSet = PointSet.getPointSetFromString(l);
		VectorSet vectorSet = VectorSet.getVectorSetFromString(l);
//		System.out.println(vectorSet);
		this.pattern = pointSet;
		this.translators = vectorSet;
	}

	public void setPattern(PointSet pattern) {
		this.pattern = pattern;
		compressionRatio = null;
	}
	
	public boolean isDual() {
		return isDual;
	}

	public void setDual(boolean dual) {
		isDual = dual;
	}

	public ArrayList<Integer> getCIS() {
		return cis;
	}

	public PointSet getPattern() {
		return pattern;
	}

	public String toString() {
		PointSet normalizedPattern = pattern.translate(translators.get(0));
		VectorSet normalizedTranslators = translators.translate(translators.get(0).inverse());
		return "T("+normalizedPattern+","+normalizedTranslators+")";
	}

	public int getPatternSize() {
		if (pattern == null) return 0;
		return pattern.size();
	}

	public int getEncodingLength() {
		return getTranslators().size()-1+getPatternSize();
	}
	
	public int getTranslatorSetSize() {
		return translators.size();
	}

	public double getCompressionRatio() {
		if (compressionRatio != null) return compressionRatio;
		return (compressionRatio = (1.0*getCoverage())/getEncodingLength());
	}

	public PointSet getCoveredPoints() {
		if (coveredPoints == null) {
			coveredPoints = new PointSet();
			TreeSet<Point> points = pattern.getPoints();
			TreeSet<Vector> vectors = translators.getVectors();
			for(Point point : points)
				for(Vector vector : vectors)
					coveredPoints.add(point.translate(vector));
		}
		return coveredPoints;
	}

	public int getCoverage() {
		if (coverage != null) return coverage;
		coveredPoints = getCoveredPoints();
		return (coverage = coveredPoints.size());
	}

	public int getNumPointsInBB() {
		if (numPointsInBB != null) return numPointsInBB;
		Point tl = pattern.getTopLeft();
		Point br = pattern.getBottomRight();
		PointSet bbSubset = dataset.getBBSubset(tl,br);
		return (numPointsInBB = bbSubset.size());
	}

	public PointSet getDataset() {
		return dataset;
	}
	
	public double getCompactness() {
		if (compactness != null) return compactness;
		compactness = 0.0;
		for(Vector v : getTranslators().getVectors()) {
			double c = getPattern().translate(v).getCompactness(getDataset());
			if (c > compactness) compactness = c;
		}
		return compactness;
	}

	public VectorSet getTranslators() {
		return translators;
	}

	public long getBBArea() {
		return getPattern().getBBArea();
	}

	public TEC getDual() {
		TEC dual = new TEC();
		dual.pattern = new PointSet();
		Point firstPoint = pattern.first();
		dual.pattern.add(firstPoint);
		for(Vector v : translators.getVectors())
			dual.pattern.add(firstPoint.translate(v));
		for(Point p : pattern.getPoints())
			dual.translators.add(new Vector(firstPoint,p));
		dual.coverage = coverage;
		dual.compressionRatio = compressionRatio;
		dual.coveredPoints = coveredPoints;
		dual.dataset = dataset;
		dual.setDual(true);
		return dual;
	}

	public PointFreqSet getPointFreqSet() {
		PointFreqSet pfs = new PointFreqSet();
		for(Point p : pattern.getPoints())
			for(Vector v : translators.getVectors())
				pfs.addPoint(p.translate(v));
		return pfs;
	}

	/**
	 * Removes as many translators as it can from
	 * the translator set for this TEC such that
	 * set of covered points remains unchanged.
	 */
	public void removeRedundantTranslators() {

		PointFreqSet pfs = getPointFreqSet();

		/*
		 * If there are no multipoints in pfs,
		 * then we cannot remove any translators.
		 */

		if (pfs.getMultiPoints().isEmpty()) return;

		/*
		 * The set of translators that can be removed
		 * is a subset of those that map the pattern onto
		 * a subset of the multipoints.
		 * 
		 * We do SIAM on the pattern and the multipoints.
		 * We then only need to look at vectors that map
		 * the whole pattern onto multipoints - i.e.,
		 * vectors that have the same number of points
		 * as are in the pattern.
		 */

		TreeSet<VectorPointPair> siamTable = new TreeSet<VectorPointPair>();
		for(Point p1 : pattern.getPoints()) 
			for(PointFreq p2 : pfs.getMultiPoints()) 
				siamTable.add(new VectorPointPair(p1,p2.getPoint()));

		TreeSet<Vector> removableVectors = new TreeSet<Vector>();

		Vector v = siamTable.first().getVector();
		int count = 0;
		for(VectorPointPair vp : siamTable) {
			if (count == 0) v = vp.getVector();
			if (vp.getVector().equals(v)) {
				count++;
				if (count == pattern.size()) {
					count = 0;
					removableVectors.add(v);
				}
			} else {
				count = 1;
				v = vp.getVector();
			}
		}

		////////////////////////////////////////////////////////		

		/*
		 * Check to see if all removable vectors can be removed
		 */

		PointFreqSet remVecPFS = new PointFreqSet();

		TreeSet<PointFreq> maxPoints = new TreeSet<PointFreq>();

		boolean allCanBeRemoved = true;

		for(Vector vec : removableVectors)
			for(Point pnt : pattern.getPoints()) {
				Point newPoint = pnt.translate(vec);
				PointFreq pf = remVecPFS.addPoint(newPoint,vec);
				if (pf.getFreq() == pfs.getFreq(newPoint)) { 
					//Then we've removed all instances of this point
					allCanBeRemoved = false;
					pf.setMaxPoint(true);
					maxPoints.add(pf);
				}
			}
		if (allCanBeRemoved) {
			for(Vector vec : removableVectors)
				translators.remove(vec);
			return;
		}

		////////////////////////////////////////////////////////

		/*
		 * So we can't remove all of the removable vectors.
		 * 
		 * But we now know which vectors are responsible for 
		 * maxPoints in remVecPFS. For each maxPoint, we cannot
		 * remove all of the vectors that map points onto it.
		 * 
		 * We have to find the smallest set of vectors that includes
		 * at least one vector for each maxPoint.
		 * 
		 * We begin by making a list of <vector,pointSet> pairs. Each
		 * such pair gives, for each vector, the set of max points onto which
		 * that vector maps pattern points.
		 * 
		 * We can adopt a greedy strategy, where we start by choosing
		 * the vector that has the most max points attached to it.
		 * 
		 * We then remove the covered max points from all the remaining
		 * <vector,pointSet> pairs and re-sort the <vector,pointSet> pairs
		 * by pointSet size.
		 * 
		 * Repeat until all max points are covered.
		 * 
		 * This gives us a list of retained removable vectors. The other removable
		 * vectors can be removed.
		 */

		/* First find the set of <vector,point set> pairs. This will be sorted into
		 * decreasing order of size of point set.
		 */

		TreeSet<VectorPointSetPair> vectorMaxPointSetPairs = new TreeSet<VectorPointSetPair>();

		for(PointFreq pf : maxPoints) {
			for(Vector vec : pf.getTranslators()) {
				VectorPointSetPair vmpFloor = vectorMaxPointSetPairs.floor(new VectorPointSetPair(vec, new PointSet()));
				if (vmpFloor != null && vmpFloor.getVector().equals(vec)) {
					vmpFloor.addPoint(pf.getPoint());
				} else {
					vectorMaxPointSetPairs.add(new VectorPointSetPair(vec,new PointSet(pf.getPoint())));
				}
			}
		}

		/*
		 * Now we find the set of retained vectors. The first retained vector
		 * is the one whose set of max points is the largest - this should be the
		 * first vectorPointSetPair in vectorMaxPointSetPairs.
		 */

		TreeSet<Vector> retainedVectors = new TreeSet<Vector>();

		while(!vectorMaxPointSetPairs.isEmpty()) {
			VectorPointSetPair firstVPS = vectorMaxPointSetPairs.first();
			retainedVectors.add(firstVPS.getVector());

			for(Point maxPoint : firstVPS.getPointSet().getPoints()) {
				for(VectorPointSetPair vps : vectorMaxPointSetPairs.tailSet(firstVPS, false)) {
					vps.getPointSet().removeWithoutReset(maxPoint);
				}
			}

			TreeSet<VectorPointSetPair> newVectorPointSetPairs = new TreeSet<VectorPointSetPair>();
			for(VectorPointSetPair vps: vectorMaxPointSetPairs.tailSet(firstVPS, false)) {
				if (!vps.getPointSet().isEmpty())
					newVectorPointSetPairs.add(vps);
			}
			vectorMaxPointSetPairs = newVectorPointSetPairs;
		}

		////////////////////////////////////////////////////////

		/*
		 * Now remove all removable vectors apart from retained vectors.
		 */
		
		VectorSet newTranslators = new VectorSet();
		
		for(Vector vec : translators.getVectors()) {
			if (!removableVectors.contains(vec) || retainedVectors.contains(vec)){
				newTranslators.add(vec);
			}
		}
		
		translators = newTranslators;
		
		////////////////////////////////////////////////////////
		


	}

	public double getUnsquareness() {
		return pattern.getUnsquareness();
	}
	
	public void setDataset(PointSet dataset) {
		this.dataset = dataset;
	}
	
	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof TEC)) return false;
		return compareTo((TEC)obj) == 0;
	}

	@Override
	public int compareTo(TEC tec) {
		if (tec == null) return 1;
		int d = getPattern().compareTo(tec.getPattern());
		if (d != 0) return d;
		return getTranslators().compareTo(tec.getTranslators());
	}
	
	public ArrayList<PointSet> getPointSets() {
		ArrayList<PointSet> pointSets = new ArrayList<PointSet>();
		TreeSet<Vector> translators = getTranslators().getVectors();
		PointSet pattern = getPattern();
		for (Vector vector : translators) {
			pointSets.add(pattern.translate(vector));
		}
		return pointSets;
	}
	
}
