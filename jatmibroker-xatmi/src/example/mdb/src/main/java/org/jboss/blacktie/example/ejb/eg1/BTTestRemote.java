package org.jboss.blacktie.example.ejb.eg1;

import javax.ejb.*;

@Remote
public interface BTTestRemote {
    public String txRequired(String arg);
    public String txSupports(String arg);
    public String txMandatory(String arg);
    public String txNever(String arg);
    public String txRequiresNew(String arg);
    public String txNotSupported(String arg);

	public String echo(String s);
}
