/*
 * pixyCam2.h
 *
 *  Created on: 2022. 2. 23.
 *      Author: GwangSik
 */

#ifndef PIXYCAM2_H_
#define PIXYCAM2_H_

// pixy constants
#define         PIXY_SLV_ADDR                           0x54
#define         MAX_DISP_MSG_I2C                        80
#define         COMMAND_BUF_MAX_SIZE                    25
#define         MAX_NUM_COMMAND                         10
#define         COMMAND_REQUEST_BYTE_MAX_LENGTH         10
#define         COMMAND_RESPONSE_BYTE_MAX_LENGTH        25

// pixy command request
#define         PIXY_CMD0_NAME              "getVersion"
#define         PIXY_CMD0_N_BYTES           4
#define         PIXY_CMD1_NAME              "getResolution"
#define         PIXY_CMD1_N_BYTES           5
#define         PIXY_CMD2_NAME              "getVector"
#define         PIXY_CMD2_N_BYTES           6
#define         PIXY_CMD3_NAME              "pixyTx" // this does not exist in pixy. Pixy must NACK.
#define         PIXY_CMD3_N_BYTES           10

// pixy command index
#define     GET_VERSION_IDX             0
#define     GET_RESOLUTION_IDX          1
#define     GET_VECTOR_IDX              2
#define     PIXY_TX_IDX                 3

// pixy response number of bytes
#define     PIXY_RESP0_N_BYTES          22
#define     PIXY_RESP1_N_BYTES          10
#define     PIXY_RESP2_N_BYTES          15
#define     PIXY_RESP3_N_BYTES          10



// pixy vector constants
#define     VECTOR_IDX_X0               8
#define     VECTOR_IDX_Y0               9
#define     VECTOR_IDX_X1               10
#define     VECTOR_IDX_Y1               11

#define     VECTOR_X_MIN                0
#define     VECTOR_Y_MIN                0
#define     VECTOR_X_MAX                78
#define     VECTOR_Y_MAX                51

#define     VALID_VEC_X0                (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X0] <= VECTOR_X_MAX)
#define     VALID_VEC_X1                (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_X1] <= VECTOR_X_MAX)
#define     VALID_VEC_Y0                (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y0] <= VECTOR_Y_MAX)
#define     VALID_VEC_Y1                (payload[GET_VECTOR_IDX].byteBuffer[VECTOR_IDX_Y1] <= VECTOR_Y_MAX)

// console message flag
#define     GET_VECTOR_FAIL             -1
#define     GET_VECTOR_SUCCESS          0
#define     GET_VER_FAIL                -1
#define     GET_VER_SUCCESS             0
#define     GET_RES_FAIL                -1
#define     GET_RES_SUCCESS             0

// structure
typedef struct PIXY_CMD{
    const char* name;
    unsigned char nBytes;
    unsigned char byteBuffer[COMMAND_REQUEST_BYTE_MAX_LENGTH];

}PIXY_CMD;

// structure
typedef struct PIXY_RESPONSE{
    const char* name;
    unsigned char nBytesResp;
    unsigned char byteBuffer[COMMAND_RESPONSE_BYTE_MAX_LENGTH];

}PIXY_RESPONSE;

// prototype
void pixyCmdInit(void);
void pixyResponseInit(void);
int pixyGetVersion(void);
int pixyGetResolution(void);
int pixyGetVector(unsigned char contMode);
int pixyVecContLcdDisp(void);
int pixyTx(char* string);
double computeAngVector(void);
double radToDeg(double angleRad);
int pixyVecContLcdSingleLine(void);
void pixyGetVectorConsole(int flag);
void pixyGetVerConsole(int flag);
void pixyGetResConsole(int flag);


// external global variable
extern unsigned char vecDispCont;
//extern const unsigned char slaveAddr;


#endif /* PIXYCAM2_H_ */
