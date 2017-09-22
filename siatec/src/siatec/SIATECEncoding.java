package siatec;

import java.awt.Dimension;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;

import javax.sound.midi.InvalidMidiDataException;
import javax.swing.JFrame;
import processing.core.PApplet;


public class SIATECEncoding extends Encoding {

	private ArrayList<TEC> tecs = new ArrayList<TEC>();
	private PointSet dataset = null;
	private int minMtpSize = 0;

	public SIATECEncoding(
			String inputFilePath,
			String outputDirectoryPath,
			boolean forRSuperdiagonals, int r,
			boolean withCompactnessTrawler, double a, int b) throws NoMorpheticPitchException, IOException, UnimplementedInputFileFormatException, InvalidMidiDataException {
		this(null,
				inputFilePath,
				outputDirectoryPath,
				0,
				PitchRepresentation.MORPHETIC_PITCH,
				false, //drawOutput
				false, //verbose
				forRSuperdiagonals, r,
				withCompactnessTrawler, a, b);
	}

	public SIATECEncoding(String inputFilePathName, String outputDirectoryPathName, Integer minPatternSize, PitchRepresentation pitchRepresentation, boolean drawOutput) throws NoMorpheticPitchException, IOException, UnimplementedInputFileFormatException, InvalidMidiDataException {
		this(null, 
				inputFilePathName, 
				outputDirectoryPathName, 
				minPatternSize, 
				pitchRepresentation, 
				drawOutput, 
				false, //verbose
				false, 0,
				false, 0.0, 0);
	}

	public SIATECEncoding(PointSet pointSet, 
			String inputFilePathNameOrOutputFileName, 
			String outputDirectoryPathName, 
			Integer minPatternSize, 
			PitchRepresentation pitchRepresentation, 
			boolean drawOutput, 
			boolean verbose,
			boolean forRSuperdiagonals, int r,
			boolean withCompactnessTrawler, double a, int b) throws NoMorpheticPitchException, IOException, UnimplementedInputFileFormatException, InvalidMidiDataException {
		minMtpSize = minPatternSize;
		if (pointSet == null) 
			dataset = new PointSet(inputFilePathNameOrOutputFileName, pitchRepresentation, false);
		else
			dataset = pointSet;
		VectorPointPair[][] vectorTable = SIA.computeVectorTable(dataset);
		ArrayList<MtpCisPair> mtpCisPairs = SIA.run(
				dataset,
				vectorTable,
				forRSuperdiagonals, r,
				withCompactnessTrawler, a, b,
				null, //logPrintStream
				true, //remove tran equiv mtps
				false, //merge vectors
				0 //
				);
		tecs = SIATEC.computeMtpTecs(
				dataset,
				vectorTable,
				mtpCisPairs,
				minMtpSize,
				1);

		String outputFileName = null;
		String minPatSizeString = "";
		if (minPatternSize > 0)
			minPatSizeString = minPatternSize.toString();
		String pitchRepresentationString = (pitchRepresentation.equals(PitchRepresentation.CHROMATIC_PITCH)?"CP":"MP");
		if (pointSet == null) {
			int start = inputFilePathNameOrOutputFileName.lastIndexOf("/") + 1;
			int end = inputFilePathNameOrOutputFileName.lastIndexOf(".");
			String inputFileName = inputFilePathNameOrOutputFileName.substring(start,end);
			if (!outputDirectoryPathName.endsWith("/"))
				outputDirectoryPathName = outputDirectoryPathName + "/";
			outputFileName = outputDirectoryPathName+inputFileName+(minPatternSize>0?"-":"")+minPatSizeString+"-"+pitchRepresentationString+".SIATEC";
		} else if (outputDirectoryPathName != null && inputFilePathNameOrOutputFileName != null) {
			outputFileName = outputDirectoryPathName+"/"+inputFilePathNameOrOutputFileName+(minPatternSize>0?"-":"")+minPatSizeString+"-"+pitchRepresentationString+".SIATEC";
		}

		if (outputFileName != null) {
			File outputDirectory = new File(outputDirectoryPathName);
			if (!outputDirectory.exists()) outputDirectory.mkdir();
			PrintStream ps = new PrintStream(outputFileName);
			ps.println(this);
			ps.close();
		}
		if (verbose) System.out.println(this);

		if (drawOutput)
			draw();
	}

	//	private ArrayList<MtpCisPair> computeMtpCisPairs(VectorPointPair[][] vectorTable) {
	//		System.out.print("computeMtpCisPairs...");
	//		TreeSet<VectorPointPair> sortedSIAVectorTable = new TreeSet<VectorPointPair>();
	//
	//		for(int i = 0; i < vectorTable.length; i++)
	//			for(int j = i+1; j < vectorTable.length; j++)
	//				sortedSIAVectorTable.add(vectorTable[i][j]);
	//
	//		ArrayList<PointSet> MTPs = new ArrayList<PointSet>();
	//		ArrayList<ArrayList<Integer>> CISs = new ArrayList<ArrayList<Integer>>();
	//		VectorPointPair firstVP = sortedSIAVectorTable.first(); 
	//		Vector v = firstVP.getVector();
	//		PointSet mtp = new PointSet();
	//		ArrayList<Integer> cis = new ArrayList<Integer>();
	//		mtp.add(firstVP.getPoint());
	//		cis.add(firstVP.getIndex());
	//		NavigableSet<VectorPointPair> rest = sortedSIAVectorTable.tailSet(firstVP, false); 
	//		for(VectorPointPair vp : rest) {
	//			if (vp.getVector().equals(v)) {
	//				mtp.add(vp.getPoint());
	//				cis.add(vp.getIndex());
	//			} else {
	//				if (mtp.size() >= minMtpSize) {
	//					MTPs.add(mtp);
	//					CISs.add(cis);
	//				}
	//				mtp = new PointSet();
	//				cis = new ArrayList<Integer>();
	//				v = vp.getVector();
	//				mtp.add(vp.getPoint());
	//				cis.add(vp.getIndex());
	//			}
	//		}		
	//
	//		if (mtp.size() >= minMtpSize) {
	//			MTPs.add(mtp);
	//			CISs.add(cis);
	//		}
	//
	//		ArrayList<MtpCisPair> mtpCisPairs = new ArrayList<MtpCisPair>();
	//
	//		for(int i = 0; i < MTPs.size(); i++) {
	//			mtpCisPairs.add(new MtpCisPair(MTPs.get(i),CISs.get(i)));
	//		}
	//
	//		System.out.println("completed: "+MTPs.size()+" MTPs found");
	//		return mtpCisPairs;
	//
	//	}

	//	private ArrayList<MtpCisPair> removeTranslationallyEquivalentMtps(ArrayList<MtpCisPair> mtpCisPairs) {
	//		System.out.print("removeTranslationallyEquivalentMtps");
	//		//Put the MTPs into order of size - then only have to compare each MTP 
	//		//with other MTPs of the same size when removing translationally 
	//		//equivalent MTPs.
	//
	//		PointSetSizeComparator pointSetSizeComparator = new PointSetSizeComparator();
	//		Collections.sort(mtpCisPairs, pointSetSizeComparator);
	//
	//		//Remove MTPs that are translationally equivalent to other MTPs
	//		//so that we don't have to compute the same TEC more than once.
	//
	//		ArrayList<MtpCisPair> newMtpCisPairList = new ArrayList<MtpCisPair>();
	//
	//		MtpCisPair s1, s2;
	//		int n = mtpCisPairs.size();
	//		for(int i = 0; i < n; i++) {
	//			if (i%500 == 0 && i != 0) {
	//				System.out.print(".");
	//				System.out.flush();
	//			}
	//			if (i%25000 == 0) {
	//				System.out.println();
	//				System.out.flush();
	//			}
	//			s1 = mtpCisPairs.get(i);
	//			int x = s1.getMtp().size();
	//			boolean found = false;
	//			for(int j = i + 1; !found && j < n && (s2 = mtpCisPairs.get(j)).getMtp().size() == x; j++) {
	//				if (s1.getMtp().translationallyEquivalentTo(s2.getMtp()))
	//					found = true;
	//			}
	//			if (!found)
	//				newMtpCisPairList.add(s1);
	//		}
	//
	//		System.out.println("\ncompleted: "+newMtpCisPairList.size()+" MTPs after removing translational equivalents");
	//		return newMtpCisPairList;
	//	}


	@Override
	public void draw() {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				//frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				PApplet embed = new DrawPoints(dataset,tecs);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	@Override
	public ArrayList<TEC> getTECs() {
		return tecs;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		for(TEC tec: tecs)
			sb.append(tec.toString()+"\n");
		return sb.toString();
	}

	public static void main(String[] args) throws NoMorpheticPitchException, IOException, UnimplementedInputFileFormatException, InvalidMidiDataException {
		//		String inputFilePath = "/Users/dave/Documents/Work/Research/workspace-to-2014-01-17/Points/data/Lartillot/ClosedSubstrings001.opnd";
		//		String outputFileDirectoryPath = "/Users/dave/Documents/Work/Research/workspace-to-2014-01-17/Points/data/Lartillot";
		//		String inputFilePath = "/Users/dave/Documents/Work/Research/2014-09-15-workspace/Points/data/WTCI-FUGUES-FOR-JNMR-2014/bwv847b-done.opnd";
		//		String outputFileDirectoryPath = "/Users/dave/Documents/Work/Research/2014-09-15-workspace/Points/output/points018";
		//		String inputFilePath = "/Users/dave/Documents/Work/Research/Data/Mozart-Haydn/Themefinder/Haydn/op74n1-04.mid";
		String inputFilePath = "/home/andres/Downloads/lmd_full/0/0533b5f7f5bcbe2e73389f207b178578.mid";
		String outputFileDirectoryPath = "/tmp/";
		boolean forRSubdiagonals = false;
		int r = 0;
		boolean withCompactnessTrawler = false;
		double a = 1.0;
		int b = 2;
		int minPatternSize = 5;
		PointSet pointSet = null;
		PitchRepresentation pitchRepresentation = PitchRepresentation.CHROMATIC_PITCH;
		boolean drawOutput = true;
		boolean verbose = false;
//		SIATECEncoding encoding = new SIATECEncoding(
//				inputFilePath,
//				outputFileDirectoryPath,
//				forRSubdiagonals, 
//				r,
//				withCompactnessTrawler, 
//				a, 
//				b);

		SIATECEncoding encoding = new SIATECEncoding(
				pointSet, 
				inputFilePath, 
				outputFileDirectoryPath, 
				200, 
				pitchRepresentation, 
				drawOutput, 
				verbose,
				forRSubdiagonals, 
				r,
				withCompactnessTrawler, 
				a, 
				b);
				encoding.draw();
	}

}
