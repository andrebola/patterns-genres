package siatec;


public class Pitch implements Comparable<Pitch>{

	/**
	 * If chromaticPitch or lofPos is set, this must be null
	 */
	private Integer chroma = null;

	/**
	 *  If morpheticPitch or lofPos is set, this must be null
	 */
	private Integer morph = null;

	/**
	 *  If chroma is set, this must be null
	 */
	private Integer chromaticPitch = null;

	/**
	 *  If morph is set, this must be null
	 */
	private Integer morpheticPitch = null;

	/**
	 * If lofPos is set, then morpheticModulus must be 7, chromaticModulus must be 12,
	 * and chroma, morph, chromaticPitch and morpheticPitch must all be null.
	 */
	private Integer lofPos = null;

	/**
	 *  This should always be non-null if any of lofPos, chroma or chromaticPitch is set.
	 */
	private Integer chromaticModulus = null;

	/**
	 *  This should always be non-null if any of lofPos, morph or morpheticPitch is set.
	 */
	private Integer morpheticModulus = null;

	private void reset() {
		chroma = null;
		morph = null;
		chromaticPitch = null;
		morpheticPitch = null;
		chromaticModulus = null;
		morpheticModulus = null;
		lofPos = null;
	}

	//Setters

	//	(getPitchName() != null? "\n Pitch name: " + getPitchName():"") +
	public void setPitchName(String pitchName) throws IllegalArgumentException {
		reset();
		CanonicalPitchName pn = new CanonicalPitchName(pitchName);
		String letterName = pn.getLetterName();
		String accidental = pn.getAccidental();
		Integer asaOctaveNumber = pn.getOctaveNumber();
		int morph = "ABCDEFG".indexOf(letterName.toUpperCase());
		int undisplacedChroma = new int[]{0,2,3,5,7,8,10}[morph];
		int displacement = accidental.length();
		if (accidental.equals("n"))
			displacement = 0;
		else if (pn.getAccidentalChar() == 'f')
			displacement = -accidental.length();
		int morpheticOctave = asaOctaveNumber - 1;
		if (morph < 2)
			morpheticOctave++;
		chromaticModulus = 12;
		morpheticModulus = 7;
		chromaticPitch = displacement + undisplacedChroma + (12 * morpheticOctave);
		morpheticPitch = morph + (7 * morpheticOctave);
	}

	//	(getPitchNameClass() != null? "\n Pitch name class: " + getPitchNameClass():"") +

	public void setPitchNameClass(String pitchNameClass) {
		reset();
		CanonicalPitchName pn = new CanonicalPitchName(pitchNameClass+"1");
		Integer morpheticLOFClass = "FCGDAEB".indexOf(pn.getLetterName());
		Integer morpheticLOFCycle;
		if (pn.getAccidental().equals("n"))
			morpheticLOFCycle = 0;
		else if (pn.getAccidentalChar().equals('f'))
			morpheticLOFCycle = - pn.getAccidental().length();
		else
			morpheticLOFCycle = pn.getAccidental().length();
		lofPos = 7 * morpheticLOFCycle + morpheticLOFClass;
		chromaticModulus = 12;
		morpheticModulus = 7;
	}

	//	(getPitchLetterName() != null? "\n Pitch letter name: " + getPitchLetterName():"") +

	public void setPitchLetterName(String pitchLetterName) {
		reset();
		Integer m = "ABCDEFG".indexOf(pitchLetterName);
		if (m != -1) {
			morph = m;
		}
	}
	//	(getAccidental() != null? "\n Accidental: " + getAccidental():"") +
	//	(getDisplacement() != null? "\n Displacement: " + getDisplacement():"") +
	//	(getASAOctaveNumber() != null? "\n Octave number: " + getASAOctaveNumber():"") +
	//	(getChromamorpheticPitch() != null? "\n Chromamorphetic pitch: " + getChromamorpheticPitch():"") +

	public void setChromamorpheticPitch(Integer chromaticPitch, Integer morpheticPitch, Integer chromaticModulus, Integer morpheticModulus) {
		if (chromaticModulus < 1) throw new IllegalArgumentException("Chromatic modulus must be greater than 0.");
		if (morpheticModulus < 1) throw new IllegalArgumentException("Morphetic modulus must be greater than 0.");
		if (morpheticModulus > chromaticModulus) throw new IllegalArgumentException("Morphetic modulus must be less than or equal to chromatic modulus.");
		reset();
		this.chromaticPitch = chromaticPitch;
		this.morpheticPitch = morpheticPitch;
		this.chromaticModulus = chromaticModulus;
		this.morpheticModulus = morpheticModulus;
	}

	public void setChromamorpheticPitch(Integer chromaticPitch, Integer morpheticPitch) {
		setChromamorpheticPitch(chromaticPitch, morpheticPitch, 12, 7);
	}

	//	(getContinuousBinomialRepresentation() != null? "\n Continuous binomial representation (Brinkman): " + getContinuousBinomialRepresentation():"") +

	public void setContinuousBinomialRepresentation(Integer continuousPitchCode, Integer continuousNameCode) {
		setChromamorpheticPitch(continuousPitchCode - 9, continuousNameCode - 5);
	}

	//	(getChromaticPitch() != null? "\n Chromatic pitch: " + getChromaticPitch():"") +

	public void setChromaticPitch(Integer chromaticPitch, Integer chromaticModulus) throws IllegalArgumentException {
		if (chromaticModulus < 1) throw new IllegalArgumentException("Chromatic modulus must be greater than 0.");
		reset();
		this.chromaticPitch = chromaticPitch;
		this.chromaticModulus = chromaticModulus;
	}

	public void setChromaticPitch(Integer chromaticPitch) {
		setChromaticPitch(chromaticPitch,12);
	}

	//	(getContinuousPitchCode() != null? "\n Continuous pitch code (Brinkman): "+ getContinuousPitchCode():"") +

	public void setContinuousPitchCode(Integer continuousPitchCode) {
		setChromaticPitch(continuousPitchCode - 9);
	}

	//	(getMIDINoteNumber() != null? "\n MIDI note number: " + getMIDINoteNumber():"") +

	public void setMIDINoteNumber(Integer midiNoteNumber) throws IllegalArgumentException {
		if (midiNoteNumber < 128 && midiNoteNumber > -1) 
			setChromaticPitch(midiNoteNumber - 21);
		else
			throw new IllegalArgumentException("MIDI note number must be between 0 and 127, inclusive.");
	}

	//	(getMorpheticPitch() != null? "\n Morphetic pitch: " + getMorpheticPitch():"") +

	public void setMorpheticPitch(Integer morpheticPitch, Integer morpheticModulus) throws IllegalArgumentException {
		if (morpheticModulus < 0) throw new IllegalArgumentException("Morphetic modulus must be greater than 0.");
		reset();
		this.morpheticPitch = morpheticPitch;
		this.morpheticModulus = morpheticModulus;
	}

	public void setMorpheticPitch(Integer morpheticPitch) {
		setMorpheticPitch(morpheticPitch, 7);
	}

	//	(getContinuousNameCode() != null? "\n Continuous name code (Brinkman): " + getContinuousNameCode():"") +

	public void setContinuousNameCode(Integer continuousNameCode) {
		setMorpheticPitch(continuousNameCode - 5);
	}

	//	(getDiatone() != null? "\n Diatone (Regener): " + getDiatone():"") +
	public void setDiatone(Integer diatone) {
		setMorpheticPitch(diatone + 17);
	}

	//	(getChroma() != null? "\n Chroma: " + getChroma():"") +
	public void setChroma(Integer chroma, Integer chromaticModulus) throws IllegalArgumentException {
		if (chromaticModulus <= chroma || chroma < 0) throw new IllegalArgumentException("Chroma must be greater than or equal to 0 and chromatic modulus must be greater than chroma.");
		reset();
		this.chroma = chroma;
		this.chromaticModulus = chromaticModulus;
	}

	public void setChroma(Integer chroma) {
		setChroma(chroma, 12);
	}

	//	(getPitchClass() != null? "\n Pitch class: " + getPitchClass():"") +

	public void setPitchClass(Integer pitchClass, Integer chromaticModulus){
		setChroma(Maths.mod(pitchClass+3, chromaticModulus),chromaticModulus);
	}
	//	(getMorph() != null? "\n Morph: " + getMorph():"") +

	public void setMorph(Integer morph, Integer morpheticModulus) throws IllegalArgumentException {
		if (morpheticModulus <= morph || morph < 0) throw new IllegalArgumentException("Morph must be greater than or equal to 0 and morphetic modulus must be greater than morph.");
		reset();
		this.morph = morph;
		this.morpheticModulus = morpheticModulus;
	}

	public void setMorph(Integer morph){
		setMorph(morph, 7);
	}

	//	(getNameClass() != null? "\n Name class (Brinkman): " + getNameClass():"") +

	public void setNameClass(Integer nameClass) throws IllegalArgumentException {
		if (nameClass < 0 || nameClass > 6) throw new IllegalArgumentException("Name class (Brinkman) must be between 0 and 6, inclusive.");
		setMorph(Maths.mod(nameClass+2,7));
	}

	//	(getDiatonicNoteClass() != null? "\n Diatonic note class (Regener): "+ getDiatonicNoteClass():"") +

	public void setDiatonicNoteClass(Integer diatonicNoteClass) throws IllegalArgumentException {
		if (diatonicNoteClass < 0 || diatonicNoteClass > 6) throw new IllegalArgumentException("Diatonic note class (Regener) must be between 0 and 6, inclusive.");
		setMorph(Maths.mod(diatonicNoteClass-2, 7));
	}

	//	(getChromaticOctave() != null? "\n Chromatic octave: " + getChromaticOctave():"") +
	//	(getMorpheticOctave() != null? "\n�Morphetic octave: " + getMorpheticOctave():"") +
	//	(getLOFPosition() != null? "\n Line-of-fifths position: " + getLOFPosition():"") +

	public void setLOFPosition(Integer lofPosition) {
		reset();
		this.lofPos = lofPosition;
	}

	//	(getQuint() != null? "\n Quint (Regener): " + getQuint():"") +

	public void setQuint(Integer quint) {
		setLOFPosition(quint);
	}
	//	(getSharpness() != null? "\n Sharpness (Longuet-Higgins): " + getSharpness():"") +

	public void setSharpness(Integer sharpness) {
		setLOFPosition(sharpness + 1);
	}

	//	(getTonalPitchClass() != null? "\n Tonal pitch class (Temperley): " + getTonalPitchClass():"") +

	public void setTonalPitchClass(Integer tonalPitchClass) {
		setLOFPosition(tonalPitchClass - 1);
	}

	//	(getMorpheticLOFClass() != null? "\n Morphetic line-of-fifths class: " + getMorpheticLOFClass():"") + 
	//	(getMorpheticLOFCycle() != null? "\n�Morphetic line-of-fifths cycle: " + getMorpheticLOFCycle():"") + 
	//	(getChromaticLOFClass() != null? "\n�Chromatic line-of-fifths class: " + getChromaticLOFClass():"") + 
	//	(getChromaticLOFCycle() != null? "\n�Chromatic line-of-fifths cycle: " + getChromaticLOFCycle():"") + 
	//	(getBase40() != null? "\n�Base 40 (Hewlett): " + getBase40():"");

	public void setBase40(Integer base40) {
		int i = 0;
		int newBase40 = base40;
		while (!(newBase40 >= 1 && newBase40 <= 40)) {
			if (newBase40 < 1) {
				newBase40 += 40;
				i--;
			} else {
				newBase40 -= 40;
				i++;
			}
		}
		int asaOctaveNumber = 1 + i;
		int lofPosition = Maths.mod(7 * (newBase40 - 20),40);
		int mlofc = Maths.mod(lofPosition,7);
		int displacement = Maths.floor(lofPosition,7);
		String letterName = "FCGDAEB".substring(mlofc,mlofc+1);
		String accidental = null;
		if (displacement == 0) accidental = "n";
		else if (displacement > 0) {
			accidental = "";
			for (int j = 0; j < displacement; j++) accidental += "s";
		} else {
			accidental = "";
			for (int j = 0; j < Math.abs(displacement); j++) accidental += "f";
		}
		setPitchName(letterName+accidental+asaOctaveNumber);
	}

	//Getters

	public Integer getChromaticModulus() {
		return chromaticModulus;
	}

	public Integer getMorpheticModulus() {
		return morpheticModulus;
	}

	public String getPitchName() {
		String l = getPitchLetterName();
		String a = getAccidental();
		Integer o = getASAOctaveNumber();
		if (l != null && a != null && o != null)
			return l + a + o;
		return null;
	}

	public String getPitchNameClass() {
		String l = getPitchLetterName();
		String a = getAccidental();
		if (l != null && a != null)
			return l + a;
		return null;
	}

	public String getPitchLetterName() {
		Integer m = getMorph();
		if (m != null)
			return "ABCDEFG".substring(m,m+1);
		return null;
	}

	public String getAccidental() {
		Integer displacement = getDisplacement();
		if (displacement != null) {
			String accidental = "";
			String accidentalChar = "";
			if (displacement != 0) {
				if (displacement < 0)
					accidentalChar = "f";
				else
					accidentalChar = "s";
				for (int i = 0; i < Math.abs(displacement); i++)
					accidental += accidentalChar;
			} else
				accidental = "n";
			return accidental;
		}
		return null;
	}

	public Integer getDisplacement() {
		if (morpheticPitch != null && chromaticPitch != null && morpheticModulus == 7 && chromaticModulus == 12) {
			int undisplacedChroma = new int[]{0,2,3,5,7,8,10}[getMorph()];
			return getChromaticPitch() - 12 * Maths.floor(getMorpheticPitch(), getMorpheticModulus()) - undisplacedChroma;
		}
		if (lofPos != null)
			return Maths.floor(lofPos, getMorpheticModulus());
		return null;
	}

	public Integer getASAOctaveNumber() {
		Integer o = getMorpheticOctave();
		Integer m = getMorph();
		if (o != null && m != null)
			if (m < 2)
				return o;
			else
				return o + 1;
		return null;
	}

	public Integer[] getChromamorpheticPitch() {
		Integer cp = getChromaticPitch();
		Integer mp = getMorpheticPitch();
		if (cp != null && mp != null)
			return new Integer[]{cp,mp};
		return null;
	}

	public Integer[] getContinuousBinomialRepresentation() {
		Integer[] cmp = getChromamorpheticPitch();
		if (cmp == null) return null;
		return new Integer[]{cmp[0]+9,cmp[1]+5};
	}

	public Integer getChromaticPitch() {
		if (chromaticPitch != null)
			return chromaticPitch;
		return null;
	}

	public Integer getContinuousPitchCode() {
		if (chromaticPitch != null)
			return chromaticPitch + 9;
		return null;
	}

	public Integer getMIDINoteNumber() {
		if (chromaticPitch != null) {
			Integer m = chromaticPitch + 21;
			if (m >= 0 && m < 128)
				return m;
		}
		return null;
	}

	public Integer getMorpheticPitch() {
		if (morpheticPitch != null)
			return morpheticPitch;
		else 
			return null;
	}

	public Integer getContinuousNameCode() {
		if (morpheticPitch != null)
			return morpheticPitch + 5;
		return null;
	}

	public Integer getDiatone() {
		if (morpheticPitch != null)
			return morpheticPitch - 17;
		return null;
	}

	public Integer getChroma() {
		if (chroma != null)
			return chroma;
		if (chromaticPitch != null)
			return Maths.mod(chromaticPitch, chromaticModulus);
		if (lofPos != null)
			return Maths.mod(8 + (7 * getLOFPosition()), getChromaticModulus());
		return null;
	}

	public Integer getPitchClass() {
		Integer c = getChroma();
		if (c == null) return null;
		return Maths.mod(c-3, getChromaticModulus());
	}

	public Integer getMorph() {
		if (morph != null)
			return morph;
		if (morpheticPitch != null)
			return Maths.mod(morpheticPitch, getMorpheticModulus());
		if (lofPos != null)
			return Maths.mod(5 + (4 * getLOFPosition()), getMorpheticModulus());
		return null;
	}

	public Integer getNameClass() {
		Integer m = getMorph();
		if (m == null) return null;
		return Maths.mod(m-2, getMorpheticModulus());
	}

	public Integer getDiatonicNoteClass() {
		Integer m = getMorph();
		if (m == null) return null;
		return Maths.mod(m+2, getMorpheticModulus());
	}

	public Integer getChromaticOctave() {
		Integer cp = getChromaticPitch();
		if (cp == null) return null;
		return Maths.floor(cp,getChromaticModulus());
	}

	public Integer getMorpheticOctave() {
		Integer mp = getMorpheticPitch();
		if (mp == null) return null;
		return Maths.floor(mp,getMorpheticModulus());
	}

	public Integer getLOFPosition() {
		if (lofPos != null) return lofPos;
		Integer mlcycle = getMorpheticLOFCycle();
		Integer mlclass = getMorpheticLOFClass();
		if (mlcycle != null && mlclass != null) {
			return (mlcycle * getMorpheticModulus() + mlclass);
		}
		return null;
	}

	public Integer getQuint() {
		return getLOFPosition();
	}

	public Integer getSharpness() {
		Integer l = getLOFPosition();
		if (l == null) return null;
		return l - 1;
	}

	public Integer getTonalPitchClass() {
		Integer l = getLOFPosition();
		if (l == null) return null;
		return l + 1;
	}

	public Integer getMorpheticLOFClass() {
		if (lofPos != null) return Maths.mod(lofPos, getMorpheticModulus());
		Integer m = getMorph();
		if (m != null)
			return Maths.mod(2 * (m - 5), getMorpheticModulus());
		return null;
	}

	public Integer getMorpheticLOFCycle() {
		return getDisplacement();
	}

	public Integer getChromaticLOFClass() {
		if (lofPos != null) return Maths.mod(lofPos, getChromaticModulus());
		Integer c = getChroma();
		if (c != null)
			return Maths.mod(7 * (c - 8), getChromaticModulus());
		return null;
	}

	public Integer getChromaticLOFCycle() {
		if (getLOFPosition() != null) return Maths.floor(getLOFPosition(), getChromaticModulus());
		return null;
	}

	public Integer getBase40() {
		Integer l = getLOFPosition();
		Integer o = getASAOctaveNumber();
		if (l == null || o == null || l > 20 || l < -14) return null;
		return (o - 1) * 40 + Maths.mod(((l * 23) + 20),40);
	}

	public int compareTo(Pitch p) {
		if (equals(p)) return 0;
		if (getChromaticModulus() != null && p.getChromaticModulus() != null &&
				getChromaticModulus() > p.getChromaticModulus()) return 1;
		if (getChromaticModulus() != null && p.getChromaticModulus() != null &&
				getChromaticModulus() < p.getChromaticModulus()) return -1;
		if (getMorpheticModulus() != null && p.getMorpheticModulus() != null &&
				getMorpheticModulus() > p.getMorpheticModulus()) return 1;
		if (getMorpheticModulus() != null && p.getMorpheticModulus() != null &&
				getMorpheticModulus() < p.getMorpheticModulus()) return -1;
		if (getChromaticPitch() != null && p.getChromaticPitch() != null &&
				getChromaticPitch() > p.getChromaticPitch()) return 1;
		if (getChromaticPitch() != null && p.getChromaticPitch() != null &&
				getChromaticPitch() < p.getChromaticPitch()) return -1;
		if (getMorpheticPitch() != null && p.getMorpheticPitch() != null && 
				getMorpheticPitch() > p.getMorpheticPitch()) return 1;
		if (getMorpheticPitch() != null && p.getMorpheticPitch() != null &&
				getMorpheticPitch() < p.getMorpheticPitch()) return -1;
		if (getLOFPosition()!=null && p.getLOFPosition()!= null &&
				getLOFPosition() > p.getLOFPosition()) return 1;
		if (getLOFPosition() != null && p.getLOFPosition() != null &&
				getLOFPosition() < p.getLOFPosition()) return -1;
		if (getChroma() != null && p.getChroma() != null &&
				getChroma() > p.getChroma()) return 1;
		if (getChroma() != null && p.getChroma() != null &&
				getChroma() < p.getChroma()) return -1;
		if (getMorph() != null && p.getMorph() != null &&
				getMorph() > p.getMorph()) return 1;
		if (getMorph() != null && p.getMorph() != null &&
				getMorph() < p.getMorph()) return -1;
		return 0;
	}

	public boolean lessThan(Pitch p) {
		return (compareTo(p) == -1);
	}

	public boolean greaterThan(Pitch p) {
		return (compareTo(p) == 1);
	}

	public boolean greaterThanOrEqualTo(Pitch p) {
		return (compareTo(p) != -1);
	}

	public boolean lessThanOrEqualTo(Pitch p) {
		return (compareTo(p) != 1);
	}


	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof Pitch)) return false;
		Pitch p = (Pitch)obj;
		return (getChroma() == p.getChroma() &&
				getMorph() == p.getMorph() &&
				getChromaticPitch() == p.getChromaticPitch() &&
				getMorpheticPitch() == p.getMorpheticPitch() &&
				getLOFPosition() == p.getLOFPosition() &&
				getChromaticModulus() == p.getChromaticModulus() &&
				getMorpheticModulus() == p.getMorpheticModulus());
	}

	public String toString() {
		StringBuilder sb = new StringBuilder("Pitch(");
		boolean comma = false;
		if (getPitchName() != null) {
			sb.append(getPitchName());
			comma = true;
		} else if (getPitchNameClass() != null) {
			if (comma) sb.append(",");
			sb.append(getPitchNameClass());
			comma = true;
		} else if (getChromaticPitch() != null) {
			if (comma) sb.append(",");
			sb.append("ChromaticPitch("+getChromaticPitch()+")");
			comma = true;
		} else if (getMorpheticPitch() != null) {
			if (comma) sb.append(",");
			sb.append("MorpheticPitch("+getMorpheticPitch()+")");
			comma = true;
		} else if (getChroma() != null) {
			if (comma) sb.append(",");
			sb.append("Chroma("+getChroma()+")");
			comma = true;
		} else if (getMorph() != null) {
			if (comma) sb.append(",");
			sb.append("Morph("+getMorph()+")");
			comma = true;
		}
		sb.append(")");
		return sb.toString();
	}
}
