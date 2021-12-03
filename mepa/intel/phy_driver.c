/*
 Copyright (c) 2004-2021 Microsemi Corporation "Microsemi".

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <microchip/ethernet/phy/api.h>
#include <mepa_driver.h>
#include <mepa_ts_driver.h>

#include "os.h"
#include "gpy211.h"
#include "gpy211_common.h"
#include "registers/phy/std.h"
#include "registers/phy/phy.h"
#include "registers/phy/pmapmd.h"
#include "registers/phy/pcs.h"
#include "registers/phy/aneg.h"
#include "registers/phy/vspec2.h"

#define T_D(format, ...) MEPA_trace(MEPA_TRACE_GRP_GEN, MEPA_TRACE_LVL_DEBUG, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_I(format, ...) MEPA_trace(MEPA_TRACE_GRP_GEN, MEPA_TRACE_LVL_INFO, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_W(format, ...) MEPA_trace(MEPA_TRACE_GRP_GEN, MEPA_TRACE_LVL_WARNING, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_E(format, ...) MEPA_trace(MEPA_TRACE_GRP_GEN, MEPA_TRACE_LVL_ERROR, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);


#define TRUE 1
#define FALSE 0

#define INTL_PHY_CHIPID 0x67c9dc00

typedef struct Intl_Port {
    mepa_device_t *dev;
} Intl_Port_t;

typedef struct{
    struct gpy211_device initconf;
    struct Intl_Port port_param;
} INTL_priv_data_t;

#define GPY211_DEVICE(dev) (&(((INTL_priv_data_t *)dev->data)->initconf))
#define INTL_PORT(dev) (&(((INTL_priv_data_t *)dev->data)->port_param))
#define PRIV_DATA(dev) ((INTL_priv_data_t *)dev->data)

static int (mdiobus_read)(void *mdiobus_data, u16 addr, u32 regnum)
{
    mepa_device_t *dev = ((Intl_Port_t *)mdiobus_data)->dev;
    u16 value;
    bool mmd_access = false;
    uint8_t devtype, regaddr;

    if (regnum | MII_ADDR_C45) {
        mmd_access=true;
        devtype = regnum >>16 & 0x1f;
        regaddr = regnum & 0xffff;
    }

    if (mmd_access) {
        if (dev->callout->mmd_read(dev->callout_cxt, devtype, regaddr, &value) == MEPA_RC_OK) {
            return value;
        }
    } else {
        if (dev->callout->miim_read(dev->callout_cxt, regnum, &value) == MEPA_RC_OK) {
            return value;
        }
    }

    return -1;
}

static int (mdiobus_write)(void *mdiobus_data, u16 addr, u32 regnum, u16 val)
{
    mepa_device_t *dev = ((Intl_Port_t *)mdiobus_data)->dev;
    bool mmd_access = false;
    uint8_t devtype, regaddr;

    if (regnum | MII_ADDR_C45) {
        mmd_access=true;
        devtype = regnum >>16 & 0x1f;
        regaddr = regnum & 0xffff;
    }

    if (mmd_access) {
        if (dev->callout->mmd_write(dev->callout_cxt, devtype, regaddr, val) == MEPA_RC_OK) {
            return 0;
        }
    } else {
        if (dev->callout->miim_write(dev->callout_cxt, regnum, val) == MEPA_RC_OK) {
            return 0;
        }
    }

    return -1;
}

static mesa_rc intl_if_get(mepa_device_t *dev, mesa_port_speed_t speed,
                          mesa_port_interface_t *mac_if)
{
    *mac_if = MESA_PORT_INTERFACE_SGMII_2G5;

    return MEPA_RC_OK;
}

void intl_phy_sgmii_conf(mepa_device_t *dev, mepa_status_t *status)
{
    uint16_t reg_val = 0;

    reg_val |= (status->fdx ? 1 : 0) << 8;
    switch(status->speed){
        case(MESA_SPEED_2500M): reg_val |= (1 << 13 | 1 << 6); break;
        case(MESA_SPEED_1G): reg_val |= (0 << 13 | 1 << 6); break;
        case(MESA_SPEED_100M): reg_val |= (1 << 13 | 0 << 6); break;
        case(MESA_SPEED_10M): reg_val |= (0 << 13 | 0 << 6); break;
        default:
            return;
    }
    reg_val |= 1 << 1;
    dev->callout->mmd_write(dev->callout_cxt, 0x1e, 0x8, reg_val);
}

static mesa_rc intl_poll(mepa_device_t *dev, mepa_status_t *status)
{
    struct gpy211_device *intel_status = GPY211_DEVICE(dev);
    struct Intl_Port *port_param = INTL_PORT(dev);
    INTL_priv_data_t *data = PRIV_DATA(dev);
    bool link_change = false;

    if (gpy2xx_update_link(intel_status) < 0) 
        return MESA_RC_ERROR;
    if (gpy2xx_read_status(intel_status) < 0)
        return MESA_RC_ERROR;
    if (status->link != intel_status->link.link) 
        link_change = true;
    status->link = intel_status->link.link;
    if (status->link) {
        switch(intel_status->link.speed) {
            case (SPEED_1000):
                status->speed = MESA_SPEED_1G;
                break;
            case (SPEED_100):
                status->speed = MESA_SPEED_100M;
                break;
            case (SPEED_10):
                status->speed = MESA_SPEED_10M;
                break;
            case (SPEED_2500):
                status->speed = MESA_SPEED_2500M;
                break;
            default:
                T_E("not expected speed");
                break;
        }
    }
    status->fdx = intel_status->link.duplex;
    status->aneg.obey_pause = intel_status->link.pause;
    status->aneg.generate_pause = intel_status->link.pause;
    status->copper = TRUE;
    status->fiber = FALSE;
    T_D("intl_phy_status_get: link %d, speed %d  duplex %d\n", intel_status->link.link, intel_status->link.speed, intel_status->link.duplex);

    if (link_change && status->link) {
        T_D("link change");
        intl_phy_sgmii_conf(dev, status);
    }

    return MEPA_RC_OK;
}

static mepa_device_t *intl_probe(mepa_driver_t *drv,
                                 const mepa_callout_t    MEPA_SHARED_PTR *callout,
                                 struct mepa_callout_cxt MEPA_SHARED_PTR *callout_cxt,
                                 struct mepa_board_conf              *board_conf)
{
    mepa_device_t *dev;
    INTL_priv_data_t *priv;

    dev = mepa_create_int(drv, callout, callout_cxt, board_conf, sizeof(INTL_priv_data_t));
    if (!dev) {
        return 0;
    }

    priv = dev->data;
    struct gpy211_device *initconf = &(priv->initconf);

    priv->port_param.dev = dev;
    priv->initconf.mdiobus_read = mdiobus_read;
    priv->initconf.mdiobus_write = mdiobus_write;
    priv->initconf.mdiobus_data = (void *)&priv->port_param;
    priv->initconf.lock = NULL;

    INTL_priv_data_t *data = PRIV_DATA(dev);
    T_D("intl_probe, enter\n");

    if (gpy2xx_init(initconf) < 0)
        T_E("intl phy init error\n");

    return dev;
}

static mesa_rc intl_delete(mepa_device_t *dev)
{
    return mepa_delete_int(dev);
}

static mesa_rc intl_reset(mepa_device_t *dev,
                          const mepa_reset_param_t *rst_conf)
{
    return MEPA_RC_OK;
}

static mesa_rc intl_conf_set(mepa_device_t *dev,
                                const mepa_conf_t *config)
{
    return MEPA_RC_OK;
}

static mesa_rc intl_status_1g_get(mepa_device_t    *dev,
                                  mesa_phy_status_1g_t *status)
{
    return MEPA_RC_OK;
}

mepa_drivers_t mepa_intel_driver_init()
{
#define NR_INTL_PHY 1
    mepa_drivers_t res;
    static mepa_driver_t intl_drivers[NR_INTL_PHY] = {};

    intl_drivers[0].id = INTL_PHY_CHIPID;
    intl_drivers[0].mask = 0xffffffff;
    intl_drivers[0].mepa_driver_delete = intl_delete;
    intl_drivers[0].mepa_driver_reset = intl_reset;
    intl_drivers[0].mepa_driver_poll = intl_poll;
    intl_drivers[0].mepa_driver_conf_set = intl_conf_set;
    intl_drivers[0].mepa_driver_if_get = intl_if_get;
    intl_drivers[0].mepa_driver_power_set = NULL;
    intl_drivers[0].mepa_driver_cable_diag_start = NULL;
    intl_drivers[0].mepa_driver_cable_diag_get = NULL;
    intl_drivers[0].mepa_driver_media_set = NULL;
    intl_drivers[0].mepa_driver_probe = intl_probe;
    intl_drivers[0].mepa_driver_aneg_status_get = intl_status_1g_get;

    res.phy_drv = intl_drivers;
    res.count = NR_INTL_PHY;

    return res;
#undef NR_INTL_PHY
}
