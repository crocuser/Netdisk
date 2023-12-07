#include "protocol.h"

PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen=sizeof(PDU)+uiMsgLen;
    PDU *pdu=(PDU*)malloc(uiPDULen);
    if(pdu==NULL)
    {
        exit(EXIT_FAILURE);//申请空间失败，退出程序
    }
    memset(pdu,0,uiPDULen);//初始化
    pdu->uiPDULen=uiPDULen;//数据总大小
    pdu->uiMsgLen=uiMsgLen;//实际大小
    return pdu;
}
