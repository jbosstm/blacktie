package org.jboss.blacktie.jatmibroker.nbf;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.sun.org.apache.xerces.internal.xs.ElementPSVI;
import com.sun.org.apache.xerces.internal.xs.PSVIProvider;
import com.sun.org.apache.xerces.internal.xs.XSTypeDefinition;

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
				XSTypeDefinition simpleType = psvi.getTypeDefinition();
				if(simpleType.getAnonymous()) {
					log.debug(qName + " anonymous type");
					type = "SimpleType";
				} else {
					log.debug(qName + ":" + simpleType.getName());
					type = simpleType.getName();
				}
			}
		}
	}

	public void setId(String id) {
		this.id = id;
	}
}
