# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

#!/usr/bin/python

import sys
import json
import os, subprocess
from collections import OrderedDict
import optparse
port_count = 20

class PortConfig:

    def __init__(self,port_no):
        self.port_no = port_no

    def usage(self):
        st = """
               {}
                               Port Config Application

                              Given port number is : {}



               Port config Application is done with the mepa-cmd utility
               the proceedure is demonstarted below, run this script after
               executing mepa-demo-edsx

               1. # mepa-cmd dev create {}
               This will create a dev for the the port_no {}


               2. # mepa-cmd dev attach {} [qsgmii| sfi | sgmii]
               This will attach the dev to the serdes ports of EDS X
               if the platform is not EDSX the error will be displayed here


               This is an automated step and the steps should be followed in the same
               order, when used with mepa-cmd
               {}
              """.format('-'*80, self.port_no, self.port_no, self.port_no, self.port_no, '-'*80)
        print (st)

    def mepa_cmd(self, cmd):
        return os.system("{} 2>&1 > /dev/null".format(cmd))

    def mepa_cmd_json(self, string):
        print(string)
        return os.popen(string).read()

    def mepa_cmd_json_api(self, json_str, api):
        call_api = "echo [{},{}{}] | mepa-cmd -i call {}".format(self.port_no, json_str, "}", api)
        print("Command = > {}\n\n".format(call_api))
        return os.system("{} 2>&1 > /dev/null".format(call_api))

def main():
    conf_set = open("port_config.json", "r")
    string = conf_set.read()
    string = string.readlines()[3:]
    string = string.split("}\n")
    print (string[0])
    parser = optparse.OptionParser()

    parser.add_option('-p', dest = 'port',
                      type = 'int',
                      help = 'specify the port number')
    parser.add_option('-i', dest = 'interface',
                      type = 'string',
                      help = 'specify the interface')

    (options, args) = parser.parse_args()

    if (options.port == None):
            print (parser.usage)
            exit(0)
    else:
            port_no = options.port


    Pc = PortConfig(port_no)
    Pc.usage()

    #step 1
    port_no = Pc.port_no
    rc = Pc.mepa_cmd("mepa-cmd dev create {}".format(port_no));
    if (int(rc) != 0):
        print ("Error in creating dev")
        exit(0)

    result = Pc.mepa_cmd_json("echo [{}] | mepa-cmd -i call meba_phy_info_get".format(port_no - 1))
    string = json.loads(result)
    print( " Dev Created for device : {}" .format(string["result"][1]["part_number"]))


    #step 2

    if (options.interface == None):
         interface = "sfi"
    else:
         interface = options.interface

    rc = Pc.mepa_cmd("mepa-cmd dev attach {} {}".format(port_no, interface));
    if (int(rc) != 0):
        print ("Error in attaching dev to the switch")
        exit(0)

    #step 3
    conf_set = open("port_config.json", "r")
    string = conf_set.read()
    string = string.strip("# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries. \
                           # SPDX-License-Identifier: MIT")
    string = string.split("}\n")
    for i in range(len(string) - 1):
        cmd = string[i].split("=>")
        api = cmd[0].replace("\n", "")
        json_str = cmd[1]
        json_str = json_str.replace("\n", "")
        json_str = json_str.replace(" ", "")
        json_call = Pc.mepa_cmd_json_api(json_str, api)

if __name__ == "__main__":
    main()


