// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include "phy_demo_apps.h"
#include "phy_gpio_demo.h"

meba_inst_t meba_gpio_lp_instance;

static mscc_appl_trace_module_t trace_module = {
    .name = "gpio_lp_app"
};

enum {
    TRACE_GROUP_DEFAULT,
    TRACE_GROUP_CNT
};


static mscc_appl_trace_group_t trace_groups[10] = {
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

char *gpio_txt[] = {"Output", "Input", "Alternate"};

static int phy_gpio_no_for_alternate_function(phy_family_t family, int alt_fun, gpio_table_t *gpio_table) 
{
    if(family == PHY_FAMILY_VIPER) {
        memcpy(gpio_table, &viper[alt_fun], sizeof(gpio_table_t));
        return viper[alt_fun].gpio_no;
    } if(family == PHY_FAMILY_INDY) {
        memcpy(gpio_table, &indy[alt_fun], sizeof(gpio_table_t));
        return indy[alt_fun].gpio_no;
    }
    return 0;
}

static void cli_cmd_gpio_conf(cli_req_t *req)
{
    mepa_rc rc;
    mepa_gpio_conf_t gpio_conf;
    demo_phy_info_t phy_family;
    int gpio_mode = 0, alt_mode = 0, led_number = 0, alt_fun = 0;
    uint8_t gpio_num = 0, max_alter_fun = 0;
    int vsc_phy_connected = 0;
    gpio_table_t gpio_table;

    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    if ((rc = phy_family_detect(meba_gpio_lp_instance, req->port_no, &phy_family)) != MEPA_RC_OK) {
        T_E("\n Error in Detecting PHY Family on Port %d\n", req->port_no);
        return;
    }
    T_I("\n Detected PHY Family on port %d is : %d\n", req->port_no, phy_family.family);
    if((phy_family.family == PHY_FAMILY_VIPER) || (phy_family.family == PHY_FAMILY_TESLA) || (phy_family.family == PHY_FAMILY_INDY)) {
        memset(&gpio_conf, 0, sizeof(mepa_gpio_conf_t));
        cli_printf("\n");
        cli_printf("\t 1 . Output Mode\n");
        cli_printf("\t 2 . Input Mode\n");
        cli_printf("\t 3 . Alternate Mode\n");

        /* In case of Viper and Tesla PHY the Port LED Confguration is not the alternate
         * function of GPIO PIN, it has seperate Register for LED Configuration, provided
         * sperate Mode to configure LED in case of Viper and Tesla PHYs
         */
        if(phy_family.family != PHY_FAMILY_INDY) {
            cli_printf("\t 4 . LED Configuration\n");
            vsc_phy_connected = 1;
        }
        
        cli_printf("\t Enter the Mode of GPIO Pin : ");
        scanf("%d", &gpio_mode);

        if(gpio_mode == 1) {
            gpio_conf.mode = MEPA_GPIO_MODE_OUT;
        } else if(gpio_mode == 2) {
            gpio_conf.mode = MEPA_GPIO_MODE_IN;
        } else if(gpio_mode == 3) {
            gpio_conf.mode = MEPA_GPIO_MODE_ALT;
        } else if((gpio_mode == 4) && vsc_phy_connected) {
            gpio_conf.mode = MEPA_GPIO_MODE_ALT;
        } else {
            T_E("\n Invalid Mode Selected \n");
            return;
        }

        /* Only for Input and Output Mode GPIO Number is required from User for Alternate Function
         * based on the functionality slected by User GPIO PIN is Automatically Assigned */
        if(gpio_conf.mode  == MEPA_GPIO_MODE_OUT || gpio_conf.mode  == MEPA_GPIO_MODE_IN) {
            cli_printf("\n\t Enter GPIO Number to be configured as %s mode: ", gpio_txt[gpio_mode - 1]);
            scanf("%hhu", &gpio_num);
            gpio_conf.gpio_no = gpio_num;
        }

        if(gpio_mode == 3) {
            cli_printf("\n");
            switch(phy_family.family) {
            case PHY_FAMILY_VIPER:
                cli_printf("\t 0 . Signal Detect 0\n");
                cli_printf("\t 1 . Signal Detect 1\n");
                cli_printf("\t 2 . Signal Detect 2\n");
                cli_printf("\t 3 . Siganl Detect 3\n");
                cli_printf("\t 4 . I2C SCL 0\n");
                cli_printf("\t 5 . I2C SCL 1\n");
                cli_printf("\t 6 . I2C SCL 2\n");
                cli_printf("\t 7 . I2C SCL 3\n");
                cli_printf("\t 8 . I2C SDA\n");
                cli_printf("\t 9 . Fast Link Fail\n");
                cli_printf("\t 10. 1588 load Save\n");
                cli_printf("\t 11. 1588 PPS 0\n");
                cli_printf("\t 12. 1588 SPI CS\n");
                cli_printf("\t 13. 1588 SPI DO\n");
                max_alter_fun = MAX_SUPPORTED_ALT_FUN_VIPER;
                break;
            case PHY_FAMILY_INDY:
                cli_printf("\t 0 . 1588 Event A\n");
                cli_printf("\t 1 . 1588 Event B\n");
                cli_printf("\t 2 . 1588 Ref Clk\n");
                cli_printf("\t 3 . 1588 LD Adj\n");
                cli_printf("\t 4 . 1588 STI CS\n");
                cli_printf("\t 5 . 1588 STI CLK\n");
                cli_printf("\t 6 . 1588 STI DO\n");
                cli_printf("\t 7 . RCVRD CLK IN 1\n");
                cli_printf("\t 8 . RCVRD CLK IN 2\n");
                cli_printf("\t 9 . RCVRD CLK OUT 1\n");
                cli_printf("\t 10. RCVRD CLK OUT 2\n");
                cli_printf("\t 11. SOF 0\n");
                cli_printf("\t 12. SOF 1\n");
                cli_printf("\t 13. SOF 2\n");
                cli_printf("\t 14. SOF 3\n");
                cli_printf("\t 15. Port LED Configuration\n");
                max_alter_fun = MAX_SUPPORTED_ALT_FUN_INDY;
                break;
            default:
                break;
            }

            cli_printf("\n\t Select Alternte Function of GPIO : ");
            scanf("%d", &alt_fun);
            if(alt_fun >=  max_alter_fun) {
                T_E("\n Alternate Function Not Supported \n");
                return;
            }
            gpio_conf.gpio_no = phy_gpio_no_for_alternate_function(phy_family.family, alt_fun, &gpio_table);
            cli_printf("\n\t Selected %s Alternate Mode for GPIO %d PIN\n", gpio_table.desc, gpio_conf.gpio_no);
        }
        /* In Case of INDY PHY, Port LED alternate function the GPIO Number is selected inside the API based
         * on the Channel id of the PHY */
        if(gpio_mode == 4 || (!vsc_phy_connected && (alt_fun == 15))) {
            cli_printf("\n");
            cli_printf("\t 1. LED Link Activity, any speed link\n");
            cli_printf("\t 2. LED 1000 Mbps Link Activity\n");
            cli_printf("\t 3. LED 100 Mbps Link Activity\n");
            cli_printf("\t 4. LED 10 Mbps Link Activity\n");
            cli_printf("\t 5. LED 100/1000 Mbps Link Activity\n");
            cli_printf("\t 6. LED 10/1000 Mbps Link Activity\n");
            cli_printf("\t 7. LED 100Base-Fx/1000Base-X Link Activity\n");
            cli_printf("\t 8. LED Full Duplex and Collison Detect\n");
            cli_printf("\t 9. LED Collision Detect\n");
            cli_printf("\t 10.LED Tx/Rx Activity\n");
            cli_printf("\t 11.LED 100Base-Fx/1000Base-X Fiber Activity Detect\n");
            cli_printf("\t 12.LED ANEG Fault/Paralled Detect Fault/Paralled\n");
            cli_printf("\t 13.LED 1000 Base-X Link Activity\n");
            cli_printf("\t 14.LED 100 Base-Fx Link Activity\n");
            cli_printf("\t 15.LED 1000 base Activity\n");
            cli_printf("\t 16.LED 100 Base-Fx activity\n");
            cli_printf("\t 17.Force LED OFF\n");
            cli_printf("\t 18.Force LED On\n");
            cli_printf("\t 19.LED Fast link Fail\n");
            cli_printf("\t 20.LED Link Tx\n");
            cli_printf("\t 21.LED Link Rx\n");
            cli_printf("\t 22.LED Link Fault\n");
            cli_printf("\t 23.LED Link No Activity and Any Speed\n");
            cli_printf("\t 24.LED local Rx Error Status\n");
            cli_printf("\t 25.LED remote Rx Error Status\n");
            cli_printf("\t 26.LED Negotiated Speed\n");
            cli_printf("\t 27.LED Master Slave Mode\n");
            cli_printf("\t 28.LED PCS Tx Error Status\n");
            cli_printf("\t 29.LED PCS Rx Error Status\n");
            cli_printf("\t 30.LED PCS Tx Activity\n");
            cli_printf("\t 31.LED PCS Rx Activity\n");
            cli_printf("\t 32.LED Wake on LAN\n");
            cli_printf("\n\t Select One LED Configuration : ");
            scanf("%d", &alt_mode);
            if(alt_mode == 0 || alt_mode > 32) {
                 T_E("\n Enter Valid LED Configuration \n");
                 return;
	    } 
            gpio_conf.mode = MEPA_GPIO_MODE_ALT + alt_mode; /* Alternate Mode Selected */
            cli_printf("\n");
            if(phy_family.family != PHY_FAMILY_INDY) {
                cli_printf("\t Enter the LED needs to be configured [0 - LED0 or 1 - LED1] : ");
	    } else {
                cli_printf("\t Enter the LED needs to be configured [0 - LED1 or 1 - LED2] : ");
            }
            scanf("%d", &led_number);
            gpio_conf.led_num = (led_number == 1) ? 1 : 0;
        }


        if ((rc = mepa_gpio_mode_set(meba_gpio_lp_instance->phy_devices[req->port_no], &gpio_conf)) != MEPA_RC_OK) {
            cli_printf(" Error in configuring GPIO : %d\n", req->port_no);
            return;
        }
    } else {
        T_E("\n PHY Not Supported  to be implemented..............\n");
    }
    return;
}

static void cli_cmd_gpio_out_set(cli_req_t *req)
{
    mepa_rc rc;
    gpio_configuration *mreq = req->module_req;
    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_out_set(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, req->enable)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Write : %d\n", req->port_no);
        return;
    }
    return;
}

static void cli_cmd_gpio_in_get(cli_req_t *req)
{
    mepa_rc rc;
    gpio_configuration *mreq = req->module_req;;
    mepa_bool_t value;
    if ((rc = mepa_dev_create_check(meba_gpio_lp_instance, req->port_no)) != MEPA_RC_OK) {
        cli_printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }
    if ((rc = mepa_gpio_in_get(meba_gpio_lp_instance->phy_devices[req->port_no], mreq->gpio_number, &value)) != MEPA_RC_OK) {
        cli_printf(" Error in GPIO Read : %d\n", req->port_no);
        return;
    }
    cli_printf("\n Value of GPIO %d is : %d\n", mreq->gpio_number, value);
    return;
}


static int cli_param_parse_gpio_no(cli_req_t *req)
{
    gpio_configuration *mreq = req->module_req;
    return cli_parm_u8(req, &mreq->gpio_number, 0, 0x28);
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "gpio_conf <port_no>",
        "GPIO Configuration",
        cli_cmd_gpio_conf,
    },

    {
        "gpio_out_set <port_no> <gpio_num> [enable|disable]",
        "GPIO Output Pin Set",
        cli_cmd_gpio_out_set,
    },

    {
        "gpio_in_get <port_no> <gpio_num>",
        "GPIO Value Get",
        cli_cmd_gpio_in_get,
    },
};


static cli_parm_t cli_parm_table[] = {
    {
        "<gpio_num>",
        "GPIO Number",
        CLI_PARM_FLAG_NONE,
        cli_param_parse_gpio_no,
    }
};

static void phy_cli_init(void)
{
    int i;

    /* Register CLI Commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

    /* Register CLI Params */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mepa_demo_appl_gpio_lp_demo(mscc_appl_init_t *init)
{
    meba_gpio_lp_instance = init->board_inst;
    switch(init->cmd) {
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
