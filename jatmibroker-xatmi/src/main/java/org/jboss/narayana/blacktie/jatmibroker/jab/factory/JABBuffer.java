/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.narayana.blacktie.jatmibroker.jab.factory;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

/**
 * The JABBuffer is a map of parameters that are passed to/returned from a remote XATMI service.
 * 
 * @see JABResponse
 */
public class JABBuffer {

    /**
     * Any array data types are stored here
     */
    private Map<String, Vector<Object>> arrays = new HashMap<String, Vector<Object>>();

    /**
     * Any individual items are stored here
     */
    private Map<String, Object> items = new HashMap<String, Object>();

    private int xOctetSize;

    /**
     * Get the size of the data
     * 
     * @param key The type of the data to look for
     * @return The size
     */
    public synchronized int size(String key) {
        int toReturn = 0;
        if (key.equals("X_OCTET")) {
            toReturn = xOctetSize;
        } else {
            Vector<Object> vector = arrays.get(key);
            if (vector != null) {
                toReturn = vector.size();
            }
        }
        return toReturn;
    }

    /**
     * Get the value of a certain index of the data
     * 
     * @param key The type of the data
     * @param index The index to check for
     * @return The value
     */
    public synchronized Object getValue(String key, int index) {
        Object toReturn = null;
        Vector<Object> vector = arrays.get(key);
        if (vector != null) {
            toReturn = vector.get(index);
        }
        return toReturn;
    }

    /**
     * Get the value of a certain type of data
     * 
     * @param key The type of the data
     * @return The value
     */
    public synchronized Object getValue(String key) {
        return items.get(key);
    }

    /**
     * Set the value of the data at a certain index
     * 
     * @param key The key to change
     * @param index The index to alter
     * @param value The value to set it to
     */
    public synchronized void setValue(String key, int index, Object value) {
        Vector<Object> vector = arrays.get(key);
        if (vector == null) {
            vector = new Vector<Object>();
            arrays.put(key, vector);
        }
        if (vector.size() <= index) {
            vector.setSize(index + 1);
        }
        vector.set(index, value);
    }

    /**
     * Set the value of an item
     * 
     * @param key The item name
     * @param value The value to use
     */
    public synchronized void setValue(String key, Object value) {
        items.put(key, value);
    }

    void setArrayValue(String key, byte[] array) {
        if (key.equals("X_OCTET")) {
            xOctetSize = array.length;
        }
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    void setArrayValue(String key, short[] array) {
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    void setArrayValue(String key, int[] array) {
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    void setArrayValue(String key, long[] array) {
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    void setArrayValue(String key, double[] array) {
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    void setArrayValue(String key, float[] array) {
        for (int i = 0; i < array.length; i++) {
            setValue(key, i, array[i]);
        }
    }

    byte[] getByteArray(String key) {
        byte[] toReturn = new byte[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Byte) getValue(key, i);
        }
        return toReturn;
    }

    short[] getShortArray(String key) {
        short[] toReturn = new short[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Short) getValue(key, i);
        }
        return toReturn;
    }

    int[] getIntArray(String key) {
        int[] toReturn = new int[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Integer) getValue(key, i);
        }
        return toReturn;
    }

    long[] getLongArray(String key) {
        long[] toReturn = new long[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Long) getValue(key, i);
        }
        return toReturn;
    }

    double[] getDoubleArray(String key) {
        double[] toReturn = new double[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Double) getValue(key, i);
        }
        return toReturn;
    }

    float[] getFloatArray(String key) {
        float[] toReturn = new float[size(key)];
        for (int i = 0; i < toReturn.length; i++) {
            toReturn[i] = (Float) getValue(key, i);
        }
        return toReturn;
    }
}
