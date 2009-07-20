package org.jboss.blacktie.jatmibroker.jab.factory;

import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class JABBuffer {

	private Buffer value;
	private boolean readOnly;

	public JABBuffer() {

	}

	JABBuffer(Buffer buffer) {
		readOnly = true;
		this.value = buffer;
	}

	public void setValue(String value) throws JABException {
		if (!readOnly) {
			this.value = new Buffer("X_OCTET", null);
			try {
				this.value.setData(value);
			} catch (ConnectionException e) {
				this.value = null;
				throw new JABException("Could not set data: " + e.getMessage(),
						e);
			}
		}
	}

	public String getValue() throws JABException {
		if (value != null) {
			return (String) value.getData();
		} else {
			return null;
		}
	}
}
