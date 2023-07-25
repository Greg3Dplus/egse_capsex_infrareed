/**
 * \file device_config_tester.h
 *
 * \brief Provides definitions of classes which provide examples of identifying
 *        and configuring devices using the STAR-System C++ API.
 *
 * \author STAR-Dundee Ltd\n
 *         STAR House\n
 *         166 Nethergate\n
 *         Dundee, DD1 4EE\n
 *         Scotland, UK\n
 *         e-mail: support@star-dundee.com
 *
 * Provides definitions of classes which provide examples of identifying and
 * configuring devices using the STAR-System C++ API.
 *
 * Copyright &copy; 2014 STAR-Dundee Ltd
 */

#include <stdio.h>
#include "star_system.hpp"
#include "device_type_resolver.h"
#include "device_selecter.h"
#include "device.hpp"
#include "device_configuration.hpp"
#include "mk2_device_configuration.hpp"
#include "brick_mk2_device_configuration.hpp"
#include "router_mk2s_device_configuration.hpp"
#include "brick_mk3_device_configuration.hpp"
#include "pci_mk2_device_configuration.hpp"
#include "pcie_device_configuration.hpp"
#include "brick_mk2_errors.hpp"
#include "pxi_device_configuration.hpp"

using namespace stardundee::com::starsystem::general;
using namespace stardundee::com::starsystem::deviceconfig;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::brickmk2;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::brickmk3;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::pcie;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::pcimk2;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::routermk2s;
using namespace stardundee::com::starsystem::deviceconfig::mk2devices::pxi;

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#define BIT1  0x02 /* for routing table example */
#define BIT3  0x08

#ifndef UNREFERENCED_PARAMETER
    #define UNREFERENCED_PARAMETER(a) ((void)(a))
#endif

/**
 * Provides examples of identifying and configuring devices using the
 * STAR-System C++ API.
 */
class DeviceConfigTester
{
public:
    /**
    * Demonstrates the use of the Router Configuration API to obtain the version
    * information for SpaceWire routing devices.
    */
    void PerformDeviceIdentifierExample();

    /**
     * Demonstrates the use of the Mk 2 compatible device specific configuration
     * functions.
     */
    void PerformMk2ConfigurationExample();

    /**
     * Demonstrates the use of the Router Configuration API to configure the 
     * ports of a routing device.
     */
    void PerformPortConfigurationExample();

    /**
     * Demonstrates the use of the Router Configuration API to configure the
     * router settings.
     */
    void PerformRouterConfigurationExample();

    /**
     * Demonstrates the use of the Router Configuration API to configure
     * group adaptive routing.
     */
    void PerformRoutingTableExample();

    /**
     * Demonstrates the use of the Router Configuration API to configure
     * the user registers.
     */
    void PerformUserRegistersExample();

    /**
     * Demonstrates an example of configuring a PCI Mk2 device.
     */
    void PerformPCIMk2Example();

    /**
     * Demonstrates an example of configuring a Brick Mk2 device.
     */
    void PerformBrickMk2Example();

    /**
     * Demonstrates an example of configuring a Router Mk2S device.
     */
    void PerformRouterMk2SExample();

    /**
     * Demonstrates an example of configuring a Brick Mk3 device.
     */
    void PerformBrickMk3Example();

    /**
     * Demonstrates an example of configuring a PXI device.
     */
    void PerformPXIExample();

    /**
     * Demonstrates an example of configuring a remote device.
     */
    void PerformRemoteDeviceTest();

    /**
     * Display information for a given Port, on the basis that it is a
     * configuration port.
     *
     * @param pPort A pointer to a Port object, representing the port to display
     *              information for.
     * @param clear Whether to clear errors (1) or not (0).
     */
    void DisplayConfigPortInformation(Port *pPort, bool clearErrors);

    /**
     * Display information for a given Port, on the basis that it is a
     * Link port.
     *
     * @param pPort A pointer to a Port object, representing the port to display
     *              information for.
     * @param clear Whether to clear errors (1) or not (0).
     * @param pOwningDevice The device which owns the given port.
     * @param pOwningDeviceConfiguration The configuration information for the
     *                                   device that owns the port.
     */
    void DisplayLinkPortInformation(Port *pPort, bool clearErrors,
                               STARDevice *pOwningDevice,
                               DeviceConfiguration *pOwningDeviceConfiguration);

    /**
     * Display information for a given Port, on the basis that it is an
     * External port.
     *
     * @param pPort A pointer to a Port object, representing the port to display
     *              information for.
     * @param clear Whether to clear errors (1) or not (0).
     * @param pOwningDevice The device which owns the given port.
     * @param pOwningDeviceConfiguration The configuration information for the
     *                                   device that owns the port.
     */
    void DisplayExternalPortInformation(Port *pPort, bool clearErrors,
                               STARDevice *pOwningDevice,
                               DeviceConfiguration *pOwningDeviceConfiguration);

    /* displays menu options for program */
    void DisplayMenu();

    /* Run the device config test program. */
    void RunTestProgram();
};

#endif /* DEVICE_CONFIG_H */

