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

import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Response;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.X_COMMON;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.X_C_TYPE;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.X_OCTET;

/**
 * The JABRequest class wraps the output parameter to the service.
 * 
 * @see JABServiceInvoker
 */
public class JABMessage implements Message {
    private static final Logger log = LogManager.getLogger(JABMessage.class);

    /**
     * The rcode if this is a response
     */
    private int rcode;

    private X_OCTET xOctet;

    private X_COMMON xCommon;

    private X_C_TYPE xCType;

    private Connection connection;

    /**
     * The request should be created from the JABRemoteService getRequest method.
     * 
     * @param connection The connection to use
     * @param bufferSubType
     * @param bufferType
     * @throws JABException
     */
    JABMessage(Connection connection, String bufferType, String bufferSubType) throws JABException {
        try {
            this.connection = connection;
            Buffer buffer = connection.tpalloc(bufferType, bufferSubType, 0);
            if (buffer.getType().equals("X_OCTET")) {
                xOctet = (X_OCTET) buffer;
            } else if (buffer.getType().equals("X_COMMON")) {
                xCommon = (X_COMMON) buffer;
            } else {
                xCType = (X_C_TYPE) buffer;
            }
        } catch (ConnectionException e) {
            throw new JABException("Could not create a buffer", e);
        } catch (ConfigurationException e) {
            throw new JABException("Could not create a buffer", e);
        }
    }

    /**
     * Create a message from the response
     * 
     * @param response The response
     */
    JABMessage(Response response) {
        Buffer buffer = response.getBuffer();
        if (buffer.getType().equals("X_OCTET")) {
            xOctet = (X_OCTET) buffer;
        } else if (buffer.getType().equals("X_COMMON")) {
            xCommon = (X_COMMON) buffer;
        } else {
            xCType = (X_C_TYPE) buffer;
        }
        rcode = response.getRcode();
    }

    /**
     * An internal method to access the actual buffer.
     * 
     * @return The buffer
     */
    Buffer getBuffer() {
        if (xOctet != null) {
            return xOctet;
        } else if (xCommon != null) {
            return xCommon;
        } else {
            return xCType;
        }
    }

    /**
     * Clear the real buffer
     */
    void clear() {
        if (xOctet != null) {
            xOctet.clear();
        } else if (xCommon != null) {
            xCommon.clear();
        } else {
            xCType.clear();
        }
    }

    /**
     * Get the rcode that tpreturn returned with.
     * 
     * @return The application return code.
     */
    int getRCode() {
        return rcode;
    }

    /**
     * Get the content of the buffer
     * 
     * @param key The content of the buffer to set
     * @throws JABException In case the content is malformed
     */
    public byte getByte(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                return xCommon.getByte(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCType.getByte(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public byte[] getByteArray(String key) throws JABException {
        if (xOctet != null) {
            if (key.equals("X_OCTET")) {
                return xOctet.getByteArray();
            } else {
                throw new JABException("X_OCTET buffers contain a single attribute X_OCTET");
            }
        } else if (xCommon != null) {
            try {
                return xCommon.getByteArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCType.getByteArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public byte[][] getByteArrayArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getByteArrayArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public double getDouble(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getDouble(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public double[] getDoubleArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getDoubleArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public float getFloat(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getFloat(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public float[] getFloatArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getFloatArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public int getInt(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getInt(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public int[] getIntArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                return xCType.getIntArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public short getShort(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                return xCommon.getShort(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCType.getShort(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public short[] getShortArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                return xCommon.getShortArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCType.getShortArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setByte(String key, byte data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                xCommon.setByte(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setByte(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setByteArray(String key, byte[] data) throws JABException {
        if (xOctet != null) {
            if (key.equals("X_OCTET")) {
                if (data.length != xOctet.getLen()) {
                    try {
                        xOctet = (X_OCTET) connection.tpalloc("X_OCTET", null, data.length);
                    } catch (ConnectionException e) {
                        throw new JABException("Could not allocate the X_OCTET buffer");
                    } catch (ConfigurationException e) {
                        throw new JABException("Could not allocate the X_OCTET buffer");
                    }
                }
                xOctet.setByteArray(data);
            } else {
                throw new JABException("X_OCTET buffers contain a single attribute X_OCTET");
            }
        } else if (xCommon != null) {
            try {
                xCommon.setByteArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setByteArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setByteArrayArray(String key, byte[][] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setByteArrayArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setDouble(String key, double data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setDouble(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setDoubleArray(String key, double[] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setDoubleArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setFloat(String key, float data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setFloat(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setFloatArray(String key, float[] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setFloatArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setInt(String key, int data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setInt(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setIntArray(String key, int[] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            throw new JABException("Not supported for this buffer type");
        } else {
            try {
                xCType.setIntArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setShort(String key, short data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                xCommon.setShort(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setShort(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setShortArray(String key, short[] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                xCommon.setShortArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setShortArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public Map<String, Class> getMessageFormat() {
        if (xOctet != null) {
            return xOctet.getFormat();
        } else if (xCommon != null) {
            return xCommon.getFormat();
        } else {
            return xCType.getFormat();
        }
    }

    public long getLong(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                return xCommon.getLong(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCommon.getLong(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public long[] getLongArray(String key) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                return xCommon.getLongArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                return xCType.getLongArray(key);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setLong(String key, long data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                xCommon.setLong(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setLong(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }

    public void setLongArray(String key, long[] data) throws JABException {
        if (xOctet != null) {
            throw new JABException("Not supported for this buffer type");
        } else if (xCommon != null) {
            try {
                xCommon.setLongArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        } else {
            try {
                xCType.setLongArray(key, data);
            } catch (Throwable t) {
                throw new JABException(t.getMessage(), t);
            }
        }
    }
}
