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
package org.codehaus.stomp.util;

import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.net.URISyntaxException;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.jms.JMSException;

/**
 * A helper class for working with services together with a useful base class for service implementations.
 * 
 * @version $Revision: 50 $
 */
public abstract class ServiceSupport {
    private AtomicBoolean started = new AtomicBoolean(false);
    private AtomicBoolean stopping = new AtomicBoolean(false);
    private AtomicBoolean stopped = new AtomicBoolean(false);

    public void start() throws IOException, URISyntaxException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException {
        if (started.compareAndSet(false, true)) {
            doStart();
        }
    }

    public void stop() throws InterruptedException, IOException, JMSException, URISyntaxException {
        if (stopped.compareAndSet(false, true)) {
            stopping.set(true);
            try {
                doStop();
            } finally {
                stopped.set(true);
                started.set(false);
                stopping.set(false);
            }
        }
    }

    /**
     * @return true if this service has been started
     */
    public boolean isStarted() {
        return started.get();
    }

    /**
     * @return true if this service is in the process of closing
     */
    public boolean isStopping() {
        return stopping.get();
    }

    /**
     * @return true if this service is closed
     */
    public boolean isStopped() {
        return stopped.get();
    }

    protected abstract void doStop() throws InterruptedException, IOException, JMSException, URISyntaxException;

    protected abstract void doStart() throws IOException, URISyntaxException, IllegalArgumentException, IllegalAccessException,
            InvocationTargetException;
}
