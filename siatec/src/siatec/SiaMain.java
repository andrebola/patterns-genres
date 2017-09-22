/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;

import java.io.EOFException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Collections;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.InvalidMidiDataException;
//import processing.core.PApplet;
import static siatec.SIA.computeVectorTable;
import static siatec.SIA.run;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map.Entry;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

/**
 *
 * @author andres
 */
public class SiaMain{

    private PointSet dataset = null;
    private ArrayList<PointSet> drawPatterns = null;

    public static void listf(String directoryName, ArrayList<File> files) {
        File directory = new File(directoryName);

        // get all the files from a directory
        File[] fList = directory.listFiles();
        for (File file : fList) {
            if (file.isFile()) {
                files.add(file);
            } else if (file.isDirectory()) {
                listf(file.getAbsolutePath(), files);
            }
        }
    }

    private SiaMain(ArrayList<PointSet> patterns, PointSet ds) {
        dataset = ds;
        drawPatterns = patterns;
    }
    public static void saveRhythmicPatters(int minimumPatternSize, File f, String destPath) throws NoMorpheticPitchException, UnimplementedInputFileFormatException, IOException, InvalidMidiDataException {
        PitchRepresentation pitchRepresentation = PitchRepresentation.CHROMATIC_PITCH;
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsRhythm1 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsRhythm2 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsRhythm3 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsRhythm4 = new ArrayList<>();
        ArrayList<Entry<Long, Integer>> dsRhythm = new ArrayList<>();
        PointSet ds = new PointSet(f.getAbsolutePath(), pitchRepresentation, true);
        //PointSet ds = new PointSet("/home/andres/Downloads/lmd_full/0/0533b5f7f5bcbe2e73389f207b178578.mid", pitchRepresentation, true);
        if (ds.getPoints().size() == 0) {
            return;
        }
        for (Point point : ds.getPoints()) {
            dsRhythm.add(new Pair(point.getX(), point.getY()));
        }
        VectorPointPair[][] vectorTable = computeVectorTable(ds);
        ArrayList<MtpCisPair> mtpCisPairs = run(
                ds,
                vectorTable,
                false, //forRSuperdiagonals
                0, //r
                false, // withCompactnessTrawler
                0.0, //a
                0, //b
                null, //logPrintStream
                false, //removeTranslationallyEquivalentMtps
                false, //mergeVectors
                minimumPatternSize //minPatternSize
        );
        for (MtpCisPair mtpCisPair : mtpCisPairs) {
            double minimumCompactness = 0.7;
            double minimumTemporalDensity = 0.025;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux.add(new Pair(point.getX(), point.getY()));
                }

                patternsRhythm1.add(aux);
            }

            minimumCompactness = 0.4;
            minimumTemporalDensity = 0.025;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux2 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux2.add(new Pair(point.getX(), point.getY()));
                }
                patternsRhythm2.add(aux2);
            }

            minimumCompactness = 0.7;
            minimumTemporalDensity = 0.25;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux3 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux3.add(new Pair(point.getX(), point.getY()));
                }
                patternsRhythm3.add(aux3);

            }
            minimumCompactness = 0.4;
            minimumTemporalDensity = 0.25;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux4 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux4.add(new Pair(point.getX(), point.getY()));
                }
                patternsRhythm4.add(aux4);
            }
        }

        JSONObject obj = new JSONObject();
        JSONArray jsPatternsRhythm1 = new JSONArray();
        jsPatternsRhythm1.addAll(patternsRhythm1);
        JSONArray jsPatternsRhythm2 = new JSONArray();
        jsPatternsRhythm2.addAll(patternsRhythm2);
        JSONArray jsPatternsRhythm3 = new JSONArray();
        jsPatternsRhythm3.addAll(patternsRhythm3);
        JSONArray jsPatternsRhythm4 = new JSONArray();
        JSONArray jsDsRhythm = new JSONArray();
        jsDsRhythm.addAll(dsRhythm);

        obj.put("SiaRhythm1", jsPatternsRhythm1);
        obj.put("SiaRhythm2", jsPatternsRhythm2);
        obj.put("SiaRhythm3", jsPatternsRhythm3);
        obj.put("SiaRhythm4", jsPatternsRhythm4);
        obj.put("dsRhythm", jsDsRhythm);

        try (FileWriter file = new FileWriter(destPath + "rhythm/" + f.getName().replace(".mid", ".json"))) {
            file.write(obj.toJSONString());
        }

    }

    public static void saveChromaPatterns(int minimumPatternSize, File f, String destPath) throws NoMorpheticPitchException, UnimplementedInputFileFormatException, IOException, InvalidMidiDataException {
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsChroma1 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsChroma2 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsChroma3 = new ArrayList<>();
        ArrayList<ArrayList<Entry<Long, Integer>>> patternsChroma4 = new ArrayList<>();
        
        PitchRepresentation pitchRepresentation = PitchRepresentation.CHROMATIC_PITCH;
        PointSet ds = new PointSet(f.getAbsolutePath(), pitchRepresentation, false);
        //ds = new PointSet("/home/andres/Downloads/lmd_full/0/0533b5f7f5bcbe2e73389f207b178578.mid", pitchRepresentation, false);
        VectorPointPair[][] vectorTable = computeVectorTable(ds);
        ArrayList<MtpCisPair> mtpCisPairs = run(
                ds,
                vectorTable,
                false, //forRSuperdiagonals
                0, //r
                false, // withCompactnessTrawler
                0.0, //a
                0, //b
                null, //logPrintStream
                false, //removeTranslationallyEquivalentMtps
                false, //mergeVectors
                minimumPatternSize //minPatternSize
        );
        for (MtpCisPair mtpCisPair : mtpCisPairs) {
            double minimumCompactness = 0.7;
            double minimumTemporalDensity = 0.05;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux = new ArrayList<>();

                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux.add(new Pair(point.getX(), point.getY()));
                }
                patternsChroma1.add(aux);

            }

            minimumCompactness = 0.4;
            minimumTemporalDensity = 0.05;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux2 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux2.add(new Pair(point.getX(), point.getY()));
                }
                patternsChroma2.add(aux2);
            }

            minimumCompactness = 0.4;
            minimumTemporalDensity = 0.25;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux3 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux3.add(new Pair(point.getX(), point.getY()));
                }
                patternsChroma3.add(aux3);

            }
            minimumCompactness = 0.7;
            minimumTemporalDensity = 0.25;
            if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                    && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                ArrayList<Entry<Long, Integer>> aux4 = new ArrayList<>();
                for (Point point : mtpCisPair.getMtp().getPoints()) {
                    aux4.add(new Pair(point.getX(), point.getY()));
                }
                patternsChroma4.add(aux4);
            }
        }
        JSONObject obj = new JSONObject();

        JSONArray jsPatternsChroma1 = new JSONArray();
        jsPatternsChroma1.addAll(patternsChroma1);
        JSONArray jsPatternsChroma2 = new JSONArray();
        jsPatternsChroma2.addAll(patternsChroma2);
        JSONArray jsPatternsChroma3 = new JSONArray();
        jsPatternsChroma3.addAll(patternsChroma3);
        JSONArray jsPatternsChroma4 = new JSONArray();
        jsPatternsChroma4.addAll(patternsChroma4);

        obj.put("SiaTonic1", jsPatternsChroma1);
        obj.put("SiaTonic2", jsPatternsChroma2);
        obj.put("SiaTonic3", jsPatternsChroma3);
        obj.put("SiaTonic4", jsPatternsChroma4);


        try (FileWriter file = new FileWriter(destPath + "chroma/" + f.getName().replace(".mid", ".json"))) {
            file.write(obj.toJSONString());
        }
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // Whith Pitch: CHROMA / MORPH (0.3) (0.25, 0.05)
        // NO Pitch: CHROMA (0.4, 0.7) (0.25, 0.025)

        boolean draw = false;
        String destPath = "/PATH/TO/patterns/";
        double minimumCompactness = 0.01;
        int minimumPatternSize = 3;
        double minimumTemporalDensity = 0.005;
        PitchRepresentation pitchRepresentation = PitchRepresentation.CHROMATIC_PITCH;

        ArrayList<PointSet> patterns = new ArrayList<PointSet>();
        ArrayList<String> midisList = new ArrayList<String>();

        JSONParser parser = new JSONParser();

        try {

            Object obj = parser.parse(new FileReader("/PATH/TO/midi_w_genre.json"));

            JSONArray jsMidiList = (JSONArray) obj;
            Iterator<String> iterator = jsMidiList.iterator();
            while (iterator.hasNext()) {
                midisList.add(iterator.next());
            }

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ParseException e) {
            e.printStackTrace();
        }

        if (!draw) {

            ArrayList<File> midis = new ArrayList<>();
            listf("/PATH/TO/lmd_full/", midis);
            Collections.shuffle(midis);
	    
            for (File f : midis) {
                try {
                    String midiName = f.getName().replace(".mid", "");
                    if (midisList.contains(midiName)) {
                        // Rhythm
                        String destFile = destPath + "rhythm/" + f.getName().replace(".mid", ".json");
                        File outF = new File(destFile);
                        if (!outF.exists()) {
                            System.out.print(f.getAbsolutePath());
			    try (FileWriter file = new FileWriter(destPath +"rhythm/"+ f.getName().replace(".mid", ".json"))) {
                                file.write("{'status':'pending'}");
                            }
                            saveRhythmicPatters(minimumPatternSize, f, destPath);
                        }
                        
                        // Chroma
                        String destFile2 = destPath + "chroma/" + f.getName().replace(".mid", ".json");
                        File outF2 = new File(destFile2);
                        if (!outF2.exists()) {
                            System.out.print(f.getAbsolutePath());
			    try (FileWriter file2 = new FileWriter(destPath +"chroma/"+ f.getName().replace(".mid", ".json"))) {
                                file2.write("{'status':'pending'}");
                            }
			    saveChromaPatterns(minimumPatternSize, f, destPath);
                        }
                    }
                }catch (EOFException ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                } catch (NoMorpheticPitchException ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                } catch (IOException ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                } catch (UnimplementedInputFileFormatException ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                } catch (InvalidMidiDataException ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                }catch (  Exception ex) {
                    Logger.getLogger(SiaMain.class.getName()).log(Level.SEVERE, null, ex);
                }
 

            }
        } /*else {

            try {
                //pitchRepresentation = PitchRepresentation.MORPHETIC_PITCH;
                PointSet ds = new PointSet(midiFile, pitchRepresentation, false);
                if (ds.getPoints().size() > 0) {
                    VectorPointPair[][] vectorTable = computeVectorTable(ds);
                    ArrayList<MtpCisPair> mtpCisPairs = run(
                            ds,
                            vectorTable,
                            false, //forRSuperdiagonals
                            0, //r
                            true, // withCompactnessTrawler
                            0.0, //a
                            0, //b
                            null, //logPrintStream
                            false, //removeTranslationallyEquivalentMtps
                            false, //mergeVectors
                            minimumPatternSize //minPatternSize
                    );
                    for (MtpCisPair mtpCisPair : mtpCisPairs) {
                        if (mtpCisPair.getMtp().getCompactness(ds) >= minimumCompactness
                                && mtpCisPair.getMtp().getTemporalDensity() >= minimumTemporalDensity) {
                            patterns.add(mtpCisPair.getMtp());
                        }
                        
                    }
                    SiaMain sia = new SiaMain(patterns, ds);
                    sia.draw();
                }
            } catch (NoMorpheticPitchException ex) {
                ex.printStackTrace();
            } catch (IOException ex) {
                ex.printStackTrace();
            } catch (UnimplementedInputFileFormatException ex) {
                ex.printStackTrace();
            } catch (InvalidMidiDataException ex) {
                ex.printStackTrace();
            }
        }*/

    }

}
