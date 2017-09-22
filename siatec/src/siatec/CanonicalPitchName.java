package siatec;

public class CanonicalPitchName {
	private String letterName, accidental;
	private Integer octaveNumber;
	private Character accidentalChar;
	
	public Character getAccidentalChar() {
		return accidentalChar;
	}

	public CanonicalPitchName(String pitchName) throws IllegalArgumentException {
		if (pitchName.length() < 2) throw new IllegalArgumentException("Pitch name must have more than 1 character:" + pitchName);
		String pn = pitchName.toUpperCase();
		letterName = pn.substring(0,1);
		if (!("ABCDEFG".contains(letterName))) throw new IllegalArgumentException("First character in pitch name is "+letterName+". This must be a letter between A and G, inclusive.");
		int octavePos = 1;
		while (octavePos < pn.length() && !("-0123456789".contains(""+pn.substring(octavePos,octavePos+1)))) octavePos++;
		if (octavePos == pn.length()) throw new IllegalArgumentException("Pitch name must end with an octave number.");
		String octaveNumberString = pn.substring(octavePos);
		for (int i = 1; i < octaveNumberString.length(); i++) {
			if (!Character.isDigit(octaveNumberString.charAt(i)))
				throw new IllegalArgumentException("Pitch name must end with an octave number. ("+pitchName+")");
		}
		octaveNumber = Integer.parseInt(octaveNumberString);
		accidental = pn.substring(1,octavePos).toLowerCase().replace('b', 'f').replace('#','s');
		if (accidental.equals("")) accidental = "n";
		accidentalChar = accidental.charAt(0);
		if (!"fsn".contains(accidentalChar.toString()))
			throw new IllegalArgumentException("First character of accidental must be '#', 's', 'b', 'f' or 'n'.");
		if (accidentalChar == 'n' && accidental.length() > 1)
			throw new IllegalArgumentException("If accidental is natural, then must be a single 'n' or absent.");
		for (int i = 1; i < accidental.length(); i++) {
			if (accidental.charAt(i) != accidentalChar)
				throw new IllegalArgumentException("Accidental must be a single 'n' or a sequence of '#'s, 'b's, 'f's or 's's.");
		}
	}
	
	public String getLetterName() {
		return letterName;
	}

	public String getAccidental() {
		return accidental;
	}

	public Integer getOctaveNumber() {
		return octaveNumber;
	}

}
