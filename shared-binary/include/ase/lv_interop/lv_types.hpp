// Types definitions for interoperation between LabVIEW and C++
// Without the dependency on NIs extcode.h or ni_extcode.h
// ** Here Be Dragons ** NI could change these at anytime

#pragma once

#include <cstdint>
#include <stddef.h>
#include <string>
#include <type_traits>

#include "./set_packing.hpp"

namespace ase
{
    namespace lv_interop
    {
        // define templates for pointers, handles and handle pointers
        template <typename T>
        using LV_Ptr_t = T *;
        template <typename T>
        using LV_Handle_t = T **;
        template <typename T>
        using LV_HandlePtr_t = T ***;

        // LabVIEW Manager Functions Error Type
        using LV_MgErr_t = int32_t;

        // LabVIEW Boolean
        using LV_Boolean_t = uint8_t;
        using LV_BooleanPtr_t = LV_Ptr_t<LV_Boolean_t>;
        constexpr auto LV_BOOLEAN_TRUE = LV_Boolean_t{1};
        constexpr auto LV_BOOLEAN_FALSE = LV_Boolean_t{0};

        // generic LabVIEW Refs type like EventRefs
        using LV_MagicCookie_t = uint32_t;
        using LV_InstanceDataPtr_t = void *;
        using LV_InstanceDataHandle_t = LV_Ptr_t<LV_InstanceDataPtr_t>;

        // LV Manager Function

        using LV_UserEventRef_t = LV_MagicCookie_t;
        using LV_UHandle_t = LV_Handle_t<uint8_t>;
        using LV_UHandlePtr_t = LV_HandlePtr_t<uint8_t>;

        // Error Codes
        // https://www.ni.com/docs/en-US/bundle/labview/page/labview-manager-function-errors.html
        const LV_MgErr_t LV_ERR_noError = 0;
        const LV_MgErr_t LV_ERR_mgArgErr = 1;
        const LV_MgErr_t LV_ERR_mFullErr = 2;
        const LV_MgErr_t LV_ERR_mZoneErr = 3;
        const LV_MgErr_t LV_ERR_fEOF = 4;
        const LV_MgErr_t LV_ERR_fIsOpen = 5;
        const LV_MgErr_t LV_ERR_fIOErr = 6;
        const LV_MgErr_t LV_ERR_fNotFound = 7;
        const LV_MgErr_t LV_ERR_fNoPerm = 8;
        const LV_MgErr_t LV_ERR_fDiskFull = 9;
        const LV_MgErr_t LV_ERR_fDupPath = 10;
        const LV_MgErr_t LV_ERR_ftMFOpen = 11;
        const LV_MgErr_t LV_ERR_fNotEnabled = 12;
        const LV_MgErr_t LV_ERR_rfNotFound = 13;
        const LV_MgErr_t LV_ERR_rAddFailed = 14;
        const LV_MgErr_t LV_ERR_rNotFound = 15;
        const LV_MgErr_t LV_ERR_iNotFound = 16;
        const LV_MgErr_t LV_ERR_iMemoryErr = 17;
        const LV_MgErr_t LV_ERR_dPenNotExist = 18;
        const LV_MgErr_t LV_ERR_cfgBadType = 19;
        const LV_MgErr_t LV_ERR_cfgTokenNotFound = 20;
        const LV_MgErr_t LV_ERR_cfgParseError = 21;
        const LV_MgErr_t LV_ERR_cfgAllocError = 22;
        const LV_MgErr_t LV_ERR_ecLVSBFormatError = 23;
        const LV_MgErr_t LV_ERR_ecLVSBSubrError = 24;
        const LV_MgErr_t LV_ERR_ecLVSBNoCodeError = 25;
        const LV_MgErr_t LV_ERR_wNullWindow = 26;
        const LV_MgErr_t LV_ERR_wDestroyMixup = 27;
        const LV_MgErr_t LV_ERR_menuNullMenu = 28;
        const LV_MgErr_t LV_ERR_pAbortJob = 29;
        const LV_MgErr_t LV_ERR_pBadPrintRecord = 30;
        const LV_MgErr_t LV_ERR_pDriverError = 31;
        const LV_MgErr_t LV_ERR_pWindowsError = 32;
        const LV_MgErr_t LV_ERR_pMemoryError = 33;
        const LV_MgErr_t LV_ERR_pDialogError = 34;
        const LV_MgErr_t LV_ERR_pMiscError = 35;
        const LV_MgErr_t LV_ERR_dvInvalidRefnum = 36;
        const LV_MgErr_t LV_ERR_dvDeviceNotFound = 37;
        const LV_MgErr_t LV_ERR_dvParamErr = 38;
        const LV_MgErr_t LV_ERR_dvUnitErr = 39;
        const LV_MgErr_t LV_ERR_dvOpenErr = 40;
        const LV_MgErr_t LV_ERR_dvAbortErr = 41;
        const LV_MgErr_t LV_ERR_bogusError = 42;
        const LV_MgErr_t LV_ERR_cancelError = 43;
        const LV_MgErr_t LV_ERR_OMObjLowErr = 44;
        const LV_MgErr_t LV_ERR_OMObjHiErr = 45;
        const LV_MgErr_t LV_ERR_OMObjNotInHeapErr = 46;
        const LV_MgErr_t LV_ERR_OMOHeapNotKnownErr = 47;
        const LV_MgErr_t LV_ERR_OMBadDPIdErr = 48;
        const LV_MgErr_t LV_ERR_OMNoDPinTabErr = 49;
        const LV_MgErr_t LV_ERR_OMMsgOutOfRangeErr = 50;
        const LV_MgErr_t LV_ERR_OMMethodNullErr = 51;
        const LV_MgErr_t LV_ERR_OMUnknownMsgErr = 52;
        const LV_MgErr_t LV_ERR_mgNotSupported = 53;
        const LV_MgErr_t LV_ERR_ncBadAddressErr = 54;
        const LV_MgErr_t LV_ERR_ncInProgress = 55;
        const LV_MgErr_t LV_ERR_ncTimeOutErr = 56;
        const LV_MgErr_t LV_ERR_ncBusyErr = 57;
        const LV_MgErr_t LV_ERR_ncNotSupportedErr = 58;
        const LV_MgErr_t LV_ERR_ncNetErr = 59;
        const LV_MgErr_t LV_ERR_ncAddrInUseErr = 60;
        const LV_MgErr_t LV_ERR_ncSysOutOfMem = 61;
        const LV_MgErr_t LV_ERR_ncSysConnAbortedErr = 62;
        const LV_MgErr_t LV_ERR_ncConnRefusedErr = 63;
        const LV_MgErr_t LV_ERR_ncNotConnectedErr = 64;
        const LV_MgErr_t LV_ERR_ncAlreadyConnectedErr = 65;
        const LV_MgErr_t LV_ERR_ncConnClosedErr = 66;
        const LV_MgErr_t LV_ERR_amInitErr = 67;
        const LV_MgErr_t LV_ERR_occBadOccurrenceErr = 68;
        const LV_MgErr_t LV_ERR_occWaitOnUnBoundHdlrErr = 69;
        const LV_MgErr_t LV_ERR_occFunnyQOverFlowErr = 70;
        const LV_MgErr_t LV_ERR_fDataLogTypeConflict = 71;
        const LV_MgErr_t LV_ERR_ecLVSBCannotBeCalledFromThread = 72;
        const LV_MgErr_t LV_ERR_amUnrecognizedType = 73;
        const LV_MgErr_t LV_ERR_mCorruptErr = 74;
        const LV_MgErr_t LV_ERR_ecLVSBErrorMakingTempDLL = 75;
        const LV_MgErr_t LV_ERR_ecLVSBOldCIN = 76;
        const LV_MgErr_t LV_ERR_fmtTypeMismatch = 81;
        const LV_MgErr_t LV_ERR_fmtUnknownConversion = 82;
        const LV_MgErr_t LV_ERR_fmtTooFew = 83;
        const LV_MgErr_t LV_ERR_fmtTooMany = 84;
        const LV_MgErr_t LV_ERR_fmtScanError = 85;
        const LV_MgErr_t LV_ERR_lvOLEConvertErr = 87;
        const LV_MgErr_t LV_ERR_rtMenuErr = 88;
        const LV_MgErr_t LV_ERR_pwdTampered = 89;
        const LV_MgErr_t LV_ERR_LvVariantAttrNotFound = 90;
        const LV_MgErr_t LV_ERR_LvVariantTypeMismatch = 91;
        const LV_MgErr_t LV_ERR_axEventDataNotAvailable = 92;
        const LV_MgErr_t LV_ERR_axEventStoreNotPresent = 93;
        const LV_MgErr_t LV_ERR_axOccurrenceNotFound = 94;
        const LV_MgErr_t LV_ERR_axEventQueueNotCreated = 95;
        const LV_MgErr_t LV_ERR_axEventInfoNotAvailable = 96;
        const LV_MgErr_t LV_ERR_oleNullRefnumPassed = 97;
        const LV_MgErr_t LV_ERR_iviInvalidDowncast = 102;
        const LV_MgErr_t LV_ERR_iviInvalidClassSesn = 103;
        const LV_MgErr_t LV_ERR_ncSockNotMulticast = 108;
        const LV_MgErr_t LV_ERR_ncSockNotSinglecast = 109;
        const LV_MgErr_t LV_ERR_ncBadMulticastAddr = 110;
        const LV_MgErr_t LV_ERR_ncMcastSockReadOnly = 111;
        const LV_MgErr_t LV_ERR_ncMcastSockWriteOnly = 112;
        const LV_MgErr_t LV_ERR_ncDatagramMsgSzErr = 113;
        const LV_MgErr_t LV_ERR_dataCorruptErr = 116;
        const LV_MgErr_t LV_ERR_requireFullPathErr = 117;
        const LV_MgErr_t LV_ERR_folderNotExistErr = 118;
        const LV_MgErr_t LV_ERR_ncBtInvalidModeErr = 119;
        const LV_MgErr_t LV_ERR_ncBtSetModeErr = 120;
        const LV_MgErr_t LV_ERR_mgBtInvalidGUIDStrErr = 121;
        const LV_MgErr_t LV_ERR_rVersInFuture = 122;
    }
}

#include "./reset_packing.hpp"