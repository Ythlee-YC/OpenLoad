#include "bsp_norflash.h"
#include "bsp_spi.h"
#include "delay.h"
#include "stm32f1xx_hal_gpio.h"
#include "usart.h"

uint16_t W25QXX_TYPE = W25Q64; // 默认是W25Q64

// 初始化SPI FLASH的IO口
void W25QXX_Init(void)
{
    uint8_t temp;
    GPIO_InitTypeDef GPIO_Initure;

    __HAL_RCC_GPIOB_CLK_ENABLE(); // 使能GPIOB时钟

    // PB12
    GPIO_Initure.Pin = GPIO_PIN_12;            // PB12
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;   // 推挽输出
    GPIO_Initure.Pull = GPIO_PULLUP;           // 上拉
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH; // 高速
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);       // 初始化

    W25QXX_CS(1);                            // SPI FLASH不选中
    spi2_init();                             // 初始化SPI
    spi2_set_speed(SPI_BAUDRATEPRESCALER_2); // 设置为42M时钟,高速模式
    W25QXX_TYPE = W25QXX_ReadID();           // 读取FLASH ID.
    if (W25QXX_TYPE == W25Q64)               // SPI FLASH为W25Q64
    {
        temp = W25QXX_ReadSR(3); // 读取状态寄存器3，判断地址模式
        if ((temp & 0X01) == 0)  // 如果不是4字节地址模式,则进入4字节地址模式
        {
            W25QXX_CS(0);                               // 选中
            spi2_read_write_byte(W25X_Enable4ByteAddr); // 发送进入4字节地址模式指令
            W25QXX_CS(1);                               // 取消片选
        }
    }
}

// 读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
// 状态寄存器1：
// BIT7  6   5   4   3   2   1   0
// SPR   RV  TB BP2 BP1 BP0 WEL BUSY
// SPR:默认0,状态寄存器保护位,配合WP使用
// TB,BP2,BP1,BP0:FLASH区域写保护设置
// WEL:写使能锁定
// BUSY:忙标记位(1,忙;0,空闲)
// 默认:0x00
// 状态寄存器2：
// BIT7  6   5   4   3   2   1   0
// SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
// 状态寄存器3：
// BIT7      6    5    4   3   2   1   0
// HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
// regno:状态寄存器号，范:1~3
// 返回值:状态寄存器值
uint8_t W25QXX_ReadSR(uint8_t regno)
{
    uint8_t byte = 0, command = 0;
    switch (regno)
    {
    case 1:
        command = W25X_ReadStatusReg1; // 读状态寄存器1指令
        break;
    case 2:
        command = W25X_ReadStatusReg2; // 读状态寄存器2指令
        break;
    case 3:
        command = W25X_ReadStatusReg3; // 读状态寄存器3指令
        break;
    default:
        command = W25X_ReadStatusReg1;
        break;
    }
    W25QXX_CS(0);                      // 使能器件
    spi2_read_write_byte(command);     // 发送读取状态寄存器命令
    byte = spi2_read_write_byte(0Xff); // 读取一个字节
    W25QXX_CS(1);                      // 取消片选
    return byte;
}
// 写W25QXX状态寄存器
void W25QXX_Write_SR(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;
    switch (regno)
    {
    case 1:
        command = W25X_WriteStatusReg1; // 写状态寄存器1指令
        break;
    case 2:
        command = W25X_WriteStatusReg2; // 写状态寄存器2指令
        break;
    case 3:
        command = W25X_WriteStatusReg3; // 写状态寄存器3指令
        break;
    default:
        command = W25X_WriteStatusReg1;
        break;
    }
    W25QXX_CS(0);                  // 使能器件
    spi2_read_write_byte(command); // 发送写取状态寄存器命令
    spi2_read_write_byte(sr);      // 写入一个字节
    W25QXX_CS(1);                  // 取消片选
}
// W25QXX写使能
// 将WEL置位
void W25QXX_Write_Enable(void)
{
    W25QXX_CS(0);                           // 使能器件
    spi2_read_write_byte(W25X_WriteEnable); // 发送写使能
    W25QXX_CS(1);                           // 取消片选
}
// W25QXX写禁止
// 将WEL清零
void W25QXX_Write_Disable(void)
{
    W25QXX_CS(0);                            // 使能器件
    spi2_read_write_byte(W25X_WriteDisable); // 发送写禁止指令
    W25QXX_CS(1);                            // 取消片选
}

// 读取芯片ID
// 返回值如下:
// 0XEF13,表示芯片型号为W25Q80
// 0XEF14,表示芯片型号为W25Q16
// 0XEF15,表示芯片型号为W25Q32
// 0XEF16,表示芯片型号为W25Q64
// 0XEF17,表示芯片型号为W25Q128
// 0XEF18,表示芯片型号为W25Q256
uint16_t W25QXX_ReadID(void)
{
    uint16_t Temp = 0;
    W25QXX_CS(0);
    spi2_read_write_byte(0x90); // 发送读取ID命令
    spi2_read_write_byte(0x00);
    spi2_read_write_byte(0x00);
    spi2_read_write_byte(0x00);
    Temp |= spi2_read_write_byte(0xFF) << 8;
    Temp |= spi2_read_write_byte(0xFF);
    W25QXX_CS(1);
    return Temp;
}
// 读取SPI FLASH
// 在指定地址开始读取指定长度的数据
// pBuffer:数据存储区
// ReadAddr:开始读取的地址(24bit)
// NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    uint16_t i;
    W25QXX_CS(0);                        // 使能器件
    spi2_read_write_byte(W25X_ReadData); // 发送读取命令
    if (W25QXX_TYPE == W25Q256)          // 如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        spi2_read_write_byte((uint8_t)((ReadAddr) >> 24));
    }
    spi2_read_write_byte((uint8_t)((ReadAddr) >> 16)); // 发送24bit地址
    spi2_read_write_byte((uint8_t)((ReadAddr) >> 8));
    spi2_read_write_byte((uint8_t)ReadAddr);
    for (i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = spi2_read_write_byte(0XFF); // 循环读数
    }
    W25QXX_CS(1);
}
// SPI在一页(0~65535)内写入少于256个字节的数据
// 在指定地址开始写入最大256字节的数据
// pBuffer:数据存储区
// WriteAddr:开始写入的地址(24bit)
// NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void W25QXX_Write_Page(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;
    W25QXX_Write_Enable();                  // SET WEL
    W25QXX_CS(0);                           // 使能器件
    spi2_read_write_byte(W25X_PageProgram); // 发送写页命令
    if (W25QXX_TYPE == W25Q256)             // 如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        spi2_read_write_byte((uint8_t)((WriteAddr) >> 24));
    }
    spi2_read_write_byte((uint8_t)((WriteAddr) >> 16)); // 发送24bit地址
    spi2_read_write_byte((uint8_t)((WriteAddr) >> 8));
    spi2_read_write_byte((uint8_t)WriteAddr);
    for (i = 0; i < NumByteToWrite; i++)
        spi2_read_write_byte(pBuffer[i]); // 循环写数
    W25QXX_CS(1);                         // 取消片选
    W25QXX_Wait_Busy();                   // 等待写入结束
}
// 无检验写SPI FLASH
// 必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
// 具有自动换页功能
// 在指定地址开始写入指定长度的数据,但是要确保地址不越界!
// pBuffer:数据存储区
// WriteAddr:开始写入的地址(24bit)
// NumByteToWrite:要写入的字节数(最大65535)
// CHECK OK
void W25QXX_Write_NoCheck(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain = 256 - WriteAddr % 256; // 单页剩余的字节数
    if (NumByteToWrite <= pageremain)
        pageremain = NumByteToWrite; // 不大于256个字节
    while (1)
    {
        W25QXX_Write_Page(pBuffer, WriteAddr, pageremain);
        if (NumByteToWrite == pageremain)
            break; // 写入结束了
        else       // NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain; // 减去已经写入了的字节数
            if (NumByteToWrite > 256)
                pageremain = 256; // 一次可以写入256个字节
            else
                pageremain = NumByteToWrite; // 不够256个字节了
        }
    };
}
// 写SPI FLASH
// 在指定地址开始写入指定长度的数据
// 该函数带擦除操作!
// pBuffer:数据存储区
// WriteAddr:开始写入的地址(24bit)
// NumByteToWrite:要写入的字节数(最大65535)
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *W25QXX_BUF;
    W25QXX_BUF = W25QXX_BUFFER;
    secpos = WriteAddr / 4096; // 扇区地址
    secoff = WriteAddr % 4096; // 在扇区内的偏移
    secremain = 4096 - secoff; // 扇区剩余空间大小
    // printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
    if (NumByteToWrite <= secremain)
        secremain = NumByteToWrite; // 不大于4096个字节
    while (1)
    {
        W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096); // 读出整个扇区的内容
        for (i = 0; i < secremain; i++)               // 校验数据
        {
            if (W25QXX_BUF[secoff + i] != 0XFF)
                break; // 需要擦除
        }
        if (i < secremain) // 需要擦除
        {
            W25QXX_Erase_Sector(secpos);    // 擦除这个扇区
            for (i = 0; i < secremain; i++) // 复制
            {
                W25QXX_BUF[i + secoff] = pBuffer[i];
            }
            W25QXX_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096); // 写入整个扇区
        }
        else
            W25QXX_Write_NoCheck(pBuffer, WriteAddr, secremain); // 写已经擦除了的,直接写入扇区剩余区间.
        if (NumByteToWrite == secremain)
            break; // 写入结束了
        else       // 写入未结束
        {
            secpos++;   // 扇区地址增1
            secoff = 0; // 偏移位置为0

            pBuffer += secremain;        // 指针偏移
            WriteAddr += secremain;      // 写地址偏移
            NumByteToWrite -= secremain; // 字节数递减
            if (NumByteToWrite > 4096)
                secremain = 4096; // 下一个扇区还是写不完
            else
                secremain = NumByteToWrite; // 下一个扇区可以写完了
        }
    };
}
// 擦除整个芯片
// 等待时间超长...
void W25QXX_Erase_Chip(void)
{
    W25QXX_Write_Enable(); // SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS(0);                         // 使能器件
    spi2_read_write_byte(W25X_ChipErase); // 发送片擦除命令
    W25QXX_CS(1);                         // 取消片选
    W25QXX_Wait_Busy();                   // 等待芯片擦除结束
}
// 擦除一个扇区
// Dst_Addr:扇区地址 根据实际容量设置
// 擦除一个扇区的最少时间:150ms
void W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
    // 监视falsh擦除情况,测试用
    // printf("fe:%x\r\n",Dst_Addr);
    Dst_Addr *= 4096;
    W25QXX_Write_Enable(); // SET WEL
    W25QXX_Wait_Busy();
    W25QXX_CS(0);                           // 使能器件
    spi2_read_write_byte(W25X_SectorErase); // 发送扇区擦除指令
    if (W25QXX_TYPE == W25Q256)             // 如果是W25Q256的话地址为4字节的，要发送最高8位
    {
        spi2_read_write_byte((uint8_t)((Dst_Addr) >> 24));
    }
    spi2_read_write_byte((uint8_t)((Dst_Addr) >> 16)); // 发送24bit地址
    spi2_read_write_byte((uint8_t)((Dst_Addr) >> 8));
    spi2_read_write_byte((uint8_t)Dst_Addr);
    W25QXX_CS(1);       // 取消片选
    W25QXX_Wait_Busy(); // 等待擦除完成
}
// 等待空闲
void W25QXX_Wait_Busy(void)
{
    while ((W25QXX_ReadSR(1) & 0x01) == 0x01)
        ; // 等待BUSY位清空
}
// 进入掉电模式
void W25QXX_PowerDown(void)
{
    W25QXX_CS(0);                         // 使能器件
    spi2_read_write_byte(W25X_PowerDown); // 发送掉电命令
    W25QXX_CS(1);                         // 取消片选
    delay_us(3);                          // 等待TPD
}
// 唤醒
void W25QXX_WAKEUP(void)
{
    W25QXX_CS(0);                                // 使能器件
    spi2_read_write_byte(W25X_ReleasePowerDown); //  send W25X_PowerDown command 0xAB
    W25QXX_CS(1);                                // 取消片选
    delay_us(3);                                 // 等待TRES1
}
