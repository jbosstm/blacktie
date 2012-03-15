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

/**
 * The JABResponse class extends the standard JABBuffer by allowing the programmer access to the xatmi.h::rcode state returned
 * from an XATMI service
 * 
 * @see JABBuffer
 */
public class JABResponse extends JABBuffer {

    /**
     * The response code of the service
     */
    private int responseCode;

    /**
     * The constructor is hidden from standard application programs as it should only be created when a response is available
     * for processing
     * 
     * @param responseCode The response from the XATMI service
     */
    JABResponse(int responseCode) {
        this.responseCode = responseCode;
    }

    /**
     * Get the response code from the XATMI service
     * 
     * @return The response code
     */
    public int getResponseCode() {
        return responseCode;
    }
}
