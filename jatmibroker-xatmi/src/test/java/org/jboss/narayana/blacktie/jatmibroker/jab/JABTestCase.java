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

import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.RunServer;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;

public class JABTestCase extends TestCase {
    private static final Logger log = LogManager.getLogger(JABTestCase.class);
    private RunServer runServer = new RunServer();

    public void setUp() throws InterruptedException, ConfigurationException, ConnectionException {
        log.debug("JABTestCase::setUp");
        runServer.serverinit();
    }

    public void tearDown() throws ConnectionException, TransactionException {
        log.debug("JABTestCase::tearDown");
        if (JABTransaction.current() != null) {
            JABTransaction.current().rollback();
            fail();
        }
        runServer.serverdone();
    }

    public void test_tpcall_x_octet() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_octet");
        runServer.tpadvertisetpcallXOctet();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXOctet(), jabSession, "X_OCTET",
                null);
        jabService.getRequest().setByteArray("X_OCTET", "test_tpcall_x_octet".getBytes());
        jabService.call(null);
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = jabService.getResponse().getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));

        jabSession.closeSession();
    }

    public void test_tpcall_x_octet_with_tx() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_octet_with_tx");
        runServer.tpadvertisetpcallXOctet();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABTransaction transaction = new JABTransaction(jabSession, 5000);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXOctet(), jabSession, "X_OCTET",
                null);
        jabService.getRequest().setByteArray("X_OCTET", "test_tpcall_x_octet".getBytes());
        log.debug("calling tpcall_x_octet with tx");
        jabService.call(transaction);
        log.debug("called tpcall_x_octet with tx, commiting ...");
        transaction.commit();
        log.debug("tpcall_x_octet commit ok");
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = jabService.getResponse().getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));
        jabSession.closeSession();
    }

    public void test_tpcall_x_octet_commit_tx_rollback_only() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_octet_commit_tx_rollback_only");
        runServer.tpadvertisetpcallXOctet();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABTransaction transaction = new JABTransaction(jabSession, 5000);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXOctet(), jabSession, "X_OCTET",
                null);
        transaction.rollback_only();
        jabService.getRequest().setByteArray("X_OCTET", "test_tpcall_x_octet".getBytes());
        jabService.call(transaction);
        try {
            transaction.commit();
            fail("committing a tx marked rollback only succeeded");
        } catch (JABException e) {
            // the exception is expected, but:
            // exception should be CORBA::CORBA::TRANSACTION_ROLLEDBACK but
            // JBossTM
            // returns CORBA::OBJECT_NOT_EXIST instead in which case we would
            // use presumed abort");
        }
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = jabService.getResponse().getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));

        jabSession.closeSession();
    }

    public void test_tpcall_x_octet_rollback_tx_rollback_only() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_octet_rollback_tx_rollback_only");
        runServer.tpadvertisetpcallXOctet();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABTransaction transaction = new JABTransaction(jabSession, 5000);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXOctet(), jabSession, "X_OCTET",
                null);
        transaction.rollback_only();
        jabService.getRequest().setByteArray("X_OCTET", "test_tpcall_x_octet".getBytes());
        jabService.call(transaction);
        try {
            transaction.rollback();
        } catch (JABException e) {
            fail("rolling back a tx marked rollback through exception: " + e.getMessage());
        }
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = jabService.getResponse().getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));

        jabSession.closeSession();
    }

    public void test_tpcall_x_octet_suspend_tx() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_octet_suspend_tx");
        runServer.tpadvertisetpcallXOctet();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABTransaction transaction = new JABTransaction(jabSession, 5000);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXOctet(), jabSession, "X_OCTET",
                null);
        jabService.getRequest().setByteArray("X_OCTET", "test_tpcall_x_octet".getBytes());
        jabService.call(transaction);
        transaction.commit();
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
        byte[] received = jabService.getResponse().getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));

        jabSession.closeSession();
    }

    public void test_tpcall_x_c_type() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_c_type");
        runServer.tpadvertisetpcallXCType();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXCType(), jabSession, "X_C_TYPE",
                "acct_info");

        // Assemble the message ByteArrayOutputStream baos = new
        JABMessage dos = jabService.getRequest();
        dos.setLong("acct_no", 12345678);
        dos.setByteArray("name", "TOM".getBytes());
        float[] foo = new float[2];
        foo[0] = 1.1F;
        foo[1] = 2.2F;
        dos.setFloatArray("foo", foo);

        double[] balances = new double[2];
        balances[0] = 1.1;
        balances[1] = 2.2;
        dos.setDoubleArray("balances", balances);

        jabService.call(null);

        JABMessage response = jabService.getResponse();
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_c_type".getBytes(), 0, expected, 0, 15);
        byte[] received = response.getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));
        jabSession.closeSession();
    }

    public void test_tpcall_x_c_type_with_tx() throws Exception {
        log.debug("JABTestCase::test_tpcall_x_c_type_with_tx");
        runServer.tpadvertisetpcallXCType();
        JABSessionAttributes jabSessionAttributes = new JABSessionAttributes();
        JABSession jabSession = new JABSession(jabSessionAttributes);
        JABServiceInvoker jabService = new JABServiceInvoker(RunServer.getServiceNametpcallXCType(), jabSession, "X_C_TYPE",
                "acct_info");

        // Assemble the message ByteArrayOutputStream baos = new
        JABMessage dos = jabService.getRequest();
        dos.setLong("acct_no", 12345678);
        dos.setByteArray("name", "TOM".getBytes());
        float[] foo = new float[2];
        foo[0] = 1.1F;
        foo[1] = 2.2F;
        dos.setFloatArray("foo", foo);

        double[] balances = new double[2];
        balances[0] = 1.1;
        balances[1] = 2.2;
        dos.setDoubleArray("balances", balances);

        JABTransaction transaction = new JABTransaction(jabSession, 5000);
        jabService.call(transaction);
        transaction.commit();

        JABMessage response = jabService.getResponse();
        byte[] expected = new byte[60];
        System.arraycopy("tpcall_x_c_type".getBytes(), 0, expected, 0, 15);
        byte[] received = response.getByteArray("X_OCTET");
        assertTrue(Arrays.equals(expected, received));
        jabSession.closeSession();
    }
}
