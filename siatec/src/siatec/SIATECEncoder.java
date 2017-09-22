/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;

import java.util.ArrayList;
import java.util.Collections;
import java.util.NavigableSet;
import java.util.TreeSet;

/**
 *
 * @author andres
 */
public class SIATECEncoder implements Encoder {
    
	private double minimumCompactness = 0.7;
	private int minimumPatternSize = 2;
	private double minimumTemporalDensity = 0.25;
	private double minimumTECOverlap = 0.3;

	@Override
	public Encoding encode(final PointSet points) {
		long t1 = System.currentTimeMillis();
		String thisEncoderName = this.getClass().toString();
		System.out.println(thisEncoderName + "\n=====================================\n");

		//Compute SIATEC vector table and sorted SIA vector table
		VectorPointPair[][] vectorTable = new VectorPointPair[points.size()][points.size()];
		TreeSet<VectorPointPair> sortedSIAVectorTable = new TreeSet<VectorPointPair>();
		Point[] pointsArray = new Point[points.size()];
		int k = 0;
		for(Point point : points.getPoints()) {
			pointsArray[k] = point;
			k++;
		}
		int n = points.size();
		for(int i = 0; i < n; i++) {
			for(int j = 0; j < n; j++) {
				VectorPointPair vp = new VectorPointPair(pointsArray[i],pointsArray[j],i);
				vectorTable[i][j] = vp;
				if (j > i) sortedSIAVectorTable.add(vp);
			}
		}
		//////////////////////////////

		//Compute MTPs and column index sets
		ArrayList<PointSet> MTPs = new ArrayList<PointSet>();
		ArrayList<ArrayList<Integer>> CISs = new ArrayList<ArrayList<Integer>>();
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
				MTPs.add(mtp);
				CISs.add(cis);
				mtp = new PointSet();
				cis = new ArrayList<Integer>();
				v = vp.getVector();
				mtp.add(vp.getPoint());
				cis.add(vp.getIndex());
			}
		}		

		System.out.println(MTPs.size()+" MTPs found");

		//////////////////////////////

		//Make list of PointSet-CIS pairs for MTPs

		ArrayList<TEC> mtpCISPairs = new ArrayList<TEC>();
		int m = MTPs.size();
		for(int i = 0; i < m; i++) {
			mtpCISPairs.add(new TEC(MTPs.get(i),CISs.get(i), points));
		}

		//////////////////////////////

		//Put the MTPs into order of size - then only have to compare each MTP 
		//with other MTPs of the same size when removing translationally 
		//equivalent MTPs.

		TECPointSetSizeComparator tecPointSetSizeComparator = new TECPointSetSizeComparator();
		Collections.sort(mtpCISPairs, tecPointSetSizeComparator);

		///////////////////////////////

		//Remove MTPs that are translationally equivalent to other MTPs
		//so that we don't have to compute the same TEC more than once.

		ArrayList<TEC> newTECList = new ArrayList<TEC>();
		TEC s1, s2;
		for(int i = 0; i < m; i++) {
			s1 = mtpCISPairs.get(i);
			int x = s1.getPattern().size();
			boolean found = false;
			for(int j = i + 1; !found && j < m && (s2 = mtpCISPairs.get(j)).getPattern().size() == x; j++) {
				if (s1.getPattern().translationallyEquivalentTo(s2.getPattern()))
					found = true;
			}
			if (!found)
				newTECList.add(s1);
		}

		System.out.println(newTECList.size()+" MTPs after removing translational equivalents");
		///////////////////////////////

		//Find TECs for MTPs

		m = newTECList.size();
		for(int i = 0; i < m; i++) {
			s1 = newTECList.get(i);
			Integer[] cols = new Integer[s1.getCIS().size()];
			s1.getCIS().toArray(cols);
			int patSize = s1.getCIS().size();
			int[] rows = new int[patSize];
			rows[0] = 0;
			while(rows[0] <= n - patSize) { //For each vector in the first pattern point column
				for(int j = 1; j < patSize; j++) rows[j] = rows[0]+j; //Initialize the indices for the other pattern point columns
				Vector v0 = vectorTable[cols[0]][rows[0]].getVector();
				boolean found = false;
				for(int col = 1; col < patSize; col++) { //For each pattern point
					while(rows[col] < n && vectorTable[cols[col]][rows[col]].getVector().compareTo(v0) < 0) rows[col]++; //Increment CI for this pattern point until >= v0
					if (rows[col] >= n || !v0.equals(vectorTable[cols[col]][rows[col]].getVector())) break; //If not equal then break
					if (col == patSize-1) found = true;
				}
				if (found) s1.getTranslators().add(v0);
				rows[0]++;
			}
		}

		///////////////////////////////

		//Sort TECs into decreasing order of quality

		TECQualityComparator tecQualityComparator = new TECQualityComparator();
		TreeSet<TEC> tecs = new TreeSet<TEC>(tecQualityComparator);

		for(TEC tec : newTECList) {
			TEC dual = tec.getDual();
			if (dual.getCompactness() >= minimumCompactness &&
				dual.getPattern().size() >= minimumPatternSize &&
				dual.getPattern().getTemporalDensity() >= minimumTemporalDensity)
				tecs.add(dual);
			if (tec.getCompactness() >= minimumCompactness &&
				tec.getPattern().size() >= minimumPatternSize &&
				tec.getPattern().getTemporalDensity() >= minimumTemporalDensity)
				tecs.add(tec);
		}

		System.out.println(tecs.size()+" TECs satisfy following conditions:");
		System.out.println("   Compactness >= "+minimumCompactness);
		System.out.println("   Pattern size >= "+minimumPatternSize);
		System.out.println("   Temporal density >= "+minimumTemporalDensity);
		
		////////////////////////////////////////////

		//Find set of TECs that covers whole dataset
		
		System.out.println("Finding TECs that cover this dataset");
		PointSet coveredSet = new PointSet();		
		ArrayList<TEC> newTECs = new ArrayList<TEC>();
		
		for(TEC tec : tecs) {
			//Find covered set for this tec
			PointSet tecCoveredSet = tec.getCoveredPoints();
			if (tecCoveredSet.diff(coveredSet).size() > minimumTECOverlap * tecCoveredSet.size()) {
				newTECs.add(tec);
				coveredSet.addAll(tecCoveredSet);
				if (coveredSet.size() == points.size())
					break;
			}
		}
		
		System.out.println("Found set of "+newTECs.size() + " TECs that cover "+coveredSet.size()+" out of "+points.size()+" points in the dataset");
		
		////////////////////////////////////////////
		
		//Draw TECs

		points.draw(newTECs);

		////////////////////////////////////////////

		long t2 = System.currentTimeMillis();
		System.out.println("\n"+(t2-t1)+" ms taken to analyse "+points.size()+" points");
		
		return new SIATECEncoderEncoding();
        }
}
