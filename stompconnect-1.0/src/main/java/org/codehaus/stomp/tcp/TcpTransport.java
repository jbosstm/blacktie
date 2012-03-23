/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.codehaus.stomp.tcp;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.InvocationTargetException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.URI;
import java.net.URISyntaxException;

import javax.jms.JMSException;
import javax.net.SocketFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.stomp.ProtocolException;
import org.codehaus.stomp.Stomp;
import org.codehaus.stomp.StompFrame;
import org.codehaus.stomp.StompMarshaller;
import org.codehaus.stomp.jms.ProtocolConverter;
import org.codehaus.stomp.util.ServiceSupport;

/**
 * @version $Revision: 65 $
 */
public class TcpTransport extends ServiceSupport implements Runnable {
    private static final Log log = LogFactory.getLog(TcpTransport.class);
    private StompMarshaller marshaller = new StompMarshaller();
    private ProtocolConverter inputHandler;
    private final URI remoteLocation;
    private final URI localLocation;
    private int connectionTimeout = 30000;
    private int soTimeout = 0;
    private int socketBufferSize = 64 * 1024;
    private Socket socket;
    private DataOutputStream dataOut;
    private DataInputStream dataIn;
    private SocketFactory socketFactory;
    private Boolean keepAlive;
    private Boolean tcpNoDelay;
    private boolean daemon = false;
    private Thread runner;

    /**
     * Initialize from a server Socket
     */
    public TcpTransport(Socket socket) throws IOException {
        this.socket = socket;
        this.remoteLocation = null;
        this.localLocation = null;
        this.daemon = true;
    }

    /**
     * A one way asynchronous send
     * 
     * @throws UnsupportedEncodingException
     * @throws IOException
     */
    public void onStompFrame(StompFrame command) throws IOException {
        checkStarted();
        marshaller.marshal(command, dataOut);
        dataOut.flush();
    }

    /**
     * @return pretty print of 'this'
     */
    public String toString() {
        return "tcp://" + socket.getInetAddress() + ":" + socket.getPort();
    }

    /**
     * reads packets from a Socket
     */
    public void run() {
        log.trace("StompConnect TCP consumer thread starting");
        while (!isStopped()) {
            try {
                StompFrame frame = marshaller.unmarshal(dataIn);
                log.debug("Sending stomp frame");
                try {
                    inputHandler.onStompFrame(frame);
                } catch (IOException e) {
                    if (frame.getAction().equals(Stomp.Responses.ERROR)) {
                        log.warn("Could not send frame to client: " + new String(frame.getContent()));
                    }
                    throw e;
                }
            } catch (Throwable e) {
                // no need to log EOF exceptions
                if (e instanceof EOFException) {
                    // Should only happen when a sender disconnects
                    log.debug("Caught an EOFException: " + e.getMessage(), e);
                } else {
                    log.fatal("Caught an exception: " + e.getMessage(), e);
                }
                try {
                    stop();
                } catch (Exception e2) {
                    log.warn("Caught while closing: " + e2 + ". Now Closed", e2);
                }
            }
        }
    }

    public void setProtocolConverter(ProtocolConverter protocolConverter) {
        this.inputHandler = protocolConverter;
    }

    // /**
    // * Configures the socket for use
    // *
    // * @param sock
    // * @throws SocketException
    // * @throws URISyntaxException
    // * @throws InvocationTargetException
    // * @throws IllegalAccessException
    // * @throws IllegalArgumentException
    // */
    // protected void initialiseSocket(Socket sock) throws SocketException, IllegalArgumentException, IllegalAccessException,
    // InvocationTargetException, URISyntaxException {
    //
    // // try {
    // // sock.setReceiveBufferSize(socketBufferSize);
    // // sock.setSendBufferSize(socketBufferSize);
    // // } catch (SocketException se) {
    // // log.warn("Cannot set socket buffer size = " + socketBufferSize);
    // // log.debug("Cannot set socket buffer size. Reason: " + se, se);
    // // }
    // // sock.setSoTimeout(soTimeout);
    // //
    // // if (keepAlive != null) {
    // // sock.setKeepAlive(keepAlive.booleanValue());
    // // }
    // // if (tcpNoDelay != null) {
    // // sock.setTcpNoDelay(tcpNoDelay.booleanValue());
    // // }
    // }

    protected void doStart() throws IOException, IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            URISyntaxException {
        connect();

        runner = new Thread(this, "StompConnect Transport: " + toString());
        runner.setDaemon(daemon);
        runner.start();
    }

    protected void connect() throws IOException, IllegalArgumentException, IllegalAccessException, InvocationTargetException,
            URISyntaxException {

        if (socket == null && socketFactory == null) {
            throw new IllegalStateException("Cannot connect if the socket or socketFactory have not been set");
        }

        InetSocketAddress localAddress = null;
        InetSocketAddress remoteAddress = null;

        if (localLocation != null) {
            localAddress = new InetSocketAddress(InetAddress.getByName(localLocation.getHost()), localLocation.getPort());
        }

        if (remoteLocation != null) {
            String host = remoteLocation.getHost();
            remoteAddress = new InetSocketAddress(host, remoteLocation.getPort());
        }

        if (socket != null) {

            if (localAddress != null) {
                socket.bind(localAddress);
            }

            // If it's a server accepted socket.. we don't need to connect it
            // to a remote address.
            if (remoteAddress != null) {
                if (connectionTimeout >= 0) {
                    socket.connect(remoteAddress, connectionTimeout);
                } else {
                    socket.connect(remoteAddress);
                }
            }
        } else {
            // For SSL sockets.. you can't create an unconnected socket :(
            // This means the timout option are not supported either.
            if (localAddress != null) {
                socket = socketFactory.createSocket(remoteAddress.getAddress(), remoteAddress.getPort(),
                        localAddress.getAddress(), localAddress.getPort());
            } else {
                socket = socketFactory.createSocket(remoteAddress.getAddress(), remoteAddress.getPort());
            }
        }

        // initialiseSocket(socket);
        initializeStreams();
    }

    protected void doStop() throws InterruptedException, IOException, JMSException, URISyntaxException {
        if (log.isDebugEnabled()) {
            log.debug("Stopping transport " + this);
        }
        if (inputHandler != null) {
            inputHandler.close();
        }

        dataOut.flush();
        socket.shutdownOutput();
        socket.shutdownInput();
        // Closing the streams flush the sockets before closing.. if the socket
        // is hung.. then this hangs the close.
        // closeStreams();
        // if (socket != null) {
        // socket.close();
        // }
    }

    protected void checkStarted() throws ProtocolException {
        if (!isStarted()) {
            throw new ProtocolException("The transport is not running.");
        }
    }

    protected void initializeStreams() throws IOException {
        // TcpBufferedInputStream buffIn = new TcpBufferedInputStream(socket.getInputStream(), ioBufferSize);
        this.dataIn = new DataInputStream(socket.getInputStream());// new DataInputStream(buffIn);
        // TcpBufferedOutputStream buffOut = new TcpBufferedOutputStream(socket.getOutputStream(), ioBufferSize);
        this.dataOut = new DataOutputStream(socket.getOutputStream());// new DataOutputStream(buffOut);
    }

    protected void closeStreams() throws IOException {
        if (dataOut != null) {
            dataOut.close();
        }
        if (dataIn != null) {
            dataIn.close();
        }
    }
}
