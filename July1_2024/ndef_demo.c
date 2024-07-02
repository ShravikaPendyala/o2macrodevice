/******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/*! \file
 *
 *  \author
 *
 *  \brief Demo application
 *
 *  This demo shows how to poll for several types of NFC cards/devices and how
 *  to exchange data with these devices, using the RFAL library.
 *
 *  This demo does not fully implement the activities according to the standards,
 *  it performs the required to communicate with a card/device and retrieve
 *  its UID. Also blocking methods are used for data exchange which may lead to
 *  long periods of blocking CPU/MCU.
 *  For standard compliant example please refer to the Examples provided
 *  with the RFAL library.
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "demo.h"
#include "utils.h"
#include "rfal_nfc.h"
#include "ndef_poller.h"
#include "ndef_t2t.h"
#include "ndef_t4t.h"
#include "ndef_t5t.h"
#include "ndef_message.h"
#include "ndef_types.h"
#include "ndef_dump.h"

/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT               0  /*!< Demo State:  Not initialized */
#define DEMO_ST_START_DISCOVERY       1  /*!< Demo State:  Start Discovery */
#define DEMO_ST_DISCOVERY             2  /*!< Demo State:  Discovery       */

#define NDEF_DEMO_READ              0U   /*!< NDEF menu read               */
#define NDEF_DEMO_WRITE_MSG1        1U   /*!< NDEF menu write 1 record     */
#define NDEF_DEMO_WRITE_MSG2        2U   /*!< NDEF menu write 2 records    */
#define NDEF_DEMO_FORMAT_TAG        3U   /*!< NDEF menu format tag         */
#if NDEF_FEATURE_FULL_API
#define NDEF_DEMO_MAX_FEATURES      4U   /*!< Number of menu items         */
#else
#define NDEF_DEMO_MAX_FEATURES      1U   /*!< Number of menu items         */
#endif /* NDEF_FEATURE_FULL_API */
#define NDEF_WRITE_FORMAT_TIMEOUT   10000U /*!< When write or format mode is selected, demo returns back to read mode after a timeout */
#define NDEF_LED_BLINK_DURATION       250U /*!< Led blink duration         */

#define DEMO_RAW_MESSAGE_BUF_LEN      8192 /*!< Raw message buffer len     */

#define DEMO_ST_MANUFACTURER_ID      0x02U /*!< ST Manufacturer ID         */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

/* P2P communication data */
static uint8_t NFCID3[] = {0x01, 0xFE, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
static uint8_t GB[] = {0x46, 0x66, 0x6d, 0x01, 0x01, 0x11, 0x02, 0x02, 0x07, 0x80, 0x03, 0x02, 0x00, 0x03, 0x04, 0x01, 0x32, 0x07, 0x01, 0x03};

#if RFAL_FEATURE_NFC_DEP
/* P2P communication data */
static uint8_t ndefLLCPSYMM[] = {0x00, 0x00};
static uint8_t ndefInit[] = {0x05, 0x20, 0x06, 0x0F, 0x75, 0x72, 0x6E, 0x3A, 0x6E, 0x66, 0x63, 0x3A, 0x73, 0x6E, 0x3A, 0x73, 0x6E, 0x65, 0x70, 0x02, 0x02, 0x07, 0x80, 0x05, 0x01, 0x02};
static const uint8_t ndefSnepPrefix[] = { 0x13, 0x20, 0x00, 0x10, 0x02, 0x00, 0x00, 0x00 };
static const uint8_t URL[] = "st.com/st25-demo";
static ndefConstBuffer bufURL = { URL, sizeof(URL) - 1 };
static uint8_t ndefUriBuffer[255];
#endif /* RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE
#if RFAL_SUPPORT_MODE_LISTEN_NFCA
/* NFC-A CE config */
/* 4-byte UIDs with first byte 0x08 would need random number for the subsequent 3 bytes.
 * 4-byte UIDs with first byte 0x*F are Fixed number, not unique, use for this demo
 * 7-byte UIDs need a manufacturer ID and need to assure uniqueness of the rest.*/
static uint8_t ceNFCA_NFCID[]     = {0x5F, 'S', 'T', 'M'};    /* =_STM, 5F 53 54 4D NFCID1 / UID (4 bytes) */
static uint8_t ceNFCA_SENS_RES[]  = {0x02, 0x00};             /* SENS_RES / ATQA for 4-byte UID            */
static uint8_t ceNFCA_SEL_RES     = 0x20;                     /* SEL_RES / SAK                             */
#endif /*RFAL_SUPPORT_MODE_LISTEN_NFCA */

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
/* NFC-F CE config */
static uint8_t ceNFCF_nfcid2[]     = {0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
static uint8_t ceNFCF_SC[]         = {0x12, 0xFC};
static uint8_t ceNFCF_SENSF_RES[]  = {0x01,                                                       /* SENSF_RES                                */
                                      0x02, 0xFE, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,             /* NFCID2                                   */
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x00,             /* PAD0, PAD01, MRTIcheck, MRTIupdate, PAD2 */
                                      0x00, 0x00 };                                               /* RD                                       */
#endif /*RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /*  RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */

static uint8_t *ndefStates[] =
{
    (uint8_t *)"INVALID",
    (uint8_t *)"INITIALIZED",
    (uint8_t *)"READ/WRITE",
    (uint8_t *)"READ-ONLY"
};

static const uint8_t *ndefDemoFeatureDescription[NDEF_DEMO_MAX_FEATURES] =
{
    (uint8_t *)"1. Tap a tag to read its content",
#if NDEF_FEATURE_FULL_API
    (uint8_t *)"2. Present a tag to write a Text record",
    (uint8_t *)"3. Present a tag to write a URI record and an Android Application record",
    (uint8_t *)"4. Present an ST tag to format",
#endif /* NDEF_FEATURE_FULL_API */
};

#if NDEF_FEATURE_FULL_API
static uint8_t ndefURI[]          = "st.com/st25-demo";
static uint8_t ndefTEXT[]         = "Welcome to ST NDEF demo";
static uint8_t ndefTextLangCode[] = "en";

static uint8_t ndefAndroidPackName[] = "com.st.st25nfc";
#endif /* NDEF_FEATURE_FULL_API */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfalNfcDiscoverParam discParam;
static uint8_t              state = DEMO_ST_NOTINIT;

static ndefContext          ndefCtx;
static uint8_t              ndefDemoFeature     = NDEF_DEMO_READ;
static uint8_t              ndefDemoPrevFeature = 0xFF;
static bool                 verbose             = false;

static uint8_t              rawMessageBuf[DEMO_RAW_MESSAGE_BUF_LEN];

static uint32_t             timer;
static uint32_t             timerLed;
static bool                 ledOn;

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void demoNdef(rfalNfcDevice *nfcDevice);
static void ndefCCDump(ndefContext *ctx);
#if NDEF_FEATURE_T5T
static void ndefDumpSysInfo(ndefContext *ctx);
#endif /* RFAL_FEATURE_T5T */

#if NDEF_FEATURE_FULL_API
static bool ndefIsSTTag(ndefContext *ctx);
static void LedNotificationWriteDone(void);
#endif /* NDEF_FEATURE_FULL_API */

static void demoP2P( void );
ReturnCode  demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt );

static void ledsOn(void);
static void ledsOff(void);

/*!
 *****************************************************************************
 * \brief Check user button
 *
 *  This function check whethe the user button has been pressed
 *****************************************************************************
 */

static void checkUserButton(void)
{
    /* Check if USER button is pressed */
    if( platformGpioIsLow(PLATFORM_USER_BUTTON_PORT, PLATFORM_USER_BUTTON_PIN))
    {
        ndefDemoFeature++;
        ndefDemoFeature %= NDEF_DEMO_MAX_FEATURES;

        ledsOff();
        ndefDemoPrevFeature = ndefDemoFeature;
        platformLog("%s\r\n", ndefDemoFeatureDescription[ndefDemoFeature]);
        /* Debounce button */
        while( platformGpioIsLow(PLATFORM_USER_BUTTON_PORT, PLATFORM_USER_BUTTON_PIN) );
        if( ndefDemoFeature != NDEF_DEMO_READ )
        {
            timer = platformTimerCreate(NDEF_WRITE_FORMAT_TIMEOUT);
            timerLed = platformTimerCreate(NDEF_LED_BLINK_DURATION);
        }
    }
}

/*!
 *****************************************************************************
 * \brief Show usage
 *
 *  This function displays usage information
 *****************************************************************************
 */
static void ndefShowDemoUsage()
{
#if NDEF_FEATURE_FULL_API
    uint32_t i;

    platformLog("Use the User button to cycle among the different modes:\r\n");
    for (i = 0; i < SIZEOF_ARRAY(ndefDemoFeatureDescription); i++)
    {
        platformLog("%s\r\n", ndefDemoFeatureDescription[i]);
    }
    platformLog("In Write or Format mode (menu 2, 3 or 4), the demo returns to Read mode (menu 1) if no tag detected after %d seconds\r\n\n", NDEF_WRITE_FORMAT_TIMEOUT/1000);
#endif /* NDEF_FEATURE_FULL_API */
}

/*!
 *****************************************************************************
 * \brief Demo Ini
 *
 *  This function Initializes the required layers for the demo
 *
 * \return true  : Initialization ok
 * \return false : Initialization failed
 *****************************************************************************
 */
bool demoIni( void )
{
    ReturnCode err;
    if( (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
        verbose = true;
    }
    ndefShowDemoUsage();

    err = rfalNfcInitialize();
    if( err == ERR_NONE )
    {
        rfalNfcDefaultDiscParams( &discParam );

        discParam.devLimit      = 1U;

        ST_MEMCPY( &discParam.nfcid3, NFCID3, sizeof(NFCID3) );
        ST_MEMCPY( &discParam.GB, GB, sizeof(GB) );
        discParam.GBLen         = sizeof(GB);
        discParam.p2pNfcaPrio   = true;

        discParam.notifyCb             = NULL;
        discParam.totalDuration        = 0U;
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;          /* For the demo, enable the NFC Technlogies based on RFAL Feature switches */

#if RFAL_FEATURE_NFCA
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_A;
#endif /* RFAL_FEATURE_NFCA */

#if RFAL_FEATURE_NFCB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_B;
#endif /* RFAL_FEATURE_NFCB */

#if RFAL_FEATURE_NFCF
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_F;
#endif /* RFAL_FEATURE_NFCF */

#if RFAL_FEATURE_NFCV
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_V;
#endif /* RFAL_FEATURE_NFCV */

#if RFAL_FEATURE_ST25TB
        discParam.techs2Find          |= RFAL_NFC_POLL_TECH_ST25TB;
#endif /* RFAL_FEATURE_ST25TB */

#if ST25R95
        discParam.isoDepFS           = RFAL_ISODEP_FSXI_128;          /* ST25R95 cannot support 256 bytes of data block */
#endif /* ST25R95 */

#if RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP
        discParam.techs2Find |= RFAL_NFC_POLL_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_POLL_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP */

#if RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE
        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_AP2P;
#endif /* RFAL_SUPPORT_MODE_LISTEN_ACTIVE_P2P && RFAL_FEATURE_NFC_DEP && RFAL_FEATURE_LISTEN_MODE */

#if DEMO_CARD_EMULATION_ONLY
        discParam.totalDuration        = 60000U;              /* 60 seconds */
        discParam.techs2Find           = RFAL_NFC_TECH_NONE;  /* Overwrite any previous poller modes */
#endif /* DEMO_CARD_EMULATION_ONLY */

#if RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE

#if RFAL_SUPPORT_MODE_LISTEN_NFCA
        /* Set configuration for NFC-A CE */
        ST_MEMCPY( discParam.lmConfigPA.SENS_RES, ceNFCA_SENS_RES, RFAL_LM_SENS_RES_LEN );     /* Set SENS_RES / ATQA */
        ST_MEMCPY( discParam.lmConfigPA.nfcid, ceNFCA_NFCID, RFAL_LM_NFCID_LEN_04 );           /* Set NFCID / UID */
        discParam.lmConfigPA.nfcidLen = RFAL_LM_NFCID_LEN_04;                                  /* Set NFCID length to 7 bytes */
        discParam.lmConfigPA.SEL_RES  = ceNFCA_SEL_RES;                                        /* Set SEL_RES / SAK */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_A;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCA */

#if RFAL_SUPPORT_MODE_LISTEN_NFCF
        /* Set configuration for NFC-F CE */
        ST_MEMCPY( discParam.lmConfigPF.SC, ceNFCF_SC, RFAL_LM_SENSF_SC_LEN );                 /* Set System Code */
        ST_MEMCPY( &ceNFCF_SENSF_RES[RFAL_NFCF_CMD_LEN], ceNFCF_nfcid2, RFAL_NFCID2_LEN );     /* Load NFCID2 on SENSF_RES */
        ST_MEMCPY( discParam.lmConfigPF.SENSF_RES, ceNFCF_SENSF_RES, RFAL_LM_SENSF_RES_LEN );  /* Set SENSF_RES / Poll Response */

        discParam.techs2Find |= RFAL_NFC_LISTEN_TECH_F;
#endif /* RFAL_SUPPORT_MODE_LISTEN_NFCF */
#endif /* RFAL_SUPPORT_CE && RFAL_FEATURE_LISTEN_MODE */

        /* Check for valid configuration by calling Discover once */
        err = rfalNfcDiscover( &discParam );
        rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_IDLE );

        if( err != ERR_NONE )
        {
            return false;
        }

        state = DEMO_ST_START_DISCOVERY;
        return true;
    }
    return false;
}

/*!
 *****************************************************************************
 * \brief Demo Cycle
 *
 *  This function executes the demo state machine.
 *  It must be called periodically
 *****************************************************************************
 */
void demoCycle( void )
{
    rfalNfcWorker();                                    /* Run RFAL worker periodically */

    rfalNfcDiscover( &discParam );

}

/*!
 *****************************************************************************
 * \brief Demo P2P Exchange
 *
 * Sends a NDEF URI record 'http://www.ST.com' via NFC-DEP (P2P) protocol.
 *
 * This method sends a set of static predefined frames which tries to establish
 * a LLCP connection, followed by the NDEF record, and then keeps sending
 * LLCP SYMM packets to maintain the connection.
 *
 *
 *****************************************************************************
 */
void demoP2P( void )
{
#if RFAL_FEATURE_NFC_DEP

    uint16_t   *rxLen;
    uint8_t    *rxData;
    ReturnCode err;

    ndefBuffer  bufPayload;
    ndefMessage message;
    ndefRecord  record;
    ndefType    uri;

    platformLog(" Initialize device .. ");
    err = demoTransceiveBlocking( ndefInit, sizeof(ndefInit), &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != ERR_NONE )
    {
        platformLog("failed.");
        return;
    }
    platformLog("succeeded.\r\n");

    err  = ndefRtdUriInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufURL);
    err |= ndefRtdUriToRecord(&uri, &record);

    err |= ndefMessageInit(&message);
    err |= ndefMessageAppend(&message, &record);  /* To get MB and ME bits set */

    /* Build the SNEP buffer made of the prefix, the length byte and the record */
    ST_MEMCPY(ndefUriBuffer, ndefSnepPrefix, sizeof(ndefSnepPrefix));

    /* Skip 1 byte for length byte */
    bufPayload.buffer = ndefUriBuffer + sizeof(ndefSnepPrefix) + 1;
    bufPayload.length = sizeof(ndefUriBuffer) - sizeof(ndefSnepPrefix);
    err |= ndefMessageEncode(&message, &bufPayload);

    ndefUriBuffer[sizeof(ndefSnepPrefix)] = bufPayload.length;

    bufPayload.buffer = ndefUriBuffer;
    bufPayload.length = sizeof(ndefSnepPrefix) + 1 + bufPayload.length;

    if( err != ERR_NONE )
    {
        platformLog("NDEF message creation failed (%d)\r\n", err);
        return;
    }

    ndefBufferDump("URL converted to SNEP:\r\n", (ndefConstBuffer*)&bufPayload, true);

    platformLog(" Push NDEF Uri: st.com/st25-demo ...");
    err = demoTransceiveBlocking(bufPayload.buffer, bufPayload.length, &rxData, &rxLen, RFAL_FWT_NONE);
    if( err != ERR_NONE )
    {
        platformLog("failed.");
        return;
    }
    platformLog("succeeded.\r\n");

    platformLog(" Device present, maintaining connection ");
    while(err == ERR_NONE)
    {
        err = demoTransceiveBlocking( ndefLLCPSYMM, sizeof(ndefLLCPSYMM), &rxData, &rxLen, RFAL_FWT_NONE);
        platformLog(".");
        platformDelay(50);
    }
    platformLog("\r\n Device removed.\r\n");
#endif /* RFAL_FEATURE_NFC_DEP */
}

/*!
 *****************************************************************************
 * \brief Demo Blocking Transceive
 *
 * Helper function to send data in a blocking manner via the rfalNfc module
 *
 * \warning A protocol transceive handles long timeouts (several seconds),
 * transmission errors and retransmissions which may lead to a long period of
 * time where the MCU/CPU is blocked in this method.
 * This is a demo implementation, for a non-blocking usage example please
 * refer to the Examples available with RFAL
 *
 * \param[in]  txBuf      : data to be transmitted
 * \param[in]  txBufSize  : size of the data to be transmited
 * \param[out] rxData     : location where the received data has been placed
 * \param[out] rcvLen     : number of data bytes received
 * \param[in]  fwt        : FWT to be used (only for RF frame interface,
 *                                          otherwise use RFAL_FWT_NONE)
 *
 *
 *  \return ERR_PARAM     : Invalid parameters
 *  \return ERR_TIMEOUT   : Timeout error
 *  \return ERR_FRAMING   : Framing error detected
 *  \return ERR_PROTO     : Protocol error detected
 *  \return ERR_NONE      : No error, activation successful
 *
 *****************************************************************************
 */
ReturnCode demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt )
{
    ReturnCode err;

    err = rfalNfcDataExchangeStart( txBuf, txBufSize, rxData, rcvLen, fwt );
    if( err == ERR_NONE )
    {
        do{
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        }
        while( err == ERR_BUSY );
    }
    return err;
}

static void demoNdef(rfalNfcDevice *pNfcDevice)
{
    ReturnCode       err;
    ndefMessage      message;
    uint32_t         rawMessageLen;
    ndefInfo         info;
    ndefBuffer       bufRawMessage;
    ndefConstBuffer  bufConstRawMessage;

#if NDEF_FEATURE_FULL_API
    ndefRecord       record1;
    ndefRecord       record2;

    ndefType         text;
    ndefType         uri;
    ndefType         aar;

    ndefConstBuffer8 bufTextLangCode;
    ndefConstBuffer bufTextLangText;
    ndefConstBuffer bufUri;
    ndefConstBuffer bufAndroidPackName;
#endif /* NDEF_FEATURE_FULL_API */

    /*
     * Perform NDEF Context Initialization
     */
    err = ndefPollerContextInitialization(&ndefCtx, pNfcDevice);
    if( err != ERR_NONE )
    {
        platformLog("NDEF NOT DETECTED (ndefPollerContextInitialization returns %d)\r\n", err);
        return;
    }

    #if NDEF_FEATURE_T5T
    if( verbose && (pNfcDevice->type == RFAL_NFC_LISTEN_TYPE_NFCV) )
    {
        ndefDumpSysInfo(&ndefCtx);
    }
    #endif /* RFAL_FEATURE_T5T */

    /*
     * Perform NDEF Detect procedure
     */
    err = ndefPollerNdefDetect(&ndefCtx, &info);
    if( err != ERR_NONE )
    {
        platformLog("NDEF NOT DETECTED (ndefPollerNdefDetect returns %d)\r\n", err);
        if( ndefDemoFeature != NDEF_DEMO_FORMAT_TAG)
        {
            return;
        }
    }
    else
    {
        platformLog("%s NDEF detected.\r\n", ndefStates[info.state]);
        ndefCCDump(&ndefCtx);

        if( verbose )
        {
            platformLog("NDEF Len: %d, Offset=%d\r\n", ndefCtx.messageLen, ndefCtx.messageOffset);
        }
    }

    switch( ndefDemoFeature )
    {
        /*
         * Demonstrate how to read the NDEF message from the Tag
         */
        case NDEF_DEMO_READ:
            if( info.state == NDEF_STATE_INITIALIZED )
            {
                /* Nothing to read... */
                return;
            }
            err = ndefPollerReadRawMessage(&ndefCtx, rawMessageBuf, sizeof(rawMessageBuf), &rawMessageLen, true);
            if( err != ERR_NONE )
            {
                platformLog("NDEF message cannot be read (ndefPollerReadRawMessage returns %d)\r\n", err);
                return;
            }
            if( verbose )
            {
                bufRawMessage.buffer = rawMessageBuf;
                bufRawMessage.length = rawMessageLen;
                ndefBufferDump(" NDEF Content", (ndefConstBuffer*)&bufRawMessage, verbose);
            }
            bufConstRawMessage.buffer = rawMessageBuf;
            bufConstRawMessage.length = rawMessageLen;
            err = ndefMessageDecode(&bufConstRawMessage, &message);
            if( err != ERR_NONE )
            {
                platformLog("NDEF message cannot be decoded (ndefMessageDecode returns %d)\r\n", err);
                return;
            }
            err = ndefMessageDump(&message, verbose);
            if( err != ERR_NONE )
            {
                platformLog("NDEF message cannot be displayed (ndefMessageDump returns %d)\r\n", err);
                return;
            }
            break;

#if NDEF_FEATURE_FULL_API
        /*
         * Demonstrate how to encode a text record and write the message to the tag
         */
        case NDEF_DEMO_WRITE_MSG1:
            ndefDemoFeature = NDEF_DEMO_READ; /* returns to READ mode after write */
            err  = ndefMessageInit(&message); /* Initialize message structure */
            bufTextLangCode.buffer = ndefTextLangCode;
            bufTextLangCode.length = strlen((char *)ndefTextLangCode);

            bufTextLangText.buffer = ndefTEXT;
            bufTextLangText.length = strlen((char *)ndefTEXT);

            err |= ndefRtdTextInit(&text, TEXT_ENCODING_UTF8, &bufTextLangCode, &bufTextLangText); /* Initialize Text type structure */
            err |= ndefRtdTextToRecord(&text, &record1); /* Encode Text Record */
            err |= ndefMessageAppend(&message, &record1); /* Append Text record to message */
            if( err != ERR_NONE )
            {
                platformLog("Message creation failed\r\n", err);
                return;
            }
            err = ndefPollerWriteMessage(&ndefCtx, &message); /* Write message */
            if( err != ERR_NONE )
            {
                platformLog("Message cannot be written (ndefPollerWriteMessage return %d)\r\n", err);
                return;
            }
            platformLog("Wrote 1 record to the Tag\r\n");
            if( verbose )
            {
                /* Dump raw message */
                bufRawMessage.buffer = rawMessageBuf;
                bufRawMessage.length = sizeof(rawMessageBuf);
                err = ndefMessageEncode(&message, &bufRawMessage);
                if( err == ERR_NONE )
                {
                    ndefBufferDump("Raw message", (ndefConstBuffer*)&bufRawMessage, verbose);
                }
            }
            LedNotificationWriteDone();
            break;

        /*
         * Demonstrate how to encode a URI record and a AAR record, how to encode the message to a raw buffer and then how to write the raw buffer
         */
        case NDEF_DEMO_WRITE_MSG2:
            ndefDemoFeature = NDEF_DEMO_READ;  /* returns to READ mode after write */
            err  = ndefMessageInit(&message);  /* Initialize message structure */
            bufUri.buffer = ndefURI;
            bufUri.length = strlen((char *)ndefURI);
            err |= ndefRtdUriInit(&uri, NDEF_URI_PREFIX_HTTP_WWW, &bufUri); /* Initialize URI type structure */
            err |= ndefRtdUriToRecord(&uri, &record1); /* Encode URI Record */

            bufAndroidPackName.buffer = ndefAndroidPackName;
            bufAndroidPackName.length = sizeof(ndefAndroidPackName) - 1U;
            err |= ndefRtdAarInit(&aar, &bufAndroidPackName); /* Initialize AAR type structure */
            err |= ndefRtdAarToRecord(&aar, &record2); /* Encode AAR record */

            err |= ndefMessageAppend(&message, &record1); /* Append URI to message */
            err |= ndefMessageAppend(&message, &record2); /* Append AAR to message (record #2 is an example of preformatted record) */

            bufRawMessage.buffer = rawMessageBuf;
            bufRawMessage.length = sizeof(rawMessageBuf);
            err |= ndefMessageEncode(&message, &bufRawMessage); /* Encode the message to the raw buffer */
            if( err != ERR_NONE )
            {
                platformLog("Raw message creation failed (%d)\r\n", err);
                return;
            }
            err = ndefPollerWriteRawMessage(&ndefCtx, bufRawMessage.buffer, bufRawMessage.length);
            if( err != ERR_NONE )
            {
                platformLog("Message cannot be written (ndefPollerWriteRawMessage return %d)\r\n", err);
                return;
            }
            platformLog("Wrote 2 records to the Tag\r\n");
            if( verbose )
            {
                /* Dump raw message */
                ndefBufferDump("Raw message", (ndefConstBuffer*)&bufRawMessage, verbose);
            }
            LedNotificationWriteDone();
            break;

        /*
         * Demonstrate how to format a Tag
         */
        case NDEF_DEMO_FORMAT_TAG:
            ndefDemoFeature = NDEF_DEMO_READ;
            if( !ndefIsSTTag(&ndefCtx) )
            {
                platformLog("Manufacturer ID not found or not an ST tag. Format aborted \r\n");
                return;
            }
            platformLog("Formatting Tag...\r\n");
            /* Format Tag */
            err = ndefPollerTagFormat(&ndefCtx, NULL, 0);
            if( err != ERR_NONE )
            {
                platformLog("Tag cannot be formatted (ndefPollerTagFormat returns %d)\r\n", err);
                return;
            }
            platformLog("Tag formatted\r\n");
            LedNotificationWriteDone();
            break;
#endif /* NDEF_FEATURE_FULL_API */

        default:
            ndefDemoFeature = NDEF_DEMO_READ;
            break;
    }
    return;
}

#if NDEF_FEATURE_T2T
static void ndefT2TCCDump(ndefContext *ctx)
{
    ndefConstBuffer bufCcBuf;

    platformLog(" * Magic: %2.2Xh Version: %d.%d Size: %d (%d bytes) \r\n * readAccess: %2.2xh writeAccess: %2.2xh \r\n", ctx->cc.t2t.magicNumber, ctx->cc.t2t.majorVersion, ctx->cc.t2t.minorVersion, ctx->cc.t2t.size, ctx->cc.t2t.size * 8U, ctx->cc.t2t.readAccess, ctx->cc.t2t.writeAccess);
    bufCcBuf.buffer = ctx->ccBuf;
    bufCcBuf.length = 4;
    ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);

}
#endif /* NDEF_FEATURE_T2T */

#if NDEF_FEATURE_T3T
static void ndefT3TAIBDump(ndefContext *ctx)
{
    ndefConstBuffer bufCcBuf;

    platformLog(" * Version: %d.%d Size: %d (%d bytes) NbR: %d NbW: %d\r\n * WriteFlag: %2.2xh RWFlag: %2.2xh \r\n", ctx->cc.t3t.majorVersion, ctx->cc.t3t.minorVersion, ctx->cc.t3t.nMaxB, ctx->cc.t3t.nMaxB * 16U, ctx->cc.t3t.nbR, ctx->cc.t3t.nbW, ctx->cc.t3t.writeFlag, ctx->cc.t3t.rwFlag);
    bufCcBuf.buffer = ctx->ccBuf;
    bufCcBuf.length = 16;
    ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T3T */

#if NDEF_FEATURE_T4T
static void ndefT4TCCDump(ndefContext *ctx)
{
    ndefConstBuffer bufCcBuf;

    platformLog(" * CCLEN: %d T4T_VNo: %xh MLe: %d MLc: %d FileId: %2.2x%2.2xh FileSize: %d\r\n * readAccess: %2.2xh writeAccess: %2.2xh\r\n", ctx->cc.t4t.ccLen, ctx->cc.t4t.vNo, ctx->cc.t4t.mLe, ctx->cc.t4t.mLc, ctx->cc.t4t.fileId[0], ctx->cc.t4t.fileId[1],ctx->cc.t4t.fileSize, ctx->cc.t4t.readAccess, ctx->cc.t4t.writeAccess);
    bufCcBuf.buffer = ctx->ccBuf;
    bufCcBuf.length = ctx->cc.t4t.ccLen;
    ndefBufferDump(" CC File Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T4T */

#if NDEF_FEATURE_T5T
static void ndefT5TCCDump(ndefContext *ctx)
{
    ndefConstBuffer bufCcBuf;

    platformLog(" * Block Length: %d\r\n", ctx->subCtx.t5t.blockLen);
    platformLog(" * %d bytes CC\r\n * Magic: %2.2Xh Version: %d.%d MLEN: %d (%d bytes) \r\n * readAccess: %2.2xh writeAccess: %2.2xh \r\n", ctx->cc.t5t.ccLen, ctx->cc.t5t.magicNumber, ctx->cc.t5t.majorVersion, ctx->cc.t5t.minorVersion, ctx->cc.t5t.memoryLen, ctx->cc.t5t.memoryLen * 8U, ctx->cc.t5t.readAccess, ctx->cc.t5t.writeAccess);
    platformLog(" * [%c] Special Frame\r\n",       ctx->cc.t5t.specialFrame ?      'X' : ' ');
    platformLog(" * [%c] Multiple block Read\r\n", ctx->cc.t5t.multipleBlockRead ? 'X' : ' ');
    platformLog(" * [%c] Lock Block\r\n",          ctx->cc.t5t.lockBlock ?         'X' : ' ');
    bufCcBuf.buffer = ctx->ccBuf;
    bufCcBuf.length = ctx->cc.t5t.ccLen;
    ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);
}
#endif /* NDEF_FEATURE_T5T */

static void ndefCCDump(ndefContext *ctx)
{
    if( (ctx == NULL) || !verbose)
    {
        return;
    }
    platformLog("%s", (ctx->type == NDEF_DEV_T3T) ? "NDEF Attribute Information Block\r\n" : "NDEF Capability Container\r\n");
    switch( ctx->type )
    {
        #if NDEF_FEATURE_T2T
        case NDEF_DEV_T2T:
            ndefT2TCCDump(ctx);
            break;
        #endif /* NDEF_FEATURE_T2T */
        #if NDEF_FEATURE_T3T
        case NDEF_DEV_T3T:
            ndefT3TAIBDump(ctx);
            break;
        #endif /* NDEF_FEATURE_T3T */
        #if NDEF_FEATURE_T4T
        case NDEF_DEV_T4T:
            ndefT4TCCDump(ctx);
            break;
        #endif /* NDEF_FEATURE_T4T */
        #if NDEF_FEATURE_T5T
        case NDEF_DEV_T5T:
            ndefT5TCCDump(ctx);
            break;
        #endif /* NDEF_FEATURE_T5T */
        default:
            break;
    }
}

#if NDEF_FEATURE_T5T
static void ndefDumpSysInfo(ndefContext *ctx)
{
    ndefSystemInformation *sysInfo;

    if( (ctx == NULL) || !verbose)
    {
        return;
    }

    if( !ctx->subCtx.t5t.sysInfoSupported )
    {
        return;
    }

    sysInfo = &ctx->subCtx.t5t.sysInfo;
    platformLog("System Information\r\n");
    platformLog(" * %d byte(s) memory addressing\r\n", ndefT5TSysInfoMOIValue(sysInfo->infoFlags) + 1);
    if( ndefT5TSysInfoDFSIDPresent(sysInfo->infoFlags) )
    {
        platformLog(" * DFSID=%2.2Xh\r\n", sysInfo->DFSID);
    }
    if( ndefT5TSysInfoAFIPresent(sysInfo->infoFlags) )
    {
        platformLog(" * AFI=%2.2Xh\r\n", sysInfo->AFI);
    }
    if( ndefT5TSysInfoMemSizePresent(sysInfo->infoFlags) )
    {
        platformLog(" * %d blocks, %d bytes per block\r\n", sysInfo->numberOfBlock, sysInfo->blockSize);
    }
    if( ndefT5TSysInfoICRefPresent(sysInfo->infoFlags) )
    {
        platformLog(" * ICRef=%2.2xh\r\n", sysInfo->ICRef);
    }
    if( ndefT5TSysInfoCmdListPresent(sysInfo->infoFlags) )
    {
        platformLog(" * [%c] ReadSingleBlock                \r\n", ndefT5TSysInfoReadSingleBlockSupported(sysInfo->supportedCmd)                 ? 'X' : ' ');
        platformLog(" * [%c] WriteSingleBlock               \r\n", ndefT5TSysInfoWriteSingleBlockSupported(sysInfo->supportedCmd)                ? 'X' : ' ');
        platformLog(" * [%c] LockSingleBlock                \r\n", ndefT5TSysInfoLockSingleBlockSupported(sysInfo->supportedCmd)                 ? 'X' : ' ');
        platformLog(" * [%c] ReadMultipleBlocks             \r\n", ndefT5TSysInfoReadMultipleBlocksSupported(sysInfo->supportedCmd)              ? 'X' : ' ');
        platformLog(" * [%c] WriteMultipleBlocks            \r\n", ndefT5TSysInfoWriteMultipleBlocksSupported(sysInfo->supportedCmd)             ? 'X' : ' ');
        platformLog(" * [%c] Select                         \r\n", ndefT5TSysInfoSelectSupported(sysInfo->supportedCmd)                          ? 'X' : ' ');
        platformLog(" * [%c] ResetToReady                   \r\n", ndefT5TSysInfoResetToReadySupported(sysInfo->supportedCmd)                    ? 'X' : ' ');
        platformLog(" * [%c] GetMultipleBlockSecStatus      \r\n", ndefT5TSysInfoGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd)       ? 'X' : ' ');
        platformLog(" * [%c] WriteAFI                       \r\n", ndefT5TSysInfoWriteAFISupported(sysInfo->supportedCmd)                        ? 'X' : ' ');
        platformLog(" * [%c] LockAFI                        \r\n", ndefT5TSysInfoLockAFISupported(sysInfo->supportedCmd)                         ? 'X' : ' ');
        platformLog(" * [%c] WriteDSFID                     \r\n", ndefT5TSysInfoWriteDSFIDSupported(sysInfo->supportedCmd)                      ? 'X' : ' ');
        platformLog(" * [%c] LockDSFID                      \r\n", ndefT5TSysInfoLockDSFIDSupported(sysInfo->supportedCmd)                       ? 'X' : ' ');
        platformLog(" * [%c] GetSystemInformation           \r\n", ndefT5TSysInfoGetSystemInformationSupported(sysInfo->supportedCmd)            ? 'X' : ' ');
        platformLog(" * [%c] CustomCmds                     \r\n", ndefT5TSysInfoCustomCmdsSupported(sysInfo->supportedCmd)                      ? 'X' : ' ');
        platformLog(" * [%c] FastReadMultipleBlocks         \r\n", ndefT5TSysInfoFastReadMultipleBlocksSupported(sysInfo->supportedCmd)          ? 'X' : ' ');
        platformLog(" * [%c] ExtReadSingleBlock             \r\n", ndefT5TSysInfoExtReadSingleBlockSupported(sysInfo->supportedCmd)              ? 'X' : ' ');
        platformLog(" * [%c] ExtWriteSingleBlock            \r\n", ndefT5TSysInfoExtWriteSingleBlockSupported(sysInfo->supportedCmd)             ? 'X' : ' ');
        platformLog(" * [%c] ExtLockSingleBlock             \r\n", ndefT5TSysInfoExtLockSingleBlockSupported(sysInfo->supportedCmd)              ? 'X' : ' ');
        platformLog(" * [%c] ExtReadMultipleBlocks          \r\n", ndefT5TSysInfoExtReadMultipleBlocksSupported(sysInfo->supportedCmd)           ? 'X' : ' ');
        platformLog(" * [%c] ExtWriteMultipleBlocks         \r\n", ndefT5TSysInfoExtWriteMultipleBlocksSupported(sysInfo->supportedCmd)          ? 'X' : ' ');
        platformLog(" * [%c] ExtGetMultipleBlockSecStatus   \r\n", ndefT5TSysInfoExtGetMultipleBlockSecStatusSupported(sysInfo->supportedCmd)    ? 'X' : ' ');
        platformLog(" * [%c] FastExtendedReadMultipleBlocks \r\n", ndefT5TSysInfoFastExtendedReadMultipleBlocksSupported(sysInfo->supportedCmd)  ? 'X' : ' ');
    }
    return;
}
#endif /* NDEF_FEATURE_T5T */

#if NDEF_FEATURE_FULL_API
static bool ndefIsSTTag(ndefContext *ctx)
{
    bool ret = false;

#if defined(STM32L476xx) /* Enable to format any manufacturer tag while debugging */
    if( (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0)
    {
        ret = true;
    }
#endif
    if( ctx == NULL )
    {
        return ret;
    }
    switch (ctx->device.type)
    {
        case RFAL_NFC_LISTEN_TYPE_NFCA:
            if( (ctx->device.dev.nfca.nfcId1Len != 4) && (ctx->device.dev.nfca.nfcId1[0] == 0x02 ) )
            {
                ret = true;
            }
            break;
        case RFAL_NFC_LISTEN_TYPE_NFCF:
            break;
        case RFAL_NFC_LISTEN_TYPE_NFCB:
            break;
        case RFAL_NFC_LISTEN_TYPE_NFCV:
            if( ctx->device.dev.nfcv.InvRes.UID[6] == 0x02 )
            {
                ret = true;
            }
            break;
        default:
            break;
    }
    return (ret);
}
#endif /* NDEF_FEATURE_FULL_API */

#if NDEF_FEATURE_FULL_API
static void LedNotificationWriteDone(void)
{
    uint32_t i;

    for (i = 0; i < 3; i++)
    {
        ledsOn();
        platformDelay(100);

        ledsOff();
        platformDelay(100);
    }
}
#endif /* NDEF_FEATURE_FULL_API */

static void ledsOn(void)
{
    platformLedOn(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
    platformLedOn(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
    platformLedOn(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
    platformLedOn(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
    platformLedOn(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
    platformLedOn(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);
}

static void ledsOff(void)
{
    platformLedOff(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
    platformLedOff(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
    platformLedOff(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
    platformLedOff(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
    platformLedOff(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
    platformLedOff(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);
}

