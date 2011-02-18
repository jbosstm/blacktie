package org.jboss.blacktie.jatmibroker.nbf;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import org.apache.xerces.xs.ElementPSVI;
import org.apache.xerces.xs.PSVIProvider;
import org.apache.xerces.xs.XSTypeDefinition;


public class NestedBufferHandlers extends DefaultHandler {
	private static final Logger log = LogManager.getLogger(NestedBufferHandlers.class);

	private PSVIProvider provider;
	private String id;
	private String type;
	private String value;
	private String other_value;
	private int index;
	private int curIndex;
	private boolean found;

	public NestedBufferHandlers(PSVIProvider provider) {
		this.provider = provider;
		curIndex = 0;
		value = null;
		found = false;
	}

	public String getType() {
		return type;
	}
	
	public String getValue() {
		if(type.endsWith("_type")) {
			return other_value;
		} else {
			return value;
		}
	}
	
	public void setId(String id) {
		this.id = id;
		this.value = null;
	}
	
	public void setIndex(int index) {
		this.curIndex = 0;
		this.index = index;
	}
	
	public void characters (char ch[], int start, int length)
	throws SAXException {
		if(found) {
			String strValue = new String(ch, start, length);
			if(value == null) {
				value = strValue;
			} else {
				value += strValue;
			}
			
			log.debug("curIndex = " + curIndex + " value = " + value);
		}
    }
	
	public void startElement(String uri, String localName, String qName, Attributes atts)
	throws SAXException {
		if(qName.equals(id)) {
			if(index == curIndex) {
				found = true;
			}
			curIndex ++;
			
			ElementPSVI psvi = provider.getElementPSVI();
			if(psvi != null) {
				XSTypeDefinition typeInfo = psvi.getTypeDefinition();

				while(typeInfo != null) {
					String typeName = typeInfo.getName();
					if(typeName != null &&(typeName.equals("long") || 
							typeName.equals("string") ||
							typeName.equals("integer") ||
							typeName.equals("float") ||
							typeName.endsWith("_type"))) {

						type = typeName;
						log.debug(qName + " has type of " + type);

						break;
					}
					typeInfo = typeInfo.getBaseType();
				}
			}
		}
	}

	public void endElement(String uri, String localName, String qName)  
	throws SAXException   {
		if(qName.equals(id)) {
			found = false;
		}
		
		if(found && type != null && type.endsWith("_type") && value != null) {
			String tmp = "<" + qName + ">" + value + "</" + qName + ">";
			if(other_value == null) {
				other_value =tmp;
			} else {
				other_value += tmp;
			}
			value = null;
		}
	}
}
