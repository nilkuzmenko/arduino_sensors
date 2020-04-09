#define dis_w 24
#define dis_h 12

const unsigned char dis[] = {
 0xF8, 0x81, 0x07, 0xCC, 0x81, 0x0D, 0x44, 0xF9, 0x19, 0x44, 0xF9, 0x11, 
  0xC7, 0x81, 0xF1, 0xC4, 0x81, 0x11, 0xC4, 0x81, 0x11, 0xC7, 0x81, 0xF1, 
  0x44, 0xF9, 0x11, 0x44, 0xF9, 0x19, 0xCC, 0x81, 0x0D, 0xF8, 0x81, 0x07, 
	};

#define tem_w 17
#define tem_h 5
const unsigned char tem[] = {
  0xDF, 0x17, 0x01, 0x44, 0xB0, 0x01, 0xC4, 0x51, 0x01, 0x44, 0x10, 0x01, 
  0xC4, 0x17, 0x01, };
  
#define shup_w 17
#define shup_h 6
const unsigned char shup[] = {
  0x55, 0xF4, 0x01, 0x55, 0x14, 0x01, 0xD5, 0x17, 0x01, 0x15, 0x14, 0x01, 
  0xDF, 0x17, 0x01, 0x10, 0x00, 0x00, };
  
#define gaz_w 17
#define gaz_h 5
const unsigned char gaz[] = {
  0xDF, 0xF7, 0x01, 0x41, 0x04, 0x01, 0xC1, 0xC7, 0x01, 0x41, 0x04, 0x01, 
  0x41, 0xF4, 0x01, };

#define vla_w 17
#define vla_h 5
const unsigned char vla[] = {
  0x1F, 0xF1, 0x01, 0x91, 0x12, 0x01, 0x4F, 0xF4, 0x01, 0x51, 0x14, 0x01, 
  0x5F, 0x14, 0x01, };

#define wifi_width 14
#define wifi_height 7
const unsigned char wifi_bits[] = {
  0xF0, 0x03, 0x04, 0x08, 0xF2, 0x13, 0x09, 0x24, 0xE4, 0x09, 0x10, 0x02, 
  0xC0, 0x00, 0xC0, 0x00,};


#define server_width 14
#define server_height 7
const unsigned char server_bits[] = {
  0xFE, 0x1F, 0x01, 0x20, 0x55, 0x29, 0x55, 0x21, 0x55, 0x21, 0x55, 0x29, 
  0x01, 0x20, 0xFE, 0x1F, };