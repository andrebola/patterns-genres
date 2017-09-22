package siatec;

/**
 * Represents an improper fraction in lowest terms.
 * @author David Meredith
 *
 */
public class Rational implements Comparable<Rational>{
	private Long numerator = null;
	private Long denominator = null;
	
	public Rational(Integer numerator, Integer denominator) throws IllegalArgumentException {
		this(new Long(numerator), new Long(denominator));
	}
	
	public Rational(Long numerator, Long denominator) throws IllegalArgumentException {
		if (denominator.equals(0)) 
			throw new IllegalArgumentException("Cannot construct a Rational object with a denominator equal to zero.");

		/*
		 * Express as lowest terms.
		 */
		Long n = numerator, d = denominator; 
		if (d < 0) {
			n = -n;
			d = -d;
		}
		Long gcd = Maths.gcd(n,d);
		setNumerator(n/gcd);
		setDenominator(d/gcd);
	}

	public Long getNumerator() {
		return numerator;
	}
	private void setNumerator(Long numerator) {
		this.numerator = numerator;
	}
	public Long getDenominator() {
		return denominator;
	}
	private void setDenominator(Long denominator) {
		this.denominator = denominator;
	}

	@Override
	public String toString() {
		return  getNumerator()+"/"+getDenominator();
	}

	public Rational subtract(Rational r2) {
		Long n1 = getNumerator();
		Long d1 = getDenominator();
		Long n2 = r2.getNumerator();
		Long d2 = r2.getDenominator();
		return new Rational(n1 * d2 - n2 * d1, d1 * d2);
	}

	public Rational add(Rational r2) {
		Long n1 = getNumerator();
		Long d1 = getDenominator();
		Long n2 = r2.getNumerator();
		Long d2 = r2.getDenominator();
		return new Rational(n1 * d2 + n2 * d1, d1 * d2);
	}

	public Rational multiplyBy(Rational r2) {
		Long n1 = getNumerator();
		Long d1 = getDenominator();
		Long n2 = r2.getNumerator();
		Long d2 = r2.getDenominator();
		return new Rational(n1 * n2, d1 * d2);
	}

	public Rational times(Rational r2) {
		return multiplyBy(r2);
	}

	public Rational divideBy(Rational r2) {
		Long n1 = getNumerator();
		Long d1 = getDenominator();
		Long n2 = r2.getNumerator();
		Long d2 = r2.getDenominator();
		return new Rational(n1 * d2, d1 * n2);
	}

	public Rational over(Rational r2) {
		return divideBy(r2);
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof Rational)) return false;
		return (compareTo((Rational)obj) == 0);
	}

	public int compareTo(Rational rational) {
		Rational r = this.subtract(rational);
		if (r.getNumerator() == 0) return 0;
		if (r.getNumerator() < 0) return -1;
		return 1;
	}

	public boolean greaterThan(Rational r2) {
		return (compareTo(r2) == 1);
	}

	public boolean lessThan(Rational r2) {
		return (compareTo(r2) == -1);
	}

	public boolean greaterThanOrEqualTo(Rational r2) {
		return (compareTo(r2) > -1);
	}

	public boolean lessThanOrEqualTo(Rational r2) {
		return (compareTo(r2) < 1);
	}

	public Rational copy() {
		return new Rational(getNumerator(),getDenominator());
	}

	/**
	 * Checks if s has the form [+,-]<numerator>/<denominator>
	 * The first character may be a '+', a '-' or a digit ('0'..'9')
	 * <numerator> must be a string of digit characters
	 * <denominator> must be a string of digit characters that does not
	 * represent zero.
	 */
	public static boolean isValidRationalString(String s) {
		if (s == null) return false;
		if (s.length()==0) return false;
		int slashIndex = s.indexOf("/");
		if (slashIndex < 0) return false;
		if (!"+-0123456789".contains(s.substring(0,1))) return false;
		for(int i = 1; i < slashIndex; i++)
			if (!"0123456789".contains(s.substring(i,i+1)))
				return false;
		for(int i = slashIndex+1; i < s.length(); i++)
			if (!"0123456789".contains(s.substring(i,i+1)))
				return false;
		if (Integer.parseInt(s.substring(slashIndex+1))==0)
			return false;
		return true;
	}
	
	
	public static Rational parseRational(String rationalString) {
		if (!isValidRationalString(rationalString)) 
			throw new IllegalArgumentException("Illegal argument string passed to Rational.parseRational: "+rationalString);
		int slashIndex = rationalString.indexOf("/");
		String numeratorString = rationalString.substring(rationalString.charAt(0)=='+'?1:0,slashIndex);
		long numerator = Long.parseLong(numeratorString);
		String denominatorString = rationalString.substring(slashIndex+1);
		long denominator = Long.parseLong(denominatorString); 
		return new Rational(numerator,denominator);
	}

}
