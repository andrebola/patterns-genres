package siatec;

import java.util.Comparator;

/**
 * 
 * @author David Meredith
 * 
 * Returns a value less than zero if tec1 is better than tec2.
 *
 */
public class TECQualityComparator implements Comparator<TEC> {

	private  double kCompressionRatio = 1;
	private  double kCompactness = 1;
	private  double kCoverage = 1;
	private  double kPatternWidth = 1;
	private  double kPatternSize = 1;
	private  double kBBArea = 1;

	private  double powCompressionRatio = 1;
	private  double powCompactness = 1;
	private  double powCoverage = 1;
	private  double powPatternWidth = 1;
	private  double powPatternSize = 1;
	private  double powBBArea = 1;


	public static int COMPRESSION_RATIO_USED_FREQ = 0;
	public static int COMPACTNESS_USED_FREQ = 0;
	public static int COVERAGE_USED_FREQ = 0;
	public static int PATTERN_SIZE_USED_FREQ = 0;
	public static int WIDTH_USED_FREQ = 0;
	public static int BBAREA_USED_FREQ = 0;
	public static int ZERO_RETURNED_FREQ = 0;

	public static double TOLERANCE = 0.0;
	public static double CR_TOLERANCE = TOLERANCE;
	public static double COMPACTNESS_TOLERANCE = TOLERANCE;
	public static double COVERAGE_TOLERANCE = TOLERANCE;
	public static double PATTERN_SIZE_TOLERANCE = TOLERANCE;
	public static double WIDTH_TOLERANCE = TOLERANCE;
	public static double BBAREA_TOLERANCE = TOLERANCE;

	public TECQualityComparator() {
		super();
	}
	
	public TECQualityComparator(
			double kCompressionRatio,
			double kCompactness,
			double kCoverage,
			double kPatternWidth,
			double kPatternSize,
			double kBBArea,
			double powCompressionRatio,
			double powCompactness,
			double powCoverage,
			double powPatternWidth,
			double powPatternSize,
			double powBBArea) {
		super();
		this.kCompressionRatio = kCompressionRatio;
		this.kCompactness = kCompactness;
		this.kCoverage = kCoverage;
		this.kPatternWidth = kPatternWidth;
		this.kPatternSize = kPatternSize;
		this.kBBArea = kBBArea;
		this.powCompressionRatio = powCompressionRatio;
		this.powCompactness = powCompactness;
		this.powCoverage = powCoverage;
		this.powPatternWidth = powPatternWidth;
		this.powPatternSize = powPatternSize;
		this.powBBArea = powBBArea;
	}

	public int compare2(TEC tec1, TEC tec2) {
		if (tec1 == null && tec2 == null) return 0;
		if (tec1 == null) return 1;
		if (tec2 == null) return -1;
		double d;
		double a, b;
		int aInt, bInt;
		long aLong, bLong, dLong;
		d = (a = tec2.getCompressionRatio()) - (b = tec1.getCompressionRatio());
		if (Math.abs(d*1.0)/(Math.max(a, b)) > CR_TOLERANCE) {
			COMPRESSION_RATIO_USED_FREQ++;
			return (int)Math.signum(d);
		}
		d = (aInt = tec2.getCoverage())-(bInt = tec1.getCoverage());
		if (Math.abs(d*1.0)/(Math.max(aInt, bInt) * 1.0) > COVERAGE_TOLERANCE) {
			COVERAGE_USED_FREQ++;
			return (int)Math.signum(d);
		}
		dLong = (aLong = tec1.getPattern().getWidth()) - (bLong = tec2.getPattern().getWidth());
		if (Math.abs(dLong*1.0)/(Math.max(aLong, bLong) * 1.0) > WIDTH_TOLERANCE) {
			WIDTH_USED_FREQ++;
			return (int)Math.signum(dLong);
		}
		d = (a=tec2.getCompactness())-(b = tec1.getCompactness());
		if (Math.abs(d*1.0)/(Math.max(a, b)) > COMPACTNESS_TOLERANCE) {
			COMPACTNESS_USED_FREQ++;
			return (int)Math.signum(d);
		}
		d = (aInt = tec2.getPatternSize()) - (bInt = tec1.getPatternSize());
		if (Math.abs(d*1.0)/(Math.max(aInt, bInt) * 1.0) > PATTERN_SIZE_TOLERANCE) {
			PATTERN_SIZE_USED_FREQ++;
			return (int)Math.signum(d);
		}
		dLong = (aLong = tec1.getBBArea()) - (bLong = tec2.getBBArea());
		if (Math.abs(dLong*1.0)/(Math.max(aLong, bLong) * 1.0) > BBAREA_TOLERANCE) {
			BBAREA_USED_FREQ++;
			return (int)Math.signum(dLong);
		}
		ZERO_RETURNED_FREQ++;
		return 0;
	}

	public int compare(TEC tec1, TEC tec2) {
		if (tec1 == null && tec2 == null) return 0;
		if (tec1 == null) return 1;
		if (tec2 == null) return -1;
		double f = tec2.getCompressionRatio() - tec1.getCompressionRatio();
		if (f > 0.0) return 1;
		if (f < 0.0) return -1;
		f = tec2.getCompactness()-tec1.getCompactness();
		if (f > 0.0) return 1;
		if (f < 0.0) return -1;
		int d = tec2.getCoverage()-tec1.getCoverage();
		if (d != 0) return d;
		long e = tec1.getPattern().getWidth() - tec2.getPattern().getWidth();
		if (e > 0l) return 1;
		if (e < 0l) return -1;
		d = tec2.getPatternSize() - tec1.getPatternSize();
		if (d != 0) return d;
		e = tec1.getBBArea() - tec2.getBBArea();
		if (e > 0l) return 1;
		if (e < 0l) return -1;
		d = tec1.getPattern().compareTo(tec2.getPattern());
		if (d != 0) return d;
		return tec1.getTranslators().compareTo(tec2.getTranslators());
	}

	public int compare3(TEC tec1, TEC tec2) {
		double tec1Quality = getQuality(tec1);
		double tec2Quality = getQuality(tec2);
		return (int)(Math.signum(tec2Quality - tec1Quality));
	}

	public double getQuality(TEC tec) {
		return
				kCompressionRatio 	* 	Math.pow(tec.getCompressionRatio(),powCompressionRatio) *
				kCompactness		* 	Math.pow(tec.getCompactness(),powCompactness) *
				kCoverage 			* 	Math.pow(tec.getCoverage(), powCoverage) *
				kPatternWidth		* 	Math.pow(tec.getPattern().getWidth(),powPatternWidth) *
				kPatternSize 		* 	Math.pow(tec.getPatternSize(),powPatternSize) *
				kBBArea				*	Math.pow(tec.getBBArea(),powBBArea)
				;
	}

	public static void main(String[] args) {

		/* 
		 * Find two TECs, t1 and t2, such that t1 < t2 with one
		 * definition of the comparator and t1 > t2 with the other
		 * definition.
		 * 
		 * In the following file:
		 * ~/Documents/Work/Research/Data/NLB/OUTPUT/NLB/COSIATEC/NLB/NLB070078_01-diat-2014-1-17-23-41-10.log
		 * the following is the second TEC:
		 * T(P(p(2520,31),p(2880,31),p(3000,30),p(3420,30)),V(v(0,0),v(3600,0),v(5760,0)))
		 * 
		 * In the following file:
		 * ~/Documents/Work/Research/workspace-to-2014-01-17/Points/output/points017/nlb/NLB070078_01-diat.log
		 * the following is the second TEC:
		 * T(P(p(120,31),p(1320,32),p(3960,31)),V(v(0,0),v(180,0),v(480,-4),v(4800,-1)))
		 * 
		 * The fact that COSIATEC chooses different TECs here indicates that the two TECs have different
		 * orderings with the different comparators.
		 * 
		 * The full point set is as follows (expressed as a TEC):
		 * 
		 * T(P(p(0,31),p(120,31),p(300,31),p(360,32),p(480,31),p(600,27),p(840,28),
		 *   p(960,29),p(1080,30),p(1200,31),p(1320,32),p(1500,32),p(1560,31),p(1800,28),
		 *   p(1920,31),p(2040,31),p(2160,34),p(2280,33),p(2400,32),p(2520,31),p(2760,32),
		 *   p(2880,31),p(3000,30),p(3120,32),p(3240,31),p(3420,30),p(3480,30),p(3720,29),
		 *   p(3840,31),p(3960,31),p(4140,31),p(4200,32),p(4320,31),p(4440,27),p(4680,28),
		 *   p(4800,29),p(4920,30),p(5040,31),p(5160,32),p(5400,32),p(5640,31),p(6120,31),
		 *   p(6240,33),p(6360,32),p(6480,31),p(6600,30),p(6720,29),p(6840,29),p(6960,28),
		 *   p(7020,30),p(7080,32),p(7200,32),p(7320,31),p(7560,28),p(7800,27),p(8280,31),
		 *   p(8400,33),p(8520,32),p(8640,31),p(8760,30),p(8880,29),p(9000,29),p(9120,28),
		 *   p(9180,30),p(9240,32),p(9360,32),p(9480,31),p(9720,28),p(9960,27)),
		 *   V(v(0,0)))
		 * 
		 */

		PointSet pointSet = PointSet.getPointSetFromTECString("T(P(p(0,31),p(120,31),p(300,31),p(360,32),p(480,31),p(600,27),p(840,28),p(960,29),p(1080,30),p(1200,31),p(1320,32),p(1500,32),p(1560,31),p(1800,28),p(1920,31),p(2040,31),p(2160,34),p(2280,33),p(2400,32),p(2520,31),p(2760,32),p(2880,31),p(3000,30),p(3120,32),p(3240,31),p(3420,30),p(3480,30),p(3720,29),p(3840,31),p(3960,31),p(4140,31),p(4200,32),p(4320,31),p(4440,27),p(4680,28),p(4800,29),p(4920,30),p(5040,31),p(5160,32),p(5400,32),p(5640,31),p(6120,31),p(6240,33),p(6360,32),p(6480,31),p(6600,30),p(6720,29),p(6840,29),p(6960,28),p(7020,30),p(7080,32),p(7200,32),p(7320,31),p(7560,28),p(7800,27),p(8280,31),p(8400,33),p(8520,32),p(8640,31),p(8760,30),p(8880,29),p(9000,29),p(9120,28),p(9180,30),p(9240,32),p(9360,32),p(9480,31),p(9720,28),p(9960,27)),V(v(0,0)))");

		TEC tec1 = new TEC("T(P(p(2520,31),p(2880,31),p(3000,30),p(3420,30)),V(v(0,0),v(3600,0),v(5760,0)))");
		TEC tec2 = new TEC("T(P(p(120,31),p(1320,32),p(3960,31)),V(v(0,0),v(180,0),v(480,-4),v(4800,-1)))");

		tec1.setDataset(pointSet);
		tec2.setDataset(pointSet);

		System.out.println("tec1: cr="+tec1.getCompressionRatio());
		System.out.println("tec2: cr="+tec2.getCompressionRatio());

		System.out.println("tec1: compactness="+tec1.getCompactness());
		System.out.println("tec2: compactness="+tec2.getCompactness());

		System.out.println("tec1: coverage="+tec1.getCoverage());
		System.out.println("tec2: coverage="+tec2.getCoverage());

		System.out.println("tec1: pattern size="+tec1.getPatternSize());
		System.out.println("tec2: pattern size="+tec2.getPatternSize());

		System.out.println("tec1: pattern width="+tec1.getPattern().getWidth());
		System.out.println("tec2: pattern width="+tec2.getPattern().getWidth());

		System.out.println("tec1: BB area = "+tec1.getBBArea());
		System.out.println("tec2: BB area = "+tec2.getBBArea());

		TECQualityComparator comparator = new TECQualityComparator();

		System.out.println("compare(tec1,tec2) = "+comparator.compare(tec1,tec2));
		System.out.println("compare2(tec1,tec2) = "+comparator.compare2(tec1,tec2));
	}
}

