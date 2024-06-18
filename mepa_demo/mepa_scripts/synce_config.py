# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

#!/usr/bin/python

import sys
import json
import os, subprocess
from collections import OrderedDict
import optparse
port_count = 20

class SyncEConfig:

    def __init__(self,port_no):
        self.port_no = port_no

    def usage(self):
          print("""
                               SyncE Application

               Synce Application is done with the mepa-cmd utility
               the proceedure is demonstarted below, run this script 
               after executing mepa-demo-edsx
            
               # python synce_config.py -p <port_no>

               This will configure do the synce configuration for the
               specific PHY

               """)
    
    def mepa_cmd_json(self, string):
        print(string)                                
        return os.popen(string).read()

    def mepa_cmd_json_api(self, json_str, api):
        call_api = "echo \"[{},{}]\" | mepa-cmd -i call {}".format(self.port_no, json_str, api)
        print("Command = > {}\n\n".format(call_api))
        return os.system("{} 2>&1 > /dev/null".format(call_api))

       
def main():
    parser = optparse.OptionParser()
    parser.add_option('-p', dest = 'port', type = 'int', help = 'specify the port number')
    (options, args) = parser.parse_args()
    if (options.port == None):
        print (parser.usage)
        exit(0)
    else:
        port_no = options.port
        Pc = SyncEConfig(port_no)
        Pc.usage()
        result = Pc.mepa_cmd_json("echo [{}] | mepa-cmd -i call meba_phy_info_get".format(port_no - 1))
        string = json.loads(result)
        if "result" not in string:
            print("Invalid port number");
            exit(0)
        else:
            part_no = string["result"][1]["part_number"]
            if ((part_no == 0x8814) or (part_no == 8584)):
                conf_set = open("lan8814_synce_config.json", "r")
            elif ((part_no == 0x8256) or (part_no == 0x8257) or (part_no == 0x8258)):
                conf_set = open("phy10g_synce_config.json", "r")
            else:
                print("synce support not available on this PHY")
            string = conf_set.read()
            string = string.strip("""Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
                                # SPDX-License-Identifier: MIT""")
            cmd = string.split("=>")
            api = cmd[0].replace("\n", "")
            json_str = cmd[1]
            json_str = json_str.replace("\n", "")
            json_str = json_str.replace(" ", "")
            json_call = Pc.mepa_cmd_json_api(json_str, api)

if __name__ == "__main__":
    main()


