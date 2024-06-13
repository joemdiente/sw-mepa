// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "phy_port_config.h"

static mepa_callout_t mepa_callout;
static mepa_board_conf_t board_conf = {};
static mepa_callout_ctx_t callout_ctx ;

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_port_config"
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
    mesa_port_interface_t interface;
} port_cli_req_t;

meba_inst_t meba_phy_inst;


static int mepa_drv_create(const mepa_port_no_t port_no) {
    cli_printf("\n creating dev\n");
    meba_port_entry_t   entry;
    mepa_phy_info_t phy_info = {0};

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

    meba_phy_inst->api.meba_reset(meba_phy_inst, MEBA_ENTRY_PHY_SET);
    meba_phy_inst->api.meba_port_entry_get(meba_phy_inst, port_no, &entry);

    callout_ctx.inst = 0;
    callout_ctx.port_no = port_no;
    callout_ctx.meba_inst = meba_phy_inst;
    callout_ctx.miim_controller = entry.map.miim_controller;
    callout_ctx.miim_addr = entry.map.miim_addr;
    callout_ctx.chip_no = entry.map.chip_no;

    board_conf.numeric_handle = port_no;
    board_conf.vtss_instance_create = (board_conf.vtss_instance_ptr == NULL)?1:0;
    board_conf.vtss_instance_use = (board_conf.vtss_instance_ptr == NULL)?0:1;

    mepa_device_t *dev = mepa_create(&(mepa_callout),
                                     &(callout_ctx),
                                     &board_conf);

    if(meba_phy_inst->phy_devices[port_no]) {
        T_E("Device Already existing on %d", port_no);
        return MESA_RC_ERROR;
    }
    if(dev != NULL) {
        meba_phy_inst->phy_devices[port_no] = dev;
        meba_phy_inst->phy_device_ctx[port_no] = callout_ctx;
        T_I("Phy has been probed on port %d, MAC I/F = %d", port_no, entry.mac_if);
    } else {
        T_E("Probe failed on %d", port_no);
        return MESA_RC_ERROR;
    }

    mepa_reset_param_t phy_reset = {};
    phy_reset.reset_point = MEPA_RESET_POINT_DEFAULT;
    phy_reset.media_intf = MESA_PHY_MEDIA_IF_CU;
    (mepa_reset(meba_phy_inst->phy_devices[port_no], &phy_reset));
    meba_phy_info_get(meba_phy_inst, port_no, &phy_info);
    cli_printf("Dev created for port_no %d , id is dec:(%d) : hex(%x)\n", port_no, phy_info.part_number, phy_info.part_number);
    return MESA_RC_OK;
}



static int mepa_drv_del(const mepa_port_no_t port_no) {
     cli_printf("\nDeleting dev port_no %u\n", port_no);
     if(meba_phy_inst->phy_devices[port_no]) {
         mepa_delete(meba_phy_inst->phy_devices[port_no]);
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

static void cli_cmd_dev_del(cli_req_t *req)
{
    if (mepa_drv_del(req->port_no) != MESA_RC_OK) {
       req->rc = -1;
       T_E("Error in mepa drv delete\n");
    }
    else
        req->rc = 0;

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
    /* With dev attach we need to only provide the support for mac <-> phy interface
       the speed and duplex configuration support is provided in the port module */
    {
        "Dev Attach [<port_no>] [qsgmii|sfi|sgmii]",
        "del dev for the particular port number",
        cli_cmd_dev_attach
    },
};

static int cli_parm_keyword(cli_req_t *req)
{
    const char     *found;
    port_cli_req_t *mreq = req->module_req;

    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;


    if (!strncasecmp(found, "qsgmii", 6)) {
        mreq->interface = MESA_PORT_INTERFACE_QSGMII;
    }

    if (!strncasecmp(found, "sgmii", 5))
        mreq->interface = MESA_PORT_INTERFACE_SGMII;

    if (!strncasecmp(found, "sfi", 3))
        mreq->interface = MESA_PORT_INTERFACE_SFI;

    return 0;


}

static cli_parm_t cli_parm_table[] = {
    {
        "qsgmii|sfi|sgmii",
        "QSGMII      :  Quad serial gigabit media-independent interface (supports viper, indy) \n"
        "SGMII       :  Serial gigabit media-independent interface (supports viper, indy)\n"
        "SFI         :  SerDes Framer Interface (supports Malibu)\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_keyword
    },
};

static void phy_cli_init(void)
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

}

void mscc_appl_phy_init(mscc_appl_init_t *init)
{
    meba_phy_inst = init->board_inst;
    switch (init->cmd) {
    case MSCC_INIT_CMD_REG:
        mscc_appl_trace_register(&trace_module, trace_groups, TRACE_GROUP_CNT);
        break;

    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;

    default:
        break;
    }
}