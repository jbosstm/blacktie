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
package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class encapsulates the response from the remote service and the return
 * code
 */
public class Buffer implements Serializable {

	private static List<String> bufferTypes = new ArrayList<String>();
	private static List<Class> x_octetTypes = new ArrayList<Class>();
	private static List<Class> x_commonTypes = new ArrayList<Class>();
	private static List<Class> x_c_typeTypes = new ArrayList<Class>();
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
							structure.put(keys[i], dis.readInt());
						} else if (types[i] == short.class) {
							structure.put(keys[i], dis.readShort());
						} else if (types[i] == long.class) {
							structure.put(keys[i], dis.readLong());
						} else if (types[i] == char.class) {
							structure.put(keys[i], dis.readChar());
						} else if (types[i] == float.class) {
							structure.put(keys[i], dis.readFloat());
						} else if (types[i] == double.class) {
							structure.put(keys[i], dis.readDouble());
						} else if (types[i] == int[].class) {
							int[] toRead = new int[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readInt();
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == short[].class) {
							short[] toRead = new short[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readShort();
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == long[].class) {
							long[] toRead = new long[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readLong();
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == char[].class) {
							char[] toRead = new char[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readChar();
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == float[].class) {
							float[] toRead = new float[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readFloat();
							}
							structure.put(keys[i], toRead);
						} else if (types[i] == double[].class) {
							double[] toRead = new double[lengths[i]];
							for (int j = 0; j < lengths[i]; j++) {
								toRead[j] = dis.readDouble();
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
		if (!type.equals("X_OCTET")) {
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			DataOutputStream dos = new DataOutputStream(baos);
			for (int i = 0; i < types.length; i++) {

				try {
					if (types[i] == int.class) {
						Integer toWrite = (Integer) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeInt(toWrite);
						} else {
							dos.writeInt(0);
						}
					} else if (types[i] == short.class) {
						Short toWrite = (Short) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeShort(toWrite);
						} else {
							dos.writeShort(0);
						}
					} else if (types[i] == long.class) {
						Long toWrite = (Long) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeLong(toWrite);
						} else {
							dos.writeLong(0);
						}
					} else if (types[i] == char.class) {
						Character toWrite = (Character) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeChar(toWrite);
						} else {
							dos.writeChar('\0');
						}
					} else if (types[i] == float.class) {
						Float toWrite = (Float) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeFloat(toWrite);
						} else {
							dos.writeFloat(0);
						}
					} else if (types[i] == double.class) {
						Double toWrite = (Double) structure.get(keys[i]);
						if (toWrite != null) {
							dos.writeDouble(toWrite);
						} else {
							dos.writeInt(0);
						}
					} else if (types[i] == int[].class) {
						int[] toWrite = (int[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								dos.writeInt(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeInt(0);
						}
					} else if (types[i] == short[].class) {
						short[] toWrite = (short[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								dos.writeShort(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeShort(0);
						}
					} else if (types[i] == long[].class) {
						long[] toWrite = (long[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								dos.writeLong(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeLong(0);
						}
					} else if (types[i] == char[].class) {
						char[] toWrite = (char[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < max; j++) {
								dos.writeChar(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeChar('\0');
						}
					} else if (types[i] == float[].class) {
						float[] toWrite = (float[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								dos.writeFloat(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeFloat(0);
						}
					} else if (types[i] == double[].class) {
						double[] toWrite = (double[]) structure.get(keys[i]);
						int max = 0;
						if (toWrite != null) {
							max = Math.min(lengths[i], toWrite.length);
							for (int j = 0; j < lengths[i]; j++) {
								dos.writeDouble(toWrite[j]);
							}
						}
						for (int j = max; j < lengths[i]; j++) {
							dos.writeDouble(0);
						}
					} else {
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
}
