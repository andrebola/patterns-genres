package siatec;

import java.util.ArrayList;

public class CompactnessTrawler {
	public static ArrayList<MtpCisPair> trawl(ArrayList<MtpCisPair> mtpCisPairs, double a, int b) {
//		System.out.println("\ntrawl called with "+mtpCisPairs.size()+" mtpCisPairs and a = "+a+", b = "+b);
		ArrayList<MtpCisPair> newPatternCisPairs = new ArrayList<MtpCisPair>();
		for(MtpCisPair mtpCisPair : mtpCisPairs) {
//			System.out.println("Processing "+mtpCisPair);
			MtpCisPair Q = new MtpCisPair();
			for(Vector v : mtpCisPair.getVectorSet().getVectors())
				Q.addVector(v);
			Point[] P = new Point[mtpCisPair.getMtp().size()];
			mtpCisPair.getMtp().getPoints().toArray(P);
			Integer[] IL = new Integer[mtpCisPair.getCis().size()];
			mtpCisPair.getCis().toArray(IL);
			Integer iL0 = null, s = null;
			for(int i = 0; i < P.length; i++) {
				if (Q.getCis().isEmpty()) {
					Q.add(P[i], IL[i]);
//					System.out.println("First element added to Q: "+Q);
					iL0 = IL[i];
				} else {
					s = IL[i] - iL0 + 1;
					if ((1.0* Q.getCis().size() + 1.0)/s >= a) {
						Q.add(P[i], IL[i]);
//						System.out.println("Another element added to Q: "+Q);
					} else if (Q.getCis().size() >= b) {
//						System.out.println("Adding Q="+Q);
						newPatternCisPairs.add(Q);
						Q = new MtpCisPair();
						for(Vector v : mtpCisPair.getVectorSet().getVectors())
							Q.addVector(v);
					} else {
//						System.out.println("The trawled set, "+Q+", is not large enough");
						Q = new MtpCisPair();
						for(Vector v : mtpCisPair.getVectorSet().getVectors())
							Q.addVector(v);
					}
				} 
			}
			if (Q.getCis().size() >= b) {
//				System.out.println("Final Q is large enough: "+Q);
				newPatternCisPairs.add(Q);
			}
		}
		return newPatternCisPairs;
	}
}
