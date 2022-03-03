/*
 * pixyCam2.c
 *
 *  Created on: 2022. 2. 23.
 *      Author: GwangSik
 */




#include <msp430.h>
#include "pixyCam2.h"
#include "usciI2C.h"
#include <stdio.h>// sprintf
#include <stdlib.h> // malloc
#include "ucsiUart.h"
#include "nok5110LCD.h"
#include <string.h> // strlen, strcpy
#include "nokLcdDraw.h"

#define _USE_MATH_DEFINES // M_PI
#include <math.h> // angle. atan.

const unsigned char slaveAddress = PIXY_SLV_ADDR;
PIXY_CMD pixyCmds[MAX_NUM_COMMAND];
PIXY_RESPONSE payload[MAX_NUM_COMMAND];


double vecAngle;
double psCntTemp;
int* resTempInt;
char byteTemp[2];

/************************************************************************************
* Function: pixyCmdInit
* - this function is to initialize PIXY_CMD structure array with pre-defined constants.
* Arguments: (PIXY_CMD*) cmds - pixy-commands pre-defined.
* return: (void) n/a
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void pixyCmdInit(){

    pixyCmds[GET_VERSION_IDX].name = PIXY_CMD0_NAME;
    pixyCmds[GET_VERSION_IDX].nBytes = PIXY_CMD0_N_BYTES;
    pixyCmds[GET_RESOLUTION_IDX].name = PIXY_CMD1_NAME;
    pixyCmds[GET_RESOLUTION_IDX].nBytes = PIXY_CMD1_N_BYTES;
    pixyCmds[GET_VECTOR_IDX].name = PIXY_CMD2_NAME;
    pixyCmds[GET_VECTOR_IDX].nBytes = PIXY_CMD2_N_BYTES;
    pixyCmds[PIXY_TX_IDX].name = PIXY_CMD3_NAME;
    pixyCmds[PIXY_TX_IDX].nBytes = PIXY_CMD3_N_BYTES;

    unsigned int i=0, j=0;
    for (j=0; j<MAX_NUM_COMMAND; j++){
        for (i=0; i<COMMAND_REQUEST_BYTE_MAX_LENGTH; i++){
            pixyCmds[j].byteBuffer[i] = 0;
        }
    }


    // getVersion
    pixyCmds[GET_VERSION_IDX].byteBuffer[0] = 0xAE;
    pixyCmds[GET_VERSION_IDX].byteBuffer[1] = 0xC1;
    pixyCmds[GET_VERSION_IDX].byteBuffer[2] = 0x0E;
    pixyCmds[GET_VERSION_IDX].byteBuffer[3] = 0x00;

    // getResolution
    pixyCmds[GET_RESOLUTION_IDX].byteBuffer[0] = 0xAE;
    pixyCmds[GET_RESOLUTION_IDX].byteBuffer[1] = 0xC1;
    pixyCmds[GET_RESOLUTION_IDX].byteBuffer[2] = 0x0C;
    pixyCmds[GET_RESOLUTION_IDX].byteBuffer[3] = 0x01;
    pixyCmds[GET_RESOLUTION_IDX].byteBuffer[4] = 0x00; // 0 to 255. unused...?

    // getVector --> getMainFeatures(features, wait)
    pixyCmds[GET_VECTOR_IDX].byteBuffer[0] = 0xAE; // 193d
    pixyCmds[GET_VECTOR_IDX].byteBuffer[1] = 0xC1; // 174d
    pixyCmds[GET_VECTOR_IDX].byteBuffer[2] = 0x30; // 48d
    pixyCmds[GET_VECTOR_IDX].byteBuffer[3] = 0x02;
    pixyCmds[GET_VECTOR_IDX].byteBuffer[4] = 0x00; // main features
    pixyCmds[GET_VECTOR_IDX].byteBuffer[5] = 0x07; // vectors

    // pixyTx
    pixyCmds[PIXY_TX_IDX].byteBuffer[0] = 0x0; // initialized.


}


/************************************************************************************
* Function: pixyResponseInit
* - this function is to initialize PIXY_RESPONSE structure array with pre-defined constants
* Arguments: (PIXY_RESPONSE*) payload - pixy-response received through i2c
* return:  (void) n/a
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
void pixyResponseInit(){

    payload[GET_VERSION_IDX].name = PIXY_CMD0_NAME;
    payload[GET_VERSION_IDX].nBytesResp = PIXY_RESP0_N_BYTES;

    payload[GET_RESOLUTION_IDX].name = PIXY_CMD1_NAME;
    payload[GET_RESOLUTION_IDX].nBytesResp = PIXY_RESP1_N_BYTES;

    payload[GET_VECTOR_IDX].name = PIXY_CMD2_NAME;
    payload[GET_VECTOR_IDX].nBytesResp = PIXY_RESP2_N_BYTES;

    payload[PIXY_TX_IDX].name = PIXY_CMD3_NAME;
    payload[PIXY_TX_IDX].nBytesResp = PIXY_RESP3_N_BYTES;

    unsigned int i=0, j=0;
    for (j=0; j<MAX_NUM_COMMAND; j++){
        for (i=0; i<COMMAND_RESPONSE_BYTE_MAX_LENGTH; i++){
            payload[j].byteBuffer[i] = 0;
        }
    }

}


/************************************************************************************
* Function: pixyGetVersion
* - this function is to communicate pixyCam using i2c transmit/receive function
*   to request/receive getVersion.

* Arguments: (PIXY_CMD*) cmds - command structure pointer.
*            (PIXY_RESPONSE*) payload - payload structure pointer.
* return: 0 if succeeded, -1 if failed.
* Author: Gwang Sik Kim
* Date: Feb 23, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int pixyGetVersion(void){

    // request and read the command from pixy
    int flag = -1;
    flag = usciB0I2CMstTransmit(pixyCmds[GET_VERSION_IDX].byteBuffer, pixyCmds[GET_VERSION_IDX].nBytes);
    if (flag != -1){
        flag = usciB0I2CMstReceive(payload[GET_VERSION_IDX].byteBuffer, payload[GET_VERSION_IDX].nBytesResp);
    }

    return flag;
}


void pixyGetVerConsole(int flag){
    char* dispMsg = (char*)malloc(sizeof(char) * MAX_DISP_MSG_I2C); // malloc start
    char* versionString;
    // display it on the console
    if (flag == GET_VER_SUCCESS){
        // frame width 16bit data : byteBuffer[7] * 256 + byteBuffer[6]
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[12]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[13]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[14]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[15]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[16]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[17]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_VERSION_IDX].byteBuffer[18]);
        TX_DISPMSG;


        versionString = (char*)&payload[GET_VERSION_IDX].byteBuffer[12];
        sprintf(dispMsg, "%s \r\n", versionString);
        TX_DISPMSG;

    }
    else{
        sprintf(dispMsg, "i2c communication failed. \r\n");
        TX_DISPMSG;
    }
    free(dispMsg);    // malloc end
}

/************************************************************************************
* Function: pixyGetResolution
* - this function is to communicate pixyCam using i2c transmit/receive function
*   to request/receive getResolution

* Arguments: (PIXY_CMD*) cmds - command structure pointer.
*            (PIXY_RESPONSE*) payload - payload structure pointer.
* return: 0 if succeeded, -1 if failed.
* Author: Gwang Sik Kim
* Date: Feb 25, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int pixyGetResolution(void){

    int flag = -1;
    flag = usciB0I2CMstTransmit(pixyCmds[GET_RESOLUTION_IDX].byteBuffer, pixyCmds[GET_RESOLUTION_IDX].nBytes);
    if (flag != -1){
        flag = usciB0I2CMstReceive(payload[GET_RESOLUTION_IDX].byteBuffer, payload[GET_RESOLUTION_IDX].nBytesResp);
    }
    return flag;
}


void pixyGetResConsole(int flag){
    //heap memory allocation = char data size * max
    char* dispMsg = (char*)malloc(sizeof(char) * MAX_DISP_MSG_I2C);

    if (flag == GET_RES_SUCCESS){
        // frame width 16bit data : byteBuffer[7] * 256 + byteBuffer[6]
        sprintf(dispMsg, "%u \r\n", payload[GET_RESOLUTION_IDX].byteBuffer[6]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_RESOLUTION_IDX].byteBuffer[7]);
        TX_DISPMSG;

        byteTemp[0] = payload[GET_RESOLUTION_IDX].byteBuffer[6];
        byteTemp[1] = payload[GET_RESOLUTION_IDX].byteBuffer[7];
        //*resTempInt =  (int)payload[GET_RESOLUTION_IDX].byteBuffer[7] * 256 +   (int)payload[GET_RESOLUTION_IDX].byteBuffer[6];
        resTempInt = byteTemp; // this warning can be ignored.

        sprintf(dispMsg, "16 bit frame width : %d \r\n", *resTempInt);
        TX_DISPMSG;

        // frame height 16bit data : byteBuffer[9] * 256 + byteBuffer[8]
        sprintf(dispMsg, "%u \r\n", payload[GET_RESOLUTION_IDX].byteBuffer[8]);
        TX_DISPMSG;
        sprintf(dispMsg, "%u \r\n", payload[GET_RESOLUTION_IDX].byteBuffer[9]);
        TX_DISPMSG;

        *resTempInt =   (int)payload[GET_RESOLUTION_IDX].byteBuffer[9] * 256 +   (int)payload[GET_RESOLUTION_IDX].byteBuffer[8];
        sprintf(dispMsg, "16 bit frame width : %d \r\n", *resTempInt);
        TX_DISPMSG;
    }
    else{
        sprintf(dispMsg, "i2c communication failed. \r\n");
        TX_DISPMSG;
    }
    free(dispMsg);


}


/************************************************************************************
* Function: pixyGetVector
* - this function is to communicate pixyCam using i2c transmit/receive function
*   to request/receive getVector
* Arguments: (PIXY_CMD*) cmds - command structure pointer.
*            (PIXY_RESPONSE*) payload - payload structure pointer.
* return: 0 if succeeded, -1 if failed.
* Author: Gwang Sik Kim
* Date: Feb 25, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int pixyGetVector(unsigned char contMode){

    if (contMode == 0){
        vecDispCont = 0; // continuous mode off.
    }
    else if (contMode == 2){
        vecDispCont = 2; // single line mode
    }
    else{
        vecDispCont = 1; // copy the vector and print it.
    }

    int flag = -1;
    flag = usciB0I2CMstTransmit(pixyCmds[GET_VECTOR_IDX].byteBuffer, pixyCmds[GET_VECTOR_IDX].nBytes);
    if (flag != -1){
        flag = usciB0I2CMstReceive(payload[GET_VECTOR_IDX].byteBuffer, payload[GET_VECTOR_IDX].nBytesResp);
    }
    return flag;
}




void pixyGetVectorConsole(int flag){
    // get vector
    char *dispMsg = (char*) malloc(sizeof(char) * MAX_DISP_MSG_I2C);    //heap memory allocation = char data size * max
    switch(flag){

    case GET_VECTOR_FAIL:
        sprintf(dispMsg, "i2c communication failed. \r\n");
        TX_DISPMSG;
        break;
    case GET_VECTOR_SUCCESS:
        // check if the value is within valid range...
        if (VALID_VEC_X0){ //x0
            if(VALID_VEC_X1){
                if(VALID_VEC_Y0){
                    if(VALID_VEC_Y1){
                        sprintf(dispMsg, "tail x0 = %u, y0 = %u, head x1 = %u, y1 = %u. angle = %.2lf rad, %.2lf deg. \r\n", payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X0], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y0], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X1], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y1], computeAngVector(), radToDeg(computeAngVector()));
                        TX_DISPMSG;
                    }
                }
            }
        }
        else{
            sprintf(dispMsg, "Vector values are not in valid range. (0 <= x <= 78, 0 <= y <= 51) \r\n");
            TX_DISPMSG;
        }
        break;

    default:
        sprintf(dispMsg, "Invalid console flag. \r\n");
        TX_DISPMSG;
        break;
    }
    free(dispMsg);
}



double computeAngVector(void){
    double angle = atan2((double) (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y0] - payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y1]),     (double) (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X1] - payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X0]));
    return angle;
}


double radToDeg(double angleRad){
    double angleDeg = (angleRad / 2 / M_PI) * 360;
    return angleDeg;
}


/************************************************************************************
* Function: pixyVecContLcdDisp
* - this function is to draw LCD line according to the pixy vectors.
* Arguments: (void) n/a vectors are given after pixyGetVector is called, in payload array.
* return: 0 if succeeded, -1 if failed.
* Author: Gwang Sik Kim
* Date: Feb 27, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int pixyVecContLcdDisp(void){
    int flag = -1;
    if (vecDispCont == 1){
        flag = pixyGetVector(vecDispCont); // update vector values

        if (flag == 0){ // valid range of x,y
            nokLcdClear(); // clear the screen.

            // LCD x - 84 pixels, y - 48 pixels. --> y value can be invalid in LCD... check flag.
            flag = nokLcdDrawLine(payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X0], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y0], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X1], payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y1]);
            if (flag != 0){
                char *dispMsg = (char*) malloc(sizeof(char) * MAX_DISP_MSG_I2C);
                sprintf(dispMsg, "Vector values are valid but not fit in LCD pixels. \r\n");
                TX_DISPMSG;
                free(dispMsg);
            }
        }
    }
    return flag;
}


int pixyVecContLcdSingleLine(void){
    int flag = -1;

    if (vecDispCont == 2){
        flag = pixyGetVector(vecDispCont); // update vector values

        if (flag == 0){ // valid range of x,y
            // we don't have posCount here. 'mimicking posCount' from angle.
            vecAngle = computeAngVector();

            //posCntFrHm = ((angle / 2 / M_PI) * 4600);
            psCntTemp = ((vecAngle / 2 / M_PI) * 4600); // for some reason, it doesn't declare the variable. should use global...

            posCntFrHm = (long int) psCntTemp;
            //nokLcdClear(); // clear the screen. nokLcdDispLineMode already included clear mode.
            nokLcdDispLineMode();

            if (flag != 0){
                char *dispMsg = (char*) malloc(sizeof(char) * MAX_DISP_MSG_I2C);
                sprintf(dispMsg, "Vector values are valid but not fit in LCD pixels. \r\n");
                TX_DISPMSG;
                free(dispMsg);
            }
        }
    }
    return flag;
}




/************************************************************************************
* Function: pixyTx
*
* Arguments:
* return: 0 if succeeded, -1 if failed.
* Author: Gwang Sik Kim
* Date: Feb 27, 2022
* Modified: <date of any mods> usually taken care of by rev control
************************************************************************************/
int pixyTx(char* string){
    int flag = -1;
    int n = strlen(string);
    if (n < COMMAND_REQUEST_BYTE_MAX_LENGTH){
        strcpy(pixyCmds[PIXY_TX_IDX].byteBuffer, string);
        flag = usciB0I2CMstTransmit(pixyCmds[PIXY_TX_IDX].byteBuffer, pixyCmds[PIXY_TX_IDX].nBytes);
        if (flag != -1){
            flag = usciB0I2CMstReceive(payload[PIXY_TX_IDX].byteBuffer, payload[PIXY_TX_IDX].nBytesResp);
        }
    }
    return flag;
}


