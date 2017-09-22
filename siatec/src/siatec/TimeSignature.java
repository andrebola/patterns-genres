package siatec;

public class TimeSignature implements Comparable<TimeSignature> {
	private Long onset, offset;
	private Integer numerator, denominator;
	
	public TimeSignature(Long onset, Long offset, Integer numerator, Integer denominator) {
		super();
		this.onset = onset;
		this.offset = offset;
		this.numerator = numerator;
		this.denominator = denominator;
	}

	public Long getOnset() {
		return onset;
	}

	public void setOnset(Long onset) {
		this.onset = onset;
	}

	public Long getOffset() {
		return offset;
	}

	public void setOffset(Long offset) {
		this.offset = offset;
	}

	public Integer getNumerator() {
		return numerator;
	}

	public void setNumerator(Integer numerator) {
		this.numerator = numerator;
	}

	public Integer getDenominator() {
		return denominator;
	}

	public void setDenominator(Integer denominator) {
		this.denominator = denominator;
	}

	public float getCrotchetsPerBar() {
		return (4.0f * getNumerator())/getDenominator();
	}
		
	@Override
	public String toString() {
		return "TimeSignature("+onset+","+offset+","+numerator+","+denominator+")";
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (!(obj instanceof TimeSignature))
			return false;
		TimeSignature other = (TimeSignature) obj;
		if (!denominator.equals(other.denominator))
			return false;
		if (numerator == null) {
			if (other.numerator != null)
				return false;
		} else if (!numerator.equals(other.numerator))
			return false;
		if (offset == null) {
			if (other.offset != null)
				return false;
		} else if (!offset.equals(other.offset))
			return false;
		if (onset == null) {
			if (other.onset != null)
				return false;
		} else if (!onset.equals(other.onset))
			return false;
		return true;
	}

	@Override
	public int compareTo(TimeSignature ts) {
		if (equals(ts)) return 0;
		int r;
		r = getOnset().compareTo(ts.getOnset()); if (r != 0) return r;
		r = getOffset().compareTo(ts.getOffset()); if (r != 0) return r;
		r = getNumerator().compareTo(ts.getNumerator()); if (r != 0) return r;
		r = getDenominator().compareTo(ts.getDenominator()); if (r != 0) return r;
		return 0;
	}
	
	
}
