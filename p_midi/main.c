/* Name:FM_parent
 * Author:awazaru
 * Micro Controller :Atmega328P
 * Date :4/9/2016
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include"usart.h"
#include"spi.h"

/*シリアル通信UART*/
#define FOSC   20000000//動作周波数
#define BAUD   9600//ボーレート //PCと通信するときは38400
#define MYUBRR (FOSC/16/BAUD)-1

/*サンプリング周波数*/
#define SAMPLE_F 22000//サンプリングF/2
#define OCR1A_F (FOSC/SAMPLE_F)-1


void spi_ini(){//spi通信設定
  //CSはPD2ピン
  SPCR|=_BV(SPE)|_BV(MSTR);
  /*  SPIE    : SPI割り込み許可
	 SPE     : SPI許可(SPI操作を許可するために必須)
	 DORD    : データ順選択,1:LSBから 0:MSBから
	 MSTR    : 1:主装置動作 0:従装置動作
	 CPOL    : SCK極性選択
	 CPHA    :SCK位相選択
	 SPR1,SPR0 : 00:SCK周波数=fosc/4
  */
  /*SPI状態レジスタ SPSR
    SPIF    : SPI割り込み要求フラグ 転送完了時1
    WCOL    :上書き発生フラグ
  */
  /*SPIデータレジスタSPDR
    8bit
    7 6 5 4 3 2 1 0
    (MSB)       (LSB)
  */

  //SPI通信用関数
}

void serial_ini(void){
  UBRR0=MYUBRR;
  UCSR0A=0b00000000;//受信すると10000000 送信有効になると00100000
  UCSR0B=0b10011000;//送受信有効,受信割り込み許可
  UCSR0C=0b00000110;//データ8bit、非同期、バリティなし、stop1bit
}

unsigned char  midi_buf[3]={"0"};//midiデータ読み込み

void midi_out(void){
  spi_send(midi_buf[2]);
  _delay_us(300);
  spi_send(midi_buf[1]);
  _delay_us(300);
  spi_send(midi_buf[0]);
  _delay_us(300);
}

ISR(USART_RX_vect){//シリアル通信割り込み
  /*MIDIメッセージ (note_on/off)(note_number)(velocity)各1Byteずつ
   *(note_on/off):midi_buf[2]
   *(note_number):midi_buf[1]
   *(velocity):midi_buf[0]*/
  midi_buf[2]=rx_usart();//上位バイト
  midi_buf[1]=rx_usart();//中位バイト
  midi_buf[0]=rx_usart();//下位バイト

  if(midi_buf[2]==0x90||midi_buf[2]==0x80){
    //noteON ch1
    PORTC=~_BV(PIN2);//ch1
    midi_out();
    PORTC|=_BV(PIN2);
  }else if(midi_buf[2]==0x91||midi_buf[2]==0x81){
        //noteON ch2
    PORTC&=~_BV(PIN3);//ch2 SS
    midi_out();
    PORTC|=_BV(PIN3);
  }else if(midi_buf[2]==0x92||midi_buf[2]==0x82){
    //noteON ch3
    PORTC&=~_BV(PIN4);//ch3 SS
    midi_out();
    PORTC|=_BV(PIN4);
  }

}

void pin_ini(void){
  DDRC|=_BV(PIN2)|_BV(PIN3)|_BV(PIN4);
  //PC2:ch1 SS
  //PC3:ch2 SS
  //PC4:ch3 SS
  DDRB|=_BV(PIN1)|_BV(PIN2)|_BV(PIN3)|_BV(PIN5);
  //PB1:LED
  //PB3:MOSI
  //PB5:SCK
  //PB2:SSの誤動作を防ぐため
  
}

int main(void)
{
  spi_ini();
  pin_ini();
  serial_ini();
  sei();//割り込み有効
  
  PORTC|=_BV(PIN2)|_BV(PIN3);
  //SS無効化
  for(;;){
    
  }
  return 0;
}

