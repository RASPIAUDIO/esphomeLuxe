
#include "es8388_component.h"
#include "esphome/core/hal.h"

#include <soc/io_mux_reg.h>

namespace esphome {
namespace es8388 {

void ES8388Component::setup() {
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
  //reset
  this->write_byte(0, 0x80);
  this->write_byte(0, 0x00);  
  // mute
  this->write_byte(0x19, 0x04);
  // powerup
  this->write_byte(0x01, 0x50);
  this->write_byte(0x02, 0x00);
  // worker mode
  this->write_byte(0x08, 0x00);
  // DAC powerdown
  this->write_byte(0x04, 0xC0);
  // vmidsel/500k ADC/DAC idem
  this->write_byte(0x00, 0x12);

  // i2s 16 bits
  this->write_byte(0x17, 0x18);
  // sample freq 256
  this->write_byte(0x18, 0x02);
  // LIN2/RIN2 for mixer
  this->write_byte(0x26, 0x00);
  // left DAC to left mixer
  this->write_byte(0x27, 0x90);
  // right DAC to right mixer
  this->write_byte(0x2A, 0x90);
  // DACLRC ADCLRC idem
  this->write_byte(0x2B, 0x80);
  this->write_byte(0x2D, 0x00);
  // DAC volume max
  this->write_byte(0x1B, 0x00);
  this->write_byte(0x1A, 0x00);

 
 //mono (L+R)/2
//    this->write_byte(29, 0x20);
 // ADC poweroff
   this->write_byte(3,0xFF);
   // ADC micboost 24 dB
   this->write_byte(9,0x88);   

   // LINPUT1/RINPUT1  
   this->write_byte(10,0x00);  
/////////////////////////////////////    
   // LINPUT2/RINPUT2  
 //  ES8388_Write_Reg(10,0x50);
   // ADC mono left
 //  ES8388_Write_Reg(11,0x02); 
/////////////////////////////////////   
   //i2S 16b
   this->write_byte(12,0x0C); 
   //MCLK 256
   this->write_byte(13,0x02); 
   // ADC high pass filter
   this->write_byte(14,0x30);    
   // ADC attenuation 0 dB   
   this->write_byte(16,0x00);
   this->write_byte(17,0x00);
/*
//ALC (optimized for voice)
   this->write_byte(18, 0xE2);
   this->write_byte(19, 0xC0);
   this->write_byte(20, 0x12);  
   this->write_byte(21, 0x06);   
*/   
// Noise gate -76.5 dB  
   this->write_byte(22, 0x03);      
 
 //ADC power  
  this->write_byte(3,0x00);
  this->write_byte(0x02, 0xF0);
  delay(1);
  this->write_byte(0x02, 0x00);
  // DAC power-up LOUT1/ROUT1 enabled
  this->write_byte(0x04, 0x30);
  this->write_byte(0x03, 0x00);
  // DAC volume max

  //fully differential output Left -L1/R1
  this->write_byte(0x2E, 0x21);
  this->write_byte(0x2F, 0x21);

  //fully differential output Right -L2/R2
  this->write_byte(0x30, 0x21);
  this->write_byte(0x31, 0x21);

  
  // unmute
  this->write_byte(0x19, 0x00);
}

}  // namespace es8388
}  // namespace esphome
