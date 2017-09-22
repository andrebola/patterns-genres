package siatec;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.NavigableSet;
//import java.util.NavigableSet;
import java.util.TreeSet;

public class SIA {

	public static int TOTAL_NUMBER_OF_MTPs = 0;

	public static VectorPointPair[][] computeVectorTable(PointSet points) {
		return computeVectorTable(points,null);
	}

	public static VectorPointPair[][] computeVectorTable(PointSet points, PrintStream logPrintStream) {
		//LogPrintStream.print(logPrintStream, "computeVectorTable...");
		TreeSet<Point> pointsTreeSet = points.getPoints();
		VectorPointPair[][] vectorTable = new VectorPointPair[points.size()][points.size()];
		int i = 0;
		for(Point p1 : pointsTreeSet) {
			int j = 0;
			for(Point p2 : pointsTreeSet) {
				VectorPointPair vp = new VectorPointPair(p1,p2,i);
				vectorTable[i][j] = vp;
				j++;
			}
			i++;
		}

		//LogPrintStream.println(logPrintStream, "completed");
		return vectorTable;
	}


	public static ArrayList<MtpCisPair> run(
			PointSet points,
			VectorPointPair[][] vectorTable,
			boolean forRSuperdiagonals, int r,
			boolean withCompactnessTrawler, double a, int b,
			PrintStream logPrintStream,
			boolean removeTranslationallyEquivalentMtps,
			boolean mergeVectors,
			int minPatternSize) throws IllegalArgumentException {
		if (mergeVectors && removeTranslationallyEquivalentMtps)
			throw new IllegalArgumentException("Cannot merge vectors and remove translationally equivalent Mtps");
		ArrayList<MtpCisPair> mtpCisPairs = null;
		if (forRSuperdiagonals)
			mtpCisPairs = SIAR.run(points, r);
		else {
			//LogPrintStream.print(logPrintStream,"computeMtpCisPairs...");
			mtpCisPairs = computeMtpCisPairs(vectorTable,0);
			TOTAL_NUMBER_OF_MTPs = mtpCisPairs.size();
			//LogPrintStream.println(logPrintStream,"completed: "+mtpCisPairs.size()+" MTPs found");
		}
		if (minPatternSize > 0) {
			//Remove all patterns smaller than minPatternSize
			ArrayList<MtpCisPair> newMtpCisPairs = new ArrayList<MtpCisPair>();
			for(MtpCisPair mtpCisPair : mtpCisPairs)
				if (mtpCisPair.getMtp().size() >= minPatternSize)
					newMtpCisPairs.add(mtpCisPair);
			mtpCisPairs = newMtpCisPairs;
		}
		if (mergeVectors) {
			mtpCisPairs = mergeVectors(mtpCisPairs);
		}
		if (removeTranslationallyEquivalentMtps) {
			//LogPrintStream.print(logPrintStream,"removeTranslationallyEquivalentMtps");
			mtpCisPairs = VectorizedMtpCisPair.removeTranslationallyEquivalentMtps(mtpCisPairs);
		}
		if (withCompactnessTrawler) {
			mtpCisPairs = CompactnessTrawler.trawl(mtpCisPairs, a, b);
			if (!mtpCisPairs.isEmpty())
				mtpCisPairs = VectorizedMtpCisPair.removeTranslationallyEquivalentMtps(mtpCisPairs);
		}
		return mtpCisPairs;
	}

	public static ArrayList<MtpCisPair> mergeVectors(ArrayList<MtpCisPair> mtpCisPairs) {
		Collections.sort(mtpCisPairs);
		ArrayList<MtpCisPair> newMtpCisPairs = new ArrayList<MtpCisPair>();
		PointSet pattern = mtpCisPairs.get(0).getMtp();
		ArrayList<Integer> cis = mtpCisPairs.get(0).getCis();
		VectorSet vectorSet = mtpCisPairs.get(0).getVectorSet().copy();
		for(int i = 1; i < mtpCisPairs.size(); i++) {
			MtpCisPair mtpCisPair = mtpCisPairs.get(i);
			if (mtpCisPair.getMtp().equals(pattern)) {
				vectorSet.addAll(mtpCisPair.getVectorSet());
			} else { // patterns are not equal
				newMtpCisPairs.add(new MtpCisPair(pattern,cis,vectorSet));
				pattern = mtpCisPair.getMtp();
				cis = mtpCisPair.getCis();
				vectorSet = mtpCisPair.getVectorSet().copy();
			}
		}
		newMtpCisPairs.add(new MtpCisPair(pattern,cis,vectorSet));
		return newMtpCisPairs;
	}

	public static ArrayList<MtpCisPair> computeMtpCisPairs(VectorPointPair[][] vectorTable, int minMtpSize) {

		//Using a direct address table (array) to store MTPs

//		int maxTime = (int)(vectorTable[vectorTable.length-1][0].getPoint().getX()+1);
//		int minPitch, maxPitch, i;
//		if (vectorTable.length%2==0) {
//			minPitch = vectorTable[0][0].getPoint().getY();
//			maxPitch = vectorTable[1][0].getPoint().getY();
//			if (maxPitch < minPitch) {
//				int temp = maxPitch;
//				maxPitch = minPitch;
//				minPitch = temp;
//			}
//			i = 2;
//		} else {
//			minPitch = vectorTable[0][0].getPoint().getY();
//			maxPitch = vectorTable[0][0].getPoint().getY();
//			i = 1;
//		}
//		for(; i < vectorTable.length-1; i += 2) {
//			int first = vectorTable[i][0].getPoint().getY();
//			int second = vectorTable[i+1][0].getPoint().getY();
//			if (first < second) {
//				if (first < minPitch)
//					minPitch = first;
//				if (second > maxPitch)
//					maxPitch = second;
//			} else {
//				if (second < minPitch)
//					minPitch = second;
//				if (first > maxPitch)
//					maxPitch = first;
//			}
//		}
//
//		MtpCisPair[][] posVecs = new MtpCisPair[maxTime][maxPitch-minPitch+1];
//		MtpCisPair[][] negVecs = new MtpCisPair[maxTime][maxPitch-minPitch+1];
//		MtpCisPair[][] vecs = null;
//		ArrayList<MtpCisPair> mtpCisPairs = new ArrayList<MtpCisPair>();
//		for(int k = 0; k < vectorTable.length-1; k++)
//			for(int j = k+1; j < vectorTable.length; j++) {
//				int x = (int)(vectorTable[k][j].getVector().getX());
//				int y = (vectorTable[k][j].getVector().getY());
//				if (y < 0) {
//					vecs = negVecs;
//					y = -y;
//				} else
//					vecs = posVecs;
//				if (vecs[x][y]==null) {
//					PointSet mtp = new PointSet(vectorTable[k][j].getPoint());
//					ArrayList<Integer> cis = new ArrayList<Integer>();
//					cis.add(vectorTable[k][j].getIndex());
//					Vector vector = vectorTable[k][j].getVector();
//					vecs[x][y] = new MtpCisPair(mtp, cis, vector);
//					mtpCisPairs.add(vecs[x][y]);
//				} else {
//					vecs[x][y].getMtp().add(vectorTable[k][j].getPoint());
//					vecs[x][y].getCis().add(vectorTable[k][j].getIndex());
//				}
//			}
//
//		ArrayList<MtpCisPair> finalMtpCisPairs = new ArrayList<MtpCisPair>();
//		for(MtpCisPair mtpCisPair: mtpCisPairs)
//			if (mtpCisPair.getMtp().size() >= minMtpSize)
//				finalMtpCisPairs.add(mtpCisPair);
//		return finalMtpCisPairs;
		
//		Following uses comparison sort to partition vectors
				TreeSet<VectorPointPair> sortedSIAVectorTable = new TreeSet<VectorPointPair>();
				for(int i = 0; i < vectorTable.length; i++)
					for(int j = i+1; j < vectorTable.length; j++)
						sortedSIAVectorTable.add(vectorTable[i][j]);
		
				ArrayList<PointSet> MTPs = new ArrayList<PointSet>();
				ArrayList<ArrayList<Integer>> CISs = new ArrayList<ArrayList<Integer>>();
				ArrayList<Vector> vectors = new ArrayList<Vector>();
				VectorPointPair firstVP = sortedSIAVectorTable.first(); 
				Vector v = firstVP.getVector();
				PointSet mtp = new PointSet();
				ArrayList<Integer> cis = new ArrayList<Integer>();
				mtp.add(firstVP.getPoint());
				cis.add(firstVP.getIndex());
				NavigableSet<VectorPointPair> rest = sortedSIAVectorTable.tailSet(firstVP, false); 
				for(VectorPointPair vp : rest) {
					if (vp.getVector().equals(v)) {
						mtp.add(vp.getPoint());
						cis.add(vp.getIndex());
					} else {
						if (mtp.size() >= minMtpSize) {
							MTPs.add(mtp);
							CISs.add(cis);
							vectors.add(v);
						}
						mtp = new PointSet();
						cis = new ArrayList<Integer>();
						v = vp.getVector();
						mtp.add(vp.getPoint());
						cis.add(vp.getIndex());
					}
				}		
		
				if (mtp.size() >= minMtpSize) {
					MTPs.add(mtp);
					CISs.add(cis);
					vectors.add(v);
				}
		
				ArrayList<MtpCisPair> mtpCisPairs = new ArrayList<MtpCisPair>();
		
				for(int i = 0; i < MTPs.size(); i++) {
					mtpCisPairs.add(new MtpCisPair(MTPs.get(i),CISs.get(i),vectors.get(i)));
				}
		
				return mtpCisPairs;
		
	}

	public static void main(String[] args) {
		PointSet dataset = new PointSet(
				new Point(0,0), new Point(1,1), new Point(1,0), new Point(0,1), 
				new Point(3,3), new Point(3,4), new Point(4,3), new Point(4,4),
				new Point(6,0), new Point(6,1), new Point(7,0), new Point(7,1));
		VectorPointPair[][] vectorTable = computeVectorTable(dataset);
		ArrayList<MtpCisPair> mtpCisPairs = computeMtpCisPairs(vectorTable,0);
		System.out.println(dataset);
		for(MtpCisPair mtpCisPair : mtpCisPairs)
			System.out.println(mtpCisPair);
		mtpCisPairs = run(
				dataset,
				vectorTable,
				false, //forRSuperdiagonals
				0, //r
				false, // withCompactnessTrawler
				0.0, //a
				0, //b
				null, //logPrintStream
				false, //removeTranslationallyEquivalentMtps
				true, //mergeVectors
				0 //minPatternSize
				);
		System.out.println("Output of run");
		for(MtpCisPair mtpCisPair : mtpCisPairs)
			System.out.println(mtpCisPair);
	}

}
