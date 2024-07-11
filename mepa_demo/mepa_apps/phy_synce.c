// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <stdio.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "microchip/ethernet/phy/api/types.h"

#include "vtss_phy_10g_api.h"
#include "vtss_private.h"

#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "mepa_driver.h"

#define INDY_PHYID 0x8814
#define M10G_PHYID1 0x8256
#define M10G_PHYID2 0x8257
#define M10G_PHYID3 0x8258
#define VIPER_PHYID 8584
meba_inst_t meba_phy_instances;

static int cli_parm_clk_src (cli_req_t *req);
static int cli_parm_freq (cli_req_t *req);
static int cli_parm_clk_out (cli_req_t *req);
static void cli_cmd_synce_conf_set (cli_req_t *req);

static mscc_appl_trace_module_t trace_module = {
    .name = "phy_synce_config"
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

typedef enum {
    SYNCE_CLOCK_SRC_DISABLED,
    SYNCE_CLOCK_SRC_SERDES_MEDIA,
    SYNCE_CLOCK_SRC_COPPER_MEDIA,
    SYNCE_CLOCK_SRC_CLOCK_IN_1,
    SYNCE_CLOCK_SRC_CLOCK_IN_2,
} synce_clock_src_t;

typedef enum {
    SYNCE_CLOCK_DST_1 = 0,
    SYNCE_CLOCK_DST_2
} synce_clock_dst_t;

typedef enum {
    LAN8814_FREQ_25M,    /**< 25Mhz recovered clock */
    LAN8814_FREQ_31_25M, /**< 31.25Mhz receovered clock */
    LAN8814_FREQ_125M,   /**< 125Mhz recovered clock */
    PHY_10G_SCKOUT_156_25,  /**< 156,25 MHz */
    PHY_10G_SCKOUT_125_00,  /**< 125,00 MHz */
    PHY_10G_SCKOUT_INVALID, /**< Other values are not allowed*/
} freq_t;

typedef struct {
    synce_clock_src_t src; /**< clock source */
    synce_clock_dst_t dst; /**< recovered clock output A/B */
    freq_t            freq;/**< recovered clock out frequency */
} port_synce_conf_set_t;

static cli_cmd_t cli_cmd_table[] = {
    {
        "synce_conf_set <port_no> [disableclk|serdes|coppermedia|clk1|clk2] [25mhz|31.25mhz|125mhz|156.25mhz] [clkouta|clkoutb]",
        "syncE configuration set for particular port number",
        cli_cmd_synce_conf_set
    },
};

static cli_parm_t cli_parm_table[] = {
    {
        "disableclk|serdes|coppermedia|clk1|clk2",
        "DISABLECLK: Disable clock source\n"
        "SERDES: Serdes clock source\n"
        "COPPERMEDIA: Coppermedia clock source\n"
        "CLK1: clock source1\n"
        "CLK2: clock source2\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_clk_src
    },
    {
        "25mhz|31.25mhz|125mhz|156.25mhz",
        "25MHZ: 25mhz expected clock frequency\n"
        "31.25MHZ: 31.25mhz expected clock frequency\n"
        "125MHZ: 125mhz expected clock frequency\n"
        "156.25MHZ: 156.25mhz expected clock frequency\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_freq
    },
    {
        "clkouta|clkoutb",
        "clkoutA: Recovered clock output A\n"
        "clkoutB: Recovered clock output B\n",
        CLI_PARM_FLAG_SET,
        cli_parm_clk_out
    },
};
static void cli_cmd_synce_conf_set(cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    mepa_phy_info_t phy_info = {0};
    mepa_synce_clock_conf_t indy_synce_conf;
    vtss_phy_10g_sckout_conf_t phy10g_synce_conf;
    struct mepa_device *dev = meba_phy_instances->phy_devices[req->port_no];
    phy_data_t *data = (phy_data_t *)dev->data;

    if(!meba_phy_instances->phy_devices[req->port_no])
    {
        T_E("\n Not phy is connected on this port number\n");
        return;
    }
    if(!req->set)
    {
        cli_printf("\n Syntax : synce_conf_set [<port_no> <clk_src> <freq> <clk-out>] \n");
        T_E("\n Provide Paramter clock source/frequency/clock output\n");
        return;
    }
    meba_phy_info_get(meba_phy_instances, req->port_no, &phy_info);
    if((phy_info.part_number == INDY_PHYID) || (phy_info.part_number == VIPER_PHYID))
    {
        /* INDY PHY or VIPER PHY*/
        if(mreq->src > SYNCE_CLOCK_SRC_CLOCK_IN_2)
        {
            T_E("\n Provide valid clock source for the PHY \n");
            return;
        }
        if(mreq->freq > LAN8814_FREQ_125M)
        {
            T_E("\n Provide valid clock frequency for the PHY \n");
            return;
        }
        if(mreq->dst > SYNCE_CLOCK_DST_2)
        {
            T_E("\n Provide valid clock output for the PHY \n");
            return;
        }
        indy_synce_conf.src = mreq->src;
        indy_synce_conf.freq = mreq->freq;
        indy_synce_conf.dst = mreq->dst;
        if((meba_phy_synce_clock_conf_set(meba_phy_instances, req->port_no, &indy_synce_conf)) != MESA_RC_OK)
        {
            T_E("\nINDY: Error setting Synce configuration \n");
            return;
        }
    } else if((phy_info.part_number == M10G_PHYID1) || (phy_info.part_number == M10G_PHYID2) || (phy_info.part_number == M10G_PHYID3)) {
        /* MALIBU10G PHY*/
        switch(mreq->src)
        {
        case SYNCE_CLOCK_SRC_DISABLED:
            phy10g_synce_conf.mode = VTSS_PHY_10G_SYNC_DISABLE;
            phy10g_synce_conf.enable = FALSE;
            break;
        case SYNCE_CLOCK_SRC_SERDES_MEDIA:
            phy10g_synce_conf.mode = VTSS_PHY_10G_LINE0_RECVRD_CLOCK;
            break;
        case SYNCE_CLOCK_SRC_COPPER_MEDIA:
            T_E("\n Copper media not supported for the PHY \n");
            return;
        case SYNCE_CLOCK_SRC_CLOCK_IN_1:
            phy10g_synce_conf.mode = VTSS_PHY_10G_SREFCLK;
            break;
        case SYNCE_CLOCK_SRC_CLOCK_IN_2:
            T_E("\n Not supported for the PHY \n");
            return;
        default:
            T_E("\n Provide valid clock source for the PHY \n");
            return;
        }
        if(mreq->freq == LAN8814_FREQ_125M)
        {
            mreq->freq = PHY_10G_SCKOUT_125_00;
        }
        if(mreq->freq > PHY_10G_SCKOUT_125_00)
        {
            T_E("\n Provide valid clock frequency for the PHY \n");
            return;
        }
        if(mreq->dst != SYNCE_CLOCK_DST_1)
        {
            T_E("\n Provide valid clock output for the PHY \n");
            return;
        }
        phy10g_synce_conf.freq = mreq->freq - 3;
        phy10g_synce_conf.src = VTSS_CKOUT_NO_SQUELCH;
        if(mreq->src != SYNCE_CLOCK_SRC_DISABLED)
        {
            phy10g_synce_conf.enable = TRUE;
        }
        phy10g_synce_conf.squelch_inv = FALSE;
        if((vtss_phy_10g_sckout_conf_set(data->vtss_instance, req->port_no, &phy10g_synce_conf)) != MESA_RC_OK)
        {
            T_E("\nMALIBU10G: Error setting Synce configuration \n");
            return;
        }
    } else {
        T_E("SyncE not supported for this PHY\n");
        return;
    }
    cli_printf("SyncE configured for the PHY\n");
}

static int cli_parm_clk_src (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    if (!strncasecmp(found, "disableclk", 10))
    {
        mreq->src = SYNCE_CLOCK_SRC_DISABLED;
    } else if (!strncasecmp(found, "serdes", 6))
    {
        mreq->src = SYNCE_CLOCK_SRC_SERDES_MEDIA;
    } else if (!strncasecmp(found, "coppermedia", 11))
    {
        mreq->src = SYNCE_CLOCK_SRC_COPPER_MEDIA;
    } else if (!strncasecmp(found, "clk1", 4))
    {
        mreq->src = SYNCE_CLOCK_SRC_CLOCK_IN_1;
    } else if (!strncasecmp(found, "clk2", 4))
    {
        mreq->src = SYNCE_CLOCK_SRC_CLOCK_IN_2;
    } else
    {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static int cli_parm_freq (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    if (!strncasecmp(found, "25mhz", 5))
    {
        mreq->freq = LAN8814_FREQ_25M;
    } else if (!strncasecmp(found, "31.25mhz", 8))
    {
        mreq->freq = LAN8814_FREQ_31_25M;
    } else if (!strncasecmp(found, "125mhz", 6))
    {
        mreq->freq = LAN8814_FREQ_125M;
    } else if (!strncasecmp(found, "156.25mhz", 9))
    {
        mreq->freq = PHY_10G_SCKOUT_156_25;
    } else
    {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static int cli_parm_clk_out (cli_req_t *req)
{
    port_synce_conf_set_t *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    if (!strncasecmp(found, "clkouta", 7))
    {
        mreq->dst = SYNCE_CLOCK_DST_1;
    } else if (!strncasecmp(found, "clkoutb", 7))
    {
        mreq->dst = SYNCE_CLOCK_DST_2;
    } else
    {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static void phy_cli_init(void)
{
    int i;

    /* Register commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++)
    {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }
    /* Register parameters */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++)
    {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mscc_appl_phy_synce(mscc_appl_init_t *init)
{
    meba_phy_instances = init->board_inst;
    switch (init->cmd)
    {
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
