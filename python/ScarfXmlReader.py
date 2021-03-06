#!/usr/bin/python

#  Copyright 2016 Brandon G. Klein
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

import xml.etree.ElementTree as etree


class ScarfXmlReader:

    # initialize information
    def __init__(self, inputFile):
        self.inputFile = inputFile
        self.encoding = None
        #self.callback = callback
        self.callback = dict()
        self.validStart = 0
        self.validBody = 0

    #################Callback Accessors/Mutators##############
    def SetEncoding(self, encoding):
        self.encoding = encoding

    def SetInitialCallback(self, callback):
        self.callback["InitialCallback"] = callback

    def SetBugCallback(self, callback):
        self.callback["BugCallback"] = callback

    def SetMetricCallback(self, callback):
        self.callback["MetricCallback"] = callback

    def SetBugSummaryCallback(self, callback):
        self.callback["BugSummaryCallback"] = callback

    def SetMetricSummaryCallback(self, callback):
        self.callback["MetricSummaryCallback"] = callback

    def SetFinalCallback(self, callback):
        self.callback["FinalCallback"] = callback

    def SetCallbackData(self, callbackData):
        self.callback["CallbackData"] = callbackData

    def GetEncoding(self):
        return self.encoding

    def GetInitialCallback(self):
        return self.callback["InitialCallback"]

    def GetBugCallback(self):
        return self.callback["BugCallback"]

    def GetMetricCallback(self):
        return self.callback["MetricCallback"]

    def GetBugSummaryCallback(self):
        return self.callback["BugSummaryCallback"]

    def GetMetricSummaryCallback(self):
        return self.callback["MetricSummaryCallback"]

    def GetFinalCallback(self):
        return self.callback["FinalCallback"]

    def GetCallbackData(self):
        return self.callback["CallbackData"]

    # parse file
    def Parse(self):

        ret = None
        #setup parser
        callback = self.callback
        for event, elem in etree.iterparse(self.inputFile,
                                           events=("start", "end"),
                                           parser=etree.XMLParser(encoding=self.encoding)):

            #parse initial information
            if elem.tag == "AnalyzerReport" and event == "start":
                initialDetails = {
                    'assess_fw': elem.get('assess_fw'),
                    'assess_fw_version': elem.get('assess_fw_version'),
                    'assessment_start_ts': elem.get('assessment_start_ts'),
                    'build_fw': elem.get('build_fw'),
                    'build_fw_version': elem.get('build_fw_version'),
                    'build_root_dir': elem.get('build_root_dir'),
                    'package_name': elem.get('package_name'),
                    'package_root_dir': elem.get('package_root_dir'),
                    'package_version': elem.get('package_version'),
                    'parser_fw': elem.get('parser_fw'),
                    'parser_fw_version': elem.get('parser_fw_version'),
                    'platform_name': elem.get('platform_name'),
                    'tool_name': elem.get('tool_name'),
                    'tool_version': elem.get('tool_version'),
                    'uuid': elem.get('uuid')
                }

                parent = elem

                if "InitialCallback" in callback:
                    if "CallbackData" in callback:
                        ret = callback["InitialCallback"](initialDetails, callback["CallbackData"])
                        if ret is not None:
                            if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret, callback["CallbackData"])
                            break
                    else:
                        ret = callback["InitialCallback"](initialDetails)
                        if ret is not None:
                            if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret)
                            break
                parent.clear()
                self.startValid = 1

            # parse bug instance
            elif elem.tag == "BugInstance" and "BugCallback" in callback and event == "end":
                if not self.startValid:
                    print("Misformed SCARF File: No AnalyzerReport Tag before first element")
                self.validBody = 1
                bug = {"BugId": elem.get("id")}
                cweids = []
                methods = []
                locations = []

                for subelement in elem:
                    tag = subelement.tag

                    if tag in ["ClassName", "BugGroup", "BugCode",
                               "BugRank", "BugSeverity", "BugMessage",
                               "ResolutionSuggestion"]:
                        bug[tag] = subelement.text.strip()

                    elif tag == "BugTrace":
                        for children in subelement:
                            if children.tag in ["AssessmentReportFile", "BuildId"]:
                                bug[children.tag] = children.text.strip()
                            elif children.tag == "InstanceLocation":
                                instanceLocation = {}
                                for inst in children:
                                    if inst.tag == "Xpath":
                                        instanceLocation["Xpath"] = inst.text.strip()
                                    elif inst.tag == "LineNum":
                                        lineInfo = {}
                                        for lineNum in inst:
                                            lineInfo[lineNum.tag] = lineNum.text.strip()
                                        instanceLocation["LineNum"] = lineInfo
                                bug["InstanceLocation"] = instanceLocation

                    elif tag == "CweId":
                        cweids.append(subelement.text.strip())

                    elif tag == "Methods":
                        for method in subelement:
                            methodMap = {"MethodId": method.get("id")}
                            if method.get("primary") == "true":
                                methodMap["primary"] = 1
                            else:
                                methodMap["primary"] = 0
                            methodMap["name"] = method.text.strip()
                            methods.append(methodMap)

                    elif tag == "BugLocations":
                        for location in subelement:
                            locationMap = {"LocationId": location.get("id")}
                            if location.get("primary") == "true":
                                locationMap["primary"] = 1
                            else:
                                locationMap["primary"] = 0
                            for locationElem in location:
                                locationMap[locationElem.tag] = locationElem.text.strip()
                            locations.append(locationMap)

                if len(locations) != 0:
                    bug["BugLocations"] = locations
                if len(methods) != 0:
                    bug["Methods"] = methods
                if len(cweids) != 0:
                    bug["CweIds"] = cweids
                if "CallbackData" in callback:
                    ret = callback["BugCallback"](bug, callback["CallbackData"])
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret, callback["CallbackData"])
                        break
                else:
                    ret = callback["BugCallback"](bug)
                    if ret is not None:
                        if "FinalCallback" in callback:
                            ret = callback["FinalCallback"](ret)
                        break
                parent.clear()

            #parse metric
            elif elem.tag == "Metric" and "MetricCallback" in callback and event == "end":
                if not self.startValid:
                    print("Misformed SCARF File: No AnalyzerReport Tag before first element")
                self.validBody = 1
                metric = {"MetricId": elem.get("id")}

                for subelement in elem:
                    if subelement.tag == "Location":
                        metric["SourceFile"] = subelement[0].text.strip()
                    else:
                        metric[subelement.tag] = subelement.text.strip()
                if "CallbackData" in callback:
                    ret = callback["MetricCallback"](metric, callback["CallbackData"])
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret, callback["CallbackData"])
                        break
                else:
                    ret = callback["MetricCallback"](metric)
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret)
                        break
                parent.clear()

            #parse summaries
            elif elem.tag == "MetricSummaries" and "MetricSummaryCallback" in callback and event == "end":
                if not self.startValid:
                    print("Misformed SCARF File: No AnalyzerReport Tag before first element")
                summary = {}
                for metricSum in elem:
                    sum_hash = {}
                    for values in metricSum:
                        if values.tag != "Type":
                            sum_hash[values.tag] = values.text.strip()
                        else:
                            metricType = values.text.strip()
                    summary[metricType] = sum_hash
                if "CallbackData" in callback:
                    ret = callback["MetricSummaryCallback"](summary, callback["CallbackData"])
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret, callback["CallbackData"])
                        break
                else:
                    ret = callback["MetricSummaryCallback"](summary)
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret)
                        break
                parent.clear()

            elif elem.tag == "BugSummary" and "BugSummaryCallback" in callback and event == "end":
                if not self.startValid:
                    print("Misformed SCARF File: No AnalyzerReport Tag before first element")
                summary = {}
                for category in elem:
                    sum_hash = {"bytes": category.get("bytes"), "count": category.get("count")}
                    if category.get("code") not in summary:
                        summary[category.get("code")] = {}
                    summary[category.get("code")][category.get("group")] = sum_hash

                if "CallbackData" in callback:
                    ret = callback["BugSummaryCallback"](summary, callback["CallbackData"])
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret, callback["CallbackData"])
                        break
                else:
                    ret = callback["BugSummaryCallback"]()
                    if ret is not None:
                        if "FinalCallback" in callback:
                                ret = callback["FinalCallback"](ret)
                        break
                parent.clear()

            elif elem.tag == "AnalyzerReport" and event == "end":
                if "FinalCallback" in callback:
                    if "CallbackData" in callback:
                        ret = callback["FinalCallback"](ret, callback["CallbackData"])
                        if ret is not None:
                            break
                    else:
                        ret = callback["FinalCallback"](ret)
                        if ret is not None:
                            break
        if not self.validBody:
            print("No BugInstances or Metrics present")
        return ret

