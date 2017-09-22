package siatec;

import java.util.Comparator;

class TECPointSetSizeComparator implements Comparator<TEC> {

	@Override
	public int compare(TEC pc1, TEC pc2) {
		PointSet s1 = pc1.getPattern();
		PointSet s2 = pc2.getPattern();
		if (s1 == null && s2 == null) return 0;
		if (s2 == null) return -1;
		if (s1 == null) return 1;
		int d = s2.size() - s1.size();
		return d;
	}

}

