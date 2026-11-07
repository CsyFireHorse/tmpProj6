#pragma once

#include "zecm_zh.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECATMKPA_API(...)   __VA_ARGS__

typedef void * ECAT_MASTER_PTR;
typedef ECAT_MASTER_PTR* ECAT_MASTER_PPTR;

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif



/*! @brief Network states of a master and constants relevant to network state.
    @ingroup master_state_control_master
    @see EcatGetMasterNetworkState() */
typedef enum tagEcatMasterNetworkState
{
    EcatMasterNetworkStateNormal            = 0x0000, /*!< @brief State when redundancy is not used. */
    EcatMasterNetworkStateRedundancyNormal  = 0x0001, /*!< @brief State when redundancy is used and In-port of a slave is connected to the main NIC. */
    EcatMasterNetworkStateRedundancyInverse = 0x0002, /*!< @brief State when redundancy is used and In-port of a slave is connected to the redundant NIC. */
    EcatMasterNetworkStateRedundancySafe    = 0x0003, /*!< @brief State when redundancy is used and there is a brake between slaves. */
    EcatMasterNetworkStateMask              = 0x00FF, /*!< @brief Bitmask for extracting the network state value. */
    EcatMasterNetworkStateFlagOpposite      = 0x0100  /*!< @brief Flag that indicates that EtherCAT&reg; frames reach the opposite card when redundancy is used. */
} EcatMasterNetworkState;

/* Diagnostic */

/*! @brief Slave's diagnostic states.
    @ingroup master_diagnostics */
typedef enum tagEcatSlaveDiagnosticState
{
    /*! @brief Slave doesn't respond to commands. */
    EcatSlaveDiagnosticStateOffLine             = 0x00000001,

    /*! @brief Slave's state is different as the set one. */
    EcatSlaveDiagnosticStateErrorEcatState      = 0x00000002,

    /*! @brief Slave is not configured. */
    EcatSlaveDiagnosticStateNotConfigured       = 0x00000004,

    /*! @brief Slave's configuration doesn't match the configuration for this
        slave found in master. */
    EcatSlaveDiagnosticStateWrongConfiguration  = 0x00000008,

    /*! @brief Error while Init Cmd */
    EcatSlaveDiagnosticStateInitCmdError        = 0x00000010,

    /*! @brief Error while Mailbox Init Cmd */
    EcatSlaveDiagnosticStateMailboxInitCmdError = 0x00000020
}EcatSlaveDiagnosticState;


/*! @brief Slave's status.
    @ingroup master_diagnostics */
typedef enum tagEcatSlaveStatus
{
#define ECAT_SLAVE_STATUS_STATE_MASK	0x000F
	 /*! @brief 
	  0x___X = current slave state (#EcatStateI, #EcatStateB, #EcatStateP, #EcatStateS, #EcatStateO) 
	  */
#define ECAT_SLAVE_STATUS_FLAGS_MASK	0xFFF0
    /*! @brief An error occurred while setting/getting slave signals. */
    EcatSlaveStatusSignalsError					= 0x0010,

	/*! @brief Invalid vendor ID, product code ... */
	EcatSlaveStatusWrongDevice					= 0x0020,

	/*! @brief Initialization error occurred */
	EcatSlaveStatusInitializationError			= 0x0040,

	/*! @brief Slave is disabled */
	EcatSlaveStatusDisabled						= 0x0080,

	/*! @brief Slave not present */
	EcatSlaveStatusNotPresent					= 0x0100,

	/*! @brief Slave signals link error */
	EcatSlaveStatusSignalsLinkError				= 0x0200,

	/*! @brief Slave signals missing link */
	EcatSlaveStatusSignalsMissingLink			= 0x0400,

	/*! @brief Slave signals unexpected link */
	EcatSlaveStatusSignalsUnexpectedLink		= 0x0800,

	/*! @brief Communication Port A */
	EcatSlaveStatusPortA						= 0x1000,

	/*! @brief Communication Port B */
	EcatSlaveStatusPortB						= 0x2000,

	/*! @brief Communication Port C */
	EcatSlaveStatusPortC						= 0x4000,

	/*! @brief Communication Port D */
	EcatSlaveStatusPortD						= 0x8000
    
}EcatSlaveStatus;

/*! @brief Master's diagnostic states.
    @ingroup master_diagnostics */
typedef enum tagEcatMasterDiagnosticState
{
    /*! @brief Diagnostics completed successfully. */
    EcatMasterDiagnosticStateUpdated					= 0x00000001,
    /*! @brief Error while sending/receiving a frame. */
    EcatMasterDiagnosticStateSendReceiveError			= 0x00000002,
    /*! @brief Error while processing the received frame. */
    EcatMasterDiagnosticStateParseError					= 0x00000004,
    /*! @brief No connection between the NIC adapter and slaves. */
    EcatMasterDiagnosticStateLinkDown					= 0x00000008,
    /*! @brief Wrong configuration */
    EcatMasterDiagnosticStateWrongConfiguration			= 0x00000010,
    /*! @brief Slave-to-slave timeout */
    EcatMasterDiagnosticStateS2STimeout					= 0x00000020,
    /*! @brief Default data was set */
    EcatMasterDiagnosticStateDefaultDataWasSet			= 0x00000040,
    /*! @brief WatchDogTimeOut */
    EcatMasterDiagnosticStateWatchDogTimeOut			= 0x00000080,
    /*! @brief MasterIsLocked */
    EcatMasterDiagnosticMasterIsLocked					= 0x00000100,
	/*! @brief Internal DC synchronisation is established */
    EcatMasterDiagnosticDCInternalSyncEstablished		= 0x00000200,
	/*! @brief External DC synchronisation is established */
    EcatMasterDiagnosticDCExternalSyncEstablished		= 0x00000400,
	/*! @brief DC Propagation delay initialized */
    EcatMasterDiagnosticDCPropagationDelayInitialized	= 0x00000800,
	/*! @brief Outputs was not delivered completely	*/
	EcatMasterDiagnosticStateOutputsUpdate			    = 0x00001000,
}EcatMasterDiagnosticState;



/*! @brief Master Diagnostic Information.
    @ingroup master_diagnostics
    @see see EcatGetDiagnostics, EcatMasterNetworkState, EcatGetMasterNetworkState() */
#pragma pack(1)
typedef struct _ECAT_MASTER_DIAGNOSTIC_INFO
{
    ECAT_DWORD  dwDiagState;            /*!< @brief Master's diagnostic state. See #EcatMasterDiagnosticState for more details. */
    ECAT_WORD   wNetworkState;          /*!< @brief Network states of a master. See #EcatMasterNetworkState for more details. */
    ECAT_WORD   wMainSlaveCount;        /*!< @brief Count of slaves on the main cable (i.e. slaves connected on the main NIC). */
    ECAT_WORD   wRedundancySlaveCount;  /*!< @brief Count of slaves on the redundancy cable (i.e. slaves on the redundancy NIC). */
    ECAT_WORD   wValidLicense;          /*!< @brief Type of license. See #ECAT_LICENSE_TYPE. */
    ECAT_BYTE   byExpirationDate[32];   /*!< @brief "License expiration date" information. */
    ECAT_INT32  nEDLength;              /*!< @brief Actual(current) length(in bytes) of byExpirationDate field. */
    ECAT_BYTE   byLicensedTo[64];       /*!< @brief "Licensed to" information. */
    ECAT_INT32  nLTLength;              /*!< @brief Actual(current) length(in bytes) of "byLicensedTo" field. */

    ECAT_BYTE  Reserved[2];             /*!< @brief Reserved. */
} ECAT_MASTER_DIAGNOSTIC_INFO;
#pragma pack()



#pragma pack(1)
/*! @brief Collection of Slave Identity information. */
typedef struct _ECAT_SLAVE_IDENTITY
{
    ECAT_DWORD  dwVendorId;         /*!< @brief Vendor ID from slave's EEPROM.*/
    ECAT_DWORD  dwProductCode;      /*!< @brief Product code from slave's EEPROM.*/
    ECAT_DWORD  dwRevisionNo;       /*!< @brief Revision number from slave's EEPROM.*/
    ECAT_DWORD  dwSerialNo;         /*!< @brief Serial number from slave's EEPROM.*/
} ECAT_SLAVE_IDENTITY;
#pragma pack()


#pragma pack(1)
/*! @brief Slave Diagnostic Information. */
typedef struct _ECAT_SLAVE_DIAGNOSTIC_INFO
{
    ECAT_DWORD          dwDiagState;                /*!< @brief Slave state. See #EcatSlaveDiagnosticState for more details. */
    ECAT_SLAVE_IDENTITY Identity;                   /*!< @brief Collection of Slave Identity information @ref ECAT_SLAVE_IDENTITY. */
    /* Port 0 (A)*/
    ECAT_BYTE           byRXErrorsPort0;            /*!< @brief RX Error counter of Port 0 (For additional information see
                                                    description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byInvalidFramesPort0;       /*!< @brief Invalid frame counter of Port 0 (For additional information see
                                                    description of RX Error Counter register (0x0300:0x0307) in slave
                                                    controller hardware data sheet).*/
    ECAT_BYTE           byLostLinkErrorsPort0;      /*!< @brief Lost Link counter of Port 0 (For additional information see
                                                    description of RX Error Counter register (0x0310:0x0313) in slave
                                                    controller hardware data sheet).*/
    /* Port 1 (B)*/
    ECAT_BYTE           byRXErrorsPort1;            /*!< @brief RX Error counter of Port 1 (For additional information see
                                                    description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byInvalidFramesPort1;       /*!< @brief Invalid frame counter of Port 1 (For additional information
                                                    see description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byLostLinkErrorsPort1;      /*!< @brief Lost Link counter of Port 1 (For additional information see
                                                    description of RX Error Counter register (0x0310:0x0313) in slave
                                                    controller hardware data sheet).*/
    /* Port 2 (C)*/
    ECAT_BYTE           byRXErrorsPort2;            /*!< @brief RX Error counter of Port 2 (For additional information see
                                                    description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byInvalidFramesPort2;       /*!< @brief Invalid frame counter of Port 2 (For additional information
                                                    see description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byLostLinkErrorsPort2;      /*!< @brief Lost Link counter of Port 2 (For additional information see
                                                    description of RX Error Counter register (0x0310:0x0313) in slave
                                                    controller hardware data sheet).*/
    /* Port 3 (D)*/
    ECAT_BYTE           byRXErrorsPort3;            /*!< @brief RX Error counter of Port 3 (For additional information
                                                    see description of RX Error Counter register (0x0300:0x0307) in
                                                    slave controller hardware data sheet).*/
    ECAT_BYTE           byInvalidFramesPort3;       /*!< @brief Invalid frame counter of Port 3 (For additional information see
                                                    description of RX Error Counter register (0x0300:0x0307) in slave
                                                    controller hardware data sheet).*/
    ECAT_BYTE           byLostLinkErrorsPort3;      /*!< @brief Lost link counter of Port 3 (For additional information see
                                                    description of RX Error Counter register (0x0310:0x0313) in slave
                                                    controller hardware data sheet).*/

    /* Port 0 (A)*/
    ECAT_BYTE           byForwardedRXErrorsPort0;   /*!< @brief Forwarded RX Error counter of Port 0 (For additional information see
                                                    description of Forwarded RX Error Counter register (0x0308:0x030B) in slave
                                                    controller hardware data sheet).*/

    /* Port 1 (B)*/
    ECAT_BYTE           byForwardedRXErrorsPort1;   /*!< @brief Forwarded RX Error counter of Port 1 (For additional information see
                                                    description of Forwarded RX Error Counter register (0x0308:0x030B) in slave
                                                    controller hardware data sheet).*/
    /* Port 2 (C)*/
    ECAT_BYTE           byForwardedRXErrorsPort2;   /*!< @brief Forwarded RX Error counter of Port 2 (For additional information see
                                                    description of Forwarded RX Error Counter register (0x0308:0x030B) in slave
                                                    controller hardware data sheet).*/
    /* Port 3 (D)*/
    ECAT_BYTE           byForwardedRXErrorsPort3;   /*!< @brief Forwarded RX Error counter of Port 3 (For additional information see
                                                    description of Forwarded RX Error Counter register (0x0308:0x030B) in slave
                                                    controller hardware data sheet).*/

    /* Processing unit */
    ECAT_BYTE           byProcessingUnitErrors;     /*!< @brief Processing unit Error counter (For additional information see
                                                    description of Processing unit Error Counter register (0x030C) in slave
                                                    controller hardware data sheet).*/

    /* PDI */
    ECAT_BYTE           byPDIErrors;                /*!< @brief PDI Error counter (For additional information see
                                                    description of PDI Error Counter register (0x030D) in slave
                                                    controller hardware data sheet).*/

    ECAT_BYTE           Reserved[2];                /*!< Reserved for future use. */
} ECAT_SLAVE_DIAGNOSTIC_INFO;
#pragma pack()



#pragma pack(4)
/*! @brief Network interface statistics.
    @ingroup master_statistics */
typedef struct _EcatLinkStatistics
{
    ECAT_WORD wFramesPerSecond;     /*!< @brief Frames rate (frames per second).*/

    ECAT_DWORD dwRxPackets;         /*!< @brief Number of received frames.*/
    ECAT_DWORD dwTxPackets;         /*!< @brief Number of transmitted frames.*/
    ECAT_DWORD dwRxBytes;           /*!< @brief Number of received bytes.*/
    ECAT_DWORD dwTxBytes;           /*!< @brief Number of transmitted bytes.*/
    ECAT_DWORD dwRxErrors;          /*!< @brief Number of wrongly received frames.*/
    ECAT_DWORD dwTxErrors;          /*!< @brief Number of wrongly transmitted frames.*/
    ECAT_DWORD dwRxDropped;         /*!< @brief Number of dropped received frames.*/
    ECAT_DWORD dwTxDropped;         /*!< @brief Number of dropped transmitted frames.*/

    ECAT_DWORD dwMulticast;         /*!< @brief Number of multicast frames.*/
    ECAT_DWORD dwCollisions;        /*!< @brief Number of collisions.*/

} ECAT_LINK_STATISTICS;

/*! @brief Type of pointer to ECAT_LINK_STATISTICS.
    @ingroup master_statistics */
typedef ECAT_LINK_STATISTICS* ECAT_LINK_STATISTICS_PTR;

/*! @cond code_not_osal_dev */

/*! @brief Statistics of EtherCAT&reg; master activity.
    @ingroup master_statistics */
typedef struct tagEcatRTStatistics
{
    ECAT_DWORD dwAvgCycleTime;      /*!< @brief Average observed cycle time.    */
    ECAT_DWORD dwAvgCycleJitter;    /*!< @brief Average observed cycle jitter.  */
    ECAT_DWORD dwMinCycleTime;      /*!< @brief Minimal observed cycle time.    */
    ECAT_DWORD dwMaxCycleTime;      /*!< @brief Maximal observed cycle time    */

    ECAT_DWORD dwAvgSubCycleTime;   /*!< @brief Average observed subcycle time.   */
    ECAT_DWORD dwAvgSubCycleJitter; /*!< @brief Average observed subcycle jitter. */
    ECAT_DWORD dwMinSubCycleTime;   /*!< @brief Minimal observed subcycle time.   */
    ECAT_DWORD dwMaxSubCycleTime;   /*!< @brief Maximal observed subcycle time.   */

    ECAT_DWORD dwSendErrors;        /*!< @brief Number of send-errors */
    ECAT_DWORD dwReceiveErrors;     /*!< @brief Number of receive-errors */
    ECAT_DWORD dwWrongWC;           /*!< @brief Number of received frames with wrong working counter */
    ECAT_DWORD dwParseErrors;       /*!< @brief Number of parse errors. */

} ECAT_RT_STATISTICS;

/*! @brief Type of pointer to ECAT_RT_STATISTICS.
    @ingroup master_statistics */
typedef ECAT_RT_STATISTICS* ECAT_RT_STATISTICS_PTR;


/*! @brief System statistics.
    @ingroup master_statistics */
typedef struct tagEcatSystemStatistics
{
    ECAT_DWORD dwCPULoad;           /*!< @brief CPU load (percents multiplied by 10) */
    ECAT_DWORD dwBusLoad;           /*!< @brief Bus load (Bytes per second) */
    ECAT_DWORD dwRespondTimeUs;     /*!< @brief Slave response time */

} ECAT_SYSTEM_STATISTICS;
/*! @brief Type of pointer to ECAT_SYSTEM_STATISTICS.
    @ingroup master_statistics */
typedef ECAT_SYSTEM_STATISTICS* ECAT_SYSTEM_STATISTICS_PTR;


/*! @brief Composite statistics (Network interface statistics combined with
    EtherCAT&reg; master activity statistics).
    @ingroup master_statistics */
typedef struct tagEcatStatistics
{
    ECAT_LINK_STATISTICS    LinkStat;   /*!< @brief Network interface statistics. */
    ECAT_RT_STATISTICS      RTStat;     /*!< @brief EtherCAT&reg; master activity statistics. */
    ECAT_SYSTEM_STATISTICS  SysStat;    /*!< @brief System statistics. */
} ECAT_STATISTICS;
#pragma pack()
/*! @brief Type of pointer to ECAT_STATISTICS.
    @ingroup master_statistics */
typedef ECAT_STATISTICS* ECAT_STATISTICS_PTR;


/* Registers */

/* 1. Data Link Entity */

/* 1.1. DL Information */

#define ECAT_RADDR_DL_TYPE          0x0000  /*!< @brief Physical Address where Type ID of a device is located     */
#define ECAT_RADDR_DL_REVISION      0x0001  /*!< @brief Physical Address where Revision ID of a device is located */
#define ECAT_RADDR_DL_BUILD         0x0002  /*!< @brief Physical Address where Build ID of a device is located    */
#define ECAT_RADDR_DL_CHANNELS_FMMU 0x0004  /*!< @brief Physical Address where Number of supported FMMU channels is located */
#define ECAT_RADDR_DL_CHANNELS_SM   0x0005  /*!< @brief Physical Address where Number of supported Sync Manager channels is located */

#define ECAT_REG_DL_INFO_SIZE       10      /*!< @brief Size of the DL Information structure measured in bytes. */
#pragma pack(1)
/*! @brief Structure that contains array of bytes that represent DL Information structure.

    Format of the description below: BITS: PHYSICAL ADDRESS: FIELD DESCRIPTION
    <UL>
    <LI>bits    0..7: 0x0000: Type
    <LI>bits   8..15: 0x0001: Revision
    <LI>bits  16..31: 0x0002: Build
    <LI>bits  32..39: 0x0004: Number of supported FMMU channels (0x0001-0x0010)
    <LI>bits  40..47: 0x0005: Number of supported Sync Manager channels (0x0001-0x0010)
    <LI>bits  48..55: 0x0006: RAM Size (RAM Size in kByte (5-64))
    <LI>bits  56..63: 0x0007: Reserved
    <LI>bit       64: 0x0008: FMMU Bit Operation Not Supported
                             (0: Bit Operation supported,
                              1: Bit Operation not supported)
    <LI>bits  65..79: 0x0008: Reserved
    </UL>
    @see #ECAT_REG_DL_INFO_SIZE */
typedef struct _ECAT_REG_DL_INFO
{
    ECAT_BYTE RawData[ECAT_REG_DL_INFO_SIZE];
} ECAT_REG_DL_INFO;


/*! @brief Type of pointer to pointer to ECAT_REG_DL_INFO. */
typedef ECAT_REG_DL_INFO**  ECAT_REG_DL_INFO_PPTR;
#pragma pack()


/* Type */
#define ECAT_REG_DL_INFO_SET_TYPE(pDl, Val)                                 \
            ((ECAT_PBYTE)pDl)[0] = (Val);
/*! @brief Get Type field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_TYPE(pDl)                                      \
            ((ECAT_PBYTE)pDl)[0]

/* Revision */
#define ECAT_REG_DL_INFO_SET_REVISION(pDl, Val)                             \
            ((ECAT_PBYTE)pDl)[1] = (Val);
/*! @brief Get Revision field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_REVISION(pDl)                                  \
            ((ECAT_PBYTE)pDl)[1]

/* Build */
#define ECAT_REG_DL_INFO_SET_BUILD(pDl, Val)                                \
            ((ECAT_PBYTE)pDl)[2] = ECAT_LOBYTE(Val);                        \
            ((ECAT_PBYTE)pDl)[3] = ECAT_HIBYTE(Val);
/*! @brief Get Build field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_BUILD(pDl)                                     \
            ECAT_MAKEWORD(((ECAT_PBYTE)pDl)[3], ((ECAT_PBYTE)pDl)[2])

/* Number of FMMU */
#define ECAT_REG_DL_INFO_SET_NUMBEROFFMMU(pDl, Val)                         \
            ((ECAT_PBYTE)pDl)[4] = (Val);
/*! @brief Get Number of supported FMMU channels field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_NUMBEROFFMMU(pDl)                              \
            ((ECAT_PBYTE)pDl)[4]

/* Number of SM */
#define ECAT_REG_DL_INFO_SET_NUMBEROFSM(pDl, Val)                           \
            ((ECAT_PBYTE)pDl)[5] = (Val);
/*! @brief Get Number of supported Sync Manager channels field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_NUMBEROFSM(pDl)                                \
            ((ECAT_PBYTE)pDl)[5]

/* RAM Size */
#define ECAT_REG_DL_INFO_SET_RAMSIZE(pDl, Val)                              \
            ((ECAT_PBYTE)pDl)[6] = (Val);
/*! @brief Get RAM Size field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_RAMSIZE(pDl)                                   \
            ((ECAT_PBYTE)pDl)[6]

/* FMMU Bit Operation Not Supported */
#define ECAT_REG_DL_INFO_SET_FMMUBITOP(pDl, Val)                            \
            ECAT_SET_LE_BIT(pDl, 64, Val)
/*! @brief Get FMMU Bit Operation Not Supported field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_FMMUBITOP(pDl)                                 \
            ECAT_GET_LE_BIT(pDl, 64)

/* Reserved */
#define ECAT_REG_DL_INFO_SET_RESERVED(pDl, Val)                             \
            ((ECAT_PBYTE)pDl)[8] &= ~0xFE;                                  \
            ((ECAT_PBYTE)pDl)[8] |= ECAT_LOBYTE((Val) << 1);                \
            ((ECAT_PBYTE)pDl)[9] = ECAT_HIBYTE((Val) << 1);
/*! @brief Get FMMU Bit Operation Not Supported field of DL Information structure. */
#define ECAT_REG_DL_INFO_GET_RESERVED(pDl)                                  \
            (ECAT_MAKEWORD(((ECAT_PBYTE)pDl)[9],                            \
                        ((ECAT_PBYTE)pDl)[8] & 0xFE) >> 1)


#define ECAT_RADDR_ECS_FEATURES 0x0008

/* Distributed Clocks Not Available/Available */
#define ECAT_ECS_CONTROL_GET_DC(pCtrl)                             \
    ECAT_GET_LE_BIT(pCtrl, 2)

/* Distributed Clocks width 32 bit/64 bit */
#define ECAT_ECS_CONTROL_GET_DC_WIDTH(pCtrl)                       \
    ECAT_GET_LE_BIT(pCtrl, 3)

/* 1.2. Fixed Station Address( = Configured Station Address) */

/*! @brief Physical Address where value of Fixed Station Address
           (i.e. Configured Station Address) of a device is located.
            That value is of type #ECAT_WORD (16 bit). */
#define ECAT_RADDR_DL_FIXED_ADDRESS	0x0010

/* 1.2.1 Second Fixed Station Address(Allias) */

/*! @brief Physical Address where value of Second Fixed Station Address
           (Allias) of a device is located.
            That value is of type #ECAT_WORD (16 bit). */
#define ECAT_RADDR_DL_FIXED_ADDRESS_ALIAS	0x0012

/* 1.3. DL Control */

/*! @brief Physical Address of DL Control register. */
#define ECAT_RADDR_DL_CONTROL           0x0100
/*! @brief DL Control register size measured in bytes. */
#define ECAT_REG_DL_CONTROL_SIZE        2

/*! @brief Physical Address of DL Control register Loop cfg. */
#define ECAT_RADDR_DL_CONTROL_LOOP       0x0101

#pragma pack(1)
/*! @brief Structure that contains array of bytes that represent DL Control register.

    <UL>
        <LI>bit       0: Forwarding Rule
        <LI>bits   1..3: Reserved
        <LI>bit       4: Extended Link detection activated on Port 0
        <LI>bit       5: Extended Link detection activated on Port 1
        <LI>bit       6: Extended Link detection activated on Port 2
        <LI>bit       7: Extended Link detection activated on Port 3
        <LI>bits   8..9: Loop Port 0
        <LI>bits 10..11: Loop Port 1
        <LI>bits 12..13: Loop Port 2
        <LI>bits 14..15: Loop Port 3
    </UL>
    @see #ECAT_REG_DL_CONTROL_SIZE */
typedef struct _ECAT_REG_DL_CONTROL
{
    ECAT_BYTE RawData[ECAT_REG_DL_CONTROL_SIZE];
} ECAT_REG_DL_CONTROL;
#pragma pack()

/* Forwarding Rule */
#define ECAT_REG_DL_CONTROL_SET_FWD_RULE(pDl, Val)                          \
            ECAT_SET_LE_BIT(pDl, 0, Val)
#define ECAT_REG_DL_CONTROL_GET_FWD_RULE(pDl)                               \
            ECAT_GET_LE_BIT(pDl, 0)

/* Reserved */
#define ECAT_REG_DL_CONTROL_SET_RESERVED(pDl, Val)                          \
            ECAT_SET_LE_BYTE_BITS(pDl, 1, 3, Val)
#define ECAT_REG_DL_CONTROL_GET_RESERVED(pDl)                               \
            ECAT_GET_LE_BYTE_BITS(pDl, 1, 3)

/* Extended Link on Port 0 */
#define ECAT_REG_DL_CONTROL_SET_EXT_LINK_ON_PORT0(pDl, Val)                 \
            ECAT_SET_LE_BIT(pDl, 4, Val)
#define ECAT_REG_DL_CONTROL_GET_EXT_LINK_ON_PORT0(pDl)                      \
            ECAT_GET_LE_BIT(pDl, 4)

/* Extended Link on Port 1 */
#define ECAT_REG_DL_CONTROL_SET_EXT_LINK_ON_PORT1(pDl, Val)                 \
            ECAT_SET_LE_BIT(pDl, 5, Val)
#define ECAT_REG_DL_CONTROL_GET_EXT_LINK_ON_PORT1(pDl)                      \
            ECAT_GET_LE_BIT(pDl, 5)

/* Extended Link on Port 2 */
#define ECAT_REG_DL_CONTROL_SET_EXT_LINK_ON_PORT2(pDl, Val)                 \
            ECAT_SET_LE_BIT(pDl, 6, Val)
#define ECAT_REG_DL_CONTROL_GET_EXT_LINK_ON_PORT2(pDl)                      \
            ECAT_GET_LE_BIT(pDl, 6)

/* Extended Link on Port 3 */
#define ECAT_REG_DL_CONTROL_SET_EXT_LINK_ON_PORT3(pDl, Val)                 \
            ECAT_SET_LE_BIT(pDl, 7, Val)
#define ECAT_REG_DL_CONTROL_GET_EXT_LINK_ON_PORT3(pDl)                      \
            ECAT_GET_LE_BIT(pDl, 7)

/* Loop Port 0 */
#define ECAT_REG_DL_CONTROL_SET_LOOP_PORT0(pDl, Val)                        \
            ECAT_SET_LE_BYTE_BITS(pDl, 8, 2, Val)
#define ECAT_REG_DL_CONTROL_GET_LOOP_PORT0(pDl)                             \
            ECAT_GET_LE_BYTE_BITS(pDl, 8, 2)

/* Loop Port 1 */
#define ECAT_REG_DL_CONTROL_SET_LOOP_PORT1(pDl, Val)                        \
            ECAT_SET_LE_BYTE_BITS(pDl, 10, 2, Val)
#define ECAT_REG_DL_CONTROL_GET_LOOP_PORT1(pDl)                             \
            ECAT_GET_LE_BYTE_BITS(pDl, 10, 2)

/* Loop Port 2 */
#define ECAT_REG_DL_CONTROL_SET_LOOP_PORT2(pDl, Val)                        \
            ECAT_SET_LE_BYTE_BITS(pDl, 12, 2, Val)
#define ECAT_REG_DL_CONTROL_GET_LOOP_PORT2(pDl)                             \
            ECAT_GET_LE_BYTE_BITS(pDl, 12, 2)

/* Loop Port 3 */
#define ECAT_REG_DL_CONTROL_SET_LOOP_PORT3(pDl, Val)                        \
            ECAT_SET_LE_BYTE_BITS(pDl, 14, 2, Val)
#define ECAT_REG_DL_CONTROL_GET_LOOP_PORT3(pDl)                             \
            ECAT_GET_LE_BYTE_BITS(pDl, 14, 2)

#define ECAT_RADDR_DL_CONTROL_LOOP      0x0101  /* DL Control Loop Port */
#define ECAT_REG_DL_CONTROL_LOOP_SIZE   1
/* Loop Port 2 */
#define ECAT_REG_DL_CONTROL_LOOP_SET_LOOP_PORT2(pDl, Val)                   \
            ECAT_SET_LE_BYTE_BITS(pDl, (12 - 8), 2, Val)
#define ECAT_REG_DL_CONTROL_LOOP_GET_LOOP_PORT2(pDl)                        \
            ECAT_GET_LE_BYTE_BITS(pDl, (12 - 8), 2)

/* 1.4. DLL Status */

/*! @brief Physical Address of DL Status register. */
#define ECAT_RADDR_DL_STATUS        0x0110
/*! @brief DL Status register size measured in bytes. */
#define ECAT_REG_DL_STATUS_SIZE     2

#pragma pack(1)
/*! @brief Structure that contains array of bytes that represent DL Status register.

    <UL>
    <LI> bit     0: PDI Operational
    <LI> bit     1: PDI Watchdog Status
    <LI> bits 2..3: Reserved
    <LI> bit     4: Link on Port 0
    <LI> bit     5: Link on Port 1
    <LI> bit     6: Link on Port 2
    <LI> bit     7: Link on Port 3
    <LI> bit     8: Loop Port 0 (Channel A Loop Status)
    <LI> bit     9: Communication on Port 0 (Channel A Signal Detection)
    <LI> bit    10: Loop Port 1 (Channel B Loop Status)
    <LI> bit    11: Communication on Port 1 (Channel B Signal Detection)
    <LI> bit    12: Loop Port 2 (Channel C Loop Status)
    <LI> bit    13: Communication on Port 2 (Channel C Signal Detection)
    <LI> bit    14: Loop Port 3 (Channel D Loop Status)
    <LI> bit    15: Communication on Port 3 (Channel D Signal Detection)
    </UL>
    @see #ECAT_REG_DL_STATUS_SIZE */
typedef struct _ECAT_REG_DL_STATUS
{
    ECAT_BYTE RawData[ECAT_REG_DL_STATUS_SIZE];
} ECAT_REG_DL_STATUS;

/*! @brief Type of pointer to ECAT_REG_DL_STATUS. */
typedef ECAT_REG_DL_STATUS* ECAT_REG_DL_STATUS_PTR;

#pragma pack()

/* PDI Operational */
#define ECAT_REG_DL_STATUS_SET_PDI_OPERATIONAL(pDl, Val)                    \
            ECAT_SET_LE_BIT(pDl, 0, Val)
#define ECAT_REG_DL_STATUS_GET_PDI_OPERATIONAL(pDl)                         \
            ECAT_GET_LE_BIT(pDl, 0)

/* PDI Watchdog Status */
#define ECAT_REG_DL_STATUS_SET_PDI_WATCHDOG_STATUS(pDl, Val)                \
            ECAT_SET_LE_BIT(pDl, 1, Val)
#define ECAT_REG_DL_STATUS_GET_PDI_WATCHDOG_STATUS(pDl)                     \
            ECAT_GET_LE_BIT(pDl, 1)

/* Reserved */
#define ECAT_REG_DL_STATUS_SET_RESERVED(pDl, Val)                           \
            ECAT_SET_LE_BYTE_BITS(pDl, 2, 2, Val)
#define ECAT_REG_DL_STATUS_GET_RESERVED(pDl)                                \
            ECAT_GET_LE_BYTE_BITS(pDl, 2, 2)

/* Link on Port 0 */
#define ECAT_REG_DL_STATUS_SET_LINK_ON_PORT0(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 4, Val)
#define ECAT_REG_DL_STATUS_GET_LINK_ON_PORT0(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 4)

/* Link on Port 1 */
#define ECAT_REG_DL_STATUS_SET_LINK_ON_PORT1(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 5, Val)
#define ECAT_REG_DL_STATUS_GET_LINK_ON_PORT1(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 5)

/* Link on Port 2 */
#define ECAT_REG_DL_STATUS_SET_LINK_ON_PORT2(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 6, Val)
#define ECAT_REG_DL_STATUS_GET_LINK_ON_PORT2(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 6)

/* Link on Port 3 */
#define ECAT_REG_DL_STATUS_SET_LINK_ON_PORT3(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 7, Val)
#define ECAT_REG_DL_STATUS_GET_LINK_ON_PORT3(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 7)

/* Loop Port 0 */
#define ECAT_REG_DL_STATUS_SET_LOOP_PORT0(pDl, Val)                         \
            ECAT_SET_LE_BIT(pDl, 8, Val)
#define ECAT_REG_DL_STATUS_GET_LOOP_PORT0(pDl)                              \
            ECAT_GET_LE_BIT(pDl, 8)

/* Communication on Port 0 */
#define ECAT_REG_DL_STATUS_SET_COMM_ON_PORT0(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 9, Val)
#define ECAT_REG_DL_STATUS_GET_COMM_ON_PORT0(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 9)

/* Loop Port 1 */
#define ECAT_REG_DL_STATUS_SET_LOOP_PORT1(pDl, Val)                         \
            ECAT_SET_LE_BIT(pDl, 10, Val)
#define ECAT_REG_DL_STATUS_GET_LOOP_PORT1(pDl)                              \
            ECAT_GET_LE_BIT(pDl, 10)

/* Communication on Port 1 */
#define ECAT_REG_DL_STATUS_SET_COMM_ON_PORT1(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 11, Val)
#define ECAT_REG_DL_STATUS_GET_COMM_ON_PORT1(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 11)

/* Loop Port 2 */
#define ECAT_REG_DL_STATUS_SET_LOOP_PORT2(pDl, Val)                         \
            ECAT_SET_LE_BIT(pDl, 12, Val)
#define ECAT_REG_DL_STATUS_GET_LOOP_PORT2(pDl)                              \
            ECAT_GET_LE_BIT(pDl, 12)

/* Communication on Port 2 */
#define ECAT_REG_DL_STATUS_SET_COMM_ON_PORT2(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 13, Val)
#define ECAT_REG_DL_STATUS_GET_COMM_ON_PORT2(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 13)

/* Loop Port 3 */
#define ECAT_REG_DL_STATUS_SET_LOOP_PORT3(pDl, Val)                         \
            ECAT_SET_LE_BIT(pDl, 14, Val)
#define ECAT_REG_DL_STATUS_GET_LOOP_PORT3(pDl)                              \
            ECAT_GET_LE_BIT(pDl, 14)

/* Communication on Port 3 */
#define ECAT_REG_DL_STATUS_SET_COMM_ON_PORT3(pDl, Val)                      \
            ECAT_SET_LE_BIT(pDl, 15, Val)
#define ECAT_REG_DL_STATUS_GET_COMM_ON_PORT3(pDl)                           \
            ECAT_GET_LE_BIT(pDl, 15)

            

/* 2. Slave Information Interface */
#pragma pack(1)
#define ECAT_SII_CONTENT_SIZE  128

/*! @brief Structure that contains array of bytes that represent
    Slave Information Interface contents.
    @see #ECAT_REG_COUNTERS_SIZE */
typedef struct _ECAT_SII_CONTENT
{
    ECAT_BYTE RawData[ECAT_SII_CONTENT_SIZE];
} ECAT_SII_CONTENT;

/*! @brief Type of pointer to pointer to ECAT_SII_CONTENT. */
typedef ECAT_SII_CONTENT** ECAT_SII_CONTENT_PPTR;

#pragma pack()

/* Vendor ID */
#define ECAT_SII_CONTENT_GET_VENDOR_ID(pSII)                                \
            ECAT_GET_LE_DWORD(((ECAT_PBYTE)pSII) + 0x10)

/* Product Code */
#define ECAT_SII_CONTENT_GET_PRODUCT_CODE(pSII)                             \
            ECAT_GET_LE_DWORD(((ECAT_PBYTE)pSII) + 0x14)

/* Revision Number */
#define ECAT_SII_CONTENT_GET_REVISION_NUMBER(pSII)                          \
            ECAT_GET_LE_DWORD(((ECAT_PBYTE)pSII) + 0x18)
            
            
#pragma pack(1)
/*! @brief Part of Slave Information Interface Area (machine-dependent format).
 */
typedef struct _ECAT_SII_CONST_CONTENT_SHORT
{
    ECAT_DWORD dwVendorId;
    ECAT_DWORD dwProductCode;
    ECAT_DWORD dwRevisionNo;
    ECAT_DWORD dwSerialNo;
} ECAT_SII_CONST_CONTENT_SHORT;
#pragma pack()
            
/*----------------------- Statistics ---------------------------------------*/

/*! @brief Acquires composite statistics for master and used network interfaces.

    @ingroup master_statistics
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pStatistics Pointer to #ECAT_STATISTICS structure that receives statistic values.
    @return #ECAT_S_OK,
            #ECAT_E_INVALIDARG,
            #ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONNECTED,
            #ECAT_E_NOTIMPL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see ECAT_STATISTICS, EcatConnectMaster
*/
ECATMKPA_API(ECAT_RESULT) EcatGetStatistics(
    IN ECAT_HANDLE hMaster,
    OUT ECAT_STATISTICS_PTR pStatistics);

/*! @brief Resets composite statistics of master and used network interfaces.

    @ingroup master_statistics
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @return #ECAT_S_OK,
            #ECAT_E_INVALIDARG,
            #ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONNECTED,
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatConnectMaster
*/
ECATMKPA_API(ECAT_RESULT) EcatResetStatistics(
    IN ECAT_HANDLE hMaster);

/*! @brief print statistics info

    @param pStatistics Master handle (for Master handle refer to #EcatCreateMaster function).
*/
void KPAShowMasterStatistics(
    IN ECAT_STATISTICS* pStatistics);
#define EcatShowMasterStatistics KPAShowMasterStatistics



/*----------------------- Slaves state control -----------------------------*/

/*! @brief Retrieves count of slaves in the current configuration.

    The function retrieves count of slaves in the configuration that is
    currently in the Master (that was previously loaded e.g. through EcatLoadConfigFromString).\n
    If the function is executed successfully, it returns the slaves
    count set in the current master configuration to the variable at
    the address set in the pwCount pointer.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pwCount Pointer to variable that receives the count of slaves.
    @return #ECAT_S_OK,
            #ECAT_E_INVALIDARG,
            #ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
        see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatRequestSlaveState, EcatGetSlaveStateByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveCount(
    IN  ECAT_HANDLE hMaster,
    OUT ECAT_WORD*  pwCount);

/*! @brief Requests the state of the slave.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in the configuration.
                    It varies from 0 to EcatGetSlaveCount()-1.
    @param byState Requested state.
                   Possible states:\n
                     EcatStateI -  EtherCAT&reg; State "Init"             \n
                     EcatStateP -  EtherCAT&reg; State "Pre-Operational"  \n
                     EcatStateB -  EtherCAT&reg; State "Bootstrap"        \n
                     EcatStateS -  EtherCAT&reg; State "Safe-Operational" \n
                     EcatStateO -  EtherCAT&reg; State "Operational"      \n
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_NOTIMPL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatRequestMasterState, EcatGetMasterState,
         EcatGetSlaveCount, EcatGetSlaveStateByPosIndex, EcatState
*/
ECATMKPA_API(ECAT_RESULT) EcatRequestSlaveState(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wSlaveId,
    IN ECAT_BYTE    byState);

/*! @brief Returns detected states of Slaves.

    The function reports the current states of the slaves in the network.
    Slaves to be scanned are defined by indexes(automatically incremented
    positions) located in an array of slaves' indexes.
    The respective slave's state is returned to each element of array of
    slaves' states.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosIndexArrLength Number of elements in the array of auto-increment(position) indexes.
    @param pwSlavePosIndexArr Pointer to array containing the auto-increment(position) indexes of slaves.
                              Number of elements inside this array must be
                              the same as specified by wSlavePosIndexArrLength.
    @param pbyState Pointer to array that receives states of slaves.
                    Number of elements inside this array must be
                    the same as specified by wSlavePosIndexArrLength.
                    For state types refer to #EcatState.

    @return #ECAT_S_OK, #ECAT_E_INVALIDARG, #ECAT_E_FAIL,
            #ECAT_E_READ_SLAVE_STATES, #ECAT_E_INVALID_WC.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatRequestMasterState, EcatGetMasterState,EcatGetSlaveCount,EcatRequestSlaveState
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveStateByPosIndex(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosIndexArrLength,
    IN  ECAT_WORD*  pwSlavePosIndexArr,
    OUT ECAT_BYTE*  pbyState);

/*! @brief Gets information about Slave's Process Image Inputs.
    The function requests the information for Slave's inputs from the configuration.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in configuration.
    @param pwBitSize Pointer to variable that receives the Slave Process Image Bit size.
    @param pwBitOffset Pointer to variable that receives the Slave Process Image Bit offset.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_INVALID_SLAVE_INDEX,
            #ECAT_E_SLAVE_PI_NOT_EXIST,#ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlavePIOutputsInfo
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlavePIInputsInfo(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_WORD*  pwBitSize,
    OUT ECAT_WORD*  pwBitOffset);

/*! @brief Gets information about Slave's Process Image Inputs.
    The function requests the information for Slave's inputs from the configuration.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in configuration.
    @param pdwBitSize Pointer to variable that receives the Slave Process Image Bit size.
    @param pdwBitOffset Pointer to variable that receives the Slave Process Image Bit offset.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_INVALID_SLAVE_INDEX,
            #ECAT_E_SLAVE_PI_NOT_EXIST,#ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlavePIOutputsInfo
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlavePIInputsInfoEx(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_DWORD*  pdwBitSize,
    OUT ECAT_DWORD*  pdwBitOffset);

/*! @brief Gets information about Slave Process Image Outputs.
    The function requests the information for Slave's outputs from the configuration.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave Identifier : from 0 to EcatGetSlaveCount()-1.
    @param pwBitSize Pointer to variable that receives the Slave Process Image Bit size.
    @param pwBitOffset Pointer to variable that receives the Slave Process Image Bit offset.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_INVALID_SLAVE_INDEX,
            #ECAT_E_SLAVE_PI_NOT_EXIST,#ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlavePIInputsInfo
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlavePIOutputsInfo(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_WORD*  pwBitSize,
    OUT ECAT_WORD*  pwBitOffset);

/*! @brief Gets information about Slave Process Image Outputs.
    The function requests the information for Slave's outputs from the configuration.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave Identifier : from 0 to EcatGetSlaveCount()-1.
    @param pdwBitSize Pointer to variable that receives the Slave Process Image Bit size.
    @param pdwBitOffset Pointer to variable that receives the Slave Process Image Bit offset.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_INVALID_SLAVE_INDEX,
            #ECAT_E_SLAVE_PI_NOT_EXIST,#ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlavePIInputsInfo
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlavePIOutputsInfoEx(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_DWORD*  pdwBitSize,
    OUT ECAT_DWORD*  pdwBitOffset);

/*! @brief Gets the slave's fixed address.
    The function retrieves Slave's fixed address from the configuration.

    If the function is executed successfully, it returns a fixed address of
    the slave in the current master's configuration into the variable pointed by pwFixAddr.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave Identifier: from 0 to EcatGetSlaveCount()-1.
    @param pwFixAddr Pointer to variable that receives the Slave's fixed address.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_INVALID_SLAVE_INDEX.\n
            For more information about result
        see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlaveCount
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveFixAddress(
    IN  ECAT_HANDLE  hMaster,
    IN  ECAT_WORD    wSlaveId,
    OUT ECAT_WORD*   pwFixAddr);


/*----------------------- Process image ------------------------------------*/

/*! @brief Gets the process image size.

    The function allows acquisition of sizes of Process Image
    inputs and outputs measured in bytes.
    @ingroup master_process_image_general
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pwInputsSize Pointer to variable that receives the size of inputs in Process Image.
    @param pwOutputsSize Pointer to variable that receives the size of outputs in Process Image.
    @return #ECAT_S_OK, #ECAT_E_INVALIDARG, #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetInputs, EcatGetOutputs, EcatSetOutputs
*/
ECATMKPA_API(ECAT_RESULT) EcatGetProcessImageSize(
    IN  ECAT_HANDLE hMaster,
    OUT ECAT_WORD*  pwInputsSize,
    OUT ECAT_WORD*  pwOutputsSize);

/*! @brief Gets the process image size.

    The function allows acquisition of sizes of Process Image
    inputs and outputs measured in bytes.
    @ingroup master_process_image_general
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pdwInputsSize Pointer to variable that receives the size of inputs in Process Image.
    @param pdwOutputsSize Pointer to variable that receives the size of outputs in Process Image.
    @return #ECAT_S_OK, #ECAT_E_INVALIDARG, #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetInputs, EcatGetOutputs, EcatSetOutputs
*/
ECATMKPA_API(ECAT_RESULT) EcatGetProcessImageSizeEx(
    IN  ECAT_HANDLE hMaster,
    OUT ECAT_DWORD*  pdwInputsSize,
    OUT ECAT_DWORD*  pdwOutputsSize);



/*! @brief [obsolete] Gets Process image inputs.
    The function retrieves inputs defined by size and offset.
    This function is obsolete, use #EcatStartReadInputs/#EcatDoneReadInputs instead.

    @ingroup master_process_image_inputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wOffset Inputs offset.
    @param pbyData Pointer to buffer that receives retrieved inputs.
    @param wSize Total size of inputs to be retrieved (measured in bytes).
    @return #ECAT_S_OK,
            #ECAT_E_INVALIDARG,
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetInputVariable, EcatGetOutputs, EcatSetOutputs, EcatGetProcessImageSize,
         EcatGetData,EcatGetVariable, EcatSetVariable, EcatStartReadInputs,
         EcatDoneReadInputs, EcatStartWriteOutputs, EcatDoneWriteOutputs.
*/
ECATMKPA_API(ECAT_RESULT) EcatGetInputs(
    IN  ECAT_HANDLE  hMaster,
    IN  ECAT_WORD    wOffset,
    OUT ECAT_BYTE*   pbyData,
    IN  ECAT_WORD    wSize);

/*! @brief [obsolete] Gets Process image input variable.

    The function retrieves input variable defined by bit size and bit offset.
    This function is obsolete, use #EcatStartReadInputs/#EcatGetVariable/#EcatDoneReadInputs instead.

    @ingroup master_process_image_inputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wBitSize Size of data (measured in bits).
    @param wBitOffs Data offset (measured in bits).
    @param pdwValue Pointer to buffer that receives a retrieved value.
    @return #ECAT_S_OK - function executed successfully,\n
            #ECAT_E_INVALID_POINTER - if one of parameters is not a valid pointer,\n
            #ECAT_E_INVALIDARG - if one of parameters is not a valid parameter,\n
            #ECAT_E_FAIL - if cyclic exchange is not started.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetVariable,  EcatSetVariable,EcatGetData,EcatSetData,
         EcatStartReadInputs, EcatDoneReadInputs,
         EcatStartWriteOutputs, EcatDoneWriteOutputs.
*/
ECATMKPA_API(ECAT_RESULT) EcatGetInputVariable(
    IN  ECAT_HANDLE  hMaster,
    IN  ECAT_WORD    wBitSize,
    IN  ECAT_WORD    wBitOffs,
    OUT ECAT_DWORD*  pdwValue);

/*! @brief [obsolete] Gets Process image outputs.
    The function retrieves outputs defined by size and offset.
    This function is obsolete, use #EcatStartWriteOutputs/#EcatDoneWriteOutputs instead.

    @ingroup master_process_image_outputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wOffset Outputs offset.
    @param pbyData Pointer to buffer that receives retrieved outputs.
    @param wSize Size of data to be retrieved.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONFIGURED,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetOutputVariable, EcatGetInputs,
         EcatSetOutputs, EcatGetProcessImageSize
*/
ECATMKPA_API(ECAT_RESULT) EcatGetOutputs(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wOffset,
    OUT ECAT_BYTE*  pbyData,
    IN  ECAT_WORD   wSize);

/*! @brief [obsolete] Sets Process image outputs.

    The function sets master's outputs defined by size and offset. \n
    This function is obsolete.\n
    If the function is used for several outputs,
    data consistency can not be assured.
    It is recommended to use #EcatStartWriteOutputs,
    #EcatSetVariable or #EcatSetData, #EcatDoneWriteOutputs.

    @ingroup master_process_image_outputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wOffset Outputs offset.
    @param pbyData Pointer to the buffer that contains the data that
                   is to be written to outputs.
    @param pbyDataMask Pointer to the buffer that contains the data mask.
                       Bit '1' in the mask means that the respective bit in
                       the output variable will be replaced by respective bit
                       from pbyData.
                       Bit '0' in the mask means that the respective bit in
                       the output variable will not be affected.\n
    @param wSize Size of data.
    @return #ECAT_S_OK,\n
            #ECAT_E_INVALID_POINTER,\n
            #ECAT_E_INVALIDARG,\n
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetInputs,EcatGetData,EcatGetVariable, EcatSetVariable,
         EcatStartReadInputs, EcatDoneReadInputs, EcatStartWriteOutputs,
         EcatDoneWriteOutputs.
*/
ECATMKPA_API(ECAT_RESULT) EcatSetOutputs(
    IN ECAT_HANDLE      hMaster,
    IN ECAT_WORD        wOffset,
    IN const ECAT_BYTE* pbyData,
    IN const ECAT_BYTE* pbyDataMask,
    IN ECAT_WORD        wSize);

/*! @brief [obsolete] Gets Process image output variable.

    The function retrieves an output variable defined by bit size and bit offset.
    This function is obsolete, use #EcatStartWriteOutputs/#EcatGetVariable/#EcatDoneWriteOutputs instead.
    @ingroup master_process_image_outputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wBitSize Size of output variable (measured in bits).
    @param wBitOffs Data offset of output variable (measured in bits).
    @param pdwValue Pointer to buffer that receives retrieved value.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_INVALID_POINTER.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSetOutputVariable, EcatGetInputs,
         EcatSetOutputs, EcatGetProcessImageSize
*/
ECATMKPA_API(ECAT_RESULT) EcatGetOutputVariable(
    IN  ECAT_HANDLE  hMaster,
    IN  ECAT_WORD    wBitSize,
    IN  ECAT_WORD    wBitOffs,
    OUT ECAT_DWORD*  pdwValue);

/*! @brief [obsolete] Sets Process image output variable.

    The function sets output variable defined by bit size and bit offset.
    The function is used to set values of master's outputs and
    has the same limitations as #EcatSetOutputs.
    This function is obsolete, use #EcatStartWriteOutputs/#EcatDoneWriteOutputs/#EcatGetVariable instead.
    @ingroup master_process_image_outputs
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wBitSize Size of output variable (measured in bits).
    @param wBitOffs Data offset of output variable (measured in bits).
    @param dwValue Value that is to be set.
    @return #ECAT_S_OK,\n
            #ECAT_E_INVALIDARG,\n
            #ECAT_E_INVALID_POINTER,\n
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSetOutputs, EcatGetInputs,EcatGetData,EcatGetVariable,
         EcatSetVariable, EcatStartReadInputs, EcatDoneReadInputs,
         EcatStartWriteOutputs, EcatDoneWriteOutputs.
*/
ECATMKPA_API(ECAT_RESULT) EcatSetOutputVariable(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wBitSize,
    IN ECAT_WORD    wBitOffs,
    IN ECAT_DWORD   dwValue);

    
/*! @brief Gets slave state.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in the configuration.
    @param pbyState Pointer to a variable that receives a slave state.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError(). */
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveState(
    IN ECAT_HANDLE  hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_BYTE*  pbyState);

/*! @brief Clears slave state error

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in the configuration.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
*/
ECATMKPA_API(ECAT_RESULT) EcatClearSlaveStateError(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wSlaveId);

/*! @brief Gets previously requested slave's state.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveId Slave identifier. It is a slave position in the configuration.
    @param pbyState Pointer to a variable that receives previously requested slave's state.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveReqState(
    IN ECAT_HANDLE  hMaster,
    IN  ECAT_WORD   wSlaveId,
    OUT ECAT_BYTE*  pbyState);


/*----------------------- Error handling functions -------------------------*/
/*! @brief Retrieves system message string that corresponds to
    the specified system message code.

    System messages are used to report results of operations or abnormal
    situations(errors) that might occur during master operation.

    This function copies not more than dwMaxBuffLen characters from
    the message string to the array pointed to by pszMessageBuff.
    Thus, if there is no null-terminator among the first dwMaxBuffLen
    characters of the message string, the result will not be null-terminated.

    @ingroup master_err_handling
    @param nErrorCode Code of a message.
    @param pszMessageBuff Pointer to the buffer that receives the string
           that corresponds to the code.
    @param dwMaxBuffLen Maximum number of characters the buffer can hold.
    @param pdwRealMessageLen Pointer to variable that receives the real length
           of the full message string(excluding the terminating '\\0' character).
    @return #ECAT_S_OK,\n
            #ECAT_E_INVALIDARG.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetLastError, EcatSetTraceCategories, EcatGetTraceCategories
*/
ECATMKPA_API(ECAT_RESULT) EcatGetErrorMessage(
    IN  ECAT_RESULT  nErrorCode,
    OUT ECAT_TCHAR*  pszMessageBuff,
    IN  ECAT_DWORD   dwMaxBuffLen,
    OUT ECAT_DWORD*  pdwRealMessageLen);

/*! @brief Retrieves the last error message.

    This function retrieves severity, location and message string for
    the last error occurred.
    This function copies not more than dwMaxBuffLen characters from
    the message string to the array pointed to by pszMessageBuff.
    Thus, if there is no null-terminator among the first dwMaxBuffLen
    characters of the message string, the result will not be null-terminated.
    Sequential calls to this function return remaining part of the message
    string until all characters of the message are returned.
    New messages that came while retrieving current message remain
    in an internal buffer and can be obtained later.
    Normally a caller of this function should supply a large enough
    message buffer in order to get a complete message string.
    Alternatively user can call the function sequentially,
    reading as many characters as the size of message buffer allows,
    until pdwRealMessageLen becomes less or equal to dwMaxBuffLen(that means
    that the last fragment is obtained).

    @ingroup master_err_handling
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pwSeverity Pointer to variable that receives severity of error message.
    @param pwLocation Pointer to variable that receives location of error message.
    @param pszMessageBuff Pointer to the buffer that receives the message string.
    @param dwMaxBuffLen Maximum number of characters the buffer can hold.
    @param pdwRealMessageLen Pointer to variable that receives the real length
           of the remaining message string(excluding the terminating '\\0' character).

    @return #ECAT_S_OK,\n
            #ECAT_E_INVALID_POINTER,\n
            #ECAT_E_INVALIDARG,\n
            #ECAT_S_NOITEMS.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetErrorMessage,
         EcatSetTraceCategories,EcatGetTraceCategories
*/
ECATMKPA_API(ECAT_RESULT) EcatGetLastError(
    IN  ECAT_HANDLE hMaster,
    OUT ECAT_WORD*  pwSeverity,
    OUT ECAT_WORD*  pwLocation,
    OUT ECAT_TCHAR* pszMessageBuff,
    IN  ECAT_DWORD  dwMaxBuffLen,
    OUT ECAT_DWORD* pdwRealMessageLen);

/*! @brief get error string info

    @param nErrorCode  error code

    @return error string info
*/
const ECAT_CHAR* KPAGetErrorDescription(ECAT_RESULT nErrorCode);
#define EcatGetErrorDescription KPAGetErrorDescription

/*------------------------- Network state ----------------------------------*/

/*! @brief Detects actual number of slaves in the network.

    The function returns to the variable that pwCount points to a real
    count of slaves in the network. The function can be used to create online
    configuration of slaves. May be called before loading of configuration
    (e.g. through EcatLoadConfigFromString) and starting of cyclic exchange
    (via EcatStartCyclicOperation).

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pwCount Pointer to variable that receives the actual number of slaves.
    @return #ECAT_S_OK,\n
            #ECAT_E_INVALID_POINTER,\n
            #ECAT_E_INVALIDARG,\n
            #ECAT_E_OUTOFMEMORY,\n
            #ECAT_E_MASTER_NOT_CONNECTED,\n
            #ECAT_E_SEND_FRAME_FAILED,\n
            #ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSIIContentEnumByPosIndex, EcatReadEEPROMByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatSSGetSlaveCount(
    IN  ECAT_HANDLE hMaster,
    OUT ECAT_WORD*  pwCount);

/*! @brief [obsolete] Gets SII content.

    This function is obsolete, use #EcatReadEEPROMByPosIndex instead.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosIndexArrLength Length of slaves auto increment (position) indexes array.
    @param pwSlavePosIndexArr Slaves indexes array.
    @param pSIIContent Pointer to buffer that receives SII content of slaves.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see ECAT_SII_CONTENT, ECAT_SII_CONTENT_SIZE,
         EcatSSGetSlaveCount,EcatReadEEPROMByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatSSGetSIIContentEnumByPosIndex(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosIndexArrLength,
    IN  ECAT_WORD*  pwSlavePosIndexArr,
    OUT ECAT_SII_CONTENT* pSIIContent);

/*! @brief Downloads EEPROM content by position index.

    The function reads firmware from one or several slaves at once
    using auto increment (position) addresses.

    The function is used to read data from EEPROM of several slaves in the network.
    The pData buffer should be interpreted as an array of blocks of size wDataLen.
    The number of blocks in pData array is assumed to be equal to wSlavePosIndexArrLength.
    The EEPROM data of each slave mentioned in pwSlavePosIndexArr array
    is written to a respective block of pData array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosIndexArrLength Length of slaves auto increment
           (position) indexes array (i.e. number of slaves).
    @param pwSlavePosIndexArr Pointer to array that contains the indexes of slaves.
    @param wStartAdr Start EEPROM address.
    @param wDataLen Length of data(number of bytes to be read from each slave).
    @param pData Pointer to the buffer that receives the data read from
           slaves (buffer size should be equal to wDataLen multiplied
           by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_INVALID_WC.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSlaveCount,EcatReadEEPROMByFixAddress
*/
ECATMKPA_API(ECAT_RESULT) EcatReadEEPROMByPosIndex(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosIndexArrLength,
    IN  ECAT_WORD*  pwSlavePosIndexArr,
    IN  ECAT_WORD   wStartAdr,
    IN  ECAT_WORD   wDataLen,
    OUT ECAT_BYTE*  pData);

/*! @brief Downloads EEPROM content by fixed address.

    The function reads firmware from one or several slaves at once using fixed addresses.

    The function is used to read data from EEPROM of several slaves in the network.
    The pData buffer should be interpreted as an array of blocks of size wDataLen.
    The number of blocks in pData array is assumed to be equal to wSlaveCount.
    The EEPROM data of each slave mentioned in pwSlavesFixAddrArr array
    is written to a respective block of pData array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of slaves fixed addresses array(i.e. number of slaves).
    @param pwSlavesFixAddrArr Pointer to array that contains fixed addresses of slaves.
    @param wStartAdr Start EEPROM address.
    @param wDataLen Length of data(number of bytes to be read from each slave).
    @param pData Pointer to the buffer that receives the data read from slaves
                (buffer size should be equal to wDataLen multiplied by slaves count).

    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatReadEEPROMByPosIndex,EcatSSGetSlaveCount
*/
ECATMKPA_API(ECAT_RESULT) EcatReadEEPROMByFixAddress(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveCount,
    IN  ECAT_WORD*  pwSlavesFixAddrArr,
    IN  ECAT_WORD   wStartAdr,
    IN  ECAT_WORD   wDataLen,
    OUT ECAT_BYTE*  pData);

/*! @brief Uploads EEPROM content by position index.

    The function writes firmware to one or several slaves at once
    using auto increment (position) addresses.

    The function is used to write data to EEPROM of several slaves in the network.
    The pData buffer should be interpreted as an array of blocks of size wDataLen.
    The number of blocks in pData array is assumed to be equal to wSlavePosIndexArrLength.
    The EEPROM data of each slave mentioned in pwSlavesFixAddrArr array
    is read from a respective block of pData array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosIndexArrLength Length of slaves auto increment (position)
           indexes array(i.e. number of slaves).
    @param pwSlavePosIndexArr Pointer to array that contains the indexes of slaves.
    @param wStartAdr Start EEPROM address.
    @param wDataLen Length of data(number of bytes to be written to each slave).
    @param pData Pointer to the buffer that contains the data that is to be
           written to slaves (buffer size should be equal to wDataLen
           multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSlaveCount,EcatReadEEPROMByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatWriteEEPROMByPosIndex(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosIndexArrLength,
    IN  ECAT_WORD*  pwSlavePosIndexArr,
    IN  ECAT_WORD   wStartAdr,
    IN  ECAT_WORD   wDataLen,
    OUT ECAT_BYTE*  pData);

/*! @brief Uploads EEPROM content by fixed address.

    The function writes firmware to one or several slaves at once
    using fixed address.

    The function is used to write data to EEPROM of several slaves in the network.
    The pData buffer should be interpreted as an array of blocks of size wDataLen.
    The number of blocks in pData array is assumed to be equal to wSlaveCount.
    The EEPROM data of each slave mentioned in pwSlavesFixAddrArr array
    is read from a respective block of pData array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of slaves fixed addresses array(i.e. number of slaves).
    @param pwSlavesFixAddrArr Pointer to array that contains the fixed addresses of slaves.
    @param wStartAdr Start EEPROM address.
    @param wDataLen Length of data(number of bytes to be written to each slave).
    @param pData Pointer to the buffer that contains the data that is to be
           written to slaves (buffer size should be equal to wDataLen
           multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSlaveCount,EcatReadEEPROMByPosIndex,
         EcatReadEEPROMByFixAddress,EcatWriteEEPROMByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatWriteEEPROMByFixAddress(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveCount,
    IN  ECAT_WORD*  pwSlavesFixAddrArr,
    IN  ECAT_WORD   wStartAdr,
    IN  ECAT_WORD   wDataLen,
    OUT ECAT_BYTE*  pData);

/*! @brief Updates EEPROM checksum by position index.

    The function updates firmware checksum for one or several slaves using
    auto increment (position) addresses.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of slaves' auto increment (position) indexes array(i.e. number of slaves).
    @param pwSlaves Pointer to array that contains slaves' indexes.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSlaveCount, EcatReadEEPROMByPosIndex, EcatWriteEEPROMByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatUpdateEEPROMChecksumByPosIndex(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveCount,
    IN  ECAT_WORD*  pwSlaves);

/*! @brief Gets DL status register contents by position index.

    The function obtains DL status register of one or several slaves which are
    defined by indexes (automatically incremented positions).

    The function is used to read DL status register of one or several slaves in the network.
    The pDLStatus buffer should be interpreted as an array of #ECAT_REG_DL_STATUS structures.
    The number of elements in pDLStatus array is assumed to be equal to wSlavePosAddrArrLength.
    The DL status register data of each slave mentioned in pwSlavePosAddrArr array
    is written to a respective block of pDLStatus array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosAddrArrLength Length of slaves auto increment (position) addresses array(i.e. number of slaves).
    @param pwSlavePosAddrArr Pointer to array that contains the indexes of slaves.
    @param pDLStatus Pointer to the buffer that receives the DL Status register contents of slaves.
            (buffer size should be equal to sizeof(ECAT_REG_DL_STATUS) multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetDLStatusByFixAddress,
*/
ECATMKPA_API(ECAT_RESULT) EcatGetDLStatus(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosAddrArrLength,
    IN  ECAT_WORD*  pwSlavePosAddrArr,
    OUT ECAT_REG_DL_STATUS* pDLStatus);

/*! @brief Gets DL status register contents by fixed address.

    The function obtains DL status register of one or several slaves which are
    defined by fixed addresses.

    The function is used to read DL status register of one or several slaves in the network.
    The pDLStatus buffer should be interpreted as an array of #ECAT_REG_DL_STATUS structures.
    The number of elements in pDLStatus array is assumed to be equal to wSlaveCount.
    The DL status register data of each slave mentioned in pwSlaves array
    is written to a respective block of pDLStatus array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of slaves' fixed addresses array (i.e. number of slaves).
    @param pwSlaves Pointer to array that contains the fixed addresses of slaves.
    @param pDLStatus Pointer to the buffer that receives the DL Status register contents of slaves.
           (buffer size should be equal to sizeof(#ECAT_REG_DL_STATUS) multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetDLStatus
*/
ECATMKPA_API(ECAT_RESULT) EcatGetDLStatusByFixAddress(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveCount,
    IN  ECAT_WORD*  pwSlaves,
    OUT ECAT_REG_DL_STATUS* pDLStatus);

/*! @brief Gets DL information register contents by position index.

    The function obtains DL information register of one or several slaves which are
    defined by auto increment indexes(positions).

    The function is used to read DL information register of one or several slaves in the network.
    The pDLInfo buffer should be interpreted as an array of #ECAT_REG_DL_INFO structures.
    The number of elements in pDLInfo array is assumed to be equal to wSlavePosAddrArrLength.
    The DL Information Register data of each slave mentioned in pwSlavePosAddrArr array
    is written to a respective block of pDLInfo array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlavePosAddrArrLength Length of slaves' auto increment (position) addresses array(i.e. number of slaves).
    @param pwSlavePosAddrArr Pointer to array that contains auto increment addresses of slaves.
    @param pDLInfo Pointer to the buffer that receives the DL Information register contents of slaves.
           (buffer size should be equal to sizeof(ECAT_REG_DL_INFO) multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetDLInfoByFixAddress
*/
ECATMKPA_API(ECAT_RESULT) EcatGetDLInfo(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlavePosAddrArrLength,
    IN  ECAT_WORD*  pwSlavePosAddrArr,
    OUT ECAT_REG_DL_INFO* pDLInfo);

/*! @brief Gets DL information register contents by fixed address.

    The function obtains DL information register of one or several slaves which are
    defined by fixed addresses.

    The function is used to read DL information register of one or several slaves in the network.
    The pDLInfo buffer should be interpreted as an array of #ECAT_REG_DL_INFO structures.
    The number of elements in pDLInfo array is assumed to be equal to wSlaveCount.
    The DL Information Register data of each slave mentioned in pwSlaves array
    is written to a respective block of pDLInfo array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Number of slaves to be asked.
    @param pwSlaves Pointer to array that contains fixed addresses of slaves.
    @param pDLInfo  Pointer to the buffer that receives the DL Information register contents of slaves.
           (buffer size should be equal to sizeof(ECAT_REG_DL_INFO) multiplied by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetDLInfo
*/
ECATMKPA_API(ECAT_RESULT) EcatGetDLInfoByFixAddress(
    IN  ECAT_HANDLE hMaster,
    IN  ECAT_WORD   wSlaveCount,
    IN  ECAT_WORD*  pwSlaves,
    OUT ECAT_REG_DL_INFO* pDLInfo);

/*! @brief Acquire content of memory by position index

    The function reads contents of memory from several slaves at once using
    auto increment (position) addresses.

    The function is used to read data from memory of several slaves in the network.
    The pData buffer should be interpreted as an array of blocks of size wDataLen.
    The number of blocks in pData array is assumed to be equal to wSlaveCount.
    The contents of memory of each slave mentioned in pwSlaves array
    is written to a respective block of pData array.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of slaves' auto increment (position)
           indexes array (i.e. number of slaves).
    @param pwSlaves Pointer to array that contains the indexes of slaves.
    @param wStartAdr Start memory address.
    @param wDataLen Length of data(number of bytes to be read from each slave).
    @param pData Pointer to the buffer that receives the data read from
           slaves (buffer size should be equal to wDataLen multiplied
           by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,#ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatWriteMemoryByPosIndex,EcatSSGetSlaveCount,
         EcatReadEEPROMByPosIndex,EcatWriteEEPROMByPosIndex,
         EcatWriteEEPROMByFixAddress
*/
ECATMKPA_API(ECAT_RESULT) EcatReadMemoryByPosIndex(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wSlaveCount,
    IN ECAT_WORD*   pwSlaves,
    IN ECAT_WORD    wStartAdr,
    IN ECAT_WORD    wDataLen,
    OUT ECAT_BYTE*  pData);

/*! @brief Write data to memory by position index.

    The function writes data to the slave's memory using
    auto increment (position) addresses.

    @ingroup master_net_state
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Length of array containing auto increment (position)
           indexes of slaves(i.e. number of slaves).
    @param pwSlaves Pointer to array that contains the indexes of slaves.
    @param wStartAdr Start memory address.
    @param wDataLen Length of data(number of bytes to be written to each slave).
    @param pData Pointer to the buffer that contains the data to be written to
           slaves (buffer size should be equal to wDataLen multiplied
           by slaves count).
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_TRANSITION_FORBIDDEN.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatSSGetSlaveCount,EcatReadMemoryByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatWriteMemoryByPosIndex(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wSlaveCount,
    IN ECAT_WORD*   pwSlaves,
    IN ECAT_WORD    wStartAdr,
    IN ECAT_WORD    wDataLen,
    IN ECAT_BYTE*   pData);
    
/*--------------------------------------------------------------------------*/

/*! @brief Acquires count of active slaves.

    The function retrieves count of Slaves in the network which match the
    current configuration.
    The acquired count may differ from that one in configuration in case
    if some hot-plug slaves are not available at the moment. May be called
    when cyclic exchange is started (through EcatStartCyclicOperation).

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pwSlaveCount Pointer to variable that receives the number
           of active slaves.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetSlaveStateByPosIndex
*/
ECATMKPA_API(ECAT_RESULT) EcatGetActiveSlaveCount(
    IN ECAT_HANDLE hMaster,
    OUT ECAT_WORD* pwSlaveCount);

/*! @brief Acquires active positions of slaves in the network.

    @ingroup master_state_control_slave
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param wSlaveCount Number of slaves to retrieve current positions in the network.
    @param pwSlaveId Pointer to array containing slaves' identifiers.
    @param pwSlavePos Pointer to array that receives current positions of active slaves.
    @return #ECAT_S_OK,#ECAT_E_INVALIDARG,#ECAT_E_FAIL,
            #ECAT_E_MASTER_NOT_CONFIGURED.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetActiveSlaveCount
*/

ECATMKPA_API(ECAT_RESULT) EcatGetSlaveActivePosIndex(
    IN ECAT_HANDLE  hMaster,
    IN ECAT_WORD    wSlaveCount,
    IN ECAT_WORD*   pwSlaveId,
    OUT ECAT_WORD*  pwSlavePos);

/*! @brief Acquires diagnostic states for master and/or slaves.

    @ingroup master_diagnostics
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pdwMasterDiagState Pointer to variable that receives the master's diagnostic state.
    See also #EcatMasterDiagnosticState.
    @param wSlaveCount Number of slaves to retrieve diagnostic information.
    @param pwSlaveId Pointer to array containing the Id's of slaves to retrieve diagnostic states.
    @param pdwSlaveDiagState Pointer to array that receives slaves' diagnostic states.
    @param pSIIContent Pointer to array that receives the SII contents of
           each slave mentioned in pwSlaveId array. Each element of
           the pSIIContent array contains Vendor ID, Product Code, Revision
           Number and Serial Number (see #ECAT_SII_CONST_CONTENT_SHORT).
    @return #ECAT_S_OK,#ECAT_E_INVALID_POINTER.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetMasterState, EcatGetSlaveCount
*/
ECATMKPA_API(ECAT_RESULT) EcatGetSlaveDiagnosticStates(
    IN ECAT_HANDLE  hMaster,
    OUT ECAT_DWORD* pdwMasterDiagState,
    IN ECAT_WORD    wSlaveCount,
    IN ECAT_WORD*   pwSlaveId,
    OUT ECAT_DWORD* pdwSlaveDiagState,
    OUT ECAT_SII_CONST_CONTENT_SHORT* pSIIContent);

/*! @brief Retrieves diagnostic information for master and/or slaves.

    @ingroup master_diagnostics
    @param hMaster Master handle (for Master handle refer to #EcatCreateMaster function).
    @param pMasterDiagInfo Pointer to structure that receives the master's diagnostic information.
    @param wSlaveCount Number of slaves to retrieve diagnostic information.
    @param pwSlaveIds Pointer to array containing the Id's of slaves to retrieve diagnostic info.
    @param pSlaveDiagInfo Pointer to array that receives slaves' diagnostic information.
    @return #ECAT_S_OK,#ECAT_E_INVALID_POINTER.\n
            For more information about result
            see EcatGetErrorMessage(), EcatGetLastError().
    @see EcatGetMasterState, EcatGetSlaveCount
*/
ECATMKPA_API(ECAT_RESULT) EcatGetDiagnostics(
    IN ECAT_HANDLE                   hMaster,
    OUT ECAT_MASTER_DIAGNOSTIC_INFO* pMasterDiagInfo,
    IN ECAT_WORD                     wSlaveCount,
    IN ECAT_WORD*                    pwSlaveIds,
    OUT ECAT_SLAVE_DIAGNOSTIC_INFO*  pSlaveDiagInfo);


    
#ifdef __cplusplus
}
#endif

