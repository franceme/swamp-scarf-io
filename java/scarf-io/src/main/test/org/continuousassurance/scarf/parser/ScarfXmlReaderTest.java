package org.continuousassurance.scarf.parser;

import org.continuousassurance.scarf.datastructures.*;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;

import static org.junit.Assert.*;

public class ScarfXmlReaderTest {

    //region Attributes
    private ScarfXmlReader xmlReader;

    private File sampleXMLFileOne;
    private InitialInfo stubbedInitialInfoOne;
    private BugInstance stubbedBugInstOne;

    private InitialInfo retrievedInitialInfoOne;
    private BugInstance retrievedBugInstOne;
    private Metric retrievedMetricOne;
    private MetricSummary retrievedMetricSumOne;
    private BugSummary retrievedBugSummaryOne;
    //endregion

    //region Test Environment Creation
    @Before
    public void setUp() throws Exception {
        sampleXMLFileOne = new File(getClass().getResource("SampleScarf.xml").getPath());

        //region stubbedInitialInfo
        stubbedInitialInfoOne = new InitialInfo("java-assess","2.6.5",
                "1504815630.1270213",null,null,
                "/home/vamshi/build","solr","pkg1","4.5.0",
                "resultparser","3.1.3","ubuntu-16.04-64",
                "dependency-check","2.1.1","c803d179-b3df-4eff-84be-67f62238a413");
        //endregion
        //region stubbedBugInstOne
        stubbedBugInstOne = new BugInstance(1);
        stubbedBugInstOne.setClassName("org.apache.lucene.codecs.BlockTreeTermsReader$FieldReader$IntersectEnum");
        stubbedBugInstOne.setBugSeverity("1");
        stubbedBugInstOne.setBugRank("6");
        stubbedBugInstOne.setResolutionSuggestion("  ");
        stubbedBugInstOne.setBugMessage("Possible null pointer dereference of BlockTreeTermsReader$FieldReader.index in new org.apache.lucene.codecs.BlockTreeTermsReader$FieldReader$IntersectEnum(BlockTreeTermsReader$FieldReader, CompiledAutomaton, BytesRef)");
        stubbedBugInstOne.setBugCode("NP_NULL_ON_SOME_PATH");
        stubbedBugInstOne.setBugGroup("CORRECTNESS");

        BugTrace stubbedBugTrace = new BugTrace();
        stubbedBugTrace.setBuildID("1");
        stubbedBugTrace.setAssessmentReportFile("AssessmentReportFile");

        InstanceLocation newLoc = new InstanceLocation();
        newLoc.setStartLine(0);
        newLoc.setEndLine(0);
        newLoc.setXPath("/BugCollection/BugInstance[5]");

        stubbedBugTrace.setInstanceLocation(newLoc);

        stubbedBugInstOne.setBugTrace(stubbedBugTrace);
        //endregion

    }

    @After
    public void tearDown() throws Exception {
        sampleXMLFileOne = null;
        stubbedInitialInfoOne = null;
        stubbedBugInstOne = null;
        retrievedInitialInfoOne = null;
        retrievedBugInstOne = null;
        retrievedMetricOne = null;
        retrievedMetricSumOne = null;
        retrievedBugSummaryOne = null;


        xmlReader = null;
    }
    //endregion

    //region Tests
    @Test
    public void readXMLFileOne()
    {
        assertNotNull(sampleXMLFileOne);

        //Creating a "pass-through" interface to set external variables
        ScarfInterface stubbedInterface = new ScarfInterface() {
            @Override
            public void initialCallback(InitialInfo initial) {
                retrievedInitialInfoOne = initial;
            }

            @Override
            public void bugCallback(BugInstance bug) {
                retrievedBugInstOne = bug;
            }

            @Override
            public void metricCallback(Metric metric) {
                retrievedMetricOne = metric;
            }

            @Override
            public void metricSummaryCallback(MetricSummary metricSum) {
                retrievedMetricSumOne = metricSum;
            }

            @Override
            public void bugSummaryCallback(BugSummary bugSum) {
                retrievedBugSummaryOne = bugSum;
            }

            @Override
            public void finalCallback() {
            }
        };

        xmlReader = new ScarfXmlReader(stubbedInterface);
        xmlReader.parseFromFile(sampleXMLFileOne);

        assertEquals(stubbedInitialInfoOne.toString(),retrievedInitialInfoOne.toString());
        assertEquals(stubbedBugInstOne.toString(),retrievedBugInstOne.toString());
        assertNull(retrievedMetricOne);
        assertNull(retrievedMetricSumOne);
        assertNull(retrievedBugSummaryOne);
    }
    //endregion
}