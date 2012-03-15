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

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.jms.JMSException;
import javax.naming.NamingException;
import javax.net.ServerSocketFactory;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.stomp.jms.StompConnect;
import org.codehaus.stomp.util.IOExceptionSupport;
import org.codehaus.stomp.util.ServiceSupport;

/**
 * @version $Revision: 52 $
 */
public class TcpTransportServer extends ServiceSupport implements Runnable {
    private static final Log log = LogFactory.getLog(TcpTransportServer.class);
    private StompConnect stompHandlerFactory;
    private ServerSocket serverSocket;
    private int backlog = 5000;
    private boolean trace;
    private Map transportOptions;
    private ServerSocketFactory serverSocketFactory;
    private boolean daemon = true;
    private Thread runner;
    private URI connectURI;
    private URI bindLocation;
    private List<TcpTransport> connections = new CopyOnWriteArrayList<TcpTransport>();

    public TcpTransportServer(StompConnect stompHandlerFactory, URI location, ServerSocketFactory serverSocketFactory)
            throws IOException, URISyntaxException {
        this.stompHandlerFactory = stompHandlerFactory;
        this.connectURI = location;
        this.bindLocation = location;
        this.serverSocketFactory = serverSocketFactory;
    }

    /**
     * @return pretty print of this
     */
    public String toString() {
        return "" + getBindLocation();
    }

    /**
     * pull Sockets from the ServerSocket
     */
    public void run() {
        while (!isStopped()) {
            Socket socket = null;
            try {
                socket = serverSocket.accept();
                if (socket != null) {
                    if (isStopped()) {
                        socket.close();
                    } else {
                        TcpTransport transport = createTransport(socket);
                        stompHandlerFactory.assignProtocolConverter(transport);
                        transport.start();
                        connections.add(transport);
                    }
                }
            } catch (SocketTimeoutException ste) {
                // expect this to happen
            } catch (IOException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a IOException", e);
                    onAcceptError(e);
                }
            } catch (NamingException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a NamingException", e);
                    onAcceptError(e);
                }
            } catch (URISyntaxException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a URISyntaxException", e);
                    onAcceptError(e);
                }
            } catch (IllegalArgumentException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a IllegalArgumentException", e);
                    onAcceptError(e);
                }
            } catch (IllegalAccessException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a IllegalAccessException", e);
                    onAcceptError(e);
                }
            } catch (InvocationTargetException e) {
                if (!isStopping() && !isStopped()) {
                    log.error("Got a InvocationTargetException", e);
                    onAcceptError(e);
                }
            }
        }
    }

    // Properties
    // -------------------------------------------------------------------------
    public boolean isDaemon() {
        return daemon;
    }

    /**
     * Sets whether the background read thread is a daemon thread or not
     */
    public void setDaemon(boolean daemon) {
        this.daemon = daemon;
    }

    /**
     * @return Returns the location.
     */
    public URI getConnectURI() {
        return connectURI;
    }

    /**
     * @param location The location to set.
     */
    public void setConnectURI(URI location) {
        this.connectURI = location;
    }

    public boolean isTrace() {
        return trace;
    }

    public void setTrace(boolean trace) {
        this.trace = trace;
    }

    public URI getBindLocation() {
        return bindLocation;
    }

    public void setBindLocation(URI bindLocation) {
        this.bindLocation = bindLocation;
    }

    public InetSocketAddress getSocketAddress() {
        return (InetSocketAddress) serverSocket.getLocalSocketAddress();
    }

    public void setTransportOption(Map transportOptions) {
        this.transportOptions = transportOptions;
    }

    // Implementation methods
    // -------------------------------------------------------------------------
    protected void doStart() throws IOException {
        bind();
        log.info("Listening for connections at: " + getConnectURI());
        runner = new Thread(this, "StompConnect Server Thread: " + toString());
        runner.setDaemon(daemon);
        runner.start();
    }

    protected void doStop() throws InterruptedException, IOException, JMSException, URISyntaxException {
        // lets stop accepting new connections first
        if (serverSocket != null) {
            serverSocket.close();
        }

        // now lets close all the connections
        try {
            for (TcpTransport connection : connections) {
                connection.stop();
            }
        } finally {
            connections.clear();
        }

        // lets join the server thread in case its blocked a little while
        if (runner != null) {
            log.debug("Attempting to join with runner");
            runner.join();
            log.debug("Joined with runner");
            runner = null;
        }
    }

    protected void bind() throws IOException {
        URI bind = getBindLocation();

        String host = bind.getHost();
        host = (host == null || host.length() == 0) ? "localhost" : host;
        InetAddress addr = InetAddress.getByName(host);

        try {
            this.serverSocket = serverSocketFactory.createServerSocket(bind.getPort(), backlog, addr);
            this.serverSocket.setSoTimeout(2000);
        } catch (IOException e) {
            throw IOExceptionSupport.create("Failed to bind to server socket: " + bind + " due to: " + e, e);
        }
        try {
            setConnectURI(new URI(bind.getScheme(), bind.getUserInfo(), resolveHostName(bind.getHost()),
                    serverSocket.getLocalPort(), bind.getPath(), bind.getQuery(), bind.getFragment()));
        } catch (URISyntaxException e) {
            throw IOExceptionSupport.create(e);
        }
    }

    protected void onAcceptError(Exception e) {
        log.error("Received accept error: " + e, e);
        try {
            stop();
        } catch (Exception e1) {
            log.error("Failed to shut down: " + e, e);
        }
    }

    /**
     * Allow derived classes to override the Transport implementation that this transport server creates.
     */
    protected TcpTransport createTransport(Socket socket) throws IOException {
        return new TcpTransport(socket, transportOptions);
    }

    protected String resolveHostName(String hostName) throws UnknownHostException {
        String result = hostName;
        // hostname can be null for vm:// protocol ...
        if (hostName != null && (hostName.equalsIgnoreCase("localhost") || hostName.equals("127.0.0.1"))) {
            result = InetAddress.getLocalHost().getHostName();
        }
        return result;
    }
}
