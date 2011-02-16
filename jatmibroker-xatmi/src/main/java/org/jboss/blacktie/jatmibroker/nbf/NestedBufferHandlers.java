package org.jboss.blacktie.jatmibroker.nbf;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
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

	public NestedBufferHandlers(PSVIProvider provider) {
		this.provider = provider;
	}

	public String getType() {
		return type;
	}

	public void endElement(String uri, String localName, String qName)  
	throws SAXException   {
		if(qName.equals(id)) {
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

	public void setId(String id) {
		this.id = id;
	}
}
