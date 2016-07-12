//  Copyright 2016 Brandon G. Klein
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
  

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <libxml/xmlwriter.h>
#include "filestructure.h"

////////////////////Writer operation structs/////////////////////////////////////


typedef struct BugSummary {
    int count;
    int byteCount;
    char * code;
    char * group;
    struct BugSummary * next;
} BugSummary;


typedef struct BugSummaries {
    char * code;
    struct BugSummary * codeSummary;
    struct BugSummaries * next;
} BugSummaries;


typedef struct MetricSummary {
    int count;
    int sum;
    int sumOfSquares;
    int max;
    int min;
    int valid;
    char * type;
    struct MetricSummary * next;
} MetricSummary;


typedef struct Writer {
    int bugId;
    int metricId;
    int errorLevel;
    struct BugSummaries * bugSums;
    struct MetricSummary * metricSum;
    xmlTextWriterPtr writer;
} Writer;


////////////////////////////Initializer/Closer////////////////////////////////////////


Writer * newWriter(char * filename)
{
    int rc;
    Writer * writerInfo = malloc(sizeof(Writer));
    writerInfo->writer = xmlNewTextWriterFilename(filename, 0);
    xmlTextWriterSetIndent(writerInfo->writer, 2);
    if (writerInfo->writer == NULL) {
        printf("testXmlwriterFilename: Error creating the xml writer\n");
	return NULL;
    }
    rc = xmlTextWriterStartDocument(writerInfo->writer, NULL, "UTF-8", NULL);
    if (rc < 0) {
	printf("Error at xmlTextWriterStartDocument\n");
	return NULL;
    }
    writerInfo->errorLevel = 2;
    writerInfo->bugId = 1;
    writerInfo->metricId = 1;
    writerInfo->metricSum = NULL;
    writerInfo->bugSums = NULL;
    return writerInfo;
    
}


void closeWriter (Writer * writerInfo) 
{
    xmlTextWriterEndDocument(writerInfo->writer);
    free(writerInfo);
}


///////////////////Accessors/Mutators///////////////////////////////////////////
int setPretty ( Writer * writerInfo, int pretty_level ) {
    if (writerInfo != NULL && pretty_level >= 0) {
	xmlTextWriterSetIndent(writerInfo->writer, pretty_level);
	return 0;
    } else {
	return 1;
    }
}

xmlTextWriterPtr getWriter (Writer * writerInfo) 
{
    if (writerInfo != NULL) {
	return writerInfo->writer;
    } else { 
	return NULL;
    }
}

int getErrorLevel(Writer * writerInfo) 
{
    if (writerInfo != NULL) {
	return writerInfo->errorLevel;
    } else { 
	return -1;
    }
}

int setErrorLevel(Writer * writerInfo, int errorLevel) 
{
    if (writerInfo != NULL) {
	if (errorLevel == 0 || errorLevel == 1 || errorLevel == 2) {
	    writerInfo->errorLevel = errorLevel;
	    return 0;
	} else {
	    printf ("Invalid Error Level");
	    return 1;
	}
    }
    return 1;
}

//void error (int errorLevel, char * message) 
//{
//    if (writerInfo != NULL) {
//	if (errorLevel == 0) {
//	    return;
//	} else {
//	    printf("%s", message);
//	    if (errorLevel == 2) {
//		exit(1);	
//	    }
//	}
//    } else {
//	printf ("invalid writer");
//    }
//
//}

void setIndent(Writer * writerInfo, int tabSpace) {
    xmlTextWriterSetIndent(writerInfo->writer, tabSpace);
}


////////////////////Write a bug/////////////////////////////////////////////
char * checkBug(BugInstance * bug , int bugID)
{
    char * errors = malloc(strlen("\0"));
    errors[0] = '\0';
    char * temp = malloc(140);    

//    printf("errors: %p::%s\n", errors, errors);
    if (bug->bugLocations == NULL) {	
	sprintf(temp, "Required element: BugLocations could not be found in BugInstance: %d\n", bugID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    } 
    if (bug->bugMessage == NULL) {	
	sprintf(temp, "Required element: BugMessage could not be found in BugInstance: %d\n", bugID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    } 
    if (bug->buildId == NULL) {	
	sprintf(temp, "Required element: BuildId could not be found in BugInstance: %d\n", bugID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    } 
    if (bug->assessmentReportFile == NULL) {	
	sprintf(temp, "Required element: AssessmentReportFile could not be found in BugInstance: %d\n", bugID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    } 
    if (bug->methods != NULL) {
	int methodID = 1;
	int methodPrimary = 0;
	Method * method = bug->methods;
	while (method != NULL) {
	    if (method->primary != 0 && method->primary != 1) {		
		sprintf(temp, "Invalid primary attribute for Method:%d in BugInstance: %d\n", methodID, bugID);
		errors = realloc(errors, strlen(errors) + strlen(temp));
		errors = strcat(errors, temp);
	    } else if (method->primary) {
	        if (methodPrimary) {		    
		    sprintf(temp, "Multiple primary methods in BugInstance: %d\n", bugID);
		    errors = realloc(errors, strlen(errors) + strlen(temp));
		    errors = strcat(errors, temp);
		} else {
		    methodPrimary = 1;
		}
	    }
	    if (method->name == NULL) {		
		sprintf(temp, "Required text not found: name of Method: %d in BugInstance %d\n", methodID, bugID);
		errors = realloc(errors, strlen(errors) + strlen(temp));
		errors = strcat(errors, temp);
	    }
	    method = method->next;
	    methodID++;
	}
	if (methodPrimary == 0) {	    
	    sprintf(temp, "Misformed Element: No primary Method found in  BugInstance: %d\n", bugID);
	    errors = realloc(errors, strlen(errors) + strlen(temp));
	    errors = strcat(errors, temp);
	}
    }

    Location * loc = bug->bugLocations;
    int locID = 1;
    int locPrimary = 0;
    while (loc != NULL) {
        if (loc->primary != 0 && loc->primary != 1) {	    
	    sprintf(temp, "Invalid primary attribute for a Location:%d in BugInstance: %d\n", locID, bugID);
	    errors = realloc(errors, strlen(errors) + strlen(temp));
	    errors = strcat(errors, temp);
        } else if (loc->primary) {
	   /* if (locPrimary) {
		sprintf(temp, "Multiple primary Locations in BugInstance: %d\n", bugID);
		errors = realloc(errors, strlen(errors) + strlen(temp) + 1);
		errors = strcat(errors, temp);
	    } else { */
		locPrimary = 1;
	   // }
	}
        if (loc->sourceFile == NULL) {	    
	    sprintf(temp, "Required Element: SourceFile of Location:%d in BugInstance: %d\n", locID, bugID);
	    errors = realloc(errors, strlen(errors) + strlen(temp));
	    errors = strcat(errors, temp);
        }

        loc = loc->next;
    }
    if (locPrimary == 0) {	    
        sprintf(temp, "Misformed Element: No primary Location found in  BugInstance: %d\n", bugID);
        errors = realloc(errors, strlen(errors) + strlen(temp));
        errors = strcat(errors, temp);
    }

    if (bug->instanceLocation != NULL) {
	if (bug->instanceLocation->lineNum.start == 0 && bug->instanceLocation->lineNum.end == 0 && bug->instanceLocation->xPath == NULL) {
    	    sprintf(temp, "Misformed Element: Neither LineNum or Xpath children were present in InstanceLocation BugInstance: %d", bugID);
	    errors = realloc(errors, strlen(errors) + strlen(temp));
	    errors = strcat(errors, temp);
	}
    }
    free(temp);
    return errors;
}


int addBug(Writer * writerInfo, BugInstance * bug)
{
//    if (bug->bugLocations == NULL) {
//	error(writerInfo->errorLevel, "Required element: BugLocations could not be found in BugInstance");
//    } 
//    if (bug->bugMessage == NULL) {
//	error(writerInfo->errorLevel, "Required element: BugMessage could not be found in BugInstance");
//    } 
//    if (bug->buildId == NULL) {
//	error(writerInfo->errorLevel, "Required element: BuildId could not be found in BugInstance");
//    } 
//    if (bug->assessmentReportFile == NULL) {
//	error(writerInfo->errorLevel, "Required element: AssessmentReportFile could not be found in BugInstance");
//    } 
//    if (bug->methods != NULL) {
//	Method temp = bug->methods;
//	while (temp != NULL) {
//	    if (temp->primary != 0 && temp->primary != 1) {
//		error(writerInfo->errorLevel, "Invalid primary attribute for Method");
//	    }
//	    if (temp->name == NULL) {
//		error(writerInfo->errorLevel, "Required text: name of Method");
//	    }
//	    temp = temp->next;
//	}
//    }
//    Location temp = bug->bugLocations;
//    while (temp != NULL) {
//        if (temp->primary != 0 && temp->primary != 1) {
//	   error(writerInfo->errorLevel, "Invalid primary attribute for a Location");
//        }
//        if (temp->sourceFile == NULL) {
//	   error(writerInfo->errorLevel, "Required Element: SourceFile of Location");
//        }
//        temp = temp->next;
//    }
    char * errors = NULL;
    errors = checkBug(bug, writerInfo->bugId);
    if ( strcmp(errors,"") != 0  && writerInfo->errorLevel != 0) {
	printf("%s", errors);
	if ( writerInfo->errorLevel == 2 ) {
	    exit(1);
	}
    }
    free(errors);

    char temp[24];
    int rc;
    long bytes = 0;
    xmlTextWriterPtr writer = writerInfo->writer;
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "BugInstance");
    if (rc < 0) {
        printf("Error at addBug adding BugInstance tag\n");
        return 1;
    }
    bytes += rc;

    sprintf(temp, "%d", writerInfo->bugId);
    
    rc = xmlTextWriterWriteAttribute(writer, (xmlChar *) "id", (xmlChar *) temp);
    
    if (rc < 0) {
        printf("Error adding BugInstance attribute id\n");
        return 1;
    }
    
    if (bug->className != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Class", (xmlChar *) bug->className);
        if (rc < 0) {
            printf("Error writing Class Element of BugInstance\n");
            return 1;
        }
    }

    if (bug->methods != NULL) {
        int methodId = 1;
        bytes += rc;
        rc = xmlTextWriterStartElement(writer, (xmlChar *) "Methods");
        if (rc < 0) {
            printf("Error at addBug adding Methods tag\n");
            return 1;
        }
        Method * cur = bug->methods;
        while (cur != NULL) {
            bytes += rc;
            rc = xmlTextWriterStartElement(writer, (xmlChar *) "Method");
            if (rc < 0) {
                printf("Error at addBug adding Method tag\n");
                return 1;
            }
            bytes += rc;
            sprintf(temp, "%d", methodId);
            rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "id", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error adding method attribute id\n");
                return 1;
            }
            if (cur->primary == 0) {
                bytes += rc;
                rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "primary", (xmlChar *) "false");
                if (rc < 0) {
                    printf("Error adding method attribute primary\n");
                    return 1;
                }
            } else { 
                bytes += rc;
                rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "primary", (xmlChar *) "true");
                if (rc < 0) {
                    printf("Error adding method attribute primary\n");
                    return 1;
                }
            }           
            bytes += rc;
            rc = xmlTextWriterWriteString(writer, (xmlChar *) cur->name);
            if (rc < 0) {
                printf("Error adding method content\n");
                return 1;
            }
            bytes += rc;
            rc = xmlTextWriterEndElement(writer);
            if (rc < 0) {
                printf("Error closing Method\n");
                return 1;
            }
            cur = cur->next;
            methodId++;
        }
        bytes += rc;
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) {
            printf("Error closing Methods\n");
            return 1;
        }
        
    } 

    bytes += rc;
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "BugLocations");
    if (rc < 0) {
        printf("Error at addBug adding BugLocations tag\n");
        return 1;
    }
    Location * curLoc = bug->bugLocations;
    int locId = 1;
    while (curLoc != NULL) {    
        bytes += rc;
        rc = xmlTextWriterStartElement(writer, (xmlChar *) "Location");
        if (rc < 0) {
            printf("Error at addBug adding Location tag\n");
            return 1;
        }
        bytes += rc;
        sprintf(temp, "%d", locId);
        rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "id", (xmlChar *) temp);
        if (rc < 0) {
            printf("Error adding Location  attribute id\n");
            return 1;
        }
        if (curLoc->primary == 0) {
            bytes += rc;
            rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "primary", (xmlChar *) "false");
            if (rc < 0) {
                printf("Error adding Location attribute primary\n");
                return 1;
            }
        } else { 
            bytes += rc;
            rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "primary", (xmlChar *) "true");
            if (rc < 0) {
                printf("Error adding Location attribute primary\n");
                return 1;
            }
        }
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "SourceFile", (xmlChar *) curLoc->sourceFile);
        if (rc < 0) {
            printf("Error writing SourceFile Element of Location in BugInstance\n");
            return 1;
        }
    
        if (curLoc->startLine != 0) {
            bytes += rc;
            sprintf(temp, "%d", curLoc->startLine);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "StartLine", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing StartLine Element of Location\n");
                return 1;
            }
        } 
        if (curLoc->endLine != 0) {
            bytes += rc;
            sprintf(temp, "%d", curLoc->endLine);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "EndLine", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing EndLine Element of Location\n");
                return 1;
            }
        }
        if (curLoc->startColumn != 0) {
            bytes += rc;
            sprintf(temp, "%d", curLoc->startColumn);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "StartColumn", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing StartColumn Element of Location\n");
                return 1;
            }
        }
        if (curLoc->endColumn != 0) {
            bytes += rc;
            sprintf(temp, "%d", curLoc->endColumn);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "EndColumn", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing EndColumn Element of Location\n");
                return 1;
            }
        }
        if (curLoc->explanation != NULL) {
            bytes += rc;
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Explanation", (xmlChar *) curLoc->explanation);
            if (rc < 0) {
                printf("Error writing Explanation Element of Location\n");
                return 1;
            }
        }
        bytes += rc;
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) {
            printf("Error closing Location\n");
            return 1;
        }
        curLoc = curLoc->next;
        locId++;
    }
    bytes += rc;
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("Error closing BugLocations\n");
        return 1;
    }
 
    CweIds * cwe = bug->cweIds;
    while (cwe != NULL) {
        bytes += rc;
        sprintf(temp, "%d", cwe->cweid);
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "CweId", (xmlChar *) temp);
        if (rc < 0) {
            printf("Error writing CweID Element of BugInstance\n");
            return 1;
        }
        cwe = cwe->next;
    }

    if (bug->bugGroup != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BugGroup", (xmlChar *) bug->bugGroup);
        if (rc < 0) {
            printf("Error writing BugGroup Element of BugInstance\n");
            return 1;
        }       
    }
    if (bug->bugCode != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BugCode", (xmlChar *) bug->bugCode);
        if (rc < 0) {
            printf("Error writing BugCode Element of BugInstance\n");
            return 1;
        }       
    }
    if (bug->bugRank != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BugRank", (xmlChar *) bug->bugRank);
        if (rc < 0) {
            printf("Error writing BugRank Element of BugInstance\n");
            return 1;
        }       
    }
    if (bug->bugSeverity != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BugSeverity", (xmlChar *) bug->bugSeverity);
        if (rc < 0) {
            printf("Error writing BugSeverity Element of BugInstance\n");
            return 1;
        }       
    }
    bytes += rc;
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BugMessage", (xmlChar *) bug->bugMessage);
    if (rc < 0) {
        printf("Error writing BugMessage Element of BugInstance\n");
        return 1;
    }   
     if (bug->resolutionSuggestion != NULL) {
        bytes += rc;
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "ResolutionSuggestion", (xmlChar *) bug->resolutionSuggestion);
        if (rc < 0) {
            printf("Error writing ResolutionSuggestion Element of BugInstance\n");
            return 1;
        }       
    }
    
    bytes += rc;
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "BugTrace");
    if (rc < 0) {
        printf("Error at addBug adding BugTrace tag\n");
        return 1;
    }
    bytes += rc;
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "BuildId", (xmlChar *) bug->buildId);
    if (rc < 0) {
        printf("Error writing BuildId Element of BugTrace\n");
        return 1;
    }   
    bytes += rc;
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "AssessmentReportFile", (xmlChar *) bug->assessmentReportFile);
    if (rc < 0) {
        printf("Error writing AssessmentReportFile Element of BugTrace\n");
        return 1;
    }
    InstanceLocation * inst = bug->instanceLocation;
    if (inst != NULL){
        bytes += rc;
        rc = xmlTextWriterStartElement(writer, (xmlChar *) "InstanceLocation");
        if (rc < 0) {
            printf("Error adding InstanceLocation start tag in BugTrace\n");
            return 1;
        }
        if (inst->xPath != NULL) {
            bytes += rc;
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Xpath", (xmlChar *) inst->xPath);
            if (rc < 0) {
                printf("Error writing BugMessage Element of InstanceLocation\n");
                return 1;
            }
        }
        if (inst->lineNum.start != 0 || inst->lineNum.end != 0){
            bytes += rc;
            rc = xmlTextWriterStartElement(writer, (xmlChar *) "LineNum");
            if (rc < 0) {
                printf("Error adding LineNum start tag in InstanceLocation\n");
                return 1;
            }
             LineNum lineNum = inst->lineNum;
            bytes += rc;
            sprintf(temp, "%d", lineNum.start);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Start", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing Start Element of LineNum\n");
                return 1;
            }
            bytes += rc;
            sprintf(temp, "%d", lineNum.end);
            rc = xmlTextWriterWriteElement(writer, (xmlChar *) "End", (xmlChar *) temp);
            if (rc < 0) {
                printf("Error writing End Element of LineNum\n");
                return 1;
            }
            bytes += rc;
            rc = xmlTextWriterEndElement(writer);
            if (rc < 0) {
                printf("Error closing LineNum\n");
                return 1;
            }
        }
        bytes += rc;
        rc = xmlTextWriterEndElement(writer);
        if (rc < 0) {
            printf("Error closing InstanceLocation\n");
            return 1;
        }
    }
    bytes += rc;
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("Error closing BugTrace\n");
        return 1;
    }
    
    bytes += rc;
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("Error closing BugInstance\n");
        return 1;
    }
    bytes += rc;
    rc = xmlTextWriterFlush(writer);
    bytes += rc;
    ///////////////////////////////Group bugs/////////////////////
    char * code = bug->bugCode;
    if (code == NULL) {
        code = "undefined";
    }
    char * group = bug->bugGroup;
    if (group == NULL) {
        group = "undefined";
    } 

    BugSummaries * prev = NULL;
    BugSummaries * cur = writerInfo->bugSums;
    while (cur != NULL && strcmp(cur->code, code) != 0){
        prev = cur;
        cur = cur->next;
    }
    
    if (cur == NULL) {
        BugSummaries * summaries = malloc(sizeof(BugSummaries));
        summaries->code = malloc(strlen(code));
        strcpy(summaries->code, code);
    
        BugSummary * summary = malloc(sizeof(BugSummary));
        summary->count = 1;
        summary->byteCount = bytes;
        summary->next = NULL;
        summary->code = malloc(strlen(code));
        strcpy(summary->code, code);
        summary->group = malloc(strlen(group));
        strcpy(summary->group, group);
        summaries->codeSummary = summary;
        summaries->next = NULL;
        
        if (prev == NULL) {
            writerInfo->bugSums = summaries;
        } else {
            prev->next = summaries;
        }
    } else {
        BugSummary * prevGroup = NULL;
        BugSummary * curGroup = cur->codeSummary;
        while (curGroup != NULL && strcmp(curGroup->group, group) != 0){
            prevGroup = curGroup;
            curGroup = curGroup->next;
        }
        if (curGroup == NULL) {
            BugSummary * summary = malloc(sizeof(BugSummary));
            summary->count = 1;
            summary->byteCount = bytes;
            summary->next = NULL;
            summary->code = cur->code;
            summary->group = malloc(strlen(group));
            strcpy(summary->group, group);

            if (prevGroup == NULL) {
                cur->codeSummary = summary;
            } else {
                prevGroup->next = summary;
            }

        } else {
            curGroup->count++;
            curGroup->byteCount += bytes;
        }
    }

    
    writerInfo->bugId++;
    return 0;
}


////////////////////////Add a metric/////////////////////////////////////////////////////////////
char * checkMetric(Metric * metric, int metricID)
{
    char * errors = malloc(strlen("\0"));
    errors[0] = '\0';
    char * temp = malloc(140);
    if (metric->value == NULL){
	sprintf(temp, "Required Element: Value not found in Metric: %d\n", metricID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    if ( metric->type == NULL ) {
	sprintf(temp, "Required Element: Type not found in Metric: %d\n", metricID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    if ( metric->sourceFile == NULL ) {
	sprintf(temp, "Required Element: SourceFile not found in Metric: %d\n", metricID);
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    return errors;


}


int addMetric(Writer *  writerInfo, Metric * metric)
{
    char * errors = NULL;
    errors = checkMetric(metric, writerInfo->metricId);
    if ( strcmp(errors,"") != 0  && writerInfo->errorLevel != 0) {
	printf("%s", errors);
	if ( writerInfo->errorLevel == 2 ) {
	    exit(1);
	}
    }
    free(errors);


    int rc;
    char temp[24];
    xmlTextWriterPtr writer = writerInfo->writer;
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "Metric");
    if (rc < 0) {
        printf("Error at addMetric adding Metric tag\n");
        return 1;
    }
    sprintf(temp, "%d", writerInfo->metricId);
    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "id", (xmlChar *) temp);
    if (rc < 0) {
        printf("Error adding metric attribute id\n");
        return 1;
    }
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "Location");
    if (rc < 0) {
        printf("Error at adding Location start tag to metric\n");
        return 1;
    }
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "SourceFile", (xmlChar *) metric->sourceFile);
    if (rc < 0) {
        printf("Error writing SourceFile Element\n");
        return 1;
    }
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("Error closing Location\n");
        return 1;
    }
    if (metric->clas != NULL) {
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Class", (xmlChar *) metric->clas);
        if (rc < 0) {
            printf("Error writing Class Element of metric\n");
            return 1;
        }
    }
    if(metric->method != NULL) {
        rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Method", (xmlChar *) metric->method);
        if (rc < 0) {
            printf("Error writing Method Element of metric\n");
            return 1;
        }
    }
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Type", (xmlChar *) metric->type);
    if (rc < 0) {
        printf("Error writing Type Element of metric\n");
        return 1;
    }
    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Value", (xmlChar *) metric->value);
    if (rc < 0) {
        printf("Error writing Value Element of metric\n");
        return 1;
    }
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
        printf("Error closing Metric\n");
        return 1;
    }
    xmlTextWriterFlush(writer);

    char * type = metric->type;
    if (type == NULL) {
        type = "undefined";
    } 

    double value;
    char * buffer = NULL;
    MetricSummary * prev = NULL;
    MetricSummary * cur = writerInfo->metricSum;
    while (cur != NULL && strcmp(cur->type, type) != 0){
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) {
        MetricSummary * summary = malloc(sizeof(MetricSummary));
        summary->type = malloc(strlen(type));
        strcpy(summary->type, type);
        summary->count = 1;
        value = strtod(metric->value, &buffer);
        if (buffer != metric->value) {
            summary->sum = value;
            summary->sumOfSquares = value * value;
            summary->max = value;
            summary->min = value;
            summary->valid = 1;
        }
        summary->next = NULL;

        if (prev == NULL) {
            writerInfo->metricSum = summary;
        } else {
            prev->next = summary;
        }
    } else {
        cur->count++;
        if (cur->valid != 0) {
            value = strtod(metric->value, &buffer);                 
            if (buffer != metric->value) {
                cur->sum += value;
                cur->sumOfSquares += value * value;
                if (value > cur->max){
                    cur->max = value;
                }
                if (value < cur->min){
                    cur->min = value;
                }
            } else {
                cur->valid = 0;
            }
        }
    }
    writerInfo->metricId++;
    return 0;
}
  

//////////////////////////////////////////////////////////////////////////////

char * checkStart(Initial * initial){
    char * errors = malloc(strlen("\0"));
    errors[0] = '\0'; 
    if (initial->tool_name == NULL){
	char * temp = "Required Attribute: tool_name not found in Initial";
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    if ( initial->tool_version == NULL ) {
	char * temp = "Required Attribute: tool_version not found in Initial";
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    if ( initial->uuid == NULL ) {
	char * temp = "Required Attribute: uuid not found in Initial";
	errors = realloc(errors, strlen(errors) + strlen(temp));
	errors = strcat(errors, temp);
    }
    return errors;
}

int addStartTag(Writer * writerInfo, Initial * initial)
{

    char * errors = NULL;
    errors = checkStart(initial);
    if ( strcmp(errors,"") != 0  && writerInfo->errorLevel != 0) {
	printf("%s", errors);
	if ( writerInfo->errorLevel == 2 ) {
	    exit(1);
	}
    }
    free(errors);
    
//    if ( initial->tool_name == NULL ) {
//	error(writerInfo->errorLevel, "Required Attribute: tool_name not found in Initial");	
//    }
//    if ( initial->tool_version == NULL ) {
//	error(writerInfo->errorLevel, "Required Attribute: tool_version not found in Initial");	
//    }
//    if ( initial->uuid == NULL ) {
//	error(writerInfo->errorLevel, "Required Attribute: uuid not found in Initial");	
//    }

    int rc;
    xmlTextWriterPtr writer = writerInfo->writer;
    rc = xmlTextWriterStartElement(writer, (xmlChar *) "AnalyzerReport");
    if (rc < 0) {
	printf("Error at addStartTag\n");
	return 1;
    }

    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "tool_name", (xmlChar *) initial->tool_name);
    if (rc < 0) {
	printf("Error adding start tag attribute tool_name\n");
	return 1;
    }

    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "tool_version", (xmlChar *) initial->tool_version);
    if (rc < 0) {
	printf("Error adding start tag attribute tool_version\n");
	return 1;
    }

    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "uuid", (xmlChar *) initial->uuid);
    if (rc < 0) {
	printf("Error adding start tag attribute uuid\n");
	return 1;
    }
    xmlTextWriterFlush(writer);
    return 0;
}


//////////////////////End initialtag/////////////////////////////////////////////
int addEndTag(Writer * writerInfo)
{
    int rc;
    xmlTextWriterPtr writer = writerInfo->writer;
    rc = xmlTextWriterEndElement(writer);
    if (rc < 0) {
	printf("Error closing document\n");
	return 1;
    }
    xmlTextWriterFlush(writer);
    return 0;
}


//////////////Add summary generated from instances//////////////////////////////////
int addSummary(Writer * writerInfo)
{
    xmlTextWriterPtr writer = writerInfo->writer;
    int rc = -1;
    char temp[24];

    BugSummaries * curBugSummary = writerInfo->bugSums;
    MetricSummary * curMetricSummary = writerInfo->metricSum;
    BugSummary * curBugSummaryGroup;
    
    if (curBugSummary != NULL) {
	rc = xmlTextWriterStartElement(writer, (xmlChar *) "BugSummary");
	if (rc < 0) {
	    printf("Error adding BugSummary start tag\n");
	    return 1;
	}
    }
    while (curBugSummary != NULL) {
	curBugSummaryGroup = curBugSummary->codeSummary;
	while (curBugSummaryGroup != NULL) {
	    rc = xmlTextWriterStartElement(writer, (xmlChar *) "BugCategory");
	    if (rc < 0) {
		printf("Error adding bug category start tag\n");
		return 1;
	    }
	    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "code", (xmlChar *) curBugSummaryGroup->code);
	    if (rc < 0) {
		printf("Error adding BugCategory attribute code\n");
		return 1;
	    }
	    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "group", (xmlChar *) curBugSummaryGroup->group);
	    if (rc < 0) {
		printf("Error adding BugCategory attribute group\n");
		return 1;
	    }
	    sprintf(temp, "%d", curBugSummaryGroup->count);
	    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "count", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error adding BugCategory attribute count\n");
		return 1;
	    }
	    sprintf(temp, "%d", curBugSummaryGroup->byteCount);
	    rc = xmlTextWriterWriteAttribute(writer,  (xmlChar *) "bytes", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error adding BugCategory attribute byteCount\n");
		return 1;
	    }
	    rc = xmlTextWriterEndElement(writer);
	    if (rc < 0) {
		printf("Error closing BugCategory\n");
		return 1;
	    }
	    curBugSummaryGroup = curBugSummaryGroup->next;
	}
	curBugSummary = curBugSummary->next;
    }
    if (rc > 0){
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
	    printf("Error closing BugSummary\n");
	    return 1;
	}
    }

    rc = -1;
    if (curMetricSummary != NULL) {
	rc = xmlTextWriterStartElement(writer, (xmlChar *) "MetricSummaries");
	if (rc < 0) {
	    printf("Error adding MetricSummaries start tag\n");
	    return 1;
	}
    }
    while (curMetricSummary != NULL){
	int count = curMetricSummary->count;
	int sum = curMetricSummary->sum;
	int sumOfSquares = curMetricSummary->sumOfSquares;

	rc = xmlTextWriterStartElement(writer, (xmlChar *) "MetricSummary");
	if (rc < 0) {
	    printf("Error adding MetricSummary start tag\n");
	    return 1;
	}
	rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Type", (xmlChar *) curMetricSummary->type);
	if (rc < 0) {
	    printf("Error writing Type Element of MetricSummary\n");
	    return 1;
	}
	sprintf(temp, "%d", count);
	rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Count", (xmlChar *) temp);
	if (rc < 0) {
	    printf("Error writing Count Element of MetricSummary\n");
	    return 1;
	}
	if (curMetricSummary->valid != 0) {
	    sprintf(temp, "%d", sum);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Sum", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing Sum Element of MetricSummary\n");
		return 1;
	    }
	    sprintf(temp, "%d", sumOfSquares);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "SumOfSquares", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing SumOfSquares Element of MetricSummary\n");
		return 1;
	    }
	    sprintf(temp, "%d", curMetricSummary->max);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Maximum", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing Maximum Element of MetricSummary\n");
		return 1;
	    }
	    sprintf(temp, "%d", curMetricSummary->min);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Minimum", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing Minimum Element of MetricSummary\n");
		return 1;
	    }

	    double average = ((double) sum) / count;
	    sprintf(temp, "%.2f", average);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "Average", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing Average Element of MetricSummary\n");
		return 1;
	    }

	    int denominator = count * (count - 1);
	    double standard_dev = 0;
	    if (denominator != 0) {
		standard_dev = sqrt(((sumOfSquares * count) - (sum * sum) ) / denominator);
	    }

	    sprintf(temp, "%.2f", standard_dev);
	    rc = xmlTextWriterWriteElement(writer, (xmlChar *) "StandardDeviation", (xmlChar *) temp);
	    if (rc < 0) {
		printf("Error writing StandardDeviation Element of MetricSummary\n");
		return 1;
	    }
	}
	
	curMetricSummary = curMetricSummary->next;
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
	    printf("Error closing MetricSummary\n");
	    return 1;
	}
    }
    if (rc > 0){
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
	    printf("Error closing MetricSummaries\n");
	    return 1;
	}
    }
    xmlTextWriterFlush(writer);
    return 0;
}

