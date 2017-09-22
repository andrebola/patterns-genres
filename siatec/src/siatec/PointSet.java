/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;


import java.awt.Component;
import java.awt.Dimension;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.NavigableSet;
import java.util.TreeSet;

import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiSystem;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Sequence;
import javax.sound.midi.Sequencer;
import javax.sound.midi.ShortMessage;
import javax.sound.midi.Track;
import javax.swing.JFrame;

import processing.core.PApplet;

public class PointSet implements Comparable<PointSet>{
	protected TreeSet<Point> points = new TreeSet<Point>();
	protected Long bbArea = null, maxX = null, minX = null;
	protected Integer maxY = null, minY = null;
	protected Double temporalDensity = null;

	public PointSet() {}

	/**
	 * Reads in points from a .pts file.
	 * Each point is given on a separate line,
	 * with co-ordinates separated by white space.
	 * For example, the unit square would be 
	 * represented as follows:
	 * 
	 * 0 0
	 * 1 0
	 * 0 1
	 * 1 1
	 * 
	 * @param fileName
	 * @throws RationalOnsetTimeException 
	 */

	public PointSet(String fileName) {
		this(fileName,false);
	}
	
	public PointSet(String fileName, boolean isDiatonic) {
		this(fileName,isDiatonic,false,false);
	}
	
	public PointSet(String fileName, boolean isDiatonic, boolean withoutChannel10, boolean onlyRhythm) {
		if (fileName.toLowerCase().endsWith(".pts"))
			makePointsObjectFromPTSFile(fileName);
		else if (fileName.toLowerCase().endsWith(".pointset"))
			makePointSetObjectFromPointSetFile(fileName);
		else if (fileName.toLowerCase().endsWith(".krn"))
			makePointSetObjectFromKernFile(fileName, isDiatonic);
		else if (fileName.toLowerCase().endsWith(".opnd"))
			makePointSetObjectFromOPNDFile(fileName, isDiatonic);
		else if (fileName.toLowerCase().endsWith(".midi") || fileName.toLowerCase().endsWith(".mid"))
			makePointSetObjectFromMIDIFile(fileName, isDiatonic, withoutChannel10, onlyRhythm);
	}

	private void makePointSetObjectFromMIDIFile(String fileName, boolean isDiatonic, boolean withoutChannel10, boolean onlyRhythm) {
		try{
			PointSet pointSet = new PointSet(Notes.fromMIDI(fileName,true, onlyRhythm),isDiatonic);
			points = pointSet.points;
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private void makePointSetObjectFromOPNDFile(String fileName, boolean isDiatonic) {
		try {
			PointSet pointSet = new PointSet(Notes.fromOPND(fileName),isDiatonic);
			points = pointSet.points;
		} catch (NoMorpheticPitchException | IOException e) {
			e.printStackTrace();
		}
	}

	private void makePointSetObjectFromKernFile(String kernFilePathName, boolean isDiatonic) {
		try {
			PointSet pointSet = new PointSet(Notes.fromKern(kernFilePathName),isDiatonic);
			points = pointSet.points;
		} catch (NoMorpheticPitchException | IOException e) {
			e.printStackTrace();
		}
	}

	private void makePointSetObjectFromPointSetFile(String pointSetFilePathName) {
		BufferedReader br;
		try {
			br = new BufferedReader(new FileReader(pointSetFilePathName));
			StringBuilder sb = new StringBuilder();
			String l;
			while ((l = br.readLine()) != null)
				sb.append(l);
			br.close();

			PointSet pointSet = PointSet.getPointSetFromString(sb.toString());
			for(Point point : pointSet.getPoints())
				add(point);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

	}

	private void makePointsObjectFromPTSFile(String fileName) {
		try {
			BufferedReader br = new BufferedReader(new FileReader(fileName));
			String l = br.readLine();
			while(l != null) {
				points.add(new Point(l));
				l = br.readLine();
			}
			br.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public PointSet(String fileName, PitchRepresentation pitchRepresentation, boolean onlyRhythm) throws NoMorpheticPitchException, IOException, UnimplementedInputFileFormatException, InvalidMidiDataException {
		boolean diatonicPitch = true;
		if (pitchRepresentation.equals(PitchRepresentation.CHROMATIC_PITCH))
			diatonicPitch = false;
		if (fileName.toLowerCase().endsWith(".pointset"))
			makePointSetObjectFromPointSetFile(fileName);
		else if (fileName.toLowerCase().endsWith("pts"))
			makePointsObjectFromPTSFile(fileName);
		else if (fileName.toLowerCase().endsWith("opnd"))
			getPointSetFromNotes(Notes.fromOPND(fileName),diatonicPitch);
		else if (fileName.toLowerCase().endsWith("mid")) {
			Notes notes = Notes.fromMIDI(fileName,diatonicPitch,onlyRhythm);
			if (notes.getNotes() != null)
                            getPointSetFromNotes(notes,diatonicPitch);
//		} else if (fileName.toLowerCase().endsWith("txt")) { //assume Collins lisp format
//			MIREX2013Entries.readLispFileIntoPointSet(fileName);
//			points = MIREX2013Entries.DATASET.getPoints();
		} else if (fileName.toLowerCase().endsWith("krn")) { //assume kern format
			points = new PointSet(fileName,diatonicPitch).points;
		} 
		else
			throw new UnimplementedInputFileFormatException(fileName+" is not of a known input file type");
	}

	public PointSet(Notes notes, boolean diatonicPitch) throws NoMorpheticPitchException {
		getPointSetFromNotes(notes,diatonicPitch);
	}

	private void getPointSetFromNotes(Notes notes, boolean diatonicPitch) throws NoMorpheticPitchException {
		for(Note note : notes.getNotes()) {
			Integer voice = note.getVoice();
			Long duration = note.getDuration();
			if (voice == null)
				voice = note.getChannel();
			if (diatonicPitch) {
				Integer morpheticPitch = note.getPitch().getMorpheticPitch();
				if (morpheticPitch == null)
					morpheticPitch = note.getComputedPitch().getMorpheticPitch();
				if (morpheticPitch == null)
					throw new NoMorpheticPitchException("The following note has no morphetic pitch: "+note);
				points.add(new Point(note.getOnset(),morpheticPitch,voice,duration));
			}
			else
				points.add(new Point(note.getOnset(),note.getMidiNoteNumber(),voice,duration));
		}
	}

	public PointSet(PointSet pattern) {
		for(Point point : pattern.getPoints())
			points.add(point);
	}

	public PointSet(Point... somePoints) {
		for(Point point : somePoints)
			points.add(point);
	}

	public Point first() {
		return points.first();
	}

	public NavigableSet<Point> tail(Point fromPoint, boolean inclusive) {
		return points.tailSet(fromPoint,inclusive);
	}

	public TreeSet<Point> getPoints() {
		return points;
	}

	/**
	 * a.diff(b) returns the set of points in a 
	 * that are not in b.
	 * @param pointSet2
	 * @return
	 */
	public PointSet diff(PointSet pointSet2) {
		PointSet newPoints = new PointSet();
		for(Point point : points)
			if (!pointSet2.contains(point))
				newPoints.add(point);
		return newPoints;
	}

	public boolean contains(Point point) {
		return points.contains(point);
	}

	public boolean contains(PointSet pointSet) {
		for (Point point : pointSet.getPoints())
			if (!contains(point)) return false;
		return true;
	}

	public Encoding encode(Encoder encoder) {
		return encoder.encode(this);
	}

	public boolean isEmpty() {
		return points.size() == 0;
	}

	public void add(Point point) {
		points.add(point);
	}

	public void addAll(PointSet pointSet) {
		for(Point p : pointSet.getPoints())
			add(p);
	}

	public Long getMaxX() {
		if (maxX != null || points == null || points.isEmpty()) return maxX;
		long max = points.first().getX();
		for(Point point : points)
			if (point.getX() > max) max = point.getX();
		return (maxX = max);
	}

	public Integer getMaxY() {
		if (maxY != null || points == null || points.isEmpty()) return maxY;
		if (points.isEmpty())
			System.out.println("points is empty and shouldn't be");
		int max = points.first().getY();
		for(Point point : points)
			if (point.getY() > max) max = point.getY();
		return (maxY = max);
	}

	public Long getMinX() {
		if (minX != null || points == null || points.isEmpty()) return minX;
		long min = points.first().getX();
		for(Point point : points)
			if (point.getX() < min) min = point.getX();
		return (minX = min);
	}

	public Integer getMinY() {
		if (minY != null || points == null || points.isEmpty()) return minY;
		int min = points.first().getY();
		for(Point point : points)
			if (point.getY() < min) min = point.getY();
		return (minY = min);
	}

	public void draw() {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public void draw(final String title, final boolean diatonicPitch) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setTitle(title);
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this,title,diatonicPitch);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public void draw(final String title) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setTitle(title);
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public void draw(final TreeSet<PatternPair> patternPairs) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this, patternPairs);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public void draw(final Collection<TEC> tecs) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this,tecs);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public void draw(final String title, final ArrayList<PointSetCollectionPair> arrayListOfPointSetCollectionPairs) {
		javax.swing.SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				JFrame frame = new JFrame();
				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
				frame.setResizable(false);
				frame.setTitle(title);
				//				frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
				PApplet embed = new DrawPoints(PointSet.this,arrayListOfPointSetCollectionPairs);
				frame.add(embed);
				embed.init();
				frame.pack();
				frame.setVisible(true);
			}
		});
	}

	public String toString() {
		if (isEmpty()) return "P()";
		StringBuilder sb = new StringBuilder("P("+points.first());
		for(Point point : points.tailSet(points.first(), false))
			sb.append(","+point);
		sb.append(")");
		return sb.toString();
	}

	protected void play(Integer msPerTatum) {
		try {
			int ticksPerQuarterNote = 500;
			int msPerTick = 1;
			Sequence sequence = new Sequence(Sequence.PPQ,ticksPerQuarterNote);
			sequence.createTrack();
			Track track = sequence.getTracks()[0];
			for(Point point : points) {
				int midiNoteNumber = point.getY();
				ShortMessage noteOnMessage = new ShortMessage();
				noteOnMessage.setMessage(ShortMessage.NOTE_ON,midiNoteNumber,96);
				ShortMessage noteOffMessage = new ShortMessage();
				noteOffMessage.setMessage(ShortMessage.NOTE_ON,midiNoteNumber,0);
				long noteOnTick = point.getX() * msPerTatum / msPerTick;
				long noteOffTick = noteOnTick + msPerTatum;
				MidiEvent noteOnEvent = new MidiEvent(noteOnMessage,noteOnTick);
				MidiEvent noteOffEvent = new MidiEvent(noteOffMessage, noteOffTick);
				track.add(noteOnEvent);
				track.add(noteOffEvent);
			}
			Sequencer sequencer = MidiSystem.getSequencer();
			sequencer.setSequence(sequence);
			sequencer.open();
			sequencer.start();
			System.out.println("Press ENTER when finished playing.");
			System.in.read();
			sequencer.stop();
			sequencer.close();
		} catch (MidiUnavailableException e) {
			e.printStackTrace();
		} catch (InvalidMidiDataException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public int size() {
		return getPoints().size();
	}

	public PointSet copy() {
		PointSet newPoints = new PointSet();
		for(Point point : points)
			newPoints.add(point.copy());
		return newPoints;
	}

	@Override
	public int compareTo(PointSet pointSet) {
		if (pointSet == null) return 1;
		int d = pointSet.size() - size();
		if (d != 0) return d;
		//Same size
		ArrayList<Point> a1 = new ArrayList<Point>(points);
		ArrayList<Point> a2 = new ArrayList<Point>(pointSet.getPoints());
		for(int i = 0; i < a1.size(); i++) {
			d = a1.get(i).compareTo(a2.get(i));
			if (d != 0) return d;
		}
		return 0;
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof PointSet)) return false;
		return compareTo((PointSet)obj) == 0;
	}

	public double getCompactness(PointSet dataSet) {
		double N_D = dataSet.getBBSubset(getTopLeft(),getBottomRight()).size();
		double N_P = size();
		double C = N_P/N_D;
		return C;
	}

	public boolean translationallyEquivalentTo(PointSet otherPointSet) {
		if (otherPointSet == null) return false;
		if (otherPointSet.size() != size()) return false;
		Vector vector = null;
		for(int i = 0; i < size(); i++) {
			if (vector == null)
				vector = new Vector(get(i),otherPointSet.get(i));
			else if (!vector.equals(new Vector(get(i),otherPointSet.get(i))))
				return false;
		}
		return true;
	}

	public Point get(int i) {
		ArrayList<Point> a = new ArrayList<Point>(points);
		return a.get(i);
	}

	public PointSet translate(Vector vector) {
		PointSet newPoints = new PointSet();
		for(Point point : points)
			newPoints.add(point.translate(vector));
		return newPoints;
	}

	public PointSet getInversion() {
		PointSet inversion = new PointSet();
		for(Point point : points)
			inversion.add(point.getInversion());
		return inversion;
	}

	public PointSet getRetrograde() {
		PointSet retrograde= new PointSet();
		for(Point point : points)
			retrograde.add(point.getRetrograde());
		return retrograde;
	}

	public PointSet getBBSubset(Point topLeft, Point bottomRight) {
		if (topLeft.getX() > bottomRight.getX())
			System.out.println("topLeft to right of bottomRight in getBBSubset: "+topLeft+", "+bottomRight);
		PointSet bbSubset = new PointSet();
		Point bottomLeft = new Point(topLeft.getX(),bottomRight.getY());
		Point topRight = new Point(bottomRight.getX(),topLeft.getY());
		for(Point point : points.subSet(bottomLeft, true, topRight, true)) {
			if (point.getY() >= bottomLeft.getY() && point.getY() <= topRight.getY())
				bbSubset.add(point);
		}
		return bbSubset;
	}

	public long getBBArea() {
		if (bbArea != null) return bbArea;
		return (bbArea = (getMaxX() - getMinX()) * (getMaxY() - getMinY()));
	}

	public Point getTopLeft() {
		return new Point(getMinX(),getMaxY());
	}

	public Point getTopRight() {
		return new Point(getMaxX(),getMaxY());
	}

	public Point getBottomLeft() {
		return new Point(getMinX(),getMinY());
	}

	public Point getBottomRight() {
		return new Point(getMaxX(),getMinY());
	}

	public double getEccentricity() {
		double sumX = 0.0;
		double sumY = 0.0;
		for(Point p : points) {
			sumX += p.getX();
			sumY += p.getY();
		}
		double meanX = sumX/points.size();
		double meanY = sumY/points.size();
		double centreX = getMinX()+(getMaxX()-getMinX())/2.0;
		double centreY = getMinY()+(getMaxY()-getMinY())/2.0;
		double deltaX = meanX-centreX;
		double deltaY = meanY-centreY;
		double width = getMaxX()-getMinX();
		double height = getMaxY()-getMinY();
		return (Math.abs(deltaX)/width)+(Math.abs(deltaY)/height);
	}

	public double getTemporalDensity() {
		if (temporalDensity != null) return temporalDensity;
		TreeSet<Long> xValues = new TreeSet<Long>();
		for(Point point : points)
			xValues.add(point.getX());
		return (temporalDensity = (1.0*xValues.size())/(getMaxX()-getMinX()));
	}

	public boolean removeWithoutReset(Point point) {
		return points.remove(point);
	}

	public boolean remove(Point point) {
		boolean setChanged = removeWithoutReset(point);
		if (setChanged) {
			bbArea = maxX = minX = null;
			maxY = minY = null;
			temporalDensity = null;
		}
		return setChanged;
	}

	public long getWidth() {
		return getMaxX() - getMinX();
	}

	public int getHeight() {
		return getMaxY() - getMinY();
	}

	public double getAspectRatio() {
		return (1.0 * getWidth())/getHeight();
	}

	/**
	 * Returns a measure of how "unsquare" the bounding box is.
	 * @return
	 */
	public double getUnsquareness() {
		return Math.abs(1.0 - getAspectRatio());
	}

	public boolean removeWithoutReset(PointSet otherPointSet) {
		boolean setChanged = false;
		for(Point point : otherPointSet.getPoints())
			if (points.contains(point)) {
				setChanged = true;
				removeWithoutReset(point);
			}
		return setChanged;
	}

	public boolean remove(PointSet otherPointSet) {
		boolean setChanged = removeWithoutReset(otherPointSet);
		if (setChanged) {
			bbArea = maxX = minX = null;
			maxY = minY = null;
			temporalDensity = null;
		}
		return setChanged;
	}

	public double getMinSIAMEuclidDistance(PointSet ps) {
		//From this to ps
		double fromThisToPS = 0.0;
		for (Point p1 : getPoints()) {
			Double leastDistance = null;
			for(Point p2 : ps.getPoints()) {
				double thisDistance = new Vector(p1,p2).getLength();
				if (leastDistance == null || thisDistance < leastDistance)
					leastDistance = thisDistance;
			}
			fromThisToPS += leastDistance;
		}
		//From ps to this
		double fromPSToThis = 0.0;
		for (Point p1 : ps.getPoints()) {
			Double leastDistance = null;
			for(Point p2 : getPoints()) {
				double thisDistance = new Vector(p1,p2).getLength();
				if (leastDistance == null || thisDistance < leastDistance)
					leastDistance = thisDistance;
			}
			fromPSToThis += leastDistance;
		}
		//return minimum
		return ((fromThisToPS<fromPSToThis)?fromThisToPS:fromPSToThis);
	}

	public PointSet intersection(PointSet ps2) {
		PointSet intersection = new PointSet();
		TreeSet<Point> thisSet = null, otherSet = null;
		if (size() <= ps2.size()) {
			thisSet = getPoints();
			otherSet = ps2.getPoints();
		} else {
			thisSet = ps2.getPoints();
			otherSet = getPoints();
		}
		for(Point p : thisSet) {
			if (otherSet.contains(p))
				intersection.add(p);
		}
		return intersection;
	}

	public PointSet getSegment(long startTime, long endTime, boolean inclusive) {
		PointSet segment = new PointSet();
		for(Point point : getPoints()) {
			if (point.getX() >= startTime && (inclusive?(point.getX() <= endTime):(point.getX() < endTime)))
				segment.add(point);
		}
		return segment;
	}

	public void writeToPtsFile(String fileName) throws IOException {
		PrintStream ps = new PrintStream(fileName);
		for(Point p : points)
			ps.println(p.getX()+" "+p.getY());
		ps.close();
	}

	public static PointSet getPointSetFromTECString(String tecString) {
		/*
		 * tecString has form:
		 * 
		 * T(P(p(x1,y1),p(x2,y2),...p(xn,yn)),V(v(u1,w1),v(u2,w2),...v(um,wm)))
		 */

		TEC tec = new TEC(tecString);
		return tec.getCoveredPoints();
	}

	public static PointSet getPointSetFromString(String l) {
		System.out.println(l);
		if (l.equals("P()"))
			return new PointSet();
		//		System.out.println("Trying to find PointSet in TEC: "+l);
		int startIndex = l.indexOf("p(");
		//		System.out.println("startIndex is "+startIndex);
		int endIndex = l.indexOf("),V(");
		if (endIndex == -1)
			endIndex = l.indexOf("))")+1;
		//		System.out.println("endIndex is "+endIndex+"("+l.substring(endIndex - 1,endIndex+5)+")");
		String pointSequence = l.substring(startIndex,endIndex);
		PointSet outputPointSet = new PointSet();
		for(int start = 0; start < pointSequence.length();) {
			int end = pointSequence.indexOf(")",start)+1;
			String pointString = pointSequence.substring(start, end);
			Point point = new Point(pointString);
			outputPointSet.add(point);
			start = end+1;
		}
		//		System.out.println(outputPointSet);
		return outputPointSet;
	}


	public ArrayList<Integer> getIndexSet(PointSet pattern) {
		if (pattern == null) return null;
		ArrayList<Integer> indexArray = new ArrayList<Integer>();
		ArrayList<Point> patternArray = new ArrayList<Point>(pattern.getPoints());
		int i = 0; //index into the dataset
		int j = 0; //index into the pattern
		for(Point point : points) {
			if (point.equals(patternArray.get(j))) {
				indexArray.add(i);
				j++;
				if (j >= pattern.size()) break;
			}
			i++;
		}
		return indexArray;
	}

//	public void draw(final ArrayList<PointSet> patterns, final String windowTitle) {
//		javax.swing.SwingUtilities.invokeLater(new Runnable() {
//			public void run() {
//				JFrame frame = new JFrame();
//				frame.setMinimumSize(new Dimension(DrawPoints.drawWindowWidth,DrawPoints.drawWindowHeight+23));
//				frame.setResizable(false);
//				frame.setTitle(windowTitle);
//				PApplet embed = new DrawPoints(patterns,PointSet.this);
//				frame.add(embed);
//				embed.init();
//				frame.pack();
//				frame.setVisible(true);
//			}
//		});
//	}

	//	public static PointSet getPointSetFromString(String pointSetString) {
	//		PointSet pointSet = new PointSet();
	//		String[] pointStrings = pointSetString.substring(pointSetString.indexOf('p'),pointSetString.lastIndexOf(')')).split(",");
	//		for(String pointString : pointStrings)
	//			pointSet.add(new Point(pointString));
	//		return pointSet;
	//	}

	/**
	 * Returns a new pattern formed by scaling this
	 * pattern by the x-axis scale factor f, with center
	 * of enlargement at the minimum point in this pattern.
	 * @param f
	 * @return
	 */
	public PointSet scale(Rational f) {
		PointSet imagePattern = new PointSet();
		Point minPoint = first();
		for(Point p : points) {
			Vector minPointToP = new Vector(minPoint,p);
			long minPointToPTimeComponent = minPointToP.getX();
			int minPointToPPitchComponent = minPointToP.getY();
			long imageOfMinPointToPTimeComponent = f.times(new Rational(minPointToPTimeComponent,1l)).getNumerator();
			Vector imageOfMinPointToP = new Vector(imageOfMinPointToPTimeComponent,minPointToPPitchComponent);
			Point q = minPoint.translate(imageOfMinPointToP);
			imagePattern.add(q);
		}
		return imagePattern;
	}

	public PointSet invert() {
		PointSet imagePattern = new PointSet();
		Point minPoint = first();
		for(Point p : points) {
			Vector minPointToP = new Vector(minPoint,p);
			long minPointToPTimeComponent = minPointToP.getX();
			int minPointToPPitchComponent = minPointToP.getY();
			Vector imageOfMinPointToP = new Vector(minPointToPTimeComponent,-minPointToPPitchComponent);
			Point q = minPoint.translate(imageOfMinPointToP);
			imagePattern.add(q);
		}
		return imagePattern;
	}

	public static void main(String[] args ) {
		//new PointSet("/Users/dave/Documents/Work/Research/2015-06-17-workspace/Points/data/WTCI-FUGUES-FOR-JNMR-2014/bwv846b-done.opnd",true).draw("Fugue in C Major, BWV 846", true);
		boolean diatonicPitch = true;
		//String filePathName = "/Users/dave/Documents/Work/Research/2015-02-15-workspace/Points/data/WTCI-FUGUES-FOR-JNMR-2014/bwv846b-done.opnd";
		String filePathName = "/home/andres/Downloads/lmd_full/0/0533b5f7f5bcbe2e73389f207b178578.mid";
//		String filePathName = null;
		boolean draw = true;
		
			for(int i = 0; i < args.length; i++) {
				if (args[i].startsWith("--draw"))
					draw = true;
				else if (!args[i].startsWith("-"))
					filePathName = args[i];
			}
		
//		String filePathName = "/Users/dave/Documents/Work/Research/Data/Sony/From Emmanuel/AllayaLeeLog/AllayaLeeLogCropped.pts";
		String fileName = filePathName.substring(filePathName.lastIndexOf("/")+1);
		boolean withoutChannel10 = true;
		PointSet dataset = new PointSet(filePathName,diatonicPitch,withoutChannel10, false);
		int start = filePathName.lastIndexOf("/")+1;
		int end = filePathName.lastIndexOf(".");
		String ptsFilePathName = filePathName.substring(0,start)+filePathName.substring(start,end)+".pts";
		try {
			dataset.writeToPtsFile(ptsFilePathName);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		if (draw)
			dataset.draw(fileName,diatonicPitch);
		//			Notes notes = Notes.fromOPND(opndFilePathName);
		//			notes.play(4,120);
		System.out.println(dataset.size()+" notes");
//		System.out.println(dataset);
		System.out.println("Dataset stored in file: "+ptsFilePathName);
	}

}