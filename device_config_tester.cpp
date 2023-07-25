/**
 * \file device_config_tester.cpp
 *
 * \brief Provides implementation of classes which provide examples of 
 * identifying and configuring devices using the STAR-System C++ API.
 *
 * \author STAR-Dundee Ltd\n
 *         STAR House\n
 *         166 Nethergate\n
 *         Dundee, DD1 4EE\n
 *         Scotland, UK\n
 *         e-mail: support@star-dundee.com
 *
 * Provides implementation of classes which provide examples of identifying and
 * configuring devices using the STAR-System C++ API.
 *
 * Copyright &copy; 2014 STAR-Dundee Ltd
 */

#include "device_config_tester.h"
#include "receive_operation.hpp"
#include "stream_item.hpp"
#include "link_speed_event.hpp"
#include "remote_device.hpp"
#include "address.hpp"
#include "link_port_errors.hpp"

using namespace stardundee::com::starsystem::transferoperations;
using namespace stardundee::com::starsystem::streamitems;
using namespace stardundee::com::starsystem::general;
using namespace stardundee::com::starsystem::deviceconfig;
using namespace stardundee::com::starsystem::channels;

/**
 * Demonstrates the use of the Router Configuration API to obtain the version
 * information for SpaceWire routing devices.
 */
void DeviceConfigTester::PerformDeviceIdentifierExample()
{
    /* declare necessary variables */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    DeviceIdentifierInfo *pDeviceIdentifierInfo = NULL;
    char manufacturerStr[DeviceIdentifierInfo::STR_DEVICE_MANUFACTURER_LEN];
    char deviceStr[DeviceIdentifierInfo::STR_DEVICE_TYPE_LEN];
    NetworkDiscoveryInfo *pNetworkDiscoveryInfo = NULL;
    int i;
    U32 runningPortsMask;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Device Identifier Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain device's configuration information.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* get the device's identification information */
    puts("Obtaining device identification information..");
    pDeviceIdentifierInfo = pDeviceConfiguration->GetDeviceIdentificationInfo();

    /* if failed to obtain device identification info */
    if (!pDeviceIdentifierInfo)
    {
        /* report error */
        puts("Unable to obtain device's identification information.");

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display the Identification Information */
    printf("\nManufacturer ID:\t%u", 
        pDeviceIdentifierInfo->GetManufacturerID());
    printf("\nChip ID:\t%u", pDeviceIdentifierInfo->GetChipType());
    printf("\nVersion:\t%u", pDeviceIdentifierInfo->GetVersionNum());

    /* get the manufacturer information as a string and display it */
    pDeviceIdentifierInfo->GetDeviceManufacturerAsString(manufacturerStr);
    printf("\nManufacturer Name:\t%s", manufacturerStr);

    /* get the manufacturer information as a string and display it */
    pDeviceIdentifierInfo->GetDeviceTypeAsString(deviceStr);
    printf("\nDevice Type:\t%s", deviceStr);

    /* get the device's network discovery information */
    puts("\nObtaining network discovery information..");
    pNetworkDiscoveryInfo = pDeviceConfiguration->GetNetworkDiscoveryInfo();

    /* display device network information */
    switch(pNetworkDiscoveryInfo->GetDeviceType())
    {
    case NetworkDiscoveryInfo::DEVICE_TYPE_ROUTER:
            printf("\nDevice Type:\tRouter");
            break;
    case NetworkDiscoveryInfo::DEVICE_TYPE_UNKNOWN:
            printf("\nDevice Type:\tUnknown");
            break;
    case NetworkDiscoveryInfo::DEVICE_TYPE_INVALID:
            printf("\nDevice Type:\tInvalid");
            break;
    }

    /* display information about the ports on the device */
    printf("\nNumber of Ports (Total/Running):\t%u/%u",
                            pNetworkDiscoveryInfo->GetPortCount(),
                            pNetworkDiscoveryInfo->GetRunningPortsCount());

    /* display which ports are running */
    printf("\nRunning port numbers: ");
    runningPortsMask = pNetworkDiscoveryInfo->GetRunningPortsMask();
    for(i = 1; i < 32; i++)
    {
        if( (1 << i) & (int)runningPortsMask)
            printf(" %d", i);
    }

    /* display the return port */
    printf("\nReturn Port:\t%u", pNetworkDiscoveryInfo->GetReturnPort());

    /* \note If an instance of a Port object is required to access the
     * properties of the Return Port, it can be obtained thus:
     *
     * U8 portNumber;
     * Port *pReturnPort = NULL;
     *
     * // get the number of the return port
     * portNumber = pNetworkDiscoveryInfo->GetReturnPort();
     *
     * // get the return port
     * pReturnPort = pDeviceConfiguration->GetPort(portNumber);
     *
     */

    /* end of test output */
    puts("");

    /* free the network discovery information */
    delete pNetworkDiscoveryInfo;

    /* free device identifier information */
    delete pDeviceIdentifierInfo;

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates the use of the Mk 2 compatible device specific configuration
 * functions.
 */
void DeviceConfigTester::PerformMk2ConfigurationExample()
{
    /* declare necessary variables */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    Mk2DeviceConfiguration *pMk2DeviceConfiguration = NULL;
    PCIeDeviceConfiguration *pPCIeDeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[Mk2DeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[Mk2DeviceConfiguration::STR_BUILD_DATE_LEN];
    ClockRateParams clockRateParams;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Mk2 Configuration Example ###");

    /* get the device to configure */
    puts("Getting Mk2 device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining Mk2 device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the Mk2 device configuration information.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a Mk2 device */
    if (!DeviceTypeResolver::IsMk2Device(pDevice))
    {
        /* cannot perform any Mk2 functions */

        /* report error */
        puts("Device is not a Mk2 device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a Mk2DeviceConfiguration object, to access general configuration 
     * functionality of Mk2 devices 
     */
    pMk2DeviceConfiguration = (Mk2DeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pMk2DeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pMk2DeviceConfiguration->GetHardwareInfoAsString(versionStr, 
        buildDateStr);

    /* display the hardware build version and date */
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s\n", buildDateStr);

    /* set general purpose register to 0xABCD */
    if (!pMk2DeviceConfiguration->SetGeneralPurpose(0xABCD))
    {
        puts("Failed to set value of general purpose register.");
    }

    /* flash the device's LEDs*/
    if (!pMk2DeviceConfiguration->Identify())
    {
        puts("Failed to identify device by flashing its LEDs.");
    }

    /* set the device to be a time-code master*/
    if (!pMk2DeviceConfiguration->EnableAsTimeCodeMaster())
    {
        puts("Failed to enable the device as a time code master.");
    }

    /* set 2 second delay between time-codes */
    if (!pMk2DeviceConfiguration->SetTimeCodePeriod(2000000))
    {
        puts("Failed to set the time code period for the device.");
    }

    /* enable interface mode on the device */
    if (!pMk2DeviceConfiguration->EnableInterfaceMode())
    {
        puts("Failed to enable interface mode on the device.");
    }

    /* disable interface mode on port 2 on the device */
    if (!pMk2DeviceConfiguration->DisableInterfaceModeOnPort(2))
    {
        puts("Failed to disable interface mode on port 2.");
    }

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 3
     */
    if (!pMk2DeviceConfiguration->EnableSourceIdentification())
    {
        puts("Failed to enable source identification on the device");
    }
    if (!pMk2DeviceConfiguration->EnableSourceIdentificationOnPort(1))
    {
        puts("Failed to enable source identification on Port 1");
    }
    if (!pMk2DeviceConfiguration->EnableSourceIdentificationOnPort(3))
    {
        puts("Failed to enable source identification on Port 3");
    }

    /* THE NEXT SECTION IS SPECIFIC TO PCIe or SPLT DEVICES */

    /* if the device isn't a PCIe or SPLT device */
    if (!DeviceTypeResolver::IsPCIeOrSPLTDevice(pDevice))
    {
        /* cannot perform PCIe or SPLT configurations */

        /* notify */
        puts("Device is not a PCIe or SPLT device. Tests skipped");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a PCIeDeviceConfiguration object, to access configuration 
     * functionality specific to a PCIe device
     */
    pPCIeDeviceConfiguration = (PCIeDeviceConfiguration*)pDeviceConfiguration;

    /* set port 2 on the PCIe device to have a base transmit rate of 150Mbps, 
     * i.e. (100*3/4)*2
     */
    clockRateParams.SetMultiplier(3);
    clockRateParams.SetDivisor(4);
    pPCIeDeviceConfiguration->SetBaseTransmitClock(2, clockRateParams);

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates the use of the Router Configuration API to configure the ports
 * of a routing device.
 *
 * \note See the Remote Device example below too for extended Port manipulation
 *       including use of specific Port objects, e.g. LinkPort, 
 *       BrickMk2LinkPort and so on.
 */
void DeviceConfigTester::PerformPortConfigurationExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    U8 configPortNumber = 0; /* 0 = Configuration port */
    U8 linkPortNumber = 1;
    Port *pConfigPort = NULL, *pConnectedPort = NULL;
    Port *pLinkPort = NULL;
    ConfigPortErrors *pConfigPortErrors = NULL;
    LinkStatus *pLinkStatus = NULL;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Port Configuration Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* obtain a specified port on the device */
    pConfigPort = pDeviceConfiguration->GetPort(configPortNumber);

    /* if failed to obtain port */
    if (!pConfigPort)
    {
        /* report error */
        printf("Unable to obtain port %d on the device.", configPortNumber);

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display the port type */
    switch(pConfigPort->GetType())
    {
    case Port::PORT_TYPE_CONFIGURATION:
        printf("\nPort type of port %u: Configuration Port", configPortNumber);
        break;
    case Port::PORT_TYPE_LINK:
        printf("\nPort type of port %u: SpaceWire Link Port", configPortNumber);
        break;
    case Port::PORT_TYPE_EXTERNAL:
        printf("\nPort type of port %u: External Port", configPortNumber);
        break;
    case Port::PORT_TYPE_INVALID:
        printf("\nPort type of port %u: Invalid port value", configPortNumber);
        break;
    }

    /* get port's connection */
    pConnectedPort = pDeviceConfiguration->GetPortConnection(pConfigPort);

    /* \note Alternative method:
     *
     * // get the port number for the port connection
     * U8 portNumber = pConfigPort->GetPortConnection();
     *
     * // if port is valid (a value of '31' indicates no connected port found)
     * if (portNumber != 31)
     * {
     *      // get the connected port
     *      pConnectedPort = pDeviceConfiguration->GetPort(portNumber);
     * }
     *
     */

    /* if no connected port obtained */
    if (!pConnectedPort)
    {
        /* report error */
        puts("Unable to obtain port's connection");

        /* free any port(s) obtained on the device */
        delete pConfigPort;

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display port's internal connection */
    printf("\nPort %d is currently connected to port %d ",
        pConfigPort->GetPortNumber(), pConnectedPort->GetPortNumber());

    /* get configuration port errors (if portNum == 0) */
    pConfigPortErrors = (ConfigPortErrors*)pConfigPort->GetErrors();

    /* if no config port errors obtained */
    if (!pConfigPortErrors)
    {
        /* report error */
        puts("Unable to obtain config errors from port. Please ensure it "
                " is a config port.");

        /* free any port(s) obtained on the device */
        delete pConnectedPort;
        delete pConfigPort;

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if errors are present on the port */
    if(pConfigPortErrors->GetErrorCount())
    {
        printf("\n%d error(s) present on configuration port ",
                pConfigPortErrors->GetErrorCount());
    }

    /* clear any errors on the config port */
    pConfigPort->ClearErrors();

    /* free errors obtained on the config port */
    delete pConfigPortErrors;

    /* free config port and its connected port */
    delete pConnectedPort;
    delete pConfigPort;

    /* get SpaceWire port Status */
    /* Assume for the purpose of this example that port one for this device is 
     * a SpaceWire link port. The variable 'linkPortNumber' has been set to a 
     * value of '1'.
     */
    pLinkPort = pDeviceConfiguration->GetPort(linkPortNumber);

    /* if failed to obtain the link port requested */
    if (!pLinkPort)
    {
        /* report error */
        printf("Unable to obtain SpaceWire Link Port %d.\n", linkPortNumber);

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* get the link's status */
    pLinkStatus = (LinkStatus*)pLinkPort->GetStatus();

    /* if failed to obtain the link's status */
    if (!pLinkStatus)
    {
        /* report error */
        puts("Unable to obtain the SpaceWire link's status.");

        /* free the link */
        delete pLinkPort;

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display some details about the link's status.
     * \note The link's state machine state can be obtained from the 
     * GetLinkState() member function.
     */
    printf("\n\nLink %u %s running.", linkPortNumber, 
        pLinkStatus->IsRunning() ? "is":"is not");
    printf("\nLink %u %s set to autostart.", linkPortNumber, 
        pLinkStatus->IsAutoStart() ? "is":"is not");
    printf("\nLink %u %s set to start.", linkPortNumber, 
        pLinkStatus->InitiatesStartUp() ? "is":"is not");
    printf("\nLink %u %s disabled.", linkPortNumber, 
        pLinkStatus->IsDisabled() ? "is":"is not");
    printf("\nLink %u %s in tri-state mode.\n", linkPortNumber, 
        pLinkStatus->IsTriState() ? "is":"is not");

    /* set the link's status: disable the link (set the disable bit, clear the
     * start bit).
     *
     * \note Setting a link's status is specific to a Link port. As can be seen
     *       below, the generic 'Port' object is cast to an instance of one of
     *       its child classes (LinkPort) in order to gain access to methods
     *       which are specific to that form of port. More specific forms of
     *       child class exist, e.g. BrickMk2LinkPort which provides additional
     *       methods specific to link ports on a Brick Mk2 device.
     */
    pLinkStatus->SetDisabled(1);
    pLinkStatus->SetInitiateStartUp(0);
    ((LinkPort*)pLinkPort)->SetSpaceWireLinkStatus(pLinkStatus);

    /* start and stop the link (note the use of casting again) */
    ((LinkPort*)pLinkPort)->StartLink();
    ((LinkPort*)pLinkPort)->StopLink();

    puts("");

    /* free the link's status */
    delete pLinkStatus;

    /* free the link */
    delete pLinkPort;

    /* start and stop a given link using the port number only, i.e. not 
     * requiring an instance of a Port object. In which case, methods on the 
     * device configuration are called instead.
     */
    pDeviceConfiguration->StartLink(linkPortNumber);
    pDeviceConfiguration->StopLink(linkPortNumber);

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates the use of the Router Configuration API to configure the
 * router settings.
 */
void DeviceConfigTester::PerformRouterConfigurationExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    U32 portMask;
    int i;
    U8  tcFlagMode;
    U8  tcValue;
    RouterGlobalState *pGlobalState = NULL;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Router Configuration Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* get time-code output ports */
    if (!pDeviceConfiguration->GetTimeCodeDistributionPorts(&portMask))
    {
        puts("Failed to obtain time-code distribution ports.");
    }

    /* display the time-code output ports */
    printf("\nTime-codes are being forwarded on ports: ");
    for(i = 1; i < 32; i++)
    {
        /* test each bit in mask to see if it is set */
        if((1 << i) & (int)portMask)
            printf(" %d", i);
    }

    /* set the device's configuration such that time codes are forwarded on 
     * ports 1, 2 and 3 only
     */
    if (!pDeviceConfiguration->SetTimeCodeDistributionPorts(0xE))
    {
        puts("Failed to set time-code distribution ports.");
    }

    /* get the time-code flags mode */
    if (!pDeviceConfiguration->GetTimeCodeFlagMode(&tcFlagMode))
    {
        puts("Failed to obtain time-code flag mode.");
    }

    /* display flag mode */
    printf("\nTime-code flag interpretation mode: %s", tcFlagMode ?
                                "Flags ignored" :
                                "Time-code discarded on flag not \"00\"");

    /* get current time-code value */
    if (!pDeviceConfiguration->GetTimeCodeValue(&tcValue))
    {
        puts("Failed to obtain current time-code value.");
    }

    /* display current time-code value */
    printf("\n Current time-code value: %u", tcValue);

    /* get the Router's global settings */
    pGlobalState = pDeviceConfiguration->GetGlobalSettings();

    /* if failed to obtain global settings */
    if (!pGlobalState)
    {
        /* report error */
        puts("Unable to obtain the router's global settings.");

        /* free the device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display the global settings */
    printf("\n\nDisable on silence: %s", 
        pGlobalState->IsDisableOnSilenceSet() ? "Enabled" : "Disabled");
    printf("\nSelf addressing: %s", 
        pGlobalState->IsSelfAddressingEnabled() ? "Enabled" : "Disabled");
    printf("\nStart on request: %s", 
        pGlobalState->IsStartOnRequestSet() ? "Enabled" : "Disabled");
    switch(pGlobalState->GetTimeoutMode())
    {
    case RouterGlobalState::TIMEOUT_MODE_WATCHDOG:
        printf("\nWatchdog mode enabled.");
        break;
    case RouterGlobalState::TIMEOUT_MODE_BLOCKING:
        printf("\nWatchdog mode disabled.");
        break;
    }
    printf("\nTimeout period: ");
    switch(pGlobalState->GetTimeoutPeriod())
    {
    case RouterGlobalState::PORT_TIMEOUT_100US:
        printf("60-100us");
        break;
    case RouterGlobalState::PORT_TIMEOUT_1MS:
        printf("1.3ms");
        break;
    case RouterGlobalState::PORT_TIMEOUT_10MS:
        printf("10ms");
        break;
    case RouterGlobalState::PORT_TIMEOUT_100MS:
        printf("82ms");
        break;
    case RouterGlobalState::PORT_TIMEOUT_1S:
        printf("1.3s");
        break;
    }
    puts("");

    /* set global settings for the device */
    pGlobalState->SetDisableOnSilence(FALSE);
    pGlobalState->EnableSelfAddressing(TRUE);
    pGlobalState->SetStartOnRequest(TRUE);
    pGlobalState->SetTimeoutMode(RouterGlobalState::TIMEOUT_MODE_WATCHDOG);
    pGlobalState->SetTimeoutPeriod(RouterGlobalState::PORT_TIMEOUT_10MS);
    if (!pDeviceConfiguration->SetGlobalSettings((*pGlobalState)))
    {
        puts("Unable to set the router's global settings.");
    }

    puts("");

    /* free the router's global settings */
    delete pGlobalState;

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates the use of the Router Configuration API to configure
 * group adaptive routing.
 */
void DeviceConfigTester::PerformRoutingTableExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    RoutingTableEntry *pRoutingTableEntry = NULL;
    U8 logicalAddress = 106;
    int i;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Routing Table Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* get a routing table entry for a given logical address */
    pRoutingTableEntry = pDeviceConfiguration->GetRoutingTableEntry(
        logicalAddress);

    /* if failed to obtain routing table entry */
    if (!pRoutingTableEntry)
    {
        /* report error */
        printf("Unable to obtain the routing table entry for address %u",
                logicalAddress);

        /* free the device configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* display routing table entry details */
    printf("\nRouting table entry for address %u.\n", logicalAddress);
    printf("\n\tAddress is %s.", pRoutingTableEntry->IsInvalidAddressSet() ? 
        "invalid":"valid");
    printf("\n\tHeader deletion %s.", pRoutingTableEntry->IsDeleteHeaderSet() ? 
        "enabled":"disabled");
    printf("\n\tPriority: %s.", pRoutingTableEntry->IsPrioritySet() ? 
        "high":"low");
    printf("\n\tOutput ports: ");
    for(i = 1; i < 32; i++)
    {
        /* test each bit in mask to see if it is set */
        if((1 << i) & (int)pRoutingTableEntry->GetPortMask())
            printf(" %d", i);
    }

    /* set a routing table entry:
     * Logical address 44, Header deletion enabled, High priority
     * Output ports: 1,3
     */
    pRoutingTableEntry->SetDeleteHeader(1);
    pRoutingTableEntry->SetInvalidAddress(0);
    pRoutingTableEntry->SetPriority(1);
    pRoutingTableEntry->SetPortMask(BIT1 | BIT3);
    if (!pDeviceConfiguration->SetRoutingTableEntry(44, (*pRoutingTableEntry)))
    {
        puts("Failed to set routing table entry");
    }

    puts("");

    /* free the routing table entry */
    delete pRoutingTableEntry;

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates the use of the Router Configuration API to configure
 * the user registers.
 */
void DeviceConfigTester::PerformUserRegistersExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    REGISTER value;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - User Registers Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* set general purpose register to 0xABCD */
    pDeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* set device identity register to 0xCAFE */
    pDeviceConfiguration->SetNetworkIdentity(0xCAFE);

    /* display user register values */
    pDeviceConfiguration->GetGeneralPurpose(&value);
    printf("General Purpose register value: %x", value);
    pDeviceConfiguration->GetNetworkIdentity(&value);
    printf("\nNetwork Identity register value: %x", value);

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates an example of configuring a PCI Mk2 device.
 */
void DeviceConfigTester::PerformPCIMk2Example()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    PCIMk2DeviceConfiguration *pPCIMk2DeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[PCIMk2DeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[PCIMk2DeviceConfiguration::STR_BUILD_DATE_LEN];
    PCIMk2LinkPort::LinkFrequency linkFreq;
    U8 divider;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - PCI Mk2 Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a PCI Mk2 device */
    if (!DeviceTypeResolver::IsPCIMk2Device(pDevice))
    {
        /* cannot perform any PCI Mk2 functions */

        /* report error */
        puts("Device is not a PCI Mk2 device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a PCIMk2DeviceConfiguration object, to access general 
     * configuration functionality of a PCI Mk2 device
     */
    pPCIMk2DeviceConfiguration = (PCIMk2DeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pPCIMk2DeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pPCIMk2DeviceConfiguration->GetHardwareInfoAsString(versionStr,
                                                        buildDateStr);

    /* display the hardware info*/
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);

    /* set general purpose register to 0xABCD */
    pPCIMk2DeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* flash the devices LEDs*/
    pPCIMk2DeviceConfiguration->Identify();

    /* set the device to be a time-code master*/
    pPCIMk2DeviceConfiguration->EnableAsTimeCodeMaster();

    /* set 2 second delay between time-codes */
    pPCIMk2DeviceConfiguration->SetTimeCodePeriod(2000000);

    /* enable interface mode */
    pPCIMk2DeviceConfiguration->EnableInterfaceMode();

    /* disable interface mode on port 2 only */
    pPCIMk2DeviceConfiguration->DisableInterfaceModeOnPort(2);

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 3
     */
    pPCIMk2DeviceConfiguration->EnableSourceIdentification();
    pPCIMk2DeviceConfiguration->EnableSourceIdentificationOnPort(1);
    pPCIMk2DeviceConfiguration->EnableSourceIdentificationOnPort(2);

    /* set port 2 to have link speed of 100Mbps (200/2) */
    pPCIMk2DeviceConfiguration->SetLinkClockFrequency(2,
                                        PCIMk2LinkPort::LINK_FREQUENCY_200MHZ);
    pPCIMk2DeviceConfiguration->SetLinkRateDivider(2, 2);

    /* read link speed of port 1 */
    pPCIMk2DeviceConfiguration->GetLinkClockFrequency(1, &linkFreq);
    pPCIMk2DeviceConfiguration->GetLinkRateDivider(1, &divider);

    /* display the link speed */
    switch(linkFreq)
    {
    case PCIMk2LinkPort::LINK_FREQUENCY_120MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 120.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_128MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 128.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_140MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 140.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_150MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 150.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_160MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 160.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_180MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 180.0/divider);
    case PCIMk2LinkPort::LINK_FREQUENCY_200MHZ:
        printf("\nLink 1 transmit rate: %f Mbps", 200.0/divider);
    default:
        printf("\nInvalid link Frequency!");
    break;
    }

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration; /* or delete pPCIMk2DeviceConfiguration; */

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates an example of configuring a Brick Mk2 device.
 */
void DeviceConfigTester::PerformBrickMk2Example()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    BrickMk2DeviceConfiguration *pBrickMk2DeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[BrickMk2DeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[BrickMk2DeviceConfiguration::STR_BUILD_DATE_LEN];
    int enabled, result;
    U16 linkSpeed;
    BrickMk2Errors linkErrors;
    Channel channel;
    ReceiveOperation linkEventRxOp;
    StreamItem *pStreamItem = NULL;
    LinkSpeedEvent *pLinkSpeedEvent = NULL;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Brick Mk2 Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a Brick Mk2 device */
    if (!DeviceTypeResolver::IsBrickMk2Device(pDevice))
    {
        /* cannot perform any Brick Mk2 functions */

        /* report error */
        puts("Device is not a Brick Mk2 device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a BrickMk2DeviceConfiguration object, to access general 
     * configuration functionality of a Brick Mk2 device
     */
    pBrickMk2DeviceConfiguration = (BrickMk2DeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pBrickMk2DeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pBrickMk2DeviceConfiguration->GetHardwareInfoAsString(versionStr,
                                                        buildDateStr);

    /* display the hardware info*/
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);

    /* set general purpose register to 0xABCD */
    pBrickMk2DeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* flash the devices LEDs*/
    pBrickMk2DeviceConfiguration->Identify();

    /* set the device to be a time-code master*/
    pBrickMk2DeviceConfiguration->EnableAsTimeCodeMaster();

    /* set 2 second delay between time-codes */
    pBrickMk2DeviceConfiguration->SetTimeCodePeriod(2000000);

    /* enable interface mode */
    pBrickMk2DeviceConfiguration->EnableInterfaceMode();

    /* enable interface mode on port 1 only */
    pBrickMk2DeviceConfiguration->EnableInterfaceModeOnPort(1);

    /* disable interface mode on port 2 only */
    pBrickMk2DeviceConfiguration->DisableInterfaceModeOnPort(2);

    /* display 'enable interface' status of port 2 */
    pBrickMk2DeviceConfiguration->GetInterfaceModeEnabledOnPort(2, &enabled);
    printf("\nInterface mode %s on port 2", enabled ? "enabled" : "disabled");

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 2
     */
    pBrickMk2DeviceConfiguration->EnableSourceIdentification();
    pBrickMk2DeviceConfiguration->EnableSourceIdentificationOnPort(1);
    pBrickMk2DeviceConfiguration->EnableSourceIdentificationOnPort(2);

    /* check if identify source is enabled for port 1 */
    pBrickMk2DeviceConfiguration->GetSourceIdentificationEnabledOnPort(1, 
        &enabled);
    printf("\nIdentify source %s on port 1", enabled ? "enabled" : "disabled");

    /* disable identify source on port 1 */
    pBrickMk2DeviceConfiguration->DisableSourceIdentificationOnPort(1);

    /* set link speed for link 1 to 100Mbps (200/2)*/
    pBrickMk2DeviceConfiguration->SetLinkClockFrequency(1, 
        BrickMk2LinkPort::LINK_FREQUENCY_200MHZ);
    pBrickMk2DeviceConfiguration->SetLinkRateDivider(1, 2);

    /* display measured link speed */
    pBrickMk2DeviceConfiguration->GetMeasuredLinkSpeed(1, &linkSpeed);
    printf("\nMeasured Link Speed: %d Kbits/s",
            linkSpeed * STAR_CFG_BRICK_MK2_LINK_SPEED_UNITS_KBPS);

    /* inject an error on port 2 */
    linkErrors.SetSuppressFCT(1);
    pBrickMk2DeviceConfiguration->InjectErrors(2, linkErrors);

    /* enable Link Speed Events on port 1 on the Brick Mk2 device */
    pBrickMk2DeviceConfiguration->EnableSpeedChangeEventsOnPort(1);

    /* to receive link speed events, we must open a connection to device 
     * channel 0. This will prevent configuration operations from being 
     * performed until we close this channel again.
     */
    channel = pDevice->OpenChannel(Channel::DIRECTION_IN, 0, 0);

    /* create receive operation for link speed events */
    result = linkEventRxOp.CreateRxOperation(1,
                                ReceiveOperation::RECEIVE_LINK_SPEED_EVENTS);

    /* if failed to create receive op for link speed events */
    if (!result)
    {
        /* report error */
        puts("Failed to create receive op for testing link speed events.");

        /* close channel opened on device */
        channel.CloseChannel();

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* submit the receive operation for link speed events on the channel 
     * opened
     */
    channel.SubmitTransferOperation(&linkEventRxOp);

    /* wait for up to 10 seconds for link speed event to occur */
    printf("\nWaiting for Link speed event...");
    linkEventRxOp.WaitOnTransferOperationCompletion(10000);

    /* display new link speed */
    switch (linkEventRxOp.GetTransferOperationStatus())
    {
    case TransferOperation::TRANSFER_STATUS_COMPLETE:
        /* get the link speed event stream item from the receive op */
        pStreamItem = linkEventRxOp.GetTransferItem(0);

        /* if stream item successfully obtained */
        if (pStreamItem)
        {
            /* if stream item is a Link speed event */
            if (pStreamItem->GetStreamItemType() == 
                StreamItem::STREAM_ITEM_TYPE_LINK_SPEED_EVENT)
            {
                /* cast to link speed event */
                pLinkSpeedEvent = (LinkSpeedEvent*)pStreamItem;

                /* display the link speed */
                printf("\nNew Link Speed: %d Kbits/s",
                                    pLinkSpeedEvent->GetLinkSpeed()/1000);
            }
            else
            {
                puts("Stream item obtained isn't a Link Speed event");
            }
        }
        else
        {
            puts("Failed to obtain stream item for Link Speed event");
        }
        break;
    case TransferOperation::TRANSFER_STATUS_STARTED:
    case TransferOperation::TRANSFER_STATUS_NOT_STARTED:
        printf("\nLink speed event did not occur");
        linkEventRxOp.CancelTransferOperation();
        break;
    default:
        printf("\nError occurred while waiting for link speed event");
    }
    puts("");

    /* disable speed change events on port 1 */
    pBrickMk2DeviceConfiguration->DisableSpeedChangeEventsOnPort(1);

    puts("");

    /* free stream item obtained for link speed event */
    if (pStreamItem != NULL)
        delete pStreamItem; /* or delete pLinkSpeedEvent; */

    /* close channel opened on device */
    channel.CloseChannel();

    /* free device configuration */
    delete pDeviceConfiguration; /* or delete pBrickMk2DeviceConfiguration; */

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates an example of configuring a Router Mk2S device.
 */
void DeviceConfigTester::PerformRouterMk2SExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    RouterMk2SDeviceConfiguration *pRouterMk2SDeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[RouterMk2SDeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[RouterMk2SDeviceConfiguration::STR_BUILD_DATE_LEN];

    int enabled;
    double precisionTxRate;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Router Mk2S Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a Router Mk2S device */
    if (!DeviceTypeResolver::IsRouterMk2SDevice(pDevice))
    {
        /* cannot perform any Router Mk2S functions */

        /* report error */
        puts("Device is not a Router Mk2S device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a RouterMk2SDeviceConfiguration object, to access general 
     * configuration functionality of a Router Mk2S device
     */
    pRouterMk2SDeviceConfiguration =
                        (RouterMk2SDeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pRouterMk2SDeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pRouterMk2SDeviceConfiguration->GetHardwareInfoAsString(versionStr,
                                                        buildDateStr);

    /* display the hardware info */
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);

    /* set general purpose register to 0xABCD */
    pRouterMk2SDeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* flash the devices LEDs */
    pRouterMk2SDeviceConfiguration->Identify();

    /* set the device to be a time-code master */
    pRouterMk2SDeviceConfiguration->EnableAsTimeCodeMaster();

    /* set 2 second delay between time-codes */
    pRouterMk2SDeviceConfiguration->SetTimeCodePeriod(2000000);

    /* enable interface mode */
    pRouterMk2SDeviceConfiguration->EnableInterfaceMode();

    /* enable interface mode on port 1 only */
    pRouterMk2SDeviceConfiguration->EnableInterfaceModeOnPort(1);

    /* disable interface mode on port 2 only */
    pRouterMk2SDeviceConfiguration->DisableInterfaceModeOnPort(2);

    /* display whether interface mode is enabled on port 2 */
    pRouterMk2SDeviceConfiguration->GetInterfaceModeEnabledOnPort(2, &enabled);
    printf("\nInterface mode %s on port 2", enabled ? "enabled" : "disabled");

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 2
     */
    if (!pRouterMk2SDeviceConfiguration->EnableSourceIdentification())
    {
        puts("Failed to enable source identification on device");
    }
    if (!pRouterMk2SDeviceConfiguration->EnableSourceIdentificationOnPort(1))
    {
        puts("Failed to enable source identification on port 1");
    }
    if (!pRouterMk2SDeviceConfiguration->EnableSourceIdentificationOnPort(2))
    {
        puts("Failed to enable source identification on port 2");
    }

    /* check if identify source is enabled for port 1 */
    pRouterMk2SDeviceConfiguration->GetSourceIdentificationEnabledOnPort(1,
                                                                    &enabled);
    printf("\nIdentify source %s on port 1", enabled ? "enabled" : "disabled");

    /* disable source identification on port 1 */
    pRouterMk2SDeviceConfiguration->DisableSourceIdentificationOnPort(1);

    /* check if precision transmit rate is enabled */
    pRouterMk2SDeviceConfiguration->GetPrecisionTransmitRateEnabled(&enabled);
    printf("\nPrecision transmit rate is %s", enabled ? "enabled" : "disabled");

    /* enable precision transmit rate */
    pRouterMk2SDeviceConfiguration->EnablePrecisionTransmitRate();

    /* wait until precision transmit rate is in use. This can sometimes take 
     * 250us
     */
    do
    {
        pRouterMk2SDeviceConfiguration->GetPrecisionTransmitRateInUse(&enabled);
    }
    while(!enabled);

    /* get current precision transmit rate */
    pRouterMk2SDeviceConfiguration->GetPrecisionTransmitRate(&precisionTxRate);
    printf("\nPrecision transmit rate is %f Mbits/s", precisionTxRate);

    /* set precision transmit rate */
    pRouterMk2SDeviceConfiguration->SetPrecisionTransmitRate(123.4);
    pRouterMk2SDeviceConfiguration->GetPrecisionTransmitRate(&precisionTxRate);
    printf("\nPrecision transmit rate is %f Mbits/s", precisionTxRate);

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration; /* or delete pRouterMk2SDeviceConfiguration; */

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates an example of configuring a Brick Mk3 device.
 */
void DeviceConfigTester::PerformBrickMk3Example()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    BrickMk3DeviceConfiguration *pBrickMk3DeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[BrickMk3DeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[BrickMk3DeviceConfiguration::STR_BUILD_DATE_LEN];
    int enabled;
    ClockRateParams clockRateParams;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Brick Mk3 Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a Brick Mk3 device */
    if (!DeviceTypeResolver::IsBrickMk3Device(pDevice))
    {
        /* cannot perform any Brick Mk3 functions */

        /* report error */
        puts("Device is not a Brick Mk3 device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a BrickMk3DeviceConfiguration object, to access general 
     * configuration functionality of a Brick Mk3 device
     */
    pBrickMk3DeviceConfiguration = 
        (BrickMk3DeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pBrickMk3DeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pBrickMk3DeviceConfiguration->GetHardwareInfoAsString(versionStr, 
        buildDateStr);

    /* display the hardware info*/
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);

    /* set general purpose register to 0xABCD */
    pBrickMk3DeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* flash the device's LEDs*/
    pBrickMk3DeviceConfiguration->Identify();

    /* set the device to be a time-code master*/
    pBrickMk3DeviceConfiguration->EnableAsTimeCodeMaster();

    /* set 2 second delay between time-codes */
    pBrickMk3DeviceConfiguration->SetTimeCodePeriod(2000000);

    /* enable interface mode */
    pBrickMk3DeviceConfiguration->EnableInterfaceMode();

    /* enable interface mode on port 1 only */
    pBrickMk3DeviceConfiguration->EnableInterfaceModeOnPort(1);

    /* disable interface mode on port 2 only */
    pBrickMk3DeviceConfiguration->DisableInterfaceModeOnPort(2);

    /* display whether interface mode is enabled on port 2 */
    pBrickMk3DeviceConfiguration->GetInterfaceModeEnabledOnPort(2, &enabled);
    printf("\nInterface mode %s on port 2", enabled ? "enabled" : "disabled");

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 2
     */
    if (!pBrickMk3DeviceConfiguration->EnableSourceIdentification())
    {
        puts("Failed to enable source identification on device");
    }
    if (!pBrickMk3DeviceConfiguration->EnableSourceIdentificationOnPort(1))
    {
        puts("Failed to enable source identification on port 1");
    }
    if (!pBrickMk3DeviceConfiguration->EnableSourceIdentificationOnPort(2))
    {
        puts("Failed to enable source identification on port 2");
    }

    /* check if identify source is enabled for port 1 */
    pBrickMk3DeviceConfiguration->GetSourceIdentificationEnabledOnPort(1, 
        &enabled);
    printf("\nIdentify source %s on port 1", enabled ? "enabled" : "disabled");

    /* disable source identification on port 1 */
    pBrickMk3DeviceConfiguration->DisableSourceIdentificationOnPort(1);

    /* set port 2 on the Brick Mk3 device to have a base transmit rate of
     * 150Mbps, i.e. (100*3/4)*2
     */
    clockRateParams.SetMultiplier(2);
    clockRateParams.SetDivisor(8);
    pBrickMk3DeviceConfiguration->SetBaseTransmitClock(2, clockRateParams);

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration; /* or delete pBrickMk3DeviceConfiguration; */

    /* free device */
    delete pDevice;
}

void DeviceConfigTester::PerformPXIExample()
{
    /* define variables required for test */
    Device *pDevice = NULL;
    DeviceConfiguration *pDeviceConfiguration = NULL;
    PXIDeviceConfiguration *pPXIDeviceConfiguration = NULL;
    HardwareInfo hardwareInfo;
    char versionStr[PXIDeviceConfiguration::STR_VERSION_LEN];
    char buildDateStr[PXIDeviceConfiguration::STR_BUILD_DATE_LEN];
    int enabled;
    ClockRateParams clockRateParams;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - PXI Example ###");

    /* get the device to configure */
    puts("Getting device to configure...");
    pDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pDevice)
    {
        /* report error */
        puts("Unable to obtain device to configure.");

        /* end processing */
        return;
    }

    /* get the device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = pDevice->GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain the device configuration.");

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* if the device isn't a PXI device */
    if (!DeviceTypeResolver::IsPXIDevice(pDevice))
    {
        /* cannot perform any PXI functions */

        /* report error */
        puts("Device is not a PXI device.");

        /* free device's configuration */
        delete pDeviceConfiguration;

        /* free device */
        delete pDevice;

        /* end processing */
        return;
    }

    /* cast to a PXIDeviceConfiguration object, to access general configuration 
     * functionality of a PXI device
     */
    pPXIDeviceConfiguration = (PXIDeviceConfiguration*)pDeviceConfiguration;

    /* get the hardware info for the device if required */
    pPXIDeviceConfiguration->GetHardwareInfo(hardwareInfo);

    /* get the hardware build version and date as strings */
    pPXIDeviceConfiguration->GetHardwareInfoAsString(versionStr,
                                                        buildDateStr);

    /* display the hardware info */
    printf("\nVersion: %s", versionStr);
    printf("\nBuildDate: %s", buildDateStr);

    /* set general purpose register to 0xABCD */
    pPXIDeviceConfiguration->SetGeneralPurpose(0xABCD);

    /* flash the device's LEDs*/
    pPXIDeviceConfiguration->Identify();

    /* set the device to be a time-code master*/
    pPXIDeviceConfiguration->EnableAsTimeCodeMaster();

    /* set 2 second delay between time-codes */
    pPXIDeviceConfiguration->SetTimeCodePeriod(2000000);

    /* enable interface mode */
    pPXIDeviceConfiguration->EnableInterfaceMode();

    /* enable interface mode on port 1 only */
    pPXIDeviceConfiguration->EnableInterfaceModeOnPort(1);

    /* disable interface mode on port 2 only */
    pPXIDeviceConfiguration->DisableInterfaceModeOnPort(2);

    /* display whether interface mode is enabled on port 2 */
    pPXIDeviceConfiguration->GetInterfaceModeEnabledOnPort(2, &enabled);
    printf("\nInterface mode %s on port 2", enabled ? "enabled" : "disabled");

    /* enable adding the source port number as a leading byte to received 
     * packets on ports 1 and 2
     */
    if (!pPXIDeviceConfiguration->EnableSourceIdentification())
    {
        puts("Failed to enable source identification on device");
    }
    if (!pPXIDeviceConfiguration->EnableSourceIdentificationOnPort(1))
    {
        puts("Failed to enable source identification on port 1");
    }
    if (!pPXIDeviceConfiguration->EnableSourceIdentificationOnPort(2))
    {
        puts("Failed to enable source identification on port 2");
    }

    /* check if identify source is enabled for port 1 */
    pPXIDeviceConfiguration->GetSourceIdentificationEnabledOnPort(1, &enabled);
    printf("\nIdentify source %s on port 1", enabled ? "enabled" : "disabled");

    /* disable source identification on port 1 */
    pPXIDeviceConfiguration->DisableSourceIdentificationOnPort(1);

    /* set port 2 on the PXI device to have a base transmit rate of 150Mbps, 
     * i.e. (100*3/4)*2
     */
    clockRateParams.SetMultiplier(3);
    clockRateParams.SetDivisor(4);
    pPXIDeviceConfiguration->SetBaseTransmitClock(2, clockRateParams);

    puts("");

    /* free device configuration */
    delete pDeviceConfiguration; /* or delete pPXIDeviceConfiguration; */

    /* free device */
    delete pDevice;
}

/**
 * Demonstrates an example of configuring a remote device.
 *
 * \note To access the Remote device one must provide a path to its 
 *       configuration port (Port 0), and the return path back to the PC. This 
 *       example uses a the path to a device which is directly connected to the 
 *       PC, e.g. such as a Brick Mk2, as a basis for creating a remote device. 
 *       In which case the path to the configuration port of the device would 
 *       be "0 254" (or "0x00 0xFE" in hexadecimal). The return path would be 
 *       "254" (or "0xFE" in hexadecimal).
 */
void DeviceConfigTester::PerformRemoteDeviceTest()
{
    /* declare necessary variables */
    Device *pLocalDevice = NULL;
    RemoteDevice remoteDevice;
    unsigned char channelOnLocalDeviceToUse = 1;
    char nameForRemoteDevice[] = "My Remote Device #1";
    unsigned char pathTo[] = { 0x00, 0xFE };
    U16 pathToLen = sizeof(pathTo);
    unsigned char pathFrom[] = { 0xFE };
    U16 pathFromLen = sizeof(pathFrom);
    Address pathToRemoteDevice(pathTo, pathToLen);
    Address pathFromRemoteDevice(pathFrom, pathFromLen);
    DeviceConfiguration *pDeviceConfiguration = NULL;
    Mk2DeviceConfiguration *pMk2DeviceConfiguration = NULL;
    DeviceIdentifierInfo *pDeviceIdentifierInfo = NULL;
    char manufacturerStr[DeviceIdentifierInfo::STR_DEVICE_MANUFACTURER_LEN];
    char deviceStr[DeviceIdentifierInfo::STR_DEVICE_TYPE_LEN];
    NetworkDiscoveryInfo *pNetworkDiscoveryInfo = NULL;
    int i;
    U32 runningPortsMask;
    Port *pPort = NULL;
    Port::PortType portType;

    /* write header for sample program */
    puts("");
    puts("### Device Config API - Remote Device Test Example ###");

    /* get the local device which will be used to open a connection to the 
     * remote device
     */
    puts("Getting local device...");
    pLocalDevice = DeviceSelector::ChooseDevice();

    /* if no device obtained */
    if (!pLocalDevice)
    {
        /* report error */
        puts("Unable to obtain local device.");

        /* end processing */
        return;
    }

    /* create remote device */
    if (!remoteDevice.CreateRemoteDevice(pLocalDevice,
                                         channelOnLocalDeviceToUse,
                                         nameForRemoteDevice,
                                         &pathToRemoteDevice,
                                         &pathFromRemoteDevice))
    {
        /* failed to create remote device  */

        /* report error */
        puts("Failed to create remote device");

        /* free local device */
        delete pLocalDevice;

        /* end processing */
        return;
    }

    /* get the remote device's configuration */
    puts("Obtaining device configuration..");
    pDeviceConfiguration = remoteDevice.GetDeviceConfiguration();

    /* if failed to obtain the device's configuration */
    if (!pDeviceConfiguration)
    {
        /* report error */
        puts("Unable to obtain device's configuration information.");

        /* free local and remote devices */
        remoteDevice.DestroyDevice();
        delete pLocalDevice;

        /* end processing */
        return;
    }

    /* if remote device is a Mk2 device */
    if (DeviceTypeResolver::IsMk2Device(&remoteDevice))
    {
        /* cast device's configuration to a Mk2 configuration */
        pMk2DeviceConfiguration = (Mk2DeviceConfiguration*)pDeviceConfiguration;

        /* identify the device */
        if (!pMk2DeviceConfiguration->Identify())
        {
            puts("Error: failed to identify Mk2 device");
        }
    }

    /* get the device's identification information */
    puts("Obtaining device identification information..");
    pDeviceIdentifierInfo = pDeviceConfiguration->GetDeviceIdentificationInfo();

    /* if failed to obtain device identification info */
    if (!pDeviceIdentifierInfo)
    {
        /* report error */
        puts("Unable to obtain device's identification information.");

        /* free device configuration */
        delete pDeviceConfiguration;

        /* free local and remote devices */
        remoteDevice.DestroyDevice();
        delete pLocalDevice;

        /* end processing */
        return;
    }

    /* display the Identification Information */
    printf("\nManufacturer ID:\t%u", pDeviceIdentifierInfo->GetManufacturerID());
    printf("\nChip ID:\t%u", pDeviceIdentifierInfo->GetChipType());
    printf("\nVersion:\t%u", pDeviceIdentifierInfo->GetVersionNum());

    /* get the manufacturer information as a string and display it */
    pDeviceIdentifierInfo->GetDeviceManufacturerAsString(manufacturerStr);
    printf("\nManufacturer Name:\t%s", manufacturerStr);

    /* get the manufacturer information as a string and display it */
    pDeviceIdentifierInfo->GetDeviceTypeAsString(deviceStr);
    printf("\nDevice Type:\t%s", deviceStr);

    /* get the device's network discovery information */
    puts("\nObtaining network discovery information..");
    pNetworkDiscoveryInfo = pDeviceConfiguration->GetNetworkDiscoveryInfo();

    /* display device network information */
    switch(pNetworkDiscoveryInfo->GetDeviceType())
    {
    case NetworkDiscoveryInfo::DEVICE_TYPE_ROUTER:
            printf("\nDevice Type:\tRouter");
            break;
    case NetworkDiscoveryInfo::DEVICE_TYPE_UNKNOWN:
            printf("\nDevice Type:\tUnknown");
            break;
    case NetworkDiscoveryInfo::DEVICE_TYPE_INVALID:
            printf("\nDevice Type:\tInvalid");
            break;
    }

    /* display information about the ports on the device */
    printf("\nNumber of Ports (Total/Running):\t%u/%u",
                            pNetworkDiscoveryInfo->GetPortCount(),
                            pNetworkDiscoveryInfo->GetRunningPortsCount());

    /* display which ports are running */
    printf("\nRunning port numbers: ");
    runningPortsMask = pNetworkDiscoveryInfo->GetRunningPortsMask();
    for(i = 1; i < 32; i++)
    {
        if( (1 << i) & (int)runningPortsMask)
            printf(" %d", i);
    }

    /* display the return port */
    printf("\nReturn Port:\t%u", pNetworkDiscoveryInfo->GetReturnPort());
    puts("");

    /* \note If an instance of a Port object is required to access the
     * properties of the Return Port, it can be obtained thus:
     *
     * U8 portNumber;
     * Port *pReturnPort = NULL;
     *
     * // get the number of the return port
     * portNumber = pNetworkDiscoveryInfo->GetReturnPort();
     *
     * // get the return port
     * pReturnPort = pDeviceConfiguration->GetPort(portNumber);
     *
     */

    /* for each port */
    for (i=0; i<pNetworkDiscoveryInfo->GetPortCount(); i++)
    {
        /* get the next port */
        pPort = pDeviceConfiguration->GetPort((U8)i);

        /* if failed to obtain port */
        if (!pPort)
        {
            /* report error */
            printf("Failed to obtain Port %d on the device\n", i);
        }
        else
        {
            /* get the port's type */
            portType = pPort->GetType();

            /* if it is a config port */
            if (portType == Port::PORT_TYPE_CONFIGURATION)
            {
                /* display config port information */
                DisplayConfigPortInformation(pPort, true);
            }
            /* else, if it is a (SpaceWire) Link port */
            else if (portType == Port::PORT_TYPE_LINK)
            {
                /* display link port information */
                DisplayLinkPortInformation(pPort, true, &remoteDevice,
                                           pDeviceConfiguration);
            }
            /* else, if it is an External port */
            else if (portType == Port::PORT_TYPE_EXTERNAL)
            {
                /* display external port information */
                DisplayExternalPortInformation(pPort, true, &remoteDevice,
                                               pDeviceConfiguration);
            }
            else
            {
                /* unknown port type, report error */
                printf("Error: Port %d is of an unknown type\n", i);
            }

            /* free the port obtained */
            delete pPort;
        }
    }

    /* end of test output */
    puts("");

    /* free the network discovery information */
    delete pNetworkDiscoveryInfo;

    /* free device identifier information */
    delete pDeviceIdentifierInfo;

    /* free device configuration */
    delete pDeviceConfiguration;

    /* free local and remote devices */
    remoteDevice.DestroyDevice();
    delete pLocalDevice;
}

/**
 * Display information for a given Port, on the basis that it is a
 * configuration port.
 *
 * @param pPort A pointer to a Port object, representing the port to display
 *              information for.
 * @param clear Whether to clear errors (1) or not (0).
 */
void DeviceConfigTester::DisplayConfigPortInformation(Port *pPort,
                                                      bool clearErrors)
{
    ConfigPort *pConfigPort;
    ConfigPortErrors *pErrors = NULL;

    /* cast given port to more specific form (it is known to be a configuration 
     * port)
     */
    pConfigPort = (ConfigPort*)pPort;

    /* display header for port information */
    printf("##\n##\nPort %d is a configuration port\n", 
        pConfigPort->GetPortNumber());

    /* get error information for the port */
    pErrors = (ConfigPortErrors*)pConfigPort->GetErrors();

    /* if failed to obtain error information for port */
    if (!pErrors)
    {
        puts("Error: failed to obtain error information for config port");
    }
    else
    {
        /* display error information */
        puts("Error information:");
        printf("Number of errors found on port: %d\n", pErrors->GetErrorCount());
        printf("Cargo too large error: %s\n",
               pErrors->GetCargoTooLarge() ? "set" : "not set");
        printf("Command not implemented error: %s\n",
               pErrors->GetCommandNotImplemented() ? "set" : "not set");
        printf("Early EEP error: %s\n",
               pErrors->GetEarlyEEP() ? "set" : "not set");
        printf("Early EOP error: %s\n",
               pErrors->GetEarlyEOP() ? "set" : "not set");
        printf("Invalid Data CRC error: %s\n",
               pErrors->GetInvalidDataCRC() ? "set" : "not set");
        printf("Invalid data length error: %s\n",
               pErrors->GetInvalidDataLength() ? "set" : "not set");
        printf("Invalid destination key error: %s\n",
               pErrors->GetInvalidDestinationKey() ? "set" : "not set");
        printf("Invalid destination logical address error: %s\n",
               pErrors->GetInvalidDestinationLogicalAddress() ? "set" : "not set");
        printf("Invalid header CRC error: %s\n",
               pErrors->GetInvalidHeaderCRC() ? "set" : "not set");
        printf("Invalid register address error: %s\n",
               pErrors->GetInvalidRegisterAddress() ? "set" : "not set");
        printf("Invalid read-modify-write data length error: %s\n",
               pErrors->GetInvalidRMWDataLength() ? "set" : "not set");
        printf("Late EEP error: %s\n",
               pErrors->GetLateEEP() ? "set" : "not set");
        printf("Late EOP error: %s\n",
               pErrors->GetLateEOP() ? "set" : "not set");
        printf("Port timeout error: %s\n",
               pErrors->GetPortTimeoutError() ? "set" : "not set");
        printf("Source logical address error: %s\n",
               pErrors->GetSourceLogicalAddressError() ? "set" : "not set");
        printf("Source path address error: %s\n",
               pErrors->GetSourcePathAddressError() ? "set" : "not set");
        printf("Unsupported protocol error: %s\n",
               pErrors->GetUnsupportedProtocol() ? "set" : "not set");
        printf("Unused RMAP command or packet type error: %s\n",
               pErrors->GetUnusedRMAPCommandOrPacketType() ? "set" : "not set");
        printf("Verify buffer overrun error: %s\n",
               pErrors->GetVerifyBufferOverrun() ? "set" : "not set");

        /* if errors are to be cleared */
        if (clearErrors)
        {
            if (!pConfigPort->ClearErrors())
            {
                puts("Error: failed to clear errors on port");
            }
        }

        /* free error information */
        delete pErrors;
    }
}

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
void DeviceConfigTester::DisplayLinkPortInformation(Port *pPort,
                            bool clearErrors,
                            STARDevice *pOwningDevice,
                            DeviceConfiguration *pOwningDeviceConfiguration)
{
    LinkPort *pLinkPort = NULL;
    Mk2LinkPort *pMk2LinkPort = NULL;
    BrickMk2LinkPort *pBrickMk2LinkPort = NULL;
    PCIMk2LinkPort *pPCIMk2LinkPort = NULL;
    RouterMk2SLinkPort *pRouterMk2SLinkPort = NULL;
    PCIeLinkPort *pPCIeLinkPort = NULL;
    BrickMk3LinkPort *pBrickMk3LinkPort = NULL;
    LinkPortErrors *pErrors = NULL;
    Port *pConnectedPort = NULL;
    int enabled;
    LinkStatus *pLinkStatus = NULL;
    LinkStatus::LinkState linkState;
    ClockRateParams clockRateParams;
    BrickMk2LinkPort::LinkFrequency brickMk2LinkFreq;
    PCIMk2LinkPort::LinkFrequency pciMk2LinkFreq;
    U16 linkSpeed;
    U8 portRoutingAddress;

    /* cast given port to more specific form (it is known to be a link port) */
    pLinkPort = (LinkPort*)pPort;

    /* display header for port information */
    printf("##\n##\nPort %d is a Link port\n", pLinkPort->GetPortNumber());

    /* get error information for the port */
    pErrors = (LinkPortErrors*)pLinkPort->GetErrors();

    /* if failed to obtain error information for port */
    if (!pErrors)
    {
        puts("Error: failed to obtain error information for link port");
    }
    else
    {
        /* display error information */
        puts("Error information:");
        printf("Number of errors found on port: %d\n", pErrors->GetErrorCount());
        printf("Character sequence error: %s\n",
               pErrors->GetCharacterSequence() ? "set" : "not set");
        printf("Credit error: %s\n",
               pErrors->GetCredit() ? "set" : "not set");
        printf("Disconnect error: %s\n",
               pErrors->GetDisconnect() ? "set" : "not set");
        printf("Escape error: %s\n",
               pErrors->GetEscape() ? "set" : "not set");
        printf("Packet address error: %s\n",
               pErrors->GetPacketAddress() ? "set" : "not set");
        printf("Parity error: %s\n",
               pErrors->GetParity() ? "set" : "not set");
        printf("Port timeout error: %s\n",
               pErrors->GetPortTimeout() ? "set" : "not set");

        /* if errors are to be cleared */
        if (clearErrors)
        {
            if (!pLinkPort->ClearErrors())
            {
                puts("Error: failed to clear errors on port");
            }
        }

        /* free error information */
        delete pErrors;
    }

    /* get the 'connected' port */
    pConnectedPort = pOwningDeviceConfiguration->GetPortConnection(pLinkPort);

    /* \note Alternative method:
     *
     * // get the port number for the port connection
     * U8 portNumber = pLinkPort->GetPortConnection();
     *
     * // if port is valid (a value of '31' indicates no connected port found)
     * if (portNumber != 31)
     * {
     *      // get the connected port
     *      pConnectedPort = pOwningDeviceConfiguration->GetPort(portNumber);
     * }
     *
     */

    /* if failed to obtain connected port */
    if (!pConnectedPort)
    {
        puts("No connected port obtained for port");
    }
    else
    {
        /* display number of connected port */
        printf("Port %d is connected to Port %d\n", pLinkPort->GetPortNumber(),
                                            pConnectedPort->GetPortNumber());

        /* free the connected port */
        delete pConnectedPort;
    }

    /* determine whether time code distribution is enabled on the port */
    if (pLinkPort->GetTimeCodeDistributionEnabled(&enabled))
    {
        if (enabled)
            puts("Time code distribution is enabled on this port");
        else
            puts("Time code distribution is NOT enabled on this port");
    }
    else
    {
        /* failed to determine time-code distribtion status for port */
        puts("Failed to determine whether time-code distribution is enabled on"
             " the port");
    }

    /* get the status of the link port */
    pLinkStatus = (LinkStatus*)pLinkPort->GetStatus();

    /* if failed to obtain the link status information */
    if (!pLinkStatus)
    {
        puts("Error: failed to obtain link's status");
    }
    else
    {
        /* display link status information */
        puts("Link status information:");
        linkState = pLinkStatus->GetLinkState();
        printf("Link state: ");
        switch (linkState)
        {
        case LinkStatus::LINK_STATE_ERROR_RESET:
            printf("Error Reset\n");
            break;
        case LinkStatus::LINK_STATE_ERROR_WAIT:
            printf("Error Wait\n");
            break;
        case LinkStatus::LINK_STATE_READY:
            printf("Ready\n");
            break;
        case LinkStatus::LINK_STATE_STARTED:
            printf("Started\n");
            break;
        case LinkStatus::LINK_STATE_CONNECTING:
            printf("Connecting\n");
            break;
        case LinkStatus::LINK_STATE_RUN:
            printf("Running\n");
            break;
        case LinkStatus::LINK_STATE_INVALID:
            printf("Invalid!\n");
            break;
        default:
            printf("Undefined!\n");
            break;

        }
        printf("Initiate start-up set?: %s\n",
               pLinkStatus->InitiatesStartUp() ? "Yes" : "No");
        printf("Auto-start set?: %s\n",
               pLinkStatus->IsAutoStart() ? "Yes" : "No");
        printf("Disabled?: %s\n",
            pLinkStatus->IsDisabled() ? "Yes" : "No");
        printf("Running?: %s\n",
            pLinkStatus->IsRunning() ? "Yes" : "No");
        printf("Tri-state?: %s\n",
            pLinkStatus->IsTriState() ? "Yes" : "No");

        /* free link status object */
        delete pLinkStatus;
    }

    /* access properties specific to link ports on Mk2 devices, if available */

    /* if port is on any form of Mk2 device, e.g. Brick Mk2, Router Mk2S, 
     * PCI Mk2, PCIe
     */
    if (DeviceTypeResolver::IsPortOnMk2Device(pLinkPort, pOwningDevice))
    {
        puts("Port is on a Mk2 device");

        /* cast port to more specific form (it is known to be a Mk2 link 
         * port)
         */
        pMk2LinkPort = (Mk2LinkPort*)pLinkPort;

        /* get port routing address */
        if (pMk2LinkPort->GetPortRoutingAddress(&portRoutingAddress))
        {
            printf("Port routing address: %d\n", portRoutingAddress);
        }
        else
        {
            /* failed to obtain port routing address for port */
            puts("Failed to obtain port routing address for port");
        }

        /* get interface mode status of port */
        if (pMk2LinkPort->GetInterfaceModeEnabled(&enabled))
        {
            printf("Interface mode is%senabled on the port\n",
                    enabled ? " " : " NOT ");
        }
        else
        {
            /* failed to obtain interface mode status for port */
            puts("Failed to obtain interface mode status for port");
        }

        /* get whether identify source in interface mode is enabled */
        if (pMk2LinkPort->GetSourceIdentificationEnabled(&enabled))
        {
            printf("Identify source in interface mode is%senabled on the port\n",
                    enabled ? " " : " NOT ");
        }
        else
        {
            puts("Failed to determine whether source identification in"
                 " interface mode is enabled for the port");
        }

        /* check for more specific Mk2 devices */

        /* if port is on a Brick Mk2 device */
        if (DeviceTypeResolver::IsPortOnBrickMk2Device(pLinkPort, pOwningDevice))
        {
            puts("Port is on a Brick Mk2");

            /* cast port to more specific form (it is a Brick Mk2 link port) */
            pBrickMk2LinkPort = (BrickMk2LinkPort*)pLinkPort;

            /* access additional functions which are specific to a Brick Mk2
             * link port, e.g. Get/SetLinkClockFrequency, GetMeasuredLinkSpeed,
             * enabling and disabling state change events and speed change
             * events.
             */

            /* get link clock frequency */
            if (pBrickMk2LinkPort->GetLinkClockFrequency(&brickMk2LinkFreq))
            {
                switch (brickMk2LinkFreq)
                {
                case BrickMk2LinkPort::LINK_FREQUENCY_120MHZ:
                    puts("Link clock frequency: 120");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_130MHZ:
                    puts("Link clock frequency: 130");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_140MHZ:
                    puts("Link clock frequency: 140");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_150MHZ:
                    puts("Link clock frequency: 150");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_160MHZ:
                    puts("Link clock frequency: 160");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_180MHZ:
                    puts("Link clock frequency: 180");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_200MHZ:
                    puts("Link clock frequency: 200");
                    break;
                default:
                    puts("Error: unknown link clock frequency");
                    break;
                }
            }
            else
            {
                puts("Failed to get link clock frequency for port");
            }

            /* get measured link speed */
            if (pBrickMk2LinkPort->GetMeasuredLinkSpeed(&linkSpeed))
            {
                printf("Measured link speed: %d\n", linkSpeed);
            }
            else
            {
                puts("Failed to get measured link speed for port");
            }

            /* get whether state change events are enabled on the port */
            if (pBrickMk2LinkPort->GetStateChangeEventsEnabled(&enabled))
            {
                printf("State change events are%senabled\n", 
                    enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if state change events are enabled");
            }

            /* get whether speed change events are enabled on the port */
            if (pBrickMk2LinkPort->GetSpeedChangeEventsEnabled(&enabled))
            {
                printf("Speed change events are%senabled\n",
                        enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if speed change events are enabled");
            }
        }

        /* if port is on a Router Mk2S device */
        if (DeviceTypeResolver::IsPortOnRouterMk2SDevice(pLinkPort, 
            pOwningDevice))
        {
            puts("Port is on a Router Mk2S");

            /* cast port to more specific form (it is a Router Mk2S link 
             * port)
             */
            pRouterMk2SLinkPort = (RouterMk2SLinkPort*)pLinkPort;

            /* access additional functions which are specific to a Router Mk2S
             * link port.
             *
             * \note Currently, these are the same as those available on a
             * BrickMk2 link porte.g. Get/SetLinkClockFrequency,
             * GetMeasuredLinkSpeed, enabling and disabling state change events
             * and speed change events
             */

            /* get link clock frequency */
            if (pRouterMk2SLinkPort->GetLinkClockFrequency(&brickMk2LinkFreq))
            {
                switch (brickMk2LinkFreq)
                {
                case BrickMk2LinkPort::LINK_FREQUENCY_120MHZ:
                    puts("Link clock frequency: 120");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_130MHZ:
                    puts("Link clock frequency: 130");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_140MHZ:
                    puts("Link clock frequency: 140");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_150MHZ:
                    puts("Link clock frequency: 150");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_160MHZ:
                    puts("Link clock frequency: 160");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_180MHZ:
                    puts("Link clock frequency: 180");
                    break;
                case BrickMk2LinkPort::LINK_FREQUENCY_200MHZ:
                    puts("Link clock frequency: 200");
                    break;
                default:
                    puts("Error: unknown link clock frequency");
                    break;
                }
            }
            else
            {
                puts("Failed to get link clock frequency for port");
            }

            /* get measured link speed */
            if (pRouterMk2SLinkPort->GetMeasuredLinkSpeed(&linkSpeed))
            {
                printf("Measured link speed: %d\n", linkSpeed);
            }
            else
            {
                puts("Failed to get measured link speed for port");
            }

            /* get whether state change events are enabled on the port */
            if (pRouterMk2SLinkPort->GetStateChangeEventsEnabled(&enabled))
            {
                printf("State change events are%senabled\n",
                        enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if state change events are enabled");
            }

            /* get whether speed change events are enabled on the port */
            if (pRouterMk2SLinkPort->GetSpeedChangeEventsEnabled(&enabled))
            {
                printf("Speed change events are%senabled\n",
                        enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if speed change events are enabled");
            }
        }

        /* if port is on a Brick Mk3 device */
        if (DeviceTypeResolver::IsPortOnBrickMk3Device(pLinkPort, 
            pOwningDevice))
        {
            puts("Port is on a Brick Mk3");

            /* cast port to more specific form (it is a Brick Mk3 link port) */
            pBrickMk3LinkPort = (BrickMk3LinkPort*)pLinkPort;

            /* access additional functions which are specific to a Brick Mk3
             * link port, e.g. Get/SetBaseTransmitClock, GetMeasuredLinkSpeed,
             * enabling and disabling state change events and speed change
             * events
             */

            /* get base transmit clock */
            pBrickMk3LinkPort->GetBaseTransmitClock(clockRateParams);
            printf("Brick Mk3 Link Port Base Transmit Clock: multiplier=%d; ",
                   clockRateParams.GetMultiplier());
            printf("divisor=%d\n", clockRateParams.GetDivisor());

            /* get measured link speed */
            if (pBrickMk3LinkPort->GetMeasuredLinkSpeed(&linkSpeed))
            {
                printf("Measured link speed: %d\n", linkSpeed);
            }
            else
            {
                puts("Failed to get measured link speed for port");
            }

            /* get whether state change events are enabled on the port */
            if (pBrickMk3LinkPort->GetStateChangeEventsEnabled(&enabled))
            {
                printf("State change events are%senabled\n",
                        enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if state change events are enabled");
            }

            /* get whether speed change events are enabled on the port */
            if (pBrickMk3LinkPort->GetSpeedChangeEventsEnabled(&enabled))
            {
                printf("Speed change events are%senabled\n",
                        enabled ? " " : " NOT ");
            }
            else
            {
                puts("Failed to determine if speed change events are enabled");
            }
        }

        /* if port is on a PCI Mk2 device */
        if (DeviceTypeResolver::IsPortOnPCIMk2Device(pLinkPort, pOwningDevice))
        {
            puts("Port is on a PCI Mk2");

            /* cast port to more specific form (PCI Mk2 link port) */
            pPCIMk2LinkPort = (PCIMk2LinkPort*)pLinkPort;

            /* access additional functions which are specific to a PCI Mk2
             * link port, e.g. Get/SetLinkClockFrequency
             */

            /* get link clock frequency */
            if (pPCIMk2LinkPort->GetLinkClockFrequency(&pciMk2LinkFreq))
            {
                switch (pciMk2LinkFreq)
                {
                case PCIMk2LinkPort::LINK_FREQUENCY_120MHZ:
                    puts("Link clock frequency: 120");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_128MHZ:
                    puts("Link clock frequency: 128");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_140MHZ:
                    puts("Link clock frequency: 140");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_150MHZ:
                    puts("Link clock frequency: 150");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_160MHZ:
                    puts("Link clock frequency: 160");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_180MHZ:
                    puts("Link clock frequency: 180");
                    break;
                case PCIMk2LinkPort::LINK_FREQUENCY_200MHZ:
                    puts("Link clock frequency: 200");
                    break;
                default:
                    puts("Error: unknown link clock frequency");
                    break;
                }
            }
            else
            {
                puts("Failed to get link clock frequency for port");
            }
        }

        /* if port is on a PCIe (or SPLT) device */
        if (DeviceTypeResolver::IsPortOnPCIeOrSPLTDevice(pLinkPort, 
            pOwningDevice))
        {
            puts("Port is on a PCIe/SPLT");

            /* cast port to more specific form (PCIe link port) */
            pPCIeLinkPort = (PCIeLinkPort*)pLinkPort;

            /* access additional functions which are specific to a PCIe link
             * port, e.g. Get/SetBaseTransmitClock
             */
            pPCIeLinkPort->GetBaseTransmitClock(clockRateParams);
            printf("PCIe Link Port Base Transmit Clock: multiplier=%d; ",
                   clockRateParams.GetMultiplier());
            printf("divisor=%d\n", clockRateParams.GetDivisor());
        }
    }
}

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
void DeviceConfigTester::DisplayExternalPortInformation(Port *pPort,
                            bool clearErrors,
                            STARDevice *pOwningDevice,
                            DeviceConfiguration *pOwningDeviceConfiguration)
{
    ExternalPort *pExternalPort;
    ExternalPortErrors *pErrors = NULL;
    Port *pConnectedPort = NULL;
    ExternalPortStatus *pExternalPortStatus = NULL;
    Mk2ExternalPort *pMk2ExternalPort = NULL;
    U8 portRoutingAddress;
    int enabled;

    /* cast given port to more specific form (it is known to be an external
     * port)
     */
    pExternalPort = (ExternalPort*)pPort;

    /* display header for port information */
    printf("##\n##\nPort %d is an external port\n", 
        pExternalPort->GetPortNumber());

    /* get error information for the port */
    pErrors = (ExternalPortErrors*)pExternalPort->GetErrors();

    /* if failed to obtain error information for port */
    if (!pErrors)
    {
        puts("Error: failed to obtain error information for external port");
    }
    else
    {
        /* display error information */
        puts("Error information:");
        printf("Number of errors found on port: %d\n", 
            pErrors->GetErrorCount());

        printf("Packet address error: %s\n",
            pErrors->GetPacketAddress() ? "set" : "not set");
        printf("Port timeout error: %s\n",
            pErrors->GetPortTimeout() ? "set" : "not set");

        /* if errors are to be cleared */
        if (clearErrors)
        {
            if (!pExternalPort->ClearErrors())
            {
                puts("Error: failed to clear errors on port");
            }
        }

        /* free error information */
        delete pErrors;
    }

    /* get the 'connected' port */
    pConnectedPort = pOwningDeviceConfiguration->GetPortConnection(
        pExternalPort);

    /* \note Alternative method:
     *
     * // get the port number for the port connection
     * U8 portNumber = pExternalPort->GetPortConnection();
     *
     * // if port is valid (a value of '31' indicates no connected port found)
     * if (portNumber != 31)
     * {
     *      // get the connected port
     *      pConnectedPort = pOwningDeviceConfiguration->GetPort(portNumber);
     * }
     *
     */

    /* if failed to obtain connected port */
    if (!pConnectedPort)
    {
        puts("No connected port obtained for port");
    }
    else
    {
        /* display number of connected port */
        printf("Port %d is connected to Port %d\n", 
            pExternalPort->GetPortNumber(), pConnectedPort->GetPortNumber());

        /* free the connected port */
        delete pConnectedPort;
    }

    /* get the status of the external port */
    pExternalPortStatus = (ExternalPortStatus*)pExternalPort->GetStatus();

    /* if failed to obtain the external port status information */
    if (!pExternalPortStatus)
    {
        puts("Error: failed to obtain external port status");
    }
    else
    {
        /* display external port status information */
        puts("External port status information:");
        printf("Input buffer empty?: %s\n",
                pExternalPortStatus->GetInputBufferEmpty() ? "Yes" : "No");
        printf("Input buffer full?: %s\n",
                pExternalPortStatus->GetInputBufferFull() ? "Yes" : "No");
        printf("Output buffer empty?: %s\n",
                pExternalPortStatus->GetOutputBufferEmpty() ? "Yes" : "No");
        printf("Output buffer full?: %s\n",
                pExternalPortStatus->GetOutputBufferFull() ? "Yes" : "No");

        /* free external port status object */
        delete pExternalPortStatus;
    }

    /* access properties specific to external ports on Mk2 devices,
     * if available
     */

    /* if port is on any form of Mk2 device, e.g. Brick Mk2, Router Mk2S, 
     * PCI Mk2, PCIe
     */
    if (DeviceTypeResolver::IsPortOnMk2Device(pExternalPort, pOwningDevice))
    {
        puts("Port is on a Mk2 device");

        /* cast port to more specific form (it is known to be a Mk2 port) */
        pMk2ExternalPort = (Mk2ExternalPort*)pExternalPort;

        /* get port routing address */
        if (pMk2ExternalPort->GetPortRoutingAddress(&portRoutingAddress))
        {
            printf("Port routing address: %d\n", portRoutingAddress);
        }
        else
        {
            /* failed to obtain port routing address for port */
            puts("Failed to obtain port routing address for port");
        }

        /* get interface mode status of port */
        if (pMk2ExternalPort->GetInterfaceModeEnabled(&enabled))
        {
            printf("Interface mode is%senabled on the port\n",
                    enabled ? " " : " NOT ");
        }
        else
        {
            /* failed to obtain interface mode status for port */
            puts("Failed to obtain interface mode status for port");
        }

        /* get whether identify source in interface mode is enabled */
        if (pMk2ExternalPort->GetSourceIdentificationEnabled(&enabled))
        {
            printf("Identify source in interface mode is%senabled on the port\n",
                    enabled ? " " : " NOT ");
        }
        else
        {
            puts("Failed to determine whether source identification in"
                 " interface mode is enabled for the port");
        }
    }
}

/**
 * Display the main options menu for the Device Config Tester Program.
 */
void DeviceConfigTester::DisplayMenu(void)
{
    puts("\nSelect Option:");
    printf(" 1. Device Identifier Example\n");
    printf(" 2. Mk2 Configuration Example\n");
    printf(" 3. Port Configuration Example\n");
    printf(" 4. Router Configuration Example\n");
    printf(" 5. Routing Table Example\n");
    printf(" 6. User Registers Example\n");
    printf(" 7. Brick Mk2 Example\n");
    printf(" 8. PCI Mk2 Example\n");
    printf(" 9. Router Mk2S Example\n");
    printf(" 10. Brick Mk3 Example\n");
    printf(" 11. PXI Example\n");
    printf(" 12. Remote Device Example\n");
    printf(" 0. Exit\n");
    printf("Please Select Menu Option: ");
}

/**
 * Run the device config test program.
 */
void DeviceConfigTester::RunTestProgram()
{
    char s[256];
    int menuSelect;
    char bExit = 0;

    puts("Device Config Test Program");
    puts("Copyright STAR-Dundee Ltd. (c) 2013-2017");
    puts("www.star-dundee.com");

    /* loop until exit option is chosen */
    while (!bExit)
    {
        /* display the main program menu */
        DisplayMenu();

        if (!fgets(s, 256, stdin))
        {
            puts("\nERROR: No value specified");
        }
        else if (!sscanf(s, "%d", &menuSelect))
        {
            puts("\nERROR: Invalid value specified");
        }
        else
        {
            switch(menuSelect)
            {
            case 1:
                PerformDeviceIdentifierExample();
                break;

            case 2:
                PerformMk2ConfigurationExample();
                break;

            case 3:
                PerformPortConfigurationExample();
                break;

            case 4:
                PerformRouterConfigurationExample();
                break;

            case 5:
                PerformRoutingTableExample();
                break;

            case 6:
                PerformUserRegistersExample();
                break;

            case 7:
                PerformBrickMk2Example();
                break;

            case 8:
                PerformPCIMk2Example();
                break;

            case 9:
                PerformRouterMk2SExample();
                break;

            case 10:
                PerformBrickMk3Example();
                break;

            case 11:
                PerformPXIExample();
                break;

            case 12:
                PerformRemoteDeviceTest();
                break;

            case 0:
                puts("\nExiting Device Config test program");
                bExit = 1;
                break;

            default:
                puts("\nERROR: Incorrect menu option");
                break;
            }
        }
    }

    puts("Exiting...\n");
}

