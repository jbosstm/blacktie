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

import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.RunServer;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.narayana.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.narayana.blacktie.jatmibroker.jab.TransactionException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;

public class JABFactoryTestCase extends TestCase {
    private static final Logger log = LogManager.getLogger(JABFactoryTestCase.class);
    private RunServer runServer = new RunServer();

    public void setUp() throws InterruptedException, ConfigurationException, ConnectionException {
        log.debug("JABFactoryTestCase::setUp");
        runServer.serverinit();
    }

    public void tearDown() throws ConnectionException, TransactionException {
        log.debug("JABFactoryTestCase::tearDown");
        if (JABTransaction.current() != null) {
            JABTransaction.current().rollback();
            fail();
        }
        runServer.serverdone();
    }

    public void test_tpcall_x_octet() throws Exception {
        log.info("JABFactoryTestCase::test_tpcall_x_octet");
        runServer.tpadvertisetpcallXOctet();
        JABConnectionFactory factory = new JABConnectionFactory("test");
        JABConnection connection = factory.getConnection("connection");
        JABBuffer toSend = new JABBuffer();
        toSend.setArrayValue("X_OCTET", "test_tpcall_x_octet".getBytes());

        Transaction transaction = connection.beginTransaction(10000);
        JABResponse call = connection.call(RunServer.getServiceNametpcallXOctet(), toSend, transaction, "X_OCTET", null);
        transaction.commit();

        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = call.getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));
        factory.closeConnection("connection");
    }

    public void test_tpcall_x_c_type() throws Exception {
        log.info("JABFactoryTestCase::test_tpcall_x_c_type");
        runServer.tpadvertisetpcallXCType();
        JABConnectionFactory factory = new JABConnectionFactory("test");
        JABConnection connection = factory.getConnection("connection");

        // Assemble the message ByteArrayOutputStream baos = new
        JABBuffer toSend = new JABBuffer();
        toSend.setValue("acct_no", 12345678l);
        toSend.setArrayValue("name", "TOM".getBytes());
        float[] foo = new float[2];
        foo[0] = 1.1F;
        foo[1] = 2.2F;
        toSend.setArrayValue("foo", foo);

        double[] balances = new double[2];
        balances[0] = 1.1;
        balances[1] = 2.2;
        toSend.setArrayValue("balances", balances);

        Transaction transaction = connection.beginTransaction(10000);
        JABResponse call = connection
                .call(RunServer.getServiceNametpcallXCType(), toSend, transaction, "X_C_TYPE", "acct_info");
        transaction.commit();

        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_c_type".getBytes(), 0, expected, 0, 15);
        byte[] received = call.getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));
        factory.closeConnection("connection");
    }
}
