#ifndef _AM2320_H_
#define _AM2320_H_

void am2320_init(i2c_port_t i2c_num);
int16_t am2320_read_temperature(void);

#endif
