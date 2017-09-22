package siatec;

import java.util.ArrayList;

public class Chromagram {
	ArrayList<Integer> histogram = null;;
	
	public Chromagram() {
		histogram = new ArrayList<Integer>();
		for (int i = 0; i < 12; i++) 
			histogram.add(0);
	}
	
	public void addOneToChromaFrequency(int chroma) {
		histogram.set(chroma, histogram.get(chroma)+1);
	}
	
	public Integer get(int i) {
		return histogram.get(i);
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder("<"+histogram.get(0));
		for(int i = 1; i < 12; i++)
			sb.append(","+histogram.get(i));
		sb.append(">");
		return sb.toString();
	}
}
