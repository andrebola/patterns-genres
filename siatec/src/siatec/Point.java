/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;

public class Point implements Comparable<Point>{
	private long x;
	private int y;
	private Integer voice;
	private Long duration;

	public Point(int x, int y) {
		setX(x);
		setY(y);
	}

	public Point(long x, int y) {
		setX(x);
		setY(y);
	}

	public Point(String l) {
		if (l.startsWith("p(")) {
			int start = 2;
			int end = l.indexOf(",");
			x = Long.parseLong(l.substring(start,end));
			start = end+1;
			end = l.indexOf(")");
			y = Integer.parseInt(l.substring(start,end));
		} else {
			String[] array = l.split(" ");
			x = Long.parseLong(array[0]);
			y = Integer.parseInt(array[1]);
		}
	}

	public Point(long x, int y, Integer voice) {
		setVoice(voice);
		setX(x);
		setY(y);
	}

	public Point(Long onset, Integer pitch, Integer voice, Long duration) {
		this(onset,pitch,voice);
		setDuration(duration);
	}
	
	private void setDuration(Long duration) {
		this.duration = duration;
	}
	
	private void setVoice(Integer voice) {
		this.voice = voice;
	}
	
	private void setX(int x) {
		this.x = x;
	}
	
	private void setX(long x) {
		this.x = x;
	}

	private void setY(int y) {
		this.y = y;
	}

	public long getX() {
		return x;
	}

	public int getY() {
		return y;
	}

	public Integer getVoice() {
		return voice;
	}
	
	public Long getDuration() {
		return duration;
	}

	public String toString() {
		return "p("+x+","+y+")";
	}

	@Override
	public int compareTo(Point p) {
		if (p == null) return 1;
		long d = getX() - p.getX();
		if (d < 0l) return -1;
		if (d > 0l) return 1;
		return getY() - p.getY();
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == null) return false;
		if (!(obj instanceof Point)) return false;
		return compareTo((Point)obj) == 0;
	}

	public Point translate(Vector vector) {
		return new Point(getX()+vector.getX(),getY()+vector.getY());
	}

	public Point copy() {
		return new Point(getX(),getY());
	}

	public Point getInversion() {
		return new Point(getX(),-getY());
	}

	public Point getRetrograde() {
		return new Point(-getX(),getY());
	}
}
