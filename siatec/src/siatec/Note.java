package siatec;

public class Note implements Comparable<Note>{
	private Long 	onset = null;
	private Long 	duration = null;
	private Pitch	pitch = new Pitch();
	private Integer staff = null;
	private Integer channel = null;
	private Integer voice = null;
	private Notes notes = null;
	private Pitch computedPitch = new Pitch();

	public static Integer STAFF = null;
	public static Long ONSET = null;
	public static Long DURATION = null;
	public static Integer VOICE = null;
	public static String PITCH_NAME = null;
	public static Long INTER_ONSET = null;
	public static boolean CHORD = false;

	public Pitch getComputedPitch() {
		return computedPitch;
	}

	public void setPitch(Pitch pitch) {
		this.pitch = pitch;
	}

	public Note() {}

	public Note(Notes notes) {
		onset = ONSET;
		duration = DURATION;
		if (INTER_ONSET != null && CHORD == false)
			ONSET += INTER_ONSET;
		else if (CHORD == false)
			ONSET += DURATION;
		pitch.setPitchName(PITCH_NAME);
		staff = STAFF;
		voice = VOICE;
		setNotes(notes);
	}

	public Note(long onset, int midiNoteNumber, long duration, int channel, Notes notes) throws IllegalArgumentException {
		setOnset(onset);
		pitch.setMIDINoteNumber(midiNoteNumber);
		setChannel(channel);
		setDuration(duration);
		setNotes(notes);
	}

	/**
	 * Assumes l has form: Staff PitchName Onset Offset Voice
	 * @param l
	 */
	public Note(String l, Notes notes) {
		//Assume l has form: Staff PitchName Onset Offset Voice
		String[] array = l.split(" ");
		staff = Integer.parseInt(array[0]);
		String pitchName = array[1];
		pitch.setPitchName(pitchName);
		onset = Long.parseLong(array[2]);
		duration = Long.parseLong(array[3])-onset;
		if (array.length > 4)
			voice = Integer.parseInt(array[4]);
		setNotes(notes);
	}

	public Pitch getPitch() {
		return pitch;
	}

	public String getPitchName() {
		return pitch.getPitchName();
	}

	public void setPitchName(String pitchName) {
		pitch.setPitchName(pitchName);
	}

	public Integer getStaff() {
		return staff;
	}

	public void setStaff(Integer staff) {
		this.staff = staff;
	}

	public Long getOnset() {
		return onset;
	}

	public Long getOffset() {
		return onset + duration;
	}

	private void setOnset(Long onset) {
		this.onset = onset;
	}

	public Long getDuration() {
		return duration;
	}

	public void setDuration(Long duration) {
		this.duration = duration;
	}

	public Integer getMidiNoteNumber() {
		return pitch.getMIDINoteNumber();
	}

	public void setMidiNoteNumber(Integer midiNoteNumber) {
		pitch.setMIDINoteNumber(midiNoteNumber);
	}

	public Integer getChannel() {
		return channel;
	}

	public void setChannel(Integer channel) {
		this.channel = channel;
	}

	public Integer getVoice() {
		return voice;
	}

	public void setVoice(Integer voice) {
		this.voice = voice;
	}

	public void setNotes(Notes notes) {
		this.notes = notes;
	}

	public Notes getNotes() {
		return notes;
	}

	public Integer getMetricLevel() {
		return getNotes().getMetricLevel(getOnset());
	}

	public int compareTo(Note n) {
		if (n == null) return 1;

		int d;

		if (getOnset() == null && n.getOnset() != null) return -1;
		if (getOnset() != null && n.getOnset() == null) return 1;
		if (getOnset() != null) {
			d = getOnset().compareTo(n.getOnset());
			if (d != 0) return d;
		}
		
		d = getPitch().compareTo(n.getPitch());
		if (d != 0) return d;

		if (getDuration() == null && n.getDuration() != null) return -1;
		if (getDuration() != null && n.getDuration() == null) return 1;
		if (getDuration() != null) {
			d = getDuration().compareTo(n.getDuration());
			if (d != 0) return d;
		}
		
		if (getChannel() == null && n.getChannel() != null) return -1;
		if (getChannel() != null && n.getChannel() == null) return 1;
		if (getChannel() != null) {
			d = getChannel().compareTo(n.getChannel());
			if (d != 0) return d;
		}

		if (getVoice() == null && n.getVoice() != null) return -1;
		if (getVoice() != null && n.getVoice() == null) return 1;
		if (getVoice() != null) {
			d = getVoice().compareTo(n.getVoice());
			if (d != 0) return d;
		}

		if (getStaff() == null && n.getStaff() != null) return -1;
		if (getStaff() != null && n.getStaff() == null) return 1;
		if (getStaff() != null) {
			d = getStaff().compareTo(n.getStaff());
			if (d != 0) return d;
		}

		return 0;
	}

	@Override
	public boolean equals(Object obj) {
		if (!(obj instanceof Note)) return false;
		return (compareTo((Note)obj) == 0);
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder("Note(");
//		private Long 	onset = null;
		sb.append("onset("+onset+")");
//		private Long 	duration = null;
		if (duration != null)
			sb.append(", duration("+duration+")");
//		private Pitch	pitch = new Pitch();
		if (pitch != null)
			sb.append(", pitch("+pitch+")");
//		private Integer staff = null;
		if (staff != null)
			sb.append(", staff("+staff+")");
//		private Integer channel = null;
		if (channel != null)
			sb.append(", channel("+channel+")");
//		private Integer voice = null;
		if (voice != null)
			sb.append(", voice("+voice+")");
//		private Notes notes = null;
//		private Pitch computedPitch = new Pitch();
		if (computedPitch != null)
			sb.append(", computedPitch("+computedPitch+")");
		return sb.toString();
	}

	/**
	 * Constructs a new Note from a string containing the contents of an OPND datum.
	 * Such a string always has the format
	 * onset pitchName duration voiceNumber
	 * @param x
	 * @return
	 */
	public static Note fromOPNDString(String s, Notes notes) {
		Note note = new Note();
		String[] a = s.split(" ");
		if (a.length != 4)
			System.out.println(s);
		note.setOnset(Long.parseLong(a[0]));
		Pitch pitch = new Pitch();
		pitch.setPitchName(a[1]);
		note.setPitch(pitch);
		note.setDuration(Long.parseLong(a[2]));
		if (a.length > 3)
			note.setVoice(Integer.parseInt(a[3]));
		note.setNotes(notes);
		return note;
	}

	/**
	 * Constructs a new Note from an OPND datum.
	 * onset pitchName duration voiceNumber
	 * @param onset, pitchName, duration, voice, notes
	 * @return
	 */
	public static Note fromOPND(Long onsetTime, String pitchName, Long duration, int voice, Notes notes) {
		Note note = new Note();
		note.setOnset(onsetTime);
		Pitch pitch = new Pitch();
		try {
		pitch.setPitchName(pitchName);
		} catch(IllegalArgumentException e) {
			System.out.println("Illegal pitch name in note: "+onsetTime+" "+pitchName+" "+duration+" "+voice);
		}
		note.setPitch(pitch);
		note.setDuration(duration);
		note.setVoice(voice);
		note.setNotes(notes);
		return note;
	}

	/**
	 * Constructs a new Note from a string containing the contents of an ECOLM datum.
	 * Such a string always has the format
	 * onset midi
	 * @param x
	 * @return
	 */
	public static Note fromECOLMString(String s, Notes notes) {
		System.out.println("fromECOLMString called with "+s);
		Note note = new Note();
		String[] a = s.split(" ");
		note.setOnset(Long.parseLong(a[0]));
		int midiNoteNumber = Integer.parseInt(a[1]);
		System.out.println(midiNoteNumber);
		note.setMidiNoteNumber(midiNoteNumber);
		note.setNotes(notes);
		return note;
	}


}
