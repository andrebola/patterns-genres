package siatec;

import java.util.ArrayList;
import java.util.TreeSet;

public class VectorizedMtpCisPair implements Comparable<VectorizedMtpCisPair> {

	private MtpCisPair mtpCisPair;
	private VectorSet vectorizedRepresentation = new VectorSet();

	public MtpCisPair getMtpCisPair() {
		return mtpCisPair;
	}

	public VectorSet getVectorizedRepresentation() {
		return vectorizedRepresentation;
	}

	public VectorizedMtpCisPair(MtpCisPair mtpCisPair) {
		this.mtpCisPair = mtpCisPair;
		//		System.out.println("\n  Computing VectorizedMtpCisPair for "+mtpCisPair);
		computeVectorizedRepresentation();
	}

	private void computeVectorizedRepresentation() {
		PointSet mtp = getMtpCisPair().getMtp();

		Point firstPoint = mtp.first();
		for(int i = 1; i < mtp.size(); i++)
			vectorizedRepresentation.add(new Vector(firstPoint, mtp.get(i)));
	}

	@Override
	public int compareTo(VectorizedMtpCisPair o) {
		if (o == null) return 1;
		int d = getVectorizedRepresentation().compareTo(o.getVectorizedRepresentation());
		if (d != 0) return d;
		return getMtpCisPair().compareTo(o.getMtpCisPair());
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof VectorizedMtpCisPair)) return false;
		return compareTo((VectorizedMtpCisPair)obj) == 0;
	}


	public static ArrayList<MtpCisPair> removeTranslationallyEquivalentMtps(ArrayList<MtpCisPair> mtpCisPairs) {

		TreeSet<VectorizedMtpCisPair> vectorizedMtps = new TreeSet<VectorizedMtpCisPair>();
		System.out.print("  Computing vectorizedMtps...");
		for(MtpCisPair mtpCisPair : mtpCisPairs)
			vectorizedMtps.add(new VectorizedMtpCisPair(mtpCisPair));
		System.out.println("DONE: "+vectorizedMtps.size()+" vectorized MTPs created");

		/*
		 * So now all the VectorizedMtpCisPairs are sorted
		 * so that translationally equivalent Mtps are adjacent to each other.
		 * We scan the list once, each time the vectorized representation changes
		 * we put the MtpCisPair into the output list.
		 */

		System.out.print("Now computing tran distinct MTPs");
		ArrayList<MtpCisPair> outputList = new ArrayList<MtpCisPair>();
		if (!vectorizedMtps.isEmpty()) {
			VectorSet u = vectorizedMtps.first().getVectorizedRepresentation();
			outputList.add(vectorizedMtps.first().getMtpCisPair());
			int i = 0;
			for(VectorizedMtpCisPair vectorizedMtp : vectorizedMtps) {
				i++;
				if (i % 100 == 0) System.out.print("."); System.out.flush();
				if (i %5000 == 0) System.out.println();
				if (!vectorizedMtp.getVectorizedRepresentation().equals(u)) {
					outputList.add(vectorizedMtp.getMtpCisPair());
					u = vectorizedMtp.getVectorizedRepresentation();
				}
			}
		}
		return outputList;
	}


}
