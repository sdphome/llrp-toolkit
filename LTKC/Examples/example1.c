
/*
 ***************************************************************************
 *  Copyright 2007,2008 Impinj, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ***************************************************************************
 */

/**
 *****************************************************************************
 **
 ** @file  example.c
 **
 ** @brief Simple example of LTKC that talks to an actual reader
 **
 ** This is sometimes called the "LLRP Hello World" program
 **
 ** The steps:
 **     - Connect to the reader (TCP)
 **     - Make sure the connection status is good
 **     - Clear (scrub) the reader configuration
 **     - Add and enable a ROSpec that does a simple operation
 **         - Uses mostly default settings except for ROReportSpec
 **         - Uses all antennas
 **         - Starts on command
 **         - Runs for 5 seconds
 **         - Reports all accumulated tag observations
 **     - Run the ROSpec 5 times
 **     - Clear (scrub) the reader configuration
 **
 ** This program can be run with zero, one, or two verbose options (-v).
 **      no -v -- Only prints the tag report and errors
 **      -v    -- Also prints one line progress messages
 **      -vv   -- Also prints all LLRP messages as XML text
 **
 ** IMPORTANT:
 **     This example was written before best practices were determined.
 **     Most of the command messages (including subparameters) are
 **     composed using local (auto) variables with initializers.
 **     It has been determined that this approach has drawbacks.
 **     Best practice is to use the constructor and accessors.
 **
 **     Function deleteAllROSpecs() was recoded to demonstrate
 **     good technique. Someday we might be able to fix the rest.
 **
 *****************************************************************************/


#include <stdio.h>

#include "ltkc.h"


/*
 * BEGIN forward declarations
 */
int
main (
  int                           ac,
  char *                        av[]);

void
usage (
  char *                        pProgName);

int
run (
  const char *                  pReaderHostName);

int
checkConnectionStatus (void);

int
scrubConfiguration (void);

int
resetConfigurationToFactoryDefaults (void);

int
deleteAllROSpecs (void);

int
addROSpec (void);

int
enableROSpec (void);

int
startROSpec (void);

int
awaitAndPrintReport (void);

void
printTagReportData (
  LLRP_tSRO_ACCESS_REPORT *     pRO_ACCESS_REPORT);

void
printOneTagReportData (
  LLRP_tSTagReportData *        pTagReportData);

void
handleReaderEventNotification (
  LLRP_tSReaderEventNotificationData *pNtfData);

void
handleAntennaEvent (
  LLRP_tSAntennaEvent *         pAntennaEvent);

void
handleReaderExceptionEvent (
  LLRP_tSReaderExceptionEvent * pReaderExceptionEvent);

int
checkLLRPStatus (
  LLRP_tSLLRPStatus *           pLLRPStatus,
  char *                        pWhatStr);

LLRP_tSMessage *
transact (
  LLRP_tSMessage *              pSendMsg);

LLRP_tSMessage *
recvMessage (
  int                           nMaxMS);

int
sendMessage (
  LLRP_tSMessage *              pSendMsg);

void
freeMessage (
  LLRP_tSMessage *              pMessage);

void
printXMLMessage (
  LLRP_tSMessage *              pMessage);

/*
 * END forward declarations
 */


/*
 * Global variables
 */
/** Verbose level, incremented by each -v on command line */
int                             g_Verbose;
/** Connection to the LLRP reader */
LLRP_tSConnection *             g_pConnectionToReader;



/**
 *****************************************************************************
 **
 ** @brief  Command main routine
 **
 ** Command synopsis:
 **
 **     dx201 READERHOSTNAME
 **
 ** @exitcode   0               Everything *seemed* to work.
 **             1               Bad usage
 **             2               Run failed
 **
 *****************************************************************************/

int
main (
  int                           ac,
  char *                        av[])
{
    char *                      pReaderHostName;
    int                         rc;

    /*
     * Process comand arguments, determine reader name
     * and verbosity level.
     */
    if(ac == 2)
    {
        pReaderHostName = av[1];
    }
    else if(ac == 3)
    {
        char *                  p = av[1];

        while(*p)
        {
            switch(*p++)
            {
            case '-':   /* linux conventional option warn char */
            case '/':   /* Windows/DOS conventional option warn char */
                break;

            case 'v':
            case 'V':
                g_Verbose++;
                break;

            default:
                usage(av[0]);
                /* no return */
                break;
            }
        }

        pReaderHostName = av[2];
    }
    else
    {
        usage(av[0]);
        /* no return */
    }

    /*
     * Run application, capture return value for exit status
     */
    rc = run(pReaderHostName);

    printf("INFO: Done\n");

    /*
     * Exit with the right status.
     */
    if(0 == rc)
    {
        exit(0);
    }
    else
    {
        exit(2);
    }
    /*NOTREACHED*/
}


/**
 *****************************************************************************
 **
 ** @brief  Print usage message and exit
 **
 ** @param[in]  nProgName       Program name string
 **
 ** @return     none, exits
 **
 *****************************************************************************/

void
usage (
  char *                        pProgName)
{
    printf("Usage: %s [-v] READERHOSTNAME\n", pProgName);
    printf("\n");
    printf("Each -v increases verbosity level\n");
    exit(1);
}


/**
 *****************************************************************************
 **
 ** @brief  Run the application
 **
 ** The steps:
 **     - Instantiate connection
 **     - Connect to LLRP reader (TCP)
 **     - Make sure the connection status is good
 **     - Clear (scrub) the reader configuration
 **     - Configure for what we want to do
 **     - Run inventory operation 5 times
 **     - Again, clear (scrub) the reader configuration
 **     - Disconnect from reader
 **     - Destruct connection
 **
 ** @param[in]  pReaderHostName String with reader name
 **
 ** @return      0              Everything worked.
 **             -1              Failed allocation of type registry
 **             -2              Failed construction of connection
 **             -3              Could not connect to reader
 **              1              Reader connection status bad
 **              2              Cleaning reader config failed
 **              3              Adding ROSpec failed
 **              4              Enabling ROSpec failed
 **              5              Something went wrong running the ROSpec
 **
 *****************************************************************************/

int
run (
  const char *                  pReaderHostName)
{
    LLRP_tSTypeRegistry *       pTypeRegistry;
    LLRP_tSConnection *         pConn;
    int                         rc;

    /*
     * Allocate the type registry. This is needed
     * by the connection to decode.
     */
    pTypeRegistry = LLRP_getTheTypeRegistry();
    if(NULL == pTypeRegistry)
    {
        printf("ERROR: getTheTypeRegistry failed\n");
        return -1;
    }

    /*
     * Construct a connection (LLRP_tSConnection).
     * Using a 32kb max frame size for send/recv.
     * The connection object is ready for business
     * but not actually connected to the reader yet.
     */
    pConn = LLRP_Conn_construct(pTypeRegistry, 32u*1024u);
    if(NULL == pConn)
    {
        printf("ERROR: Conn_construct failed\n");
        return -2;
    }

    /*
     * Open the connection to the reader
     */
    if(g_Verbose)
    {
        printf("INFO: Connecting to %s....\n", pReaderHostName);
    }

    rc = LLRP_Conn_openConnectionToReader(pConn, pReaderHostName);
    if(0 != rc)
    {
        printf("ERROR: connect: %s (%d)\n", pConn->pConnectErrorStr, rc);
        LLRP_Conn_destruct(pConn);
        return -3;
    }

    /*
     * Record the pointer to the connection object so other
     * routines can use it.
     */
    g_pConnectionToReader = pConn;

    if(g_Verbose)
    {
        printf("INFO: Connected, checking status....\n");
    }

    /*
     * Commence the sequence and check for errors as we go.
     * See comments for each routine for details.
     * Each routine prints messages per verbose level.
     */
    rc = 1;
    if(0 == checkConnectionStatus())
    {
        rc = 2;
        if(0 == scrubConfiguration())
        {
            rc = 3;
            if(0 == addROSpec())
            {
                rc = 4;
                if(0 == enableROSpec())
                {
                    int         i;

                    rc = 5;

                    for(i = 1; i <= 5; i++)
                    {
                        printf("INFO: Starting run %d ================\n", i);
                        if(0 != startROSpec())
                        {
                            /* already tattled */
                            break;
                        }
                        if(0 != awaitAndPrintReport())
                        {
                            /* already tattled */
                            break;
                        }
                    }

                    if(i == 5)
                    {
                        rc = 0;
                    }
                }
            }

            /*
             * After we're done, try to leave the reader
             * in a clean state for next use. This is best
             * effort and no checking of the result is done.
             */
            if(g_Verbose)
            {
                printf("INFO: Clean up reader configuration...\n");
            }
            scrubConfiguration();
        }
    }

    if(g_Verbose)
    {
        printf("INFO: Finished\n");
    }

    /*
     * Close the connection and release its resources
     */
    LLRP_Conn_closeConnectionToReader(pConn);
    LLRP_Conn_destruct(pConn);

    /*
     * Done with the registry.
     */
    LLRP_TypeRegistry_destruct(pTypeRegistry);

    /*
     * When we get here all allocated memory should have been deallocated.
     */

    return rc;
}


/**
 *****************************************************************************
 **
 ** @brief  Await and check the connection status message from the reader
 **
 ** We are expecting a READER_EVENT_NOTIFICATION message that
 ** tells us the connection is OK. The reader is suppose to
 ** send the message promptly upon connection.
 **
 ** If there is already another LLRP connection to the
 ** reader we'll get a bad Status.
 **
 ** The message should be something like:
 **
 **     <READER_EVENT_NOTIFICATION MessageID='0'>
 **       <ReaderEventNotificationData>
 **         <UTCTimestamp>
 **           <Microseconds>1184491439614224</Microseconds>
 **         </UTCTimestamp>
 **         <ConnectionAttemptEvent>
 **           <Status>Success</Status>
 **         </ConnectionAttemptEvent>
 **       </ReaderEventNotificationData>
 **     </READER_EVENT_NOTIFICATION>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
checkConnectionStatus (void)
{
    LLRP_tSMessage *            pMessage;
    LLRP_tSREADER_EVENT_NOTIFICATION *pNtf;
    LLRP_tSReaderEventNotificationData *pNtfData;
    LLRP_tSConnectionAttemptEvent *pEvent;

    /*
     * Expect the notification within 10 seconds.
     * It is suppose to be the very first message sent.
     */
    pMessage = recvMessage(10000);

    /*
     * recvMessage() returns NULL if something went wrong.
     */
    if(NULL == pMessage)
    {
        /* recvMessage() already tattled. */
        goto fail;
    }

    /*
     * Check to make sure the message is of the right type.
     * The type label (pointer) in the message should be
     * the type descriptor for READER_EVENT_NOTIFICATION.
     */
    if(&LLRP_tdREADER_EVENT_NOTIFICATION != pMessage->elementHdr.pType)
    {
        goto fail;
    }

    /*
     * Now that we are sure it is a READER_EVENT_NOTIFICATION,
     * traverse to the ReaderEventNotificationData parameter.
     */
    pNtf = (LLRP_tSREADER_EVENT_NOTIFICATION *) pMessage;
    pNtfData = pNtf->pReaderEventNotificationData;
    if(NULL == pNtfData)
    {
        goto fail;
    }

    /*
     * The ConnectionAttemptEvent parameter must be present.
     */
    pEvent = pNtfData->pConnectionAttemptEvent;
    if(NULL == pEvent)
    {
        goto fail;
    }

    /*
     * The status in the ConnectionAttemptEvent parameter
     * must indicate connection success.
     */
    if(LLRP_ConnectionAttemptStatusType_Success != pEvent->eStatus)
    {
        goto fail;
    }

    /*
     * Done with the message
     */
    freeMessage(pMessage);

    if(g_Verbose)
    {
        printf("INFO: Connection status OK\n");
    }

    /*
     * Victory.
     */
    return 0;

  fail:
    /*
     * Something went wrong. Tattle. Clean up. Return error.
     */
    printf("ERROR: checkConnectionStatus failed\n");
    freeMessage(pMessage);
    return -1;
}


/**
 *****************************************************************************
 **
 ** @brief  Scrub the reader configuration
 **
 ** The steps:
 **     - Try to reset configuration to factory defaults,
 **       this feature is optional and may not be supported
 **       by the reader.
 **     - Delete all ROSpecs
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
scrubConfiguration (void)
{
    if(0 != resetConfigurationToFactoryDefaults())
    {
        return -1;
    }

    if(0 != deleteAllROSpecs())
    {
        return -2;
    }

    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Send a SET_READER_CONFIG message that resets the
 **         reader to factory defaults.
 **
 ** NB: The ResetToFactoryDefault semantics vary between readers.
 **     It might have no effect because it is optional.
 **
 ** The message is:
 **
 **     <SET_READER_CONFIG MessageID='101'>
 **       <ResetToFactoryDefault>1</ResetToFactoryDefault>
 **     </SET_READER_CONFIG>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
resetConfigurationToFactoryDefaults (void)
{
    LLRP_tSSET_READER_CONFIG    Cmd = {
        .hdr.elementHdr.pType   = &LLRP_tdSET_READER_CONFIG,
        .hdr.MessageID          = 101,

        .ResetToFactoryDefault  = 1
    };
    LLRP_tSMessage *            pRspMsg;
    LLRP_tSSET_READER_CONFIG_RESPONSE *pRsp;

    /*
     * Send the message, expect the response of certain type
     */
    pRspMsg = transact(&Cmd.hdr);
    if(NULL == pRspMsg)
    {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a SET_READER_CONFIG_RESPONSE message.
     */
    pRsp = (LLRP_tSSET_READER_CONFIG_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */
    if(0 != checkLLRPStatus(pRsp->pLLRPStatus,
                            "resetConfigurationToFactoryDefaults"))
    {
        /* checkLLRPStatus already tattled */
        freeMessage(pRspMsg);
        return -1;
    }

    /*
     * Done with the response message.
     */
    freeMessage(pRspMsg);

    /*
     * Tattle progress, maybe
     */
    if(g_Verbose)
    {
        printf("INFO: Configuration reset to factory defaults\n");
    }

    /*
     * Victory.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Delete all ROSpecs using DELETE_ROSPEC message
 **
 ** Per the spec, the DELETE_ROSPEC message contains an ROSpecID
 ** of 0 to indicate we want all ROSpecs deleted.
 **
 ** The message is
 **
 **     <DELETE_ROSPEC MessageID='102'>
 **       <ROSpecID>0</ROSpecID>
 **     </DELETE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 ** IMPORANT:
 **     The coding of this function demonstrates best practices.
 **     Please see IMPORTANT comment at the top of this file.
 **
 *****************************************************************************/

int
deleteAllROSpecs (void)
{
    LLRP_tSDELETE_ROSPEC *      pCmd;
    LLRP_tSMessage *            pCmdMsg;
    LLRP_tSMessage *            pRspMsg;
    LLRP_tSDELETE_ROSPEC_RESPONSE *pRsp;

    /*
     * Compose the command message
     */
    pCmd = LLRP_DELETE_ROSPEC_construct();
    pCmdMsg = &pCmd->hdr;
    LLRP_Message_setMessageID(pCmdMsg, 102);
    LLRP_DELETE_ROSPEC_setROSpecID(pCmd, 0);        /* All */

    /*
     * Send the message, expect the response of certain type
     */
    pRspMsg = transact(pCmdMsg);

    /*
     * Done with the command message
     */
    freeMessage(pCmdMsg);

    /*
     * transact() returns NULL if something went wrong.
     */
    if(NULL == pRspMsg)
    {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a DELETE_ROSPEC_RESPONSE message.
     */
    pRsp = (LLRP_tSDELETE_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */
    if(0 != checkLLRPStatus(pRsp->pLLRPStatus, "deleteAllROSpecs"))
    {
        /* checkLLRPStatus already tattled */
        freeMessage(pRspMsg);
        return -1;
    }

    /*
     * Done with the response message.
     */
    freeMessage(pRspMsg);

    /*
     * Tattle progress, maybe
     */
    if(g_Verbose)
    {
        printf("INFO: All ROSpecs are deleted\n");
    }

    /*
     * Victory.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Add our ROSpec using ADD_ROSPEC message
 **
 ** This ROSpec waits for a START_ROSPEC message,
 ** then takes inventory on all antennas for 5 seconds.
 **
 ** The tag report is generated after the ROSpec is done.
 **
 ** This example is deliberately streamlined.
 ** Nothing here configures the antennas, RF, or Gen2.
 ** The current defaults are used. Remember we just reset
 ** the reader to factory defaults (above). Normally an
 ** application would be more precise in configuring the
 ** reader and in its ROSpecs.
 **
 ** Experience suggests that typical ROSpecs are about
 ** double this in size.
 **
 ** The message is
 **
 **     <ADD_ROSPEC MessageID='201'>
 **       <ROSpec>
 **         <ROSpecID>123</ROSpecID>
 **         <Priority>0</Priority>
 **         <CurrentState>Disabled</CurrentState>
 **         <ROBoundarySpec>
 **           <ROSpecStartTrigger>
 **             <ROSpecStartTriggerType>Null</ROSpecStartTriggerType>
 **           </ROSpecStartTrigger>
 **           <ROSpecStopTrigger>
 **             <ROSpecStopTriggerType>Null</ROSpecStopTriggerType>
 **             <DurationTriggerValue>0</DurationTriggerValue>
 **           </ROSpecStopTrigger>
 **         </ROBoundarySpec>
 **         <AISpec>
 **           <AntennaIDs>0</AntennaIDs>
 **           <AISpecStopTrigger>
 **             <AISpecStopTriggerType>Duration</AISpecStopTriggerType>
 **             <DurationTrigger>5000</DurationTrigger>
 **           </AISpecStopTrigger>
 **           <InventoryParameterSpec>
 **             <InventoryParameterSpecID>1234</InventoryParameterSpecID>
 **             <ProtocolID>EPCGlobalClass1Gen2</ProtocolID>
 **           </InventoryParameterSpec>
 **         </AISpec>
 **         <ROReportSpec>
 **           <ROReportTrigger>Upon_N_Tags_Or_End_Of_ROSpec</ROReportTrigger>
 **           <N>0</N>
 **           <TagReportContentSelector>
 **             <EnableROSpecID>0</EnableROSpecID>
 **             <EnableSpecIndex>0</EnableSpecIndex>
 **             <EnableInventoryParameterSpecID>0</EnableInventoryParameterSpecID>
 **             <EnableAntennaID>0</EnableAntennaID>
 **             <EnableChannelIndex>0</EnableChannelIndex>
 **             <EnablePeakRSSI>0</EnablePeakRSSI>
 **             <EnableFirstSeenTimestamp>0</EnableFirstSeenTimestamp>
 **             <EnableLastSeenTimestamp>0</EnableLastSeenTimestamp>
 **             <EnableTagSeenCount>0</EnableTagSeenCount>
 **             <EnableAccessSpecID>0</EnableAccessSpecID>
 **           </TagReportContentSelector>
 **         </ROReportSpec>
 **       </ROSpec>
 **     </ADD_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
addROSpec (void)
{
    LLRP_tSROSpecStartTrigger   ROSpecStartTrigger = {
        .hdr.elementHdr.pType   = &LLRP_tdROSpecStartTrigger,

        .eROSpecStartTriggerType = LLRP_ROSpecStartTriggerType_Null,
    };
    LLRP_tSROSpecStopTrigger    ROSpecStopTrigger = {
        .hdr.elementHdr.pType   = &LLRP_tdROSpecStopTrigger,

        .eROSpecStopTriggerType = LLRP_ROSpecStopTriggerType_Null,
        .DurationTriggerValue   = 0     /* n/a */
    };
    LLRP_tSROBoundarySpec       ROBoundarySpec = {
        .hdr.elementHdr.pType   = &LLRP_tdROBoundarySpec,

        .pROSpecStartTrigger    = &ROSpecStartTrigger,
        .pROSpecStopTrigger     = &ROSpecStopTrigger,
    };
    llrp_u16_t                  AntennaIDs[1] = { 0 };  /* All */
    LLRP_tSAISpecStopTrigger    AISpecStopTrigger = {
        .hdr.elementHdr.pType   = &LLRP_tdAISpecStopTrigger,

        .eAISpecStopTriggerType = LLRP_AISpecStopTriggerType_Duration,
        .DurationTrigger        = 5000,
    };
    LLRP_tSInventoryParameterSpec InventoryParameterSpec = {
        .hdr.elementHdr.pType   = &LLRP_tdInventoryParameterSpec,

        .InventoryParameterSpecID = 1234,
        .eProtocolID            = LLRP_AirProtocols_EPCGlobalClass1Gen2,
    };
    LLRP_tSAISpec               AISpec = {
        .hdr.elementHdr.pType   = &LLRP_tdAISpec,

        .AntennaIDs = {
            .nValue                 = 1,
            .pValue                 = AntennaIDs
        },
        .pAISpecStopTrigger     = &AISpecStopTrigger,
        .listInventoryParameterSpec = &InventoryParameterSpec,
    };
    LLRP_tSTagReportContentSelector TagReportContentSelector = {
        .hdr.elementHdr.pType   = &LLRP_tdTagReportContentSelector,

        .EnableROSpecID         = 0,
        .EnableSpecIndex        = 0,
        .EnableInventoryParameterSpecID = 0,
        .EnableAntennaID        = 0,
        .EnableChannelIndex     = 0,
        .EnablePeakRSSI         = 0,
        .EnableFirstSeenTimestamp = 0,
        .EnableLastSeenTimestamp = 0,
        .EnableTagSeenCount     = 0,
        .EnableAccessSpecID     = 0,
    };
    LLRP_tSROReportSpec         ROReportSpec = {
        .hdr.elementHdr.pType   = &LLRP_tdROReportSpec,

        .eROReportTrigger       =
                      LLRP_ROReportTriggerType_Upon_N_Tags_Or_End_Of_ROSpec,
        .N                      = 0,
        .pTagReportContentSelector = &TagReportContentSelector,
    };
    LLRP_tSROSpec               ROSpec = {
        .hdr.elementHdr.pType   = &LLRP_tdROSpec,

        .ROSpecID               = 123,
        .Priority               = 0,
        .eCurrentState          = LLRP_ROSpecState_Disabled,
        .pROBoundarySpec        = &ROBoundarySpec,
        .listSpecParameter      = &AISpec.hdr,
        .pROReportSpec          = &ROReportSpec,
    };
    LLRP_tSADD_ROSPEC           Cmd = {
        .hdr.elementHdr.pType   = &LLRP_tdADD_ROSPEC,

        .hdr.MessageID          = 201,
        .pROSpec                = &ROSpec,
    };
    LLRP_tSMessage *            pRspMsg;
    LLRP_tSADD_ROSPEC_RESPONSE *pRsp;

    /*
     * Send the message, expect the response of certain type
     */
    pRspMsg = transact(&Cmd.hdr);
    if(NULL == pRspMsg)
    {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a ADD_ROSPEC_RESPONSE message.
     */
    pRsp = (LLRP_tSADD_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */
    if(0 != checkLLRPStatus(pRsp->pLLRPStatus, "addROSpec"))
    {
        /* checkLLRPStatus already tattled */
        freeMessage(pRspMsg);
        return -1;
    }

    /*
     * Done with the response message.
     */
    freeMessage(pRspMsg);

    /*
     * Tattle progress, maybe
     */
    if(g_Verbose)
    {
        printf("INFO: ROSpec added\n");
    }

    /*
     * Victory.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Enable our ROSpec using ENABLE_ROSPEC message
 **
 ** Enable the ROSpec that was added above.
 **
 ** The message we send is:
 **     <ENABLE_ROSPEC MessageID='202'>
 **       <ROSpecID>123</ROSpecID>
 **     </ENABLE_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
enableROSpec (void)
{
    LLRP_tSENABLE_ROSPEC        Cmd = {
        .hdr.elementHdr.pType   = &LLRP_tdENABLE_ROSPEC,
        .hdr.MessageID          = 202,

        .ROSpecID               = 123,
    };
    LLRP_tSMessage *            pRspMsg;
    LLRP_tSENABLE_ROSPEC_RESPONSE *pRsp;

    /*
     * Send the message, expect the response of certain type
     */
    pRspMsg = transact(&Cmd.hdr);
    if(NULL == pRspMsg)
    {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a ENABLE_ROSPEC_RESPONSE message.
     */
    pRsp = (LLRP_tSENABLE_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */
    if(0 != checkLLRPStatus(pRsp->pLLRPStatus, "enableROSpec"))
    {
        /* checkLLRPStatus already tattled */
        freeMessage(pRspMsg);
        return -1;
    }

    /*
     * Done with the response message.
     */
    freeMessage(pRspMsg);

    /*
     * Tattle progress, maybe
     */
    if(g_Verbose)
    {
        printf("INFO: ROSpec enabled\n");
    }

    /*
     * Victory.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Start our ROSpec using START_ROSPEC message
 **
 ** Start the ROSpec that was added above.
 **
 ** The message we send is:
 **     <START_ROSPEC MessageID='202'>
 **       <ROSpecID>123</ROSpecID>
 **     </START_ROSPEC>
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
startROSpec (void)
{
    LLRP_tSSTART_ROSPEC         Cmd = {
        .hdr.elementHdr.pType   = &LLRP_tdSTART_ROSPEC,
        .hdr.MessageID          = 202,

        .ROSpecID               = 123,
    };
    LLRP_tSMessage *            pRspMsg;
    LLRP_tSSTART_ROSPEC_RESPONSE *pRsp;

    /*
     * Send the message, expect the response of certain type
     */
    pRspMsg = transact(&Cmd.hdr);
    if(NULL == pRspMsg)
    {
        /* transact already tattled */
        return -1;
    }

    /*
     * Cast to a START_ROSPEC_RESPONSE message.
     */
    pRsp = (LLRP_tSSTART_ROSPEC_RESPONSE *) pRspMsg;

    /*
     * Check the LLRPStatus parameter.
     */
    if(0 != checkLLRPStatus(pRsp->pLLRPStatus, "startROSpec"))
    {
        /* checkLLRPStatus already tattled */
        freeMessage(pRspMsg);
        return -1;
    }

    /*
     * Done with the response message.
     */
    freeMessage(pRspMsg);

    /*
     * Tattle progress, maybe
     */
    if(g_Verbose)
    {
        printf("INFO: ROSpec started\n");
    }

    /*
     * Victory.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Receive and print the RO_ACCESS_REPORT
 **
 ** Receive messages until an RO_ACCESS_REPORT is received.
 ** Time limit is 7 seconds. We expect a report within 5 seconds.
 **
 ** This shows how to determine the type of a received message.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong
 **
 *****************************************************************************/

int
awaitAndPrintReport (void)
{
    int                         bDone = 0;
    int                         retVal = 0;

    /*
     * Keep receiving messages until done or until
     * something bad happens.
     */
    while(!bDone)
    {
        LLRP_tSMessage *        pMessage;
        const LLRP_tSTypeDescriptor *pType;

        /*
         * Wait up to 7 seconds for a message. The report
         * should occur within 5 seconds.
         */
        pMessage = recvMessage(7000);
        if(NULL == pMessage)
        {
            /*
             * Did not receive a message within a reasonable
             * amount of time. recvMessage() already tattled
             */
            retVal = -2;
            bDone = 1;
            continue;
        }

        /*
         * What happens depends on what kind of message
         * received. Use the type label (pType) to
         * discriminate message types.
         */
        pType = pMessage->elementHdr.pType;

        /*
         * Is it a tag report? If so, print it out.
         */
        if(&LLRP_tdRO_ACCESS_REPORT == pType)
        {
            LLRP_tSRO_ACCESS_REPORT *pNtf;

            pNtf = (LLRP_tSRO_ACCESS_REPORT *) pMessage;

            printTagReportData(pNtf);
            bDone = 1;
            retVal = 0;
        }

        /*
         * Is it a reader event? This example only recognizes
         * AntennaEvents.
         */
        else if(&LLRP_tdREADER_EVENT_NOTIFICATION == pType)
        {
            LLRP_tSREADER_EVENT_NOTIFICATION *pNtf;
            LLRP_tSReaderEventNotificationData *pNtfData;

            pNtf = (LLRP_tSREADER_EVENT_NOTIFICATION *) pMessage;

            pNtfData =
               LLRP_READER_EVENT_NOTIFICATION_getReaderEventNotificationData(
                    pNtf);

            if(NULL != pNtfData)
            {
                handleReaderEventNotification(pNtfData);
            }
            else
            {
                /*
                 * This should never happen. Using continue
                 * to keep indent depth down.
                 */
                printf("WARNING: READER_EVENT_NOTIFICATION without data\n");
            }
        }

        /*
         * Hmmm. Something unexpected. Just tattle and keep going.
         */
        else
        {
            printf("WARNING: Ignored unexpected message during monitor: %s\n",
                pType->pName);
        }

        /*
         * Done with the received message
         */
        freeMessage(pMessage);
    }

    return retVal;
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to print a tag report
 **
 ** The report is printed in list order, which is arbitrary.
 **
 ** TODO: It would be cool to sort the list by EPC and antenna,
 **       then print it.
 **
 ** @return     void
 **
 *****************************************************************************/

void
printTagReportData (
  LLRP_tSRO_ACCESS_REPORT *     pRO_ACCESS_REPORT)
{
    LLRP_tSTagReportData *      pTagReportData;
    unsigned int                nEntry = 0;

    /*
     * Loop through and count the number of entries
     */
    for(
        pTagReportData = pRO_ACCESS_REPORT->listTagReportData;
        NULL != pTagReportData;
        pTagReportData = (LLRP_tSTagReportData *)
                                    pTagReportData->hdr.pNextSubParameter)
    {
        nEntry++;
    }

    printf("INFO: %u tag report entries\n", nEntry);

    /*
     * Loop through again and print each entry.
     */
    for(
        pTagReportData = pRO_ACCESS_REPORT->listTagReportData;
        NULL != pTagReportData;
        pTagReportData = (LLRP_tSTagReportData *)
                                    pTagReportData->hdr.pNextSubParameter)
    {
        printOneTagReportData(pTagReportData);
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to print one tag report entry on one line
 **
 ** @return     void
 **
 *****************************************************************************/

void
printOneTagReportData (
  LLRP_tSTagReportData *        pTagReportData)
{
    const LLRP_tSTypeDescriptor *pType;
    char                    aBuf[64];

    /*
     * Print the EPC. It could be an 96-bit EPC_96 parameter
     * or an variable length EPCData parameter.
     */
    if(NULL != pTagReportData->pEPCParameter)
    {
        char *              p = aBuf;
        llrp_u8_t *         pValue = NULL;
        unsigned int        n, i;

        pType = pTagReportData->pEPCParameter->elementHdr.pType;
        if(&LLRP_tdEPC_96 == pType)
        {
            LLRP_tSEPC_96 * pE96;

            pE96 = (LLRP_tSEPC_96 *) pTagReportData->pEPCParameter;
            pValue = pE96->EPC.aValue;
            n = 12u;
        }
        else if(&LLRP_tdEPCData == pType)
        {
            LLRP_tSEPCData *pEPCData;

            pEPCData = (LLRP_tSEPCData *) pTagReportData->pEPCParameter;
            pValue = pEPCData->EPC.pValue;
            n = (pEPCData->EPC.nBit + 7u) / 8u;
        }

        if(NULL != pValue)
        {
            for(i = 0; i < n; i++)
            {
                if(0 < i && i%2 == 0)
                {
                    *p++ = '-';
                }
                sprintf(p, "%02X", pValue[i]);
                while(*p) p++;
            }
        }
        else
        {
            strcpy(aBuf, "---unknown-epc-data-type---");
        }
    }
    else
    {
        strcpy(aBuf, "---missing-epc-data---");
    }
    printf("%-32s", aBuf);

    /*
     * End of line
     */
    printf("\n");
}


/**
 *****************************************************************************
 **
 ** @brief  Handle a ReaderEventNotification
 **
 ** Handle the payload of a READER_EVENT_NOTIFICATION message.
 ** This routine simply dispatches to handlers of specific
 ** event types.
 **
 ** @return     void
 **
 *****************************************************************************/

void
handleReaderEventNotification (
  LLRP_tSReaderEventNotificationData *pNtfData)
{
    LLRP_tSAntennaEvent *       pAntennaEvent;
    LLRP_tSReaderExceptionEvent *pReaderExceptionEvent;
    int                         nReported = 0;

    pAntennaEvent =
        LLRP_ReaderEventNotificationData_getAntennaEvent(pNtfData);
    if(NULL != pAntennaEvent)
    {
        handleAntennaEvent(pAntennaEvent);
        nReported++;
    }

    pReaderExceptionEvent =
        LLRP_ReaderEventNotificationData_getReaderExceptionEvent(pNtfData);
    if(NULL != pReaderExceptionEvent)
    {
        handleReaderExceptionEvent(pReaderExceptionEvent);
        nReported++;
    }

    /*
     * Similarly handle other events here:
     *      HoppingEvent
     *      GPIEvent
     *      ROSpecEvent
     *      ReportBufferLevelWarningEvent
     *      ReportBufferOverflowErrorEvent
     *      RFSurveyEvent
     *      AISpecEvent
     *      ConnectionAttemptEvent
     *      ConnectionCloseEvent
     *      Custom
     */

    if(0 == nReported)
    {
        printf("NOTICE: Unexpected (unhandled) ReaderEvent\n");
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Handle an AntennaEvent
 **
 ** An antenna was disconnected or (re)connected. Tattle.
 **
 ** @return     void
 **
 *****************************************************************************/

void
handleAntennaEvent (
  LLRP_tSAntennaEvent *         pAntennaEvent)
{
    LLRP_tEAntennaEventType     eEventType;
    llrp_u16_t                  AntennaID;
    char *                      pStateStr;

    eEventType = LLRP_AntennaEvent_getEventType(pAntennaEvent);
    AntennaID = LLRP_AntennaEvent_getAntennaID(pAntennaEvent);

    switch(eEventType)
    {
    case LLRP_AntennaEventType_Antenna_Disconnected:
        pStateStr = "disconnected";
        break;

    case LLRP_AntennaEventType_Antenna_Connected:
        pStateStr = "connected";
        break;

    default:
        pStateStr = "?unknown-event?";
        break;
    }

    printf("NOTICE: Antenna %d is %s\n", AntennaID, pStateStr);
}


/**
 *****************************************************************************
 **
 ** @brief  Handle a ReaderExceptionEvent
 **
 ** Something has gone wrong. There are lots of details but
 ** all this does is print the message, if one.
 **
 ** @return     void
 **
 *****************************************************************************/

void
handleReaderExceptionEvent (
  LLRP_tSReaderExceptionEvent * pReaderExceptionEvent)
{
    llrp_utf8v_t                Message;

    Message = LLRP_ReaderExceptionEvent_getMessage(pReaderExceptionEvent);

    if(0 < Message.nValue && NULL != Message.pValue)
    {
        printf("NOTICE: ReaderException '%.*s'\n",
             Message.nValue, Message.pValue);
    }
    else
    {
        printf("NOTICE: ReaderException but no message\n");
    }
}


/**
 *****************************************************************************
 **
 ** @brief  Helper routine to check an LLRPStatus parameter
 **         and tattle on errors
 **
 ** Helper routine to interpret the LLRPStatus subparameter
 ** that is in all responses. It tattles on an error, if one,
 ** and tries to safely provide details.
 **
 ** This simplifies the code, above, for common check/tattle
 ** sequences.
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int
checkLLRPStatus (
  LLRP_tSLLRPStatus *           pLLRPStatus,
  char *                        pWhatStr)
{
    /*
     * The LLRPStatus parameter is mandatory in all responses.
     * If it is missing there should have been a decode error.
     * This just makes sure (remember, this program is a
     * diagnostic and suppose to catch LTKC mistakes).
     */
    if(NULL == pLLRPStatus)
    {
        printf("ERROR: %s missing LLRP status\n", pWhatStr);
        return -1;
    }

    /*
     * Make sure the status is M_Success.
     * If it isn't, print the error string if one.
     * This does not try to pretty-print the status
     * code. To get that, run this program with -vv
     * and examine the XML output.
     */
    if(LLRP_StatusCode_M_Success != pLLRPStatus->eStatusCode)
    {
        if(0 == pLLRPStatus->ErrorDescription.nValue)
        {
            printf("ERROR: %s failed, no error description given\n",
                pWhatStr);
        }
        else
        {
            printf("ERROR: %s failed, %.*s\n",
                pWhatStr,
                pLLRPStatus->ErrorDescription.nValue,
                pLLRPStatus->ErrorDescription.pValue);
        }
        return -2;
    }

    /*
     * Victory. Everything is fine.
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to do an LLRP transaction
 **
 ** Wrapper to transact a request/resposne.
 **     - Print the outbound message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_transact()
 **     - LLRP_Conn_transact() receives the response or recognizes an error
 **     - Tattle on errors, if any
 **     - Print the received message in XML if verbose level is at least 2
 **     - If the response is ERROR_MESSAGE, the request was sufficiently
 **       misunderstood that the reader could not send a proper reply.
 **       Deem this an error, free the message.
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

LLRP_tSMessage *
transact (
  LLRP_tSMessage *              pSendMsg)
{
    LLRP_tSConnection *         pConn = g_pConnectionToReader;
    LLRP_tSMessage *            pRspMsg;

    /*
     * Print the XML text for the outbound message if
     * verbosity is 2 or higher.
     */
    if(1 < g_Verbose)
    {
        printf("\n===================================\n");
        printf("INFO: Transact sending\n");
        printXMLMessage(pSendMsg);
    }

    /*
     * Send the message, expect the response of certain type.
     * If LLRP_Conn_transact() returns NULL then there was
     * an error. In that case we try to print the error details.
     */
    pRspMsg = LLRP_Conn_transact(pConn, pSendMsg, 5000);
    if(NULL == pRspMsg)
    {
        const LLRP_tSErrorDetails *pError = LLRP_Conn_getTransactError(pConn);

        printf("ERROR: %s transact failed, %s\n",
            pSendMsg->elementHdr.pType->pName,
            pError->pWhatStr ? pError->pWhatStr : "no reason given");

        if(NULL != pError->pRefType)
        {
            printf("ERROR: ... reference type %s\n",
                pError->pRefType->pName);
        }

        if(NULL != pError->pRefField)
        {
            printf("ERROR: ... reference field %s\n",
                pError->pRefField->pName);
        }

        return NULL;
    }

    /*
     * Print the XML text for the inbound message if
     * verbosity is 2 or higher.
     */
    if(1 < g_Verbose)
    {
        printf("\n- - - - - - - - - - - - - - - - - -\n");
        printf("INFO: Transact received response\n");
        printXMLMessage(pRspMsg);
    }

    /*
     * If it is an ERROR_MESSAGE (response from reader
     * when it can't understand the request), tattle
     * and declare defeat.
     */
    if(&LLRP_tdERROR_MESSAGE == pRspMsg->elementHdr.pType)
    {
        const LLRP_tSTypeDescriptor *pResponseType;

        pResponseType = pSendMsg->elementHdr.pType->pResponseType;

        printf("ERROR: Received ERROR_MESSAGE instead of %s\n",
            pResponseType->pName);
        freeMessage(pRspMsg);
        pRspMsg = NULL;
    }

    return pRspMsg;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to receive a message
 **
 ** This can receive notifications as well as responses.
 **     - Recv a message using the LLRP_Conn_recvMessage()
 **     - Tattle on errors, if any
 **     - Print the message in XML if verbose level is at least 2
 **
 ** The message returned resides in allocated memory. It is the
 ** caller's obligtation to free it.
 **
 ** @param[in]  nMaxMS          -1 => block indefinitely
 **                              0 => just peek at input queue and
 **                                   socket queue, return immediately
 **                                   no matter what
 **                             >0 => ms to await complete frame
 **
 ** @return     ==NULL          Something went wrong, already tattled
 **             !=NULL          Pointer to a message
 **
 *****************************************************************************/

LLRP_tSMessage *
recvMessage (
  int                           nMaxMS)
{
    LLRP_tSConnection *         pConn = g_pConnectionToReader;
    LLRP_tSMessage *            pMessage;

    /*
     * Receive the message subject to a time limit
     */
    pMessage = LLRP_Conn_recvMessage(pConn, nMaxMS);

    /*
     * If LLRP_Conn_recvMessage() returns NULL then there was
     * an error. In that case we try to print the error details.
     */
    if(NULL == pMessage)
    {
        const LLRP_tSErrorDetails *pError = LLRP_Conn_getRecvError(pConn);

        printf("ERROR: recvMessage failed, %s\n",
            pError->pWhatStr ? pError->pWhatStr : "no reason given");

        if(NULL != pError->pRefType)
        {
            printf("ERROR: ... reference type %s\n",
                pError->pRefType->pName);
        }

        if(NULL != pError->pRefField)
        {
            printf("ERROR: ... reference field %s\n",
                pError->pRefField->pName);
        }

        return NULL;
    }

    /*
     * Print the XML text for the inbound message if
     * verbosity is 2 or higher.
     */
    if(1 < g_Verbose)
    {
        printf("\n===================================\n");
        printf("INFO: Message received\n");
        printXMLMessage(pMessage);
    }

    return pMessage;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper routine to send a message
 **
 ** Wrapper to send a message.
 **     - Print the message in XML if verbose level is at least 2
 **     - Send it using the LLRP_Conn_sendMessage()
 **     - Tattle on errors, if any
 **
 ** @param[in]  pSendMsg        Pointer to message to send
 **
 ** @return     ==0             Everything OK
 **             !=0             Something went wrong, already tattled
 **
 *****************************************************************************/

int
sendMessage (
  LLRP_tSMessage *              pSendMsg)
{
    LLRP_tSConnection *         pConn = g_pConnectionToReader;

    /*
     * Print the XML text for the outbound message if
     * verbosity is 2 or higher.
     */
    if(1 < g_Verbose)
    {
        printf("\n===================================\n");
        printf("INFO: Sending\n");
        printXMLMessage(pSendMsg);
    }

    /*
     * If LLRP_Conn_sendMessage() returns other than LLRP_RC_OK
     * then there was an error. In that case we try to print
     * the error details.
     */
    if(LLRP_RC_OK != LLRP_Conn_sendMessage(pConn, pSendMsg))
    {
        const LLRP_tSErrorDetails *pError = LLRP_Conn_getSendError(pConn);

        printf("ERROR: %s sendMessage failed, %s\n",
            pSendMsg->elementHdr.pType->pName,
            pError->pWhatStr ? pError->pWhatStr : "no reason given");

        if(NULL != pError->pRefType)
        {
            printf("ERROR: ... reference type %s\n",
                pError->pRefType->pName);
        }

        if(NULL != pError->pRefField)
        {
            printf("ERROR: ... reference field %s\n",
                pError->pRefField->pName);
        }

        return -1;
    }

    /*
     * Victory
     */
    return 0;
}


/**
 *****************************************************************************
 **
 ** @brief  Wrapper to free a message.
 **
 ** All it does is cast pMessage and let
 ** LLRP_Element_destruct() do the work.
 **
 ** @param[in]  pMessage        Pointer to message to destruct
 **
 ** @return     void
 **
 *****************************************************************************/

void
freeMessage (
  LLRP_tSMessage *              pMessage)
{
    LLRP_Element_destruct(&pMessage->elementHdr);
}


/**
 *****************************************************************************
 **
 ** @brief  Helper to print a message as XML text
 **
 ** Print a LLRP message as XML text
 **
 ** @param[in]  pMessage        Pointer to message to print
 **
 ** @return     void
 **
 *****************************************************************************/

void
printXMLMessage (
  LLRP_tSMessage *              pMessage)
{
    char                        aBuf[100*1024];

    /*
     * Convert the message to an XML string.
     * This fills the buffer with either the XML string
     * or an error message. The return value could
     * be checked.
     */

    LLRP_toXMLString(&pMessage->elementHdr, aBuf, sizeof aBuf);

    /*
     * Print the XML Text to the standard output.
     */
    printf("%s", aBuf);
}
