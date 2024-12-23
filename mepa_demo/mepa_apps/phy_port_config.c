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
#include "phy_port_config.h"
#include "phy_demo_apps.h"

static mepa_callout_t mepa_callout;
static mepa_board_conf_t board_conf = {};
static mepa_port_no_t port_cnt;
static mscc_appl_trace_module_t trace_module = {
    .name = "phy_port_config"
};

#define MALIBU_SPECIFIC_CHECK (0x8250)
#ifdef EDS2_SUPPORT_EN
#define COMA_MODE_GPIO_NUM 64
#else
#define COMA_MODE_GPIO_NUM 33
#endif

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
    mesa_port_interface_t interface;
    mepa_bool_t           file;
    char                  filename[30];
} port_cli_req_t;

meba_inst_t meba_phy_inst;

static int phy_assign_callout(void) {
    mepa_callout.mmd_read = meba_mmd_read;
    mepa_callout.mmd_read_inc = meba_mmd_read_inc;
    mepa_callout.mmd_write = meba_mmd_write;
    mepa_callout.miim_read = meba_miim_read;
    mepa_callout.miim_write = meba_miim_write;
    mepa_callout.lock_enter = meba_phy_inst->iface.lock_enter;
    mepa_callout.lock_exit = meba_phy_inst->iface.lock_exit;
    mepa_callout.mem_alloc = mem_alloc;
    mepa_callout.mem_free = mem_free;
    mepa_callout.spi_read = mepa_phy_spi_read;
    mepa_callout.spi_write = mepa_phy_spi_write;
    return MEPA_RC_OK;
}

static int mepa_drv_create(const mepa_port_no_t port_no) {
    cli_printf("\n creating dev\n");
    meba_port_entry_t   entry;
    mepa_rc rc;
    mepa_phy_info_t phy_info = {0};
    port_cnt = meba_phy_inst->api.meba_capability(meba_phy_inst, MEBA_CAP_BOARD_PORT_COUNT);
    if(port_cnt <= 0) {
        T_E("Invalid port number from meba\n");
        return MESA_RC_ERROR;
    }

    if(meba_phy_inst->phy_devices[port_no]) {
        T_E("Device Already existing on %d", port_no);
        return MESA_RC_ERROR;
    }

    meba_phy_inst->api.meba_reset(meba_phy_inst, MEBA_ENTRY_PHY_SET);
    meba_phy_inst->api.meba_port_entry_get(meba_phy_inst, port_no, &entry);

    meba_phy_inst->phy_device_ctx[port_no].inst = 0;
    meba_phy_inst->phy_device_ctx[port_no].port_no = port_no;
    meba_phy_inst->phy_device_ctx[port_no].meba_inst = meba_phy_inst;
    meba_phy_inst->phy_device_ctx[port_no].miim_controller = entry.map.miim_controller;
    meba_phy_inst->phy_device_ctx[port_no].miim_addr = entry.map.miim_addr;
    meba_phy_inst->phy_device_ctx[port_no].chip_no = entry.map.chip_no;
    board_conf.numeric_handle = port_no;

    /*Removing the vtss_instance_create approach to match with the default vtss instance created in sw-mepa application */

    meba_phy_inst->phy_devices[port_no] = mepa_create(&(mepa_callout),
                                          &meba_phy_inst->phy_device_ctx[port_no],
                                          &board_conf);

    if ((meba_phy_inst->phy_devices[port_no]) && (meba_phy_inst->phy_devices[entry.phy_base_port])) {
        T_I("Phy has been probed on port %d, MAC I/F = %d", port_no, entry.mac_if);
        if ((rc = mepa_link_base_port(meba_phy_inst->phy_devices[port_no],
                                      meba_phy_inst->phy_devices[entry.phy_base_port],
                                      entry.map.chip_port)) != MESA_RC_OK) {
            cli_printf(" Error in Linking base port to Port  : %d\n", port_no);
        }
    } else {
        T_E("Probe failed on %d", port_no);
        return MESA_RC_ERROR;
    }

    mepa_reset_param_t phy_reset = {};
    phy_reset.media_intf = MESA_PHY_MEDIA_IF_CU;
    phy_reset.reset_point = MEPA_RESET_POINT_PRE;
    if ((mepa_reset(meba_phy_inst->phy_devices[port_no], &phy_reset) != MESA_RC_OK)) {
        T_E("Pre reset failed %d", port_no);
        return MESA_RC_ERROR;
    }
    meba_phy_info_get(meba_phy_inst, port_no, &phy_info);
    phy_reset.media_intf = ((phy_info.part_number & 0xfff0) != MALIBU_SPECIFIC_CHECK)?MESA_PHY_MEDIA_IF_CU:MESA_PHY_MEDIA_IF_FI_10G_LAN;
    phy_reset.reset_point = MEPA_RESET_POINT_DEFAULT;
    if ((mepa_reset(meba_phy_inst->phy_devices[port_no], &phy_reset) != MESA_RC_OK)) {
        T_E("Default reset failed %d", port_no);
        return MESA_RC_ERROR;
    }
    phy_reset.reset_point = MEPA_RESET_POINT_POST;

    if ((mepa_reset(meba_phy_inst->phy_devices[port_no], &phy_reset)!= MESA_RC_OK)) {
        T_E("Post reset failed %d", port_no);
        return MESA_RC_ERROR;
    }

    cli_printf("Dev created for port_no %d , id is dec:(%d) : hex(%x)\n", port_no, phy_info.part_number, phy_info.part_number);
    return MESA_RC_OK;

}


static int mepa_drv_del(const mepa_port_no_t port_no) {
    cli_printf("\nDeleting dev port_no %u\n", port_no);
    if(meba_phy_inst->phy_devices[port_no]) {
        if(mepa_delete(meba_phy_inst->phy_devices[port_no]) != MEPA_RC_OK) {
            T_E("Unable to delete the mepa device %d", port_no);
            return MESA_RC_ERROR;
        }
        memset(&meba_phy_inst->phy_device_ctx[port_no], 0, sizeof(mepa_callout_ctx_t));
        meba_phy_inst->phy_devices[port_no] = NULL;
        board_conf.vtss_instance_ptr = NULL;
    }
    else {
        T_E("No Dev created for port_no %d\n", port_no);
        return MESA_RC_ERROR;
    }
    return MESA_RC_OK;
}

/*map phy to the chip port */
static void cli_cmd_dev_attach(cli_req_t *req) {
    mesa_port_conf_t      conf;
    port_cli_req_t *mreq = req->module_req;
    mepa_device_t *dev;
    dev = meba_phy_inst->phy_devices[req->port_no];
    if (mesa_port_conf_get(NULL, req->port_no, &conf) != MESA_RC_OK) {
        req->rc = -1;
        T_E("mesa_port_conf_get(%u) failed", req->port_no);
    } else
        req->rc = 0;
    conf.if_type = mreq->interface;
    if (mesa_port_conf_set(NULL, req->port_no, &conf) != MESA_RC_OK) {
        req->rc = -1;
        T_E("mesa_port_conf_set(%u) failed %d\n", req->port_no);
    } else
        req->rc = 0;
    if (mepa_if_set(dev, mreq->interface) != MESA_RC_OK) {
        T_E("mepa_if_set(%u) failed %d\n", req->port_no);
    } else
        req->rc = 0;

}

static void cli_cmd_dev_create(cli_req_t *req)
{
    if (mepa_drv_create(req->port_no) != MESA_RC_OK) {
        req->rc = -1;
        T_E("Error in mepa drv creation\n");
    }
    else
        req->rc = 0;

}

static void cli_cmd_phy_conf(cli_req_t *req)
{
    mepa_conf_t conf;
    port_cli_req_t *mreq = req->module_req;
    struct json_object *jobj;
    json_rpc_req_t json_req = {};
    json_req.ptr = json_req.buf;
    mepa_device_t *dev;
    int size;
    char port_config_file[100];
    char *buffer;
    dev = meba_phy_inst->phy_devices[req->port_no];
    if(dev != NULL) {
        snprintf(port_config_file, sizeof(port_config_file), "/root/mepa_scripts/%s", mreq->filename);
        FILE *fp = fopen(port_config_file, "r");
        if(fp == NULL) {
            T_E("%s : file open error", mreq->filename);
            return;
        }
        if(!(fseek(fp, 0, SEEK_END)) && ftell(fp)) {
            size = ftell(fp);
            buffer = (char*)malloc(size);
            if(buffer == NULL) {
                T_E("Fatal: failed to allocate %d bytes.\n", size);
                return;
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
            if(json_rpc_get_name_json_object(&json_req, jobj, "port_config", &json_req.params) != MESA_RC_OK) {
                T_E("port_config object name not found in file: %s", mreq->filename);
                goto file_close;
            }
            if(json_rpc_get_mepa_conf_t(&json_req, json_req.params, &conf) != MESA_RC_OK) {
                T_E("Error in the json configuration");
                goto file_close;
            }
            if(mepa_conf_set(dev, &conf) != MESA_RC_OK)
                T_E("Error in Configuring the PHY");
file_close:
            free(buffer);
            fclose(fp);
        }

    }

}

static void cli_cmd_dev_del(cli_req_t *req)
{
    if (mepa_drv_del(req->port_no) != MESA_RC_OK) {
        req->rc = -1;
        T_E("Error in mepa drv delete\n");
    }
    else
        req->rc = 0;

}

static void cli_cmd_coma_mode(cli_req_t *req)
{

    (void)mesa_gpio_direction_set(NULL, 0, COMA_MODE_GPIO_NUM, TRUE);
    if (mesa_gpio_write(NULL, 0, COMA_MODE_GPIO_NUM, req->enable) != MESA_RC_OK) {
        req->rc = -1;
        T_E("Error in coma mode enable/disable\n");
    }
    else
        req->rc = 0;

}

static void cli_cmd_fpp_set(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    if(meba_phy_inst->phy_devices[req->port_no] == NULL) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    if ((rc = phy_family_detect(meba_phy_inst, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if((phy_family.family != PHY_FAMILY_LAN8814) && (phy_family.family != PHY_FAMILY_MALIBU_25G)) {
        T_E("\n PHY on Port :%d doesn't support Frame Preemption\n", req->port_no);
        return;
    }
    if (mepa_framepreempt_set(meba_phy_inst->phy_devices[req->port_no], req->enable) != MEPA_RC_OK) {
        T_E("\n Error in configuration Frame Preemption on Port :%d \n", req->port_no);
        return;
    }
    return;
}

static void cli_cmd_fpp_get(cli_req_t *req)
{
    mepa_rc rc;
    demo_phy_info_t phy_family;
    mepa_bool_t val = 0;
    if(meba_phy_inst->phy_devices[req->port_no] == NULL) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    if ((rc = phy_family_detect(meba_phy_inst, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    if((phy_family.family != PHY_FAMILY_LAN8814) && (phy_family.family != PHY_FAMILY_MALIBU_25G)) {
        T_E("\n PHY on Port :%d doesn't support Frame Preemption\n", req->port_no);
        return;
    }
    if (mepa_framepreempt_get(meba_phy_inst->phy_devices[req->port_no], &val) != MEPA_RC_OK) {
        T_E("\n Error in Getting Frame Preemption Status on Port :%d \n", req->port_no);
        return;
    }
    cli_printf("\n Frame Preemption %s on Port : %d\n", val ? "Enabled" : "Disabled", req->port_no);
    return;
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "Dev Create [<port_no>]",
        "Create dev for the particular port number",
        cli_cmd_dev_create
    },
    {
        "Dev Del [<port_no>]",
        "del dev for the particular port number",
        cli_cmd_dev_del
    },
    {
        "Dev conf [<port_no>] [-f] [filename]",
        "Configure the phy from file",
        cli_cmd_phy_conf
    },
    /* With dev attach we need to only provide the support for mac <-> phy interface
       the speed and duplex configuration support is provided in the port module */
    {
        "Dev Attach [<port_no>] [qsgmii|sfi|sgmii]",
        "del dev for the particular port number",
        cli_cmd_dev_attach
    },
    {
        "coma_mode [enable|disable]",
        "Enable/Disable coma mode for the PHY",
        cli_cmd_coma_mode
    },
    {
        "fpp_set [<port_no>] [enable|disable]",
        "Enable/Disable Frame Preemption on PHY",
        cli_cmd_fpp_set
    },
    {
        "fpp_get [<port_no>]",
        "Get the Frame Preemption Status on PHY",
        cli_cmd_fpp_get
    },

};

static int cli_parm_keyword(cli_req_t *req)
{
    const char     *found;
    port_cli_req_t *mreq = req->module_req;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL) {
        /* Take the file input from the cli */
        if(mreq->file && !strcmp(req->stx,"[filename]")) {
            strcpy(mreq->filename,req->cmd);
            strcpy(req->file_name,req->cmd);
            return 0;
        }
        return 1;
    }
    if (!strncasecmp(found, "qsgmii", 6)) {
        mreq->interface = MESA_PORT_INTERFACE_QSGMII;
    }

    if (!strncasecmp(found, "sgmii", 5))
        mreq->interface = MESA_PORT_INTERFACE_SGMII;

    if (!strncasecmp(found, "sfi", 3))
        mreq->interface = MESA_PORT_INTERFACE_SFI;

    if (!strncasecmp(found, "-f", 2)) {
	req->file = 1;
        mreq->file = 1;
    }

    return 0;


}

static cli_parm_t cli_parm_table[] = {
    {
        "qsgmii|sfi|sgmii",
        "QSGMII      :  Quad serial gigabit media-independent interface (supports viper, LAN8814) \n"
        "SGMII       :  Serial gigabit media-independent interface (supports viper, LAN8814)\n"
        "SFI         :  SerDes Framer Interface (supports Malibu)\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    },

    {
        "1g_conf|10g_conf",
        "1g_conf      :  1g configuration for the PHY (supports viper, LAN8814...) \n"
        "10g_conf       :  10g configuration for the PHY (support malibu, venice..)\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    },

    {
        "-f",
        "file             :  file option \n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    },

    {
        "filename",
        "filename       :  filename)\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    },
};

static int phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
    /* Register parameters */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }
    return MESA_RC_OK;
}

void mscc_appl_phy_init(mscc_appl_init_t *init)
{
    meba_phy_inst = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        if(phy_cli_init() != MEPA_RC_OK)
            T_E("Couldn't load port config module");
        if(phy_assign_callout() != MEPA_RC_OK)
            T_E("Couldn't assign the callout");
        break;
    default:
        break;
    }
}
