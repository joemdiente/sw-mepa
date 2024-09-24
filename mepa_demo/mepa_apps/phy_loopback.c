// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "mesa-rpc.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "vtss_phy_10g_api.h"
#include "phy_demo_apps.h"
#include "vtss_private.h"
#include "mepa_driver.h"

#define LOOP_COUNT 12

static mscc_appl_trace_module_t trace_module = {
    .name = "loopback"
};


enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};

static mscc_appl_trace_group_t trace_groups[10] = {
    // TRACE_GROUP_DEFAULT
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

typedef struct {
    mepa_bool_t           file;
    char                  filename[30];
} loopback_cli_req_t;

meba_inst_t meba_lp_phy_inst;

static void cli_cmd_loopback_conf(cli_req_t *req)
{
    mepa_loopback_t conf;
    vtss_phy_10g_loopback_t curr_loopback, loopback;
    loopback_cli_req_t *mreq = req->module_req;
    struct json_object *jobj = NULL;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    int size = 0, count = 0;
    char loopback_config_file[128];
    char *buffer = NULL;
    demo_phy_info_t phy_family;
    struct mepa_device *dev = meba_lp_phy_inst->phy_devices[req->port_no];
    mepa_rc rc = MEPA_RC_OK;
    uint16_t loop_back;

    if(!req->set) {
        cli_printf("\n Syntax : loopback [<port_no>] [-f] [filename]\n");
        T_E("\n Provide Port no and filename as arguments\n");
        return;
    }
    if ((rc = phy_family_detect(meba_lp_phy_inst, req->port_no, &phy_family)) != MEPA_RC_OK)
    {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if(dev != NULL) {
        phy_data_t *data = (phy_data_t *)dev->data;
        snprintf(loopback_config_file, sizeof(loopback_config_file), "/root/mepa_scripts/%s", req->file_name);
        FILE *fp = fopen(loopback_config_file, "r");
        if(fp == NULL) {
            T_E("%s : file open error. Please provide Valid JSON file: loopback_config.json", req->file_name);
            return;
        }
        if(!(fseek(fp, 0, SEEK_END)) && ftell(fp)) {
            size = ftell(fp);
            buffer = (char*)malloc(size);
            if(buffer == NULL) {
                T_E("Fatal: failed to allocate %d bytes.\n", size);
                goto file_close;
            }
            fseek(fp, 0, SEEK_SET);
            if(!fread(buffer, 1, size, fp)) {
                T_E("File read error");
                goto file_close;
            }
            jobj = json_tokener_parse(buffer);
            if(jobj == NULL) {
                T_E("could not parse parms from file: %s", mreq->filename);
                goto file_close;
            }
            if((phy_family.family == PHY_FAMILY_INDY) || (phy_family.family == PHY_FAMILY_VIPER))
            {
                if(json_rpc_get_name_json_object(&json_req, jobj, "1g_loopback", &json_req.params) != MESA_RC_OK)
                {
                    T_E("object name not found in file: %s", mreq->filename);
                    goto file_close;
                }
                if(json_rpc_get_mepa_loopback_t(&json_req, json_req.params, &conf) != MESA_RC_OK)
                {
                    T_E("Error in the json configuration");
                    goto file_close;
                }
                loop_back = ((conf.far_end_ena << 0) | (conf.near_end_ena << 1) | (conf.connector_ena << 2) | (conf.mac_serdes_input_ena << 3) |
                             (conf.mac_serdes_facility_ena << 4) | (conf.mac_serdes_equip_ena << 5) | (conf.media_serdes_input_ena << 6) |
                             (conf.media_serdes_facility_ena << 7) | (conf.media_serdes_equip_ena << 8) | (conf.qsgmii_pcs_tbi_ena << 9) |
                             (conf.qsgmii_pcs_gmii_ena << 10) | (conf.qsgmii_serdes_ena << 11));
                for(int iter = 0; iter < LOOP_COUNT; iter++)
                {
                    if((loop_back >> iter) & 0x1)
                        count++;
                }
                if(count > 1)
                {
                    T_E("ERROR: More than one loopback got configured\n");
                    goto file_close;
                }
                if(meba_phy_loopback_set(meba_lp_phy_inst, req->port_no, &conf) != MESA_RC_OK)
                    T_E("Error in Configuring the LOOPBACK");
                goto file_close;

            } else if(phy_family.family == PHY_FAMILY_MALIBU_10G)
            {
                if(json_rpc_get_name_json_object(&json_req, jobj, "10g_loopback", &json_req.params) != MESA_RC_OK)
                {
                    T_E("object name not found in file: %s", mreq->filename);
                    goto file_close;
                }
                if(json_rpc_get_vtss_phy_10g_loopback_t(&json_req, json_req.params, &loopback) != MESA_RC_OK) {
                    T_E("Error in the json configuration");
                    goto file_close;
                }
                if(vtss_phy_10g_loopback_get(data->vtss_instance, req->port_no, &curr_loopback) != MESA_RC_OK) {
                    T_E("Error in reading the LOOPBACK configuration");
                    goto file_close;
                }
                curr_loopback.enable = 0;
                if(vtss_phy_10g_loopback_set(data->vtss_instance, req->port_no, &curr_loopback) != MESA_RC_OK) {
                    T_E("Error in disabling the current LOOPBACK");
                    goto file_close;
                }
                if(vtss_phy_10g_loopback_set(data->vtss_instance, req->port_no, &loopback) != MESA_RC_OK)
                    T_E("Error in Configuring the LOOPBACK");
                goto file_close;

            }
file_close:
            free(buffer);
            fclose(fp);
        }
    } else {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
    }
    return;
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "loopback <port_no> [-f] [filename]",
        "Configure loopback for particular port number",
        cli_cmd_loopback_conf
    },
};

static int phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
    return MESA_RC_OK;
}

void mscc_appl_phy_loopback_init(mscc_appl_init_t *init)
{
    meba_lp_phy_inst = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        if(phy_cli_init() != MEPA_RC_OK)
            T_E("Couldn't load loopback config module");
        break;
    default:
        break;
    }
}


