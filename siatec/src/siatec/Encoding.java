/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;


import java.awt.Dimension;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.TreeSet;

import javax.swing.JFrame;


public abstract class Encoding {
	abstract void draw();

	private String title = "";
	protected PointSet dataset = null;
	protected ArrayList<ArrayList<PointSet>> occurrenceSets = null;

	private ArrayList<TEC> tecs = new ArrayList<TEC>();
	public void setTECs(ArrayList<TEC> tecs) {this.tecs = tecs;}
	public ArrayList<TEC> getTECs() {return tecs;}
	public void addTEC(TEC tec) {this.tecs.add(tec);}

	private Long tatumsPerBar = null; 
	private Long barOneStartsAt = 0l;


	public void setTitle(String title) {
		this.title = title;
	}

	public String getTitle() {
		return title;
	}

	public Long getTatumsPerBar() {
		return tatumsPerBar;
	}
	public void setTatumsPerBar(Long tatumsPerBar) {
		this.tatumsPerBar = tatumsPerBar;
	}
	public Long getBarOneStartsAt() {
		return barOneStartsAt;
	}
	public void setBarOneStartsAt(Long barOneStartsAt) {
		this.barOneStartsAt = barOneStartsAt;
	}

	public ArrayList<ArrayList<PointSet>> getOccurrenceSets() {
		if (occurrenceSets == null) {
			occurrenceSets = new ArrayList<ArrayList<PointSet>>();
			for(TEC tec: getTECs())
				occurrenceSets.add(tec.getPointSets());
		}
		return occurrenceSets;
	}

	public ArrayList<ArrayList<PointSet>> getOccurrenceSets(int numberOfPatterns) {
		if (occurrenceSets == null) {
			occurrenceSets = new ArrayList<ArrayList<PointSet>>();
			ArrayList<TEC> tecs = getTECs();
			for(int i = 0; i < tecs.size() && i < numberOfPatterns; i++) {
				TEC tec = tecs.get(i);
				occurrenceSets.add(tec.getPointSets());
			}
		}
		return occurrenceSets;
	}


//	public void drawOccurrenceSets() {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				PApplet embed = new DrawPoints(dataset,getOccurrenceSets(),true);
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}
//
//	public void drawOccurrenceSets(final boolean diatonicPitch) {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				PApplet embed = new DrawPoints(dataset,getOccurrenceSets(),true,diatonicPitch,getTatumsPerBar(),getBarOneStartsAt(),getTitle());
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}
//
//	public void drawOccurrenceSets(final boolean diatonicPitch, final boolean printToPDF, final String pdfFilePath) {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				PApplet embed = new DrawPoints(
//						dataset,
//						getOccurrenceSets(),
//						true,
//						diatonicPitch,
//						getTatumsPerBar(),
//						getBarOneStartsAt(),
//						getTitle(),
//						printToPDF,
//						pdfFilePath);
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}
//
//	public void drawOccurrenceSets(final int numberOfPatterns) {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				PApplet embed = new DrawPoints(dataset,getOccurrenceSets(numberOfPatterns),true);
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}
//
//	public void drawOccurrenceSets(final String infoString) {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				PApplet embed = new DrawPoints(dataset,getOccurrenceSets(),true,infoString);
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}

	public void setTatumsPerBarAndBarOneStartsAt(String inputFilePath) {
		try {
			BufferedReader br = new BufferedReader(new FileReader(inputFilePath));
			String l;
			while((l = br.readLine()) != null) {
				if (l.trim().startsWith("%tatumsPerBar"))
					setTatumsPerBar(Long.parseLong(l.trim().split(" ")[1]));
				else if (l.trim().startsWith("%barOneStartsAt"))
					setBarOneStartsAt(Long.parseLong(l.trim().split(" ")[1]));
			}
			br.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Generates a new set of TECs by repeated pairwise merging of the old set of TECs. 
	 */
	public void mergeTECs(int minMatchSize, double minCompactness, int numIterations) {
		occurrenceSets = null;
		for(int k = 0; k < numIterations; k++) {
			TreeSet<TEC> newTECs = new TreeSet<TEC>();
			for(int i = 0; i < getTECs().size()-1; i++)
				for(int j = i+1; j < getTECs().size(); j++) {
					TEC tec1 = getTECs().get(i);
					TEC tec2 = getTECs().get(j);
					ArrayList<TEC> newTECsForThisPairOfOldTECs = mergeTECs(tec1,tec2,minMatchSize, minCompactness);
					if (newTECsForThisPairOfOldTECs.size() > 2)
						newTECs.add(newTECsForThisPairOfOldTECs.get(2));
					else {
						newTECs.add(tec1);
						newTECs.add(tec2);
					}
				}
			setTECs(new ArrayList<TEC>(newTECs)); 
			Collections.sort(getTECs(),new TECQualityComparator());
		}
	}

	private static class TECMatchPair extends TEC {
		private TreeSet<VectorVectorPair> match;
		public TreeSet<VectorVectorPair> getMatch() {return match; }
		public void setMatch(TreeSet<VectorVectorPair> match) {this.match = match;}

		public TECMatchPair(PointSet mergedPattern, VectorSet mergedTranslators, PointSet dataset, TreeSet<VectorVectorPair> maxMatch) {
			super(mergedPattern,mergedTranslators,dataset);
			setMatch(maxMatch);
		}

		public VectorSet getOriginVectors() {
			VectorSet vSet = new VectorSet();
			for(VectorVectorPair vvp : getMatch()) {
				vSet.add(vvp.getOriginVector());
			}
			return vSet;
		}

		public Vector getDifferenceVector() {
			return getMatch().first().getDifferenceVector();
		}
	}

	/**
	 * Merges tec1 and tec2 to produce a new merged tec, if 1. the size of the maximal match between 
	 * the vector sets of tec1 and tec2 is greater than or equal to minMatchSize; and 2. the
	 * compactness of the merged TEC is at least minCompactness. 
	 * 
	 * @param tec1 The first TEC to merge
	 * @param tec2 The second TEC to merge
	 * @param minMatchSize The minimum size of a match required to generate a new merged TEC
	 * @param minCompactness The minimum compactness tolerated in the merged TEC
	 * @return An ArrayList containing either two or three TECs. If it contains only two TECs, then the
	 * first TEC is the same as tec1 given as input and the second is the same as tec2 given as input.
	 * If the ArrayList contains three TECs, then the first TEC is the new replacement for tec1, the
	 * second is the new replacement for tec2 and the third is the new merged TEC.
	 */
	public ArrayList<TEC> mergeTECs(TEC tec1, TEC tec2, int minMatchSize, double minCompactness) {

		System.out.println("tec1: "+tec1);
		System.out.println("tec2: "+tec2);
		System.out.println();

		//		Find maximal match between translator sets for tec1 and tec2 using SIAM
		Vector[] tranArray1 = new Vector[tec1.getTranslatorSetSize()];
		Vector[] tranArray2 = new Vector[tec2.getTranslatorSetSize()];
		tec1.getTranslators().getVectors().toArray(tranArray1);
		tec2.getTranslators().getVectors().toArray(tranArray2);

		TreeSet<VectorVectorPair> vectorTable = new TreeSet<VectorVectorPair>();

		for(int i = 0; i < tranArray1.length; i++)
			for(int j = 0; j < tranArray2.length; j++)
				vectorTable.add(new VectorVectorPair(tranArray2[j].minus(tranArray1[i]),tranArray1[i]));

		//		Find maximal translatable vector patterns (i.e., maximal matches between vector sets)
		VectorVectorPair[] vvps = new VectorVectorPair[vectorTable.size()];
		vectorTable.toArray(vvps);
		ArrayList<TreeSet<VectorVectorPair>> maximalMatches = new ArrayList<TreeSet<VectorVectorPair>>();
		Vector thisDiffVector = vvps[0].getDifferenceVector();
		TreeSet<VectorVectorPair> thisMaxMatch = new TreeSet<VectorVectorPair>();
		thisMaxMatch.add(vvps[0]);
		for(int i = 1; i < vvps.length; i++) {
			VectorVectorPair thisVVP = vvps[i];
			if (thisVVP.getDifferenceVector().equals(thisDiffVector)) {
				thisMaxMatch.add(thisVVP);
			} else {
				maximalMatches.add(thisMaxMatch);
				thisDiffVector = thisVVP.getDifferenceVector();
				thisMaxMatch = new TreeSet<VectorVectorPair>();
				thisMaxMatch.add(thisVVP);
			}
		}
		maximalMatches.add(thisMaxMatch);

		//		Compute the merged TECs that correspond to the maximal matches
		ArrayList<TECMatchPair> mergedTECMatchPairs = new ArrayList<TECMatchPair>();
		for(TreeSet<VectorVectorPair> maxMatch : maximalMatches) {
			Vector firstOriginVector = maxMatch.first().getOriginVector();
			Vector diffVector = maxMatch.first().getDifferenceVector();
			VectorSet mergedTranslators = new VectorSet();
			for(VectorVectorPair vvp : maxMatch) {
				mergedTranslators.add(vvp.getOriginVector().minus(firstOriginVector));
			}
			PointSet pattern1 = tec1.getPattern().translate(firstOriginVector);
			PointSet pattern2 = tec2.getPattern().translate(firstOriginVector.add(diffVector));
			PointSet mergedPattern = new PointSet();
			mergedPattern.addAll(pattern1);
			mergedPattern.addAll(pattern2);
			TECMatchPair mergedTEC = new TECMatchPair(mergedPattern,mergedTranslators,dataset,maxMatch);
			mergedTECMatchPairs.add(mergedTEC);
		}

		//		Find the best merged TEC
		Collections.sort(mergedTECMatchPairs, new TECQualityComparator());
		TECMatchPair bestMergedTECMatchPair = mergedTECMatchPairs.get(0);

		//		Compute the new versions of tec1 and tec2
		//		Remove translators corresponding to occurrences of patterns in the merged TEC
		//		First find new version of tec1. This is just original translator set minus
		//		set of origin vectors in best TECMatchPair:

		VectorSet originVectors = bestMergedTECMatchPair.getOriginVectors();
		TEC newTEC1;
		if (originVectors.isEmpty())
			newTEC1 = new TEC(tec1.getPattern().copy(),tec1.getTranslators().copy(),dataset);
		else
			newTEC1 = new TEC(tec1.getPattern().copy(),tec1.getTranslators().copy().remove(originVectors),dataset);
		if (newTEC1.getTranslatorSetSize()==0)
			newTEC1 = null;

		//		... now find new version of tec2.
		Vector differenceVector = bestMergedTECMatchPair.getDifferenceVector();
		TEC newTEC2;
		if (originVectors.isEmpty())
			newTEC2 = new TEC(tec2.getPattern().copy(),tec2.getTranslators().copy(),dataset);
		else
			newTEC2 = new TEC(tec2.getPattern().copy(),tec2.getTranslators().copy().remove(originVectors.translate(differenceVector)),dataset);
		if (newTEC2.getTranslatorSetSize()==0)
			newTEC2 = null;

		ArrayList<TEC> outputList = new ArrayList<TEC>();
		outputList.add(tec1);
		outputList.add(tec2);

		//		If the best merged TEC has sufficient size and compactness then
		//		return an array consisting of the new tec1, the new tec2 and the best merged TEC, 
		//		(return null in place of tec1 or tec2 when one of them is empty)...

		if (bestMergedTECMatchPair.getTranslatorSetSize() >= minMatchSize && bestMergedTECMatchPair.getCompactness() >= minCompactness)
			outputList.add(bestMergedTECMatchPair);

		//		... otherwise return an array containing just tec1 and tec2.
		return outputList;
	}

}
