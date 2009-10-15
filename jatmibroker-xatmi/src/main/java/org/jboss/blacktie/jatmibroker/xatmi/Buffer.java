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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

/**
 * This class encapsulates the response from the remote service and the return
 * code
 */
public class Buffer implements Serializable {
	private static final Logger log = LogManager.getLogger(Buffer.class);

	private static List<String> bufferTypes = new ArrayList<String>();
	private static List<Class> x_octetTypes = new ArrayList<Class>();
	private static List<Class> x_commonTypes = new ArrayList<Class>();
	private static List<Class> x_c_typeTypes = new ArrayList<Class>();

	private static final int LONG_SIZE = 8;
	private static final int INT_SIZE = 4;
	private static final int SHORT_SIZE = 2;
	private static final int FLOAT_SIZE = INT_SIZE;
	private static final int DOUBLE_SIZE = LONG_SIZE;
	
	static {
		String[] bufferType = new String[] { "X_OCTET", "X_C_TYPE", "X_COMMON" };
		for (int i = 0; i < bufferType.length; i++) {
			bufferTypes.add(bufferType[i]);
		}

		Class[] x_octetType = new Class[] { byte[].class };
		for (int i = 0; i < x_octetType.length; i++) {
			x_octetTypes.add(x_octetType[i]);
		}
		Class[] x_commonType = new Class[] { short.class, long.class,
				char.class, short[].class, long[].class, char[].class };
		for (int i = 0; i < x_commonType.length; i++) {
			x_commonTypes.add(x_commonType[i]);
		}
		Class[] x_c_typeType = new Class[] { int.class, short.class,
				long.class, char.class, float.class, double.class, int[].class,
				short[].class, long[].class, char[].class, float[].class,
				double[].class };
		for (int i = 0; i < x_c_typeType.length; i++) {
			x_c_typeTypes.add(x_c_typeType[i]);
		}

	}

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * The response from the server
	 */
	private byte[] data;

	private String type;

	private String subtype;

	private Map<String, Object> structure = new HashMap<String, Object>();
	private String[] keys;
	private Class[] types;
	private int[] lengths;
	private boolean deserialized;
	private boolean formatted;

	int currentPos = 0;

	/**
	 * Create a new buffer
	 * 
	 * @param type
	 * @param subtype
	 * @throws ConnectionException
	 */
	public Buffer(String type, String subtype) throws ConnectionException {
		if (!bufferTypes.contains(type)) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Unknown buffer type: " + type);
		}
		this.type = type;
		this.subtype = subtype;
	}

	/**
	 * Get the return value
	 * 
	 * @return The return value
	 */
	public String getType() {
		return type;
	}

	/**
	 * Get the return code
	 * 
	 * @return The return code
	 */
	public String getSubtype() {
		return subtype;
	}

	/**
	 * Set the data to send.
	 * 
	 * @param data
	 *            The data
	 * @throws IOException
	 */
	public void setData(byte[] data) {
		this.data = data;
	}

	/**
	 * Get the data
	 * 
	 * @return The data
	 */
	public byte[] getData() {
		return data;
	}

	public int getLength() {
		if (data == null) {
			return 0;
		} else {
			return data.length;
		}
	}

	public int getShort(String key) throws ConnectionException {
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
		} else if (types[position] != short.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (Short) structure.get(key);
	}

	public void setShort(String key, short value) throws ConnectionException {
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
		} else if (types[position] != short.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public float getFloat(String key) throws ConnectionException {
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
		} else if (types[position] != float.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (Float) structure.get(key);
	}

	public void setFloat(String key, float value) throws ConnectionException {
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
		} else if (types[position] != float.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public int getInt(String key) throws ConnectionException {
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
		} else if (types[position] != int.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (Integer) structure.get(key);
	}

	public void setInt(String key, int value) throws ConnectionException {
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
		} else if (types[position] != int.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public long getLong(String key) throws ConnectionException {
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
		} else if (types[position] != long.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (Long) structure.get(key);
	}

	public void setLong(String key, long value) throws ConnectionException {
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
		} else if (types[position] != long.class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public char[] getCharArray(String key) throws ConnectionException {
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
		} else if (types[position] != char[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (char[]) structure.get(key);
	}

	public void setCharArray(String key, char[] value)
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
		} else if (types[position] != char[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public float[] getFloatArray(String key) throws ConnectionException {
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
		} else if (types[position] != float[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (float[]) structure.get(key);
	}

	public void setFloatArray(String key, float[] value)
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
		} else if (types[position] != float[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public double[] getDoubleArray(String key) throws ConnectionException {
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
		} else if (types[position] != double[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		return (double[]) structure.get(key);
	}

	public void setDoubleArray(String key, double[] value)
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
		} else if (types[position] != double[].class) {
			throw new ConnectionException(Connection.TPEITYPE,
					"Key is not request type, it is a: " + types[position]);

		}
		structure.put(key, value);
	}

	public void format(String[] keys, Class[] types, int[] lengths)
			throws ConnectionException {
		if (type.equals("X_OCTET")) {
			throw new ConnectionException(-1,
					"Tried to format an X_OCTET buffer");
		}
		structure.clear();
		if (keys.length != types.length || types.length != lengths.length) {
			throw new ConnectionException(-1,
					"Invalid format, each array description should be same length");
		}
		this.keys = keys;
		this.types = types;
		this.lengths = lengths;
		formatted = true;
		deserialize();
	}

	void deserialize() throws ConnectionException {
		currentPos = 0;
		if (!type.equals("X_OCTET")) {
			if (!deserialized && data != null) {
				if (keys == null) {
					throw new ConnectionException(Connection.TPEITYPE,
							"Message format not provided");
				}
				ByteArrayInputStream bais = new ByteArrayInputStream(data);
				DataInputStream dis = new DataInputStream(bais);
				for (int i = 0; i < types.length; i++) {
					if (type.equals("X_OCTET")
							&& !x_octetTypes.contains(types[i])) {
						throw new ConnectionException(Connection.TPEITYPE,
								"Cannot read type from X_OCTET " + types[i]);
					} else if (type.equals("X_C_TYPE")
							&& !x_c_typeTypes.contains(types[i])) {
						throw new ConnectionException(Connection.TPEITYPE,
								"Cannot read type from X_C_TYPE " + types[i]);
					} else if (type.equals("X_COMMON")
							&& !x_commonTypes.contains(types[i])) {
						throw new ConnectionException(Connection.TPEITYPE,
								"Cannot read type from X_COMMON " + types[i]);
					}

					try {
						if (types[i] == int.class) {
							structure.put(keys[i], readInt(dis));
						} else if (types[i] == short.class) {
							structure.put(keys[i], readShort(dis));
						} else if (types[i] == long.class) {
							structure.put(keys[i], readLong(dis));
						} else if (types[i] == char.class) {
							structure.put(keys[i], readChar(dis));
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
						} else if (types[i] == char[].class) {
							char[] toRead = new char[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = readChar(dis);
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
				deserialized = true;
			}
		}
	}

	void serialize() throws ConnectionException {
		currentPos = 0;
		if (!type.equals("X_OCTET")) {
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
					} else if (types[i] == char.class) {
						Character toWrite = (Character) structure.get(keys[i]);
						if (toWrite != null) {
							writeChar(dos, toWrite);
						} else {
							writeChar(dos, '\0');
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
					} else if (types[i] == char[].class) {
						char[] toWrite = (char[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < max; j++) {
								writeChar(dos, toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							writeChar(dos, '\0');
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
			data = baos.toByteArray();
		}
	}

	private void writePad(DataOutputStream dos, int size) throws IOException {
		for (int i = 0; i < size; i++) {
			writeChar(dos, '\0');
		}
	}

	private void readPad(DataInputStream dis, int size) throws IOException {
		for (int i = 0; i < size; i++) {
			readChar(dis);
		}
	}

	private void writeChar(DataOutputStream dos, char c) throws IOException {
		byte[] bytes = { (byte) (c & 0xff), (byte) (c >> 8 & 0xff) };
		dos.writeByte(bytes[0]);

		currentPos += 1;
	}

	private char readChar(DataInputStream dis) throws IOException {
		currentPos += 1;

		byte read = dis.readByte();
		return (char) read;
	}

	private void writeLong(DataOutputStream dos, long x) throws IOException {
		writePad(dos, currentPos % LONG_SIZE);

		ByteBuffer bbuf = ByteBuffer.allocate(LONG_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putLong(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		long toWrite = bbuf.getLong(0);
		dos.writeLong(toWrite);
		currentPos += LONG_SIZE;
	}

	private long readLong(DataInputStream dis) throws IOException {
		readPad(dis, currentPos % LONG_SIZE);

		currentPos += LONG_SIZE;
		long x = dis.readLong();
		ByteBuffer bbuf = ByteBuffer.allocate(LONG_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putLong(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getLong(0);
	}

	private void writeInt(DataOutputStream dos, int x) throws IOException {
		writePad(dos, currentPos % INT_SIZE);

		ByteBuffer bbuf = ByteBuffer.allocate(INT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putInt(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		int toWrite = bbuf.getInt(0);
		dos.writeInt(toWrite);
		currentPos += INT_SIZE;
	}

	private int readInt(DataInputStream dis) throws IOException {
		readPad(dis, currentPos % INT_SIZE);

		currentPos += INT_SIZE;
		int x = dis.readInt();
		ByteBuffer bbuf = ByteBuffer.allocate(INT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putInt(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getInt(0);
	}

	private void writeShort(DataOutputStream dos, short x) throws IOException {
		writePad(dos, currentPos % SHORT_SIZE);

		ByteBuffer bbuf = ByteBuffer.allocate(SHORT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putShort(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		short toWrite = bbuf.getShort(0);
		dos.writeShort(toWrite);

		currentPos += SHORT_SIZE;
	}

	private short readShort(DataInputStream dis) throws IOException {
		readPad(dis, currentPos % SHORT_SIZE);

		currentPos += SHORT_SIZE;
		short x = dis.readShort();
		ByteBuffer bbuf = ByteBuffer.allocate(SHORT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putShort(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getShort(0);
	}

	private void writeFloat(DataOutputStream dos, float x) throws IOException {
		writePad(dos, currentPos % FLOAT_SIZE);

		ByteBuffer bbuf = ByteBuffer.allocate(FLOAT_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putFloat(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		float toWrite = bbuf.getFloat(0);
		dos.writeFloat(toWrite);

		currentPos += FLOAT_SIZE;
	}

	private float readFloat(DataInputStream dis) throws IOException {
		readPad(dis, currentPos % FLOAT_SIZE);

		currentPos += FLOAT_SIZE;
		float x = dis.readFloat();
		ByteBuffer bbuf = ByteBuffer.allocate(FLOAT_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putFloat(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getFloat(0);
	}

	private void writeDouble(DataOutputStream dos, double x) throws IOException {
		writePad(dos, currentPos % DOUBLE_SIZE);

		ByteBuffer bbuf = ByteBuffer.allocate(DOUBLE_SIZE);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		bbuf.putDouble(x);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		double toWrite = bbuf.getDouble(0);
		dos.writeDouble(toWrite);

		currentPos += DOUBLE_SIZE;
	}

	private double readDouble(DataInputStream dis) throws IOException {
		readPad(dis, currentPos % DOUBLE_SIZE);

		currentPos += DOUBLE_SIZE;
		double x = dis.readDouble();
		ByteBuffer bbuf = ByteBuffer.allocate(DOUBLE_SIZE);
		bbuf.order(ByteOrder.LITTLE_ENDIAN);
		bbuf.putDouble(x);
		bbuf.order(ByteOrder.BIG_ENDIAN);
		return bbuf.getDouble(0);
	}
}
