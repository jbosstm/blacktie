import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import javax.naming.InitialContext;
import javax.naming.NameNotFoundException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.narayana.blacktie.administration.Authentication;
import org.jboss.narayana.blacktie.administration.BlacktieAdministration;
import org.jboss.narayana.blacktie.administration.BlacktieStompAdministrationService;
import org.jboss.narayana.blacktie.administration.core.AdministrationProxy;
import org.jboss.shrinkwrap.api.Archive;
import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.asset.StringAsset;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.jboss.shrinkwrap.resolver.api.DependencyResolvers;
import org.jboss.shrinkwrap.resolver.api.maven.MavenDependencyResolver;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class AdvertiseUnadvertiseTest {
    private static final Logger log = LogManager.getLogger(AdvertiseUnadvertiseTest.class);

    @Deployment
    public static Archive<?> createTestArchive() {
        // Path is set to ../pom.xml as we are configuring surefire to run from the target folder, by default it runs in / of
        // the project
        MavenDependencyResolver resolver = DependencyResolvers.use(MavenDependencyResolver.class).loadMetadataFromPom(
                "../pom.xml");
        final String ManifestMF = "Manifest-Version: 1.0\n"
                + "Dependencies: org.jboss.jts,org.jboss.as.controller-client,org.jboss.dmr\n";
        return ShrinkWrap
                .create(WebArchive.class, "test.war")
                .addClasses(BlacktieStompAdministrationService.class, Authentication.class, AdministrationProxy.class,
                        BlacktieAdministration.class)
                .addAsLibraries(resolver.artifact("org.jboss.narayana.blacktie:jatmibroker-xatmi").resolveAsFiles())
                .addAsResource("btconfig.xml").setManifest(new StringAsset(ManifestMF));
    }

    @Test
    public void testAdvertiseUnadvertise() throws Exception {
        BlacktieStompAdministrationService service = new BlacktieStompAdministrationService();
        log.info("Got the service");

        try {
            new InitialContext().lookup("java:/queue/BTR_.testsui1");
            fail("Should not be able to resolve the queue before it is created");
        } catch (NameNotFoundException e) {
            // Expected
            log.info("Got the exception");
        }

        assertTrue(service.deployQueue(".testsui1", "5.0.0.M2-SNAPSHOT") == 1);
        try {
            new InitialContext().lookup("java:/queue/BTR_.testsui1");
            log.info("Got the queue");
        } catch (NameNotFoundException e) {
            try {
                assertTrue(service.decrementConsumer(".testsui1") == 1);
            } finally {
                fail("Could not resolve the queue");
            }
        }
        assertTrue(service.decrementConsumer(".testsui1") == 1);
        log.info("Undeployed queue");
        try {
            new InitialContext().lookup("java:/queue/BTR_.testsui1");
            fail("Should not be able to resolve the queue after it is destroyed");
        } catch (NameNotFoundException e) {
            // Expected
            log.info("Got the exception 2");
        }

    }
}
