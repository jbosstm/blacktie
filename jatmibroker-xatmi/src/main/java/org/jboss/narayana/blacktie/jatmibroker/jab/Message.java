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
package org.jboss.narayana.blacktie.jatmibroker.jab;

/**
 * The message interface is a wrapper around input output buffers.
 * 
 * @see JABMessage
 * @see JABServiceInvoker
 */
public interface Message {
    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setByte(String key, byte data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setShort(String key, short data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setInt(String key, int data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setFloat(String key, float data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setDouble(String key, double data) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public byte getByte(String key) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public short getShort(String key) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public int getInt(String key) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public float getFloat(String key) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public double getDouble(String key) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setByteArray(String key, byte[] data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setShortArray(String key, short[] data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setIntArray(String key, int[] data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setFloatArray(String key, float[] data) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setDoubleArray(String key, double[] data) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public byte[] getByteArray(String key) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public short[] getShortArray(String key) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public int[] getIntArray(String key) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public float[] getFloatArray(String key) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public double[] getDoubleArray(String key) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setByteArrayArray(String key, byte[][] data) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public byte[][] getByteArrayArray(String key) throws JABException;

    /**
     * Set part of the content to send. For non-array data types
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setLong(String key, long data) throws JABException;

    /**
     * Get the content of the buffer at this location.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public long getLong(String key) throws JABException;

    /**
     * Set part of the content to send. For non-array data types,
     * 
     * @param key The key of the data
     * @param data The data to send
     * @throws JABException In case the key is not of required type/does not exist
     */
    public void setLongArray(String key, long[] data) throws JABException;

    /**
     * Get the content of the buffer for this key.
     * 
     * @return The content of the buffer
     * @throws JABException If the key does not exist or is not of requested type
     */
    public long[] getLongArray(String key) throws JABException;

}
