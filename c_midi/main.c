/* Name:FMchild
 * Author: awazaru
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include"usart.h"
#include"spi.h"

/*サンプリング周波数*/
#define FOSC 20000000//動作周波数
#define TABLE 360//sinテーブルの要素数
#define SAMPLE_F 22000//サンプリングF/2
#define OCR1A_F (FOSC/SAMPLE_F)-1
#define BASIC_F (SAMPLE_F/TABLE)//基本周波数 サンプリング周波数/c_sinの要素数

#define FIRST_POINT 5 //sinテーブルの初期位置 モジュレーションテーブルの最大値分

/*音階*/
/*cnt_sp配列*/
/*MIDI音階に対応させたカウントアップの配列*/
float midi_cnt_up[128]={0.13,0.14,0.15,0.16,0.17,0.18,0.19,0.2,0.21,0.23,0.24,0.25,0.27,0.28,0.3,0.32,0.34,0.36,0.38,0.4,0.42,0.45,0.48,0.51,0.54,0.57,0.6,0.64,0.67,0.71,0.76,0.8,0.85,0.9,0.95,1.01,1.07,1.13,1.2,1.27,1.35,1.43,1.51,1.6,1.7,1.8,1.91,2.02,2.14,2.27,2.4,2.55,2.7,2.86,3.03,3.21,3.4,3.6,3.81,4.04,4.28,4.54,4.81,5.09,5.39,5.71,6.05,6.41,6.8,7.2,7.63,8.08,8.56,9.07,9.61,10.18,10.79,11.43,12.11,12.83,13.59,14.4,15.26,16.16,17.12,18.14,19.22,20.36,21.58,22.86,24.22,25.66,27.18,28.8,30.51,32.33,34.25,36.29,38.44,40.73,43.15,45.72,48.44,51.32,54.37,57.6,61.03,64.65,68.5,72.57,76.89,81.46,86.3,91.43,96.87,102.63,108.73,115.2,122.05,129.31,137,145.14,153.77,162.92,172.6,182.87,193.74,205.26};

/*sinテーブル*/
/*キャリア用テーブル(変調をかけられる側)*/
uint8_t c_sin[TABLE+10]={130,132,134,136,139,141,143,145,147,150,152,154,156,158,160,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,202,204,206,208,209,211,213,214,216,218,219,221,222,224,225,227,228,229,231,232,233,234,236,237,238,239,240,241,242,243,244,245,246,247,247,248,249,249,250,251,251,252,252,253,253,253,254,254,254,255,255,255,255,255,255,255,255,255,255,255,254,254,254,253,253,253,252,252,251,251,250,249,249,248,247,247,246,245,244,243,242,241,240,239,238,237,236,234,233,232,231,229,228,227,225,224,222,221,219,218,216,214,213,211,209,208,206,204,202,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,160,158,156,154,152,150,147,145,143,141,139,136,134,132,130,128,125,123,121,119,116,114,112,110,108,105,103,101,99,97,95,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,53,51,49,47,46,44,42,41,39,37,36,34,33,31,30,28,27,26,24,23,22,21,19,18,17,16,15,14,13,12,11,10,9,8,8,7,6,6,5,4,4,3,3,2,2,2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,2,2,3,3,4,4,5,6,6,7,8,8,9,10,11,12,13,14,15,16,17,18,19,21,22,23,24,26,27,28,30,31,33,34,36,37,39,41,42,44,46,47,49,51,53,54,56,58,60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,95,97,99,101,103,105,108,110,112,114,116,119,121,123,125,128,130,132,134,136,139,141,143,145,147,150};

/*モジュレーション用テーブル(変調をかける側)*/
//キャリアの100倍の角速度
int8_t m_sin[TABLE]={5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0,5,0,-5,5,5,-5,0,5,0,-5,0,5,-5,-5,5,0,-5,0};


/*構造体*/
struct sound {
  float table_cnt_sp;//カウントスピード
  float buf_cnt;
  uint16_t now_sin_cnt;//sin_tableの最新のカウンタ値
};

struct sound sound;


void spi_ini(){//spi通信設定
  //CSはPD2ピン
  SPCR|=_BV(SPE)|_BV(SPIE);
}

void timer_ini(void){
  /***************ATTINY861*********
  //Timer0
  TCCR0A|=_BV(TCW);//16bit
  TCCR0B|=_BV(CS00);//No divide
  TIMSK|=_BV(OCIE0A);//Compare Match A interrupt enable
  OCR0A=(FOSC/(SAMPLE_F))-1;//Maiking Sumpling Freqency

  //Timer1
  TCCR1A|=_BV(COM1A1)|_BV(COM1B1)|_BV(PWM1A)|_BV(PWM1B);//match and cancel/ PWM from OC1A(if WGM1,0:00 fast PWM mode)
  TCCR1B|=_BV(CS10);//No divide
  OCR1A=0;
  OCR1B=0;
  **********************************/
  

  //16bit timer For Timer interrupt
  TCCR1B|=_BV(WGM12)|_BV(CS10);//0CR1A基準 比較一致タイマ 分周なし
  TIMSK1|=_BV(OCIE1A);//比較A一致割り込み有効
  OCR1A=(FOSC/(SAMPLE_F))-1;//サンプリング周波数
  
  //8bitタイマ PWM用
  TCCR0A|=_BV(COM0A1)|_BV(COM0B1)|_BV(WGM01)|_BV(WGM00);//高速PWM 比較一致でLOW OC0Aピン OC0Bピン
  TCCR0B|=_BV(CS00);
  OCR0A=0;
  OCR0B=0;

}

ISR(TIMER1_COMPA_vect){//タイマ1A比較一致割り込み
  sound.now_sin_cnt=sound.buf_cnt+m_sin[sound.now_sin_cnt];
  OCR0A=c_sin[sound.now_sin_cnt];//発音
  sound.buf_cnt+=sound.table_cnt_sp;
  
  if(sound.buf_cnt>(TABLE+FIRST_POINT)){
    sound.buf_cnt=FIRST_POINT;
  }
}

ISR(SPI_STC_vect){//SPI転送完了割り込み
  /*MIDImessage (note_on/off)(note_number)(velocity) 1Byte each
   *(note_on/off):midi_buf[2]
   *(note_number):midi_buf[1]
   *(velocity):midi_buf[0]*/
  
  unsigned char midi_buf[3]={"0"};//input MIDI data
  //  while(!(SPSR&(1<<SPIF)));
  midi_buf[2]=SPDR;//high
  while(!(SPSR&(1<<SPIF)));
  midi_buf[1]=SPDR;//middle
  while(!(SPSR&(1<<SPIF)));
  midi_buf[0]=SPDR;//low

  /*
    //CH1
  if(midi_buf[2]==0x90){
    //noteON ch1
    PORTB|=_BV(PIN1);
    sound.table_cnt_sp=midi_cnt_up[midi_buf[1]];
  }else if((midi_buf[2]==0x80||(midi_buf[2]==0x90&&midi_buf[0]==0x00))){
//noteOFF or noteON velocity 0
    PORTB&=~_BV(PIN1);
    sound.table_cnt_sp=0;
  }
  */
  
  /*
  //CH2
    if(midi_buf[2]==0x91){
  //noteON ch2
  PORTB|=_BV(PIN1);
    sound.table_cnt_sp=midi_cnt_up[midi_buf[1]];
    }else if((midi_buf[2]==0x81||(midi_buf[2]==0x91&&midi_buf[0]==0x00))){
  //noteOFF or noteON velocity 0
  PORTB&=~_BV(PIN1);
  sound.table_cnt_sp=0;
    }
    */

    
  //CH3
  if(midi_buf[2]==0x92){
    //noteON ch3
    PORTB|=_BV(PIN1);
    sound.table_cnt_sp=midi_cnt_up[midi_buf[1]];
  }else if((midi_buf[2]==0x82||(midi_buf[2]==0x92&&midi_buf[0]==0x00))){
    //noteOFF or noteON velocity 0
    PORTB&=~_BV(PIN1);
    sound.table_cnt_sp=0;
    }
    
  OCR0B=midi_buf[0]*2;//velocityの反映
}

void pin_ini(void){
  /*外部割り込み*/
  //  EICRA|=_BV(ISC01)|_BV(ISC00);
  //  EIMSK|=_BV(INT0);
  //INT0ピンがLowレベルで発生
  DDRD|=_BV(PIN6)|_BV(PIN5);
  //PD6:OC0A
  //PD5:OC0B

  DDRB|=_BV(PIN1)|_BV(PIN4);
  //PB1:LED
  //PB4:MISO
}

int main(void){
  sound.table_cnt_sp=0;
  sound.buf_cnt=0;
  sound.now_sin_cnt=FIRST_POINT;;

  spi_ini();
  pin_ini();
  timer_ini();

  sei();//割り込み有効
  
  for(;;){

  }
  return 0;
}

