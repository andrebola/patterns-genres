package siatec;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.TreeSet;

/**
 * 
 * @author David Meredith
 * @date 28 June 2013
 * 
 * This program converts all the Lisp format data files
 * in the JKU PDD to OPND format. It creates an OPND file
 * in the same folder as the input Lisp file and gives it
 * the same file name as the input Lisp file, but with the
 * suffix ".opnd".
 *
 */
public class CollinsLispToOpnd {

//	private static String rootFolder = "/Users/dave/Documents/Work/Research/workspace/Points/data/JKUPDD-noAudio-Mar2013/groundTruth";
//	private static String rootFolder = "/Users/dave/Documents/Work/Research/workspace/Points/data/JKUPDD-noAudio-Jul2013/groundTruth";
	private static String rootFolder = "/Users/dave/Documents/Work/Research/Data/JKU-PDD/JKUPDD-noAudio-Aug2013/groundTruth";

	static class GcdPod {
		String fileName = null;
		Integer gcd = null;
		Integer productOfDenominators = null;

		GcdPod(String fileName, Integer gcd, Integer productOfDenominators) {
			this.fileName = fileName;
			this.gcd = gcd;
			this.productOfDenominators = productOfDenominators;
		}
	}

	private static ArrayList<GcdPod> gcdPodList = new ArrayList<GcdPod>();


	public static void main(String[] args) {

		/* Make a list of the full pathnames of all the
		 * Lisp format files in the JKU PDD
		 */

		ArrayList<String> lispFileNames = getLispFileNames(rootFolder);

		//Convert all the files in the list
		try {
//			convert(lispFileNames.get(5));
			for(int i = 0; i < lispFileNames.size(); i++)
				convert(lispFileNames.get(i));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static ArrayList<String> getLispFileNames(String folderName) {
		ArrayList<String> outputList = new ArrayList<String>();
		File folder = new File(folderName);
		if (folder.isDirectory()) {
			String[] dirList = folder.list();
			for(String dirItemName : dirList) {
				String fullPath = folderName+"/"+dirItemName;
				File dirItem = new File(fullPath);
				if (dirItem.isDirectory())
					outputList.addAll(getLispFileNames(fullPath));
				else if (folderName.endsWith("lisp") && dirItemName.endsWith(".txt"))
					outputList.add(fullPath);
			}
		}
		return outputList;
	}

	static class CollinsNote {
		private Integer 
		onsetNumerator, 
		onsetDenominator,
		midiNoteNumber,
		collinsMorpheticPitch,
		durationNumerator,
		durationDenominator,
		voiceNumber;

		CollinsNote(String s) {
			String[] sa = s.split(" ");
			String[] onsetArray = sa[0].split("/");
			setOnsetNumerator(Integer.parseInt(onsetArray[0]));
			setOnsetDenominator((onsetArray.length > 1)?Integer.parseInt(onsetArray[1]):1);
			setMidiNoteNumber(Integer.parseInt(sa[1]));
			if (sa.length > 2) {
				setCollinsMorpheticPitch(Integer.parseInt(sa[2]));
				String[] durationArray = sa[3].split("/");
				setDurationNumerator(Integer.parseInt(durationArray[0]));
				setDurationDenominator((durationArray.length > 1)?Integer.parseInt(durationArray[1]):1);
				setVoiceNumber(Integer.parseInt(sa[4]));
			}
		}

		@Override
		public String toString() {
			return "collinsNote("+
					getOnsetNumerator() + "/" + getOnsetDenominator() + "," +
					getMidiNoteNumber() + "," +
					getCollinsMorpheticPitch() + "," +
					getDurationNumerator() + "/" + getDurationDenominator() + "," +
					getVoiceNumber()+")";
		}

		public Integer getOnsetNumerator() {
			return onsetNumerator;
		}

		public void setOnsetNumerator(Integer onsetNumerator) {
			this.onsetNumerator = onsetNumerator;
		}

		public Integer getOnsetDenominator() {
			return onsetDenominator;
		}

		public void setOnsetDenominator(Integer onsetDenominator) {
			this.onsetDenominator = onsetDenominator;
		}

		public Integer getMidiNoteNumber() {
			return midiNoteNumber;
		}

		public void setMidiNoteNumber(Integer midiNoteNumber) {
			this.midiNoteNumber = midiNoteNumber;
		}

		public Integer getCollinsMorpheticPitch() {
			return collinsMorpheticPitch;
		}

		public void setCollinsMorpheticPitch(Integer collinsMorpheticPitch) {
			this.collinsMorpheticPitch = collinsMorpheticPitch;
		}

		public Integer getDurationNumerator() {
			return durationNumerator;
		}

		public void setDurationNumerator(Integer durationNumerator) {
			this.durationNumerator = durationNumerator;
		}

		public Integer getDurationDenominator() {
			return durationDenominator;
		}

		public void setDurationDenominator(Integer durationDenominator) {
			this.durationDenominator = durationDenominator;
		}

		public Integer getVoiceNumber() {
			return voiceNumber;
		}

		public void setVoiceNumber(Integer voiceNumber) {
			this.voiceNumber = voiceNumber;
		}
	}

	private static void convert(String fileName) throws Exception {
		Integer gcd = null, productOfDenominators = null;

		//Read Collins lisp file into a StringBuilder
		StringBuilder sb = new StringBuilder();
		try {
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			for (String l = br.readLine(); l != null; l = br.readLine()) sb.append(l);
			br.close();
		} catch (Exception e) {
			e.printStackTrace();
		}

		//Convert Stringbuilder to a String
		String text = sb.toString();

		//Parse the string into an array of note strings
		String[] sa = text.split("[()]");

		//Make list of CollinsNote objects
		ArrayList<CollinsNote> collinsNotes = new ArrayList<CollinsNote>();
		System.out.println(fileName);
		for(int i = 0, k = 1; i < sa.length;i++) {
			String s = sa[i].trim();
			if (s.length() > 0) {
				CollinsNote collinsNote = new CollinsNote(s);
				collinsNotes.add(collinsNote);
				System.out.println(k++ +" "+s+": "+ collinsNote); 
			}
		}

		if (!fileName.contains("repeatedPatterns")) {
			//Calculate least multiplier required to make Collins onsets and durations
			//into integers. This is at most the product of the distinct denominators
			//used in the onsets and durations.

			TreeSet<Integer> distinctDenominators = new TreeSet<Integer>();
			for(CollinsNote c : collinsNotes) {
				Integer durationDenominator = c.getDurationDenominator(); 
				if (durationDenominator != null) 
					distinctDenominators.add(durationDenominator);
				distinctDenominators.add(c.getOnsetDenominator());
			}

			System.out.println("Distinct denominators: "+distinctDenominators);

			productOfDenominators = 1;
			for(Integer d : distinctDenominators)
				productOfDenominators *= d;

			System.out.println("Product of denominators: "+productOfDenominators);

			//Multiply all the onsets and durations by productOfDenominators
			//Make a list of all the distinct results

			//		Integer minOnset = null;

			TreeSet<Integer> distinctMultipliedOnsetsAndDurations = new TreeSet<Integer>();
			for(CollinsNote c : collinsNotes) {
				if (c.getDurationNumerator() != null)
					distinctMultipliedOnsetsAndDurations.add(c.getDurationNumerator() * productOfDenominators / c.getDurationDenominator());
				Integer onset = c.getOnsetNumerator() * productOfDenominators / c.getOnsetDenominator();
				//			if (minOnset == null || onset < minOnset) minOnset = onset;
				distinctMultipliedOnsetsAndDurations.add(onset);
			}

			System.out.println("Distinct multiplied onsets and durations: "+distinctMultipliedOnsetsAndDurations);
			//		System.out.println("Minimum onset: "+minOnset);

			//Find gcd of all multiplied onsets and durations

			//		distinctMultipliedOnsetsAndDurations.remove(0);
			gcd = Maths.gcd(distinctMultipliedOnsetsAndDurations.toArray(new Integer[1]));

			System.out.println("GCD: "+gcd);
			gcdPodList.add(new GcdPod(fileName,gcd,productOfDenominators));

		} else {//File name contains substring "repeatedPatterns"

			/*
			 * If the file contains a pattern or an occurrence, then we have to use
			 * the gcd and productOfDenominators of the parent file and we cannot
			 * necessarily calculate them from this file.
			 */

			//Find GcdPod in gcdPodList

			//			/Users/dave/Documents/Work/Research/workspace/Points/data/JKUPDD-noAudio-Mar2013/groundTruth/bachBWV889Fg/polyphonic/lisp/wtc2f20.txt
			//			/Users/dave/Documents/Work/Research/workspace/Points/data/JKUPDD-noAudio-Mar2013/groundTruth/bachBWV889Fg/polyphonic/repeatedPatterns/bruhn/A/lisp/wtc2f20.txt
			//			/Users/dave/Documents/Work/Research/workspace/Points/data/JKUPDD-noAudio-Mar2013/groundTruth/bachBWV889Fg/polyphonic/repeatedPatterns/bruhn/A/occurrences/lisp/occ1.txt

			/*
			 * Find GcdPod in gcdPostList that begins with the prefix of this fileName that ends on the character
			 * preceding the substring "repeatedPatterns".
			 */

			int repeatedPatternsIndex = fileName.indexOf("repeatedPatterns");
			String prefix = fileName.substring(0, repeatedPatternsIndex);
			for(GcdPod g : gcdPodList) {
				if (g.fileName.startsWith(prefix)) {
					gcd = g.gcd;
					productOfDenominators = g.productOfDenominators;
					break;
				}
			}
		}

		if (gcd == null || productOfDenominators == null)
			throw new Exception("GCD or productOfDenominators is null: gcd = "+gcd+", productOfDenominators = "+productOfDenominators);

		//Write list of OPND objects to a file

		String outputFileSuffix = "unknown";
		System.out.println(sa[0]);
		for(int i = 0; i < sa.length; i++) {
			String s = sa[i].trim();
			if (s.length() == 0) 
				continue;
			else if (s.split(" ").length == 2) {
				outputFileSuffix = ".opc"; 
				break;
			} else if (s.split(" ").length == 5) {
				outputFileSuffix = ".opnd";
				break;
			} else
				throw new Exception("Collins Lisp input file has incorrect format: "+s+ " has length of "+ sa[0].split(" ").length );
		}
		/*
		 * Need to deal with cases where encoding starts with a negative number.
		 */


		try {
			String opndFileName = fileName.substring(0, fileName.length()-4)+outputFileSuffix;
			PrintStream opndPrintStream = new PrintStream(opndFileName);
			opndPrintStream.print("(");
			for(CollinsNote c : collinsNotes) {
				opndPrintStream.print("(");
				opndPrintStream.print((c.getOnsetNumerator() * productOfDenominators) / (gcd * c.getOnsetDenominator()) + " ");
				Pitch pitch = new Pitch();
				int chromaticPitch = c.getMidiNoteNumber() - 21;
				if (outputFileSuffix.equals(".opnd")) {
					int morpheticPitch = c.getCollinsMorpheticPitch() - 37;
					pitch.setChromamorpheticPitch(chromaticPitch, morpheticPitch);
					opndPrintStream.print(pitch.getPitchName()+" ");
					opndPrintStream.print((c.getDurationNumerator() * productOfDenominators) / (gcd * c.getDurationDenominator()) + " ");
					opndPrintStream.print(c.getVoiceNumber());
				} else { //outputFileSuffix is ".opc"
					opndPrintStream.print(chromaticPitch);
				}
				opndPrintStream.println(")");
			}
			opndPrintStream.print(")");
			opndPrintStream.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}
}
