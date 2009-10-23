/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General public  License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General public  License for more details.
 * You should have received a copy of the GNU Lesser General public  License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AttributeStructure;
import org.jboss.blacktie.jatmibroker.core.conf.BufferStructure;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

/**
 * This class encapsulates the response from the remote service and the return
 * code
 */
public abstract class Buffer implements Serializable {
	private static final Logger log = LogManager.getLogger(Buffer.class);

	private static final int BYTE_SIZE = 1;
	private static final int LONG_SIZE = 8;
	private static final int INT_SIZE = 4;
	private static final int SHORT_SIZE = 2;
	private static final int FLOAT_SIZE = INT_SIZE;
	private static final int DOUBLE_SIZE = LONG_SIZE;

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	private Map<String, Object> structure = new HashMap<String, Object>();
	private String[] keys;
	private Class[] types;
	private int[] lengths;
	private boolean deserialized;
	private boolean formatted;

	int currentPos = 0;

	private List<Class> supportedTypes;

	private boolean requiresSerialization;

	private String type;

	private String subtype;

	private byte[] data;

	private int[] counts;

	/**
	 * Create a new buffer
	 * 
	 * @param properties
	 * @param b
	 * 
	 * @param types
	 * @throws ConnectionException
	 */
	Buffer(String type, String subtype, boolean requiresSerialization,
			List<Class> supportedTypes, Properties properties)
			throws ConnectionException {
		this.type = type;
		this.subtype = subtype;
		this.requiresSerialization = requiresSerialization;
		this.supportedTypes = supportedTypes;

		if (requiresSerialization) {
			Map<String, BufferStructure> buffers = (Map<String, BufferStructure>) properties
					.get("blacktie.domain.buffers");
			BufferStructure buffer = buffers.get(subtype);
			if (buffer == null) {
				throw new ConnectionException(Connection.TPEITYPE,
						"Subtype was not registered: " + subtype);
			}
			String[] ids = new String[buffer.attributes.size()];
			Class[] types = new Class[buffer.attributes.size()];
			int[] length = new int[buffer.attributes.size()];
			int[] count = new int[buffer.attributes.size()];
			Iterator<AttributeStructure> iterator = buffer.attributes
					.iterator();
			int i = 0;
			while (iterator.hasNext()) {
				AttributeStructure attribute = iterator.next();
				ids[i] = attribute.id;
				types[i] = attribute.type;
				if (!supportedTypes.contains(types[i])) {
					throw new ConnectionException(Connection.TPEITYPE,
							"Cannot use type configured in buffer " + types[i]);
				}
				length[i] = attribute.length;
				count[i] = attribute.count;
				i++;
			}
			format(ids, types, length, count);
		}
	}

	private void format(String[] keys, Class[] types, int[] lengths,
			int[] counts) throws ConnectionException {
		structure.clear();
		if (keys.length != types.length || types.length != lengths.length) {
			throw new ConnectionException(-1,
					"Invalid format, each array description should be same length");
		}
		this.keys = keys;
		this.types = types;
		this.lengths = lengths;
		this.counts = counts;
		formatted = true;
	}

	void deserialize(byte[] data) throws ConnectionException {
		currentPos = 0;
		if (requiresSerialization) {
			if (!deserialized && data != null) {
				if (keys == null) {
					throw new ConnectionException(Connection.TPEITYPE,
							"Message format not provided");
				}
				ByteArrayInputStream bais = new ByteArrayInputStream(data);
				DataInputStream dis = new DataInputStream(bais);
				for (int i = 0; i < types.length; i++) {
					if (!supportedTypes.contains(types[i])) {
						throw new ConnectionException(Connection.TPEITYPE,
								"Cannot read type from buffer " + types[i]);
					}

					try {
						if (types[i] == int.class) {
							structure.put(keys[i], readInt(dis));
						} else if (types[i] == short.class) {
							structure.put(keys[i], readShort(dis));
						} else if (types[i] == long.class) {
							structure.put(keys[i], readLong(dis));
						} else if (types[i] == byte.class) {
							structure.put(keys[i], readByte(dis));
						} else if (types[i] == float.class) {
							structure.put(keys[i], readFloat(dis));
						} else if (types[i] == double.class) {
							structure.put(keys[i], readDouble(dis));
						} else if (types[i] == int[].class) {
							int[] toRead = new int[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readInt(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == short[].class) {
							short[] toRead = new short[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readShort(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == long[].class) {
							long[] toRead = new long[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readLong(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == byte[].class) {
							byte[] toRead = new byte[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readByte(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == float[].class) {
							float[] toRead = new float[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readFloat(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == double[].class) {
							double[] toRead = new double[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readDouble(dis);
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == byte[][].class) {
							byte[][] toRead = new byte[counts[i]][lengths[i]];
							for (int k = 0; k < counts[i]; k++) {
								for (int j = 0; j < lengths[i]; j++) {
									toRead[k][j] = readByte(dis);
								}
							}
							structure.put(keys[i], toRead);
						} else {
							throw new ConnectionException(Connection.TPEITYPE,
									"Could not deserialize: " + types[i]);
						}
					} catch (IOException e) {
						throw new ConnectionException(
								Connection.TPEITYPE,
								"Could not parse the value as: "
										+ keys[i]
										+ " was not a "
										+ types[i]
										+ " and even if it was an array of that type its length was not: "
										+ lengths[i]);
					}
				}
			}
		} else {
			this.data = data;
		}
		deserialized = true;
	}

	byte[] serialize() throws ConnectionException {
		currentPos = 0;
		byte[] toReturn = null;
		if (requiresSerialization) {
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			DataOutputStream dos = new DataOutputStream(baos);
			for (int i = 0; i < types.length; i++) {

				try {
					if (types[i] == int.class) {
						Integer toWrite = (Integer) structure.get(keys[i]);
						if (toWrite != null) {
							writeInt(dos, toWrite);
						} else {
							writeInt(dos, 0);
						}
					} else if (types[i] == short.class) {
						Short toWrite = (Short) structure.get(keys[i]);
						if (toWrite != null) {
							writeShort(dos, toWrite);
						} else {
							writeShort(dos, (short) 0);
						}
					} else if (types[i] == long.class) {
						Long toWrite = (Long) structure.get(keys[i]);
						if (toWrite != null) {
							writeLong(dos, toWrite);
						} else {
							writeLong(dos, 0);
						}
					} else if (types[i] == byte.class) {
						Byte toWrite = (Byte) structure.get(keys[i]);
						if (toWrite != null) {
							writeByte(dos, toWrite);
						} else {
							// writeByte(dos, '\0');
							writeByte(dos, (byte) 0);
						}
					} else if (types[i] == float.class) {
						Float toWrite = (Float) structure.get(keys[i]);
						if (toWrite != null) {
							writeFloat(dos, toWrite);
						} else {
							writeFloat(dos, 0);
						}
					} else if (types[i] == double.class) {
						Double toWrite = (Double) structure.get(keys[i]);
						if (toWrite != null) {
							writeDouble(dos, toWrite);
						} else {
							writeDouble(dos, 0);
						}
					} else if (types[i] == int[].class) {
						int[] toWrite = (int[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								writeInt(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeInt(dos, 0);
						}
					} else if (types[i] == short[].class) {
						short[] toWrite = (short[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								writeShort(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeShort(dos, (short) 0);
						}
					} else if (types[i] == long[].class) {
						long[] toWrite = (long[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								writeLong(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeLong(dos, 0);
						}
					} else if (types[i] == byte[].class) {
						byte[] toWrite = (byte[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < max; j++) {
								writeByte(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							// writeByte(dos, '\0');
							writeByte(dos, (byte) 0);
						}
					} else if (types[i] == float[].class) {
						float[] toWrite = (float[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								writeFloat(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeFloat(dos, 0);
						}
					} else if (types[i] == double[].class) {
						double[] toWrite = (double[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								writeDouble(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeDouble(dos, 0);
						}
					} else if (types[i] == byte[][].class) {
						byte[][] toWrite = (byte[][]) structure.get(keys[i]);
						if (toWrite != null) {
							for (int k = 0; k < counts[i]; k++) {
								for (int j = 0; j < lengths[i]; j++) {
									writeByte(dos, toWrite[k][j]);
								}
							}
						} else {
							for (int j = 0; j < counts[i] * lengths[i]; j++) {
								writeByte(dos, (byte) 0);
							}
						}
					} else {
						if (JABTransaction.current() != null) {
							try {
								JABTransaction.current().rollback_only();
							} catch (JABException e) {
								throw new ConnectionException(
										Connection.TPESYSTEM,
										"Could not mark transaction for rollback only");
							}
						}
						throw new ConnectionException(Connection.TPEOTYPE,
								"Could not serialize: " + types[i]);
					}
				} catch (IOException e) {
					throw new ConnectionException(
							Connection.TPEOTYPE,
							"Could not parse the value as: "
									+ keys[i]
									+ " was not a "
									+ types[i]
									+ " and even if it was an array of that type its length was not: "
									+ lengths[i]);
				}
			}
			toReturn = baos.toByteArray();
		} else {
			toReturn = getRawData();
		}
		if (toReturn == null) {
			toReturn = new byte[1];
		}
		return toReturn;
	}

	private void writeByte(DataOutputStream dos, byte b) throws IOException {
		dos.writeByte(b);

		currentPos += 1;
	}

	private byte readByte(DataInputStream dis) throws IOException {
		currentPos += 1;

		byte x = dis.readByte();
		ByteBuffer bbuf = ByteBuffer.allocate(BYTE_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.put(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.get(0);
	}

	private void writeLong(DataOutputStream dos, long x) throws IOException {
		ByteBuffer bbuf = ByteBuffer.allocate(LONG_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putLong(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		long toWrite = bbuf.getLong(0);
		dos.writeLong(toWrite);
		currentPos += LONG_SIZE;
	}

	private long readLong(DataInputStream dis) throws IOException {
		currentPos += LONG_SIZE;
		long x = dis.readLong();
		ByteBuffer bbuf = ByteBuffer.allocate(LONG_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putLong(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getLong(0);
	}

	private void writeInt(DataOutputStream dos, int x) throws IOException {
		ByteBuffer bbuf = ByteBuffer.allocate(INT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putInt(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		int toWrite = bbuf.getInt(0);
		dos.writeInt(toWrite);
		currentPos += INT_SIZE;
	}

	private int readInt(DataInputStream dis) throws IOException {
		currentPos += INT_SIZE;
		int x = dis.readInt();
		ByteBuffer bbuf = ByteBuffer.allocate(INT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putInt(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getInt(0);
	}

	private void writeShort(DataOutputStream dos, short x) throws IOException {
		ByteBuffer bbuf = ByteBuffer.allocate(SHORT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putShort(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		short toWrite = bbuf.getShort(0);
		dos.writeShort(toWrite);

		currentPos += SHORT_SIZE;
	}

	private short readShort(DataInputStream dis) throws IOException {
		currentPos += SHORT_SIZE;
		short x = dis.readShort();
		ByteBuffer bbuf = ByteBuffer.allocate(SHORT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putShort(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getShort(0);
	}

	private void writeFloat(DataOutputStream dos, float x) throws IOException {
		ByteBuffer bbuf = ByteBuffer.allocate(FLOAT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putFloat(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		float toWrite = bbuf.getFloat(0);
		dos.writeFloat(toWrite);

		currentPos += FLOAT_SIZE;
	}

	private float readFloat(DataInputStream dis) throws IOException {
		currentPos += FLOAT_SIZE;
		float x = dis.readFloat();
		ByteBuffer bbuf = ByteBuffer.allocate(FLOAT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putFloat(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getFloat(0);
	}

	private void writeDouble(DataOutputStream dos, double x) throws IOException {
		ByteBuffer bbuf = ByteBuffer.allocate(DOUBLE_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putDouble(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		double toWrite = bbuf.getDouble(0);
		dos.writeDouble(toWrite);

		currentPos += DOUBLE_SIZE;
	}

	private double readDouble(DataInputStream dis) throws IOException {
		currentPos += DOUBLE_SIZE;
		double x = dis.readDouble();
		ByteBuffer bbuf = ByteBuffer.allocate(DOUBLE_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putDouble(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getDouble(0);
	}

	/**
	 * Get the type
	 * 
	 * @return The type
	 */
	public String getType() {
		return type;
	}

	/**
	 * Get the subtype
	 * 
	 * @return The subtype
	 */
	public String getSubtype() {
		return subtype;
	}

	/**
	 * Clear the content of the buffer
	 */
	public void clear() {
		structure.clear();
		data = null;
	}

	protected Object getAttributeValue(String key, Class type)
			throws ConnectionException {
		if (!formatted) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Message not formatted");
		}
		int position = -1;
		for (int i = 0; i < keys.length; i++) {
			if (keys[i].equals(key)) {
				position = i;
			}
		}
		if (position == -1) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not part of the structure: " + key);
		} else if (types[position] != type) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return structure.get(key);
	}

	protected void setAttributeValue(String key, Class type, Object value)
			throws ConnectionException {
		if (!formatted) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Message not formatted");
		}
		int position = -1;
		for (int i = 0; i < keys.length; i++) {
			if (keys[i].equals(key)) {
				position = i;
			}
		}
		if (position == -1) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not part of the structure: " + key);
		} else if (types[position] != type) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	protected void setRawData(byte[] bytes) {
		this.data = bytes;
	}

	protected byte[] getRawData() {
		return data;
	}
}
