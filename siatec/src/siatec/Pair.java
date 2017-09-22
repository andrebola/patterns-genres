/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package siatec;

import java.util.AbstractMap;
import java.util.Map;

/**
 *
 * @author andres
 */
public class Pair extends AbstractMap.SimpleEntry{

    public Pair(Object k, Object v) {
        super(k, v);
    }
    
    

    @Override
    public String toString() {
        return " [" + getKey() +", "+ getValue() + ']';
    }
    
}
