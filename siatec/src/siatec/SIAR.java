package siatec;

import java.util.ArrayList;
import java.util.Collections;
import java.util.TreeSet;

public class SIAR {

	static public ArrayList<MtpCisPair> run(PointSet dataset, int r) {
		//Compute r subdiagonals of vector table and store in VSet
		TreeSet<VectorPointPair> VSet = new TreeSet<VectorPointPair>();
		for(int i = 0; i < dataset.size()-1; i++) {
			int j = i+1;
			while (j < dataset.size() && j <= i + r) {
				VSet.add(new VectorPointPair(new Vector(dataset.get(i),dataset.get(j)),dataset.get(i)));
				j++;
			}
		}

//		System.out.println("VSet.size()="+VSet.size());
		//Make a corresponding array to VSet
		ArrayList<VectorPointPair> VArray = new ArrayList<VectorPointPair>(VSet);
//		System.out.println("VArray.size()="+VArray.size());

		// Store non-maximal translatable patterns in E by sorting and segmenting V
		ArrayList<PointSet> E = new ArrayList<PointSet>();
		Vector v = VArray.get(0).getVector();
		PointSet e = new PointSet(VArray.get(0).getPoint());
		for(int i = 1; i < VArray.size(); i++) {
			if (VArray.get(i).getVector().equals(v))
				e.add(VArray.get(i).getPoint());
			else {
				E.add(e);
				e = new PointSet(VArray.get(i).getPoint());
				v = VArray.get(i).getVector();
			}
		}
		E.add(e);
//		System.out.println("E.size()="+E.size());

		//For each pattern in E, find +ve inter-point vectors and store in L
		ArrayList<Vector> L = new ArrayList<Vector>();
		for(int i = 0; i < E.size(); i++) {
			e = E.get(i);
//			System.out.println("  e.size()="+e.size());
			for(int j = 0; j < e.size()-1; j++) {
				for(int k = j+1; k < e.size(); k++) {
					L.add(new Vector(e.get(j),e.get(k)));
				}
			}
		}

//		System.out.println("L.size()="+L.size());

		//Remove duplicates from L and order vectors by decreasing frequency of occurrence

		Collections.sort(L);
//		System.out.println("L.size()="+L.size());

		TreeSet<VectorFrequencyPair> M = new TreeSet<VectorFrequencyPair>();
		if (L.size() > 0) {
			v = L.get(0);
			int f = 1;
			for(int i = 1; i < L.size(); i++) {
				if (L.get(i).equals(v))
					f++;
				else {
					M.add(new VectorFrequencyPair(v,f));
					f = 1;
					v = L.get(i);
				}
			}
			M.add(new VectorFrequencyPair(v, f));
		}


		ArrayList<MtpCisPair> mtpCisPairs = new ArrayList<MtpCisPair>();
		for(VectorFrequencyPair vf : M) {
			PointSet pattern = dataset.intersection(dataset.translate(vf.getVector().inverse()));
			VectorSet vectorSet = new VectorSet(vf.getVector());
			MtpCisPair mtpCisPair = new MtpCisPair(pattern,dataset,vectorSet);
			mtpCisPairs.add(mtpCisPair);
		}

		return mtpCisPairs;
	}

}
