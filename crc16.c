/****************************************************************************************
*  函 数: CRC16_Checkout
*  描 述:
*  CRC16 循环冗余校验算法。
*  参 数 一: *puchMsg:需要校验的字符串指针
*  参 数 二: usDataLen:要校验的字符串长度
*  返 回 值:
*  返回 CRC16 校验码
****************************************************************************************/
unsigned int CRC16_Checkout (unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned int i,j,crc_reg,check;
    crc_reg = 0xFFFF;

    for(i = 0;i < usDataLen;i++) {
        crc_reg = (crc_reg >> 8) ^ puchMsg[i];
        for(j=0;j<8;j++) {
            check = crc_reg & 0x0001;
            crc_reg >>= 1;
            if(check==0x0001) {
                crc_reg ^= 0xA001;
            }
        }
    }

    return crc_reg;
}