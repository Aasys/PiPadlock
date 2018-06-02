//Aasys Sesta
//Padlock with display
// Using ADAfruit 128x128 1.4" color display - ST7735R
//Using the CAP1188 - 8 multitouch sensor to create a padlock using I2C
//Touching certain combination of wire unlocks the padlock
//Lock/Unlock indicated by RED and GREEN LED and colored X and O in display
//Wiring picture included
//led wires
// <Wiring>
//
//                        LEDs
// Green Led : Pin 36 | GPIO 16 | Padlock state Unocked
// Red Led     : Pin 38 | GPIO 20 | Padlock state Locked
//                        Display - Adafruit 1.4" 128x128
// Reset : Pin 35 | GPIO 19
// D/C   : Pin 37 | GPIO 26
// TCS    : Pin 24 | SPI0_CE0_N
// SI       : Pin 19 | SPI0_MOSI
// SO      : Pin 21 | SPI0_MISO
// SCK    : Pin 23 | SPI0_SCLK
// Gnd    : Ground
// Vin     : Pin 2 |  5V
//                       CAP1188
// Gnd    : Ground
// Vin     : Pin 1 |  3.3V
// SDA   : Pin 3 | SDA1 I2C
// SCK   : Pin 5 | SCL1 I2C
// Open wires C1- C8
// </Wiring>


#include <u.h>
#include <libc.h>

#define WIDTH 128
#define HEIGHT 128

#define CAP1188_SENINPUTSTATUS 0x3
#define CAP1188_MTBLK 0x2A
#define CAP1188_LEDLINK 0x72
#define CAP1188_PRODID 0xFD
#define CAP1188_MANUID 0xFE
#define CAP1188_STANDBYCFG 0x41
#define CAP1188_REV 0xFF
#define CAP1188_MAIN 0x00
#define CAP1188_MAIN_INT 0x01
#define CAP1188_LEDPOL 0x73

//Multitouch - C8 C2 C1
#define UNLOCK_CODE 131


void display_locked(void);
void display_unlocked(void);


int f_gpio, f_spi, f_spi_ctl, f_i2c;
uchar buf[2];


void padlock_lock(void) {
	print("Locking Padlock\n");
	fprint(f_gpio, "set 16 0");
	fprint(f_gpio, "set 20 1");
	display_locked();
	print("LOCKED Padlock\n");
}

void padlock_unlock(void) {
	print("Unlocking Padlock\n");
	fprint(f_gpio, "set 16 1");
	fprint(f_gpio, "set 20 0");
	display_unlocked();
	print("UNLOCKED Padlock\n");
}

void display_reset(void) {
	fprint(f_gpio, "set 19 0");
	sleep(300);
	fprint(f_gpio, "set 19 1");
}

void spi_command_mode(void) {
	fprint(f_gpio, "set 26 0");
	sleep(150);
}

void spi_data_mode(void) {
	fprint(f_gpio, "set 26 1");
	sleep(150);	
}

void spi_write(uchar val) {
	buf[0] = val;
	write(f_spi, buf, 1);
}

void spi_write2(uchar hi, uchar lo) {
	buf[0] = lo;
	buf[1] = hi;
	write(f_spi, buf, 2);
}
void spi_write_delay(uchar val, int delay) {
	buf[0] = val;
	write(f_spi, buf, 1);
	sleep(delay);
}

void display_locked(void) {
	spi_command_mode();
	spi_write(0x2A);
	spi_data_mode();
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x7F);

	spi_command_mode();
	spi_write(0x2B);
	spi_data_mode();
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x7F);

	spi_command_mode();
	spi_write(0x2C);
	spi_data_mode();
	
	int x, y;

	int d;

	for (x =0; x < WIDTH; x++) {
		for (y =0; y < HEIGHT; y++) {
			//smaller squre area
			if (x >= 14 && x <= WIDTH -14 && y >= 14 && y<= HEIGHT - 14 ) {
				// in X region
				d = (x - 14) * (HEIGHT -28) - (y - 14) * (WIDTH - 28 - 14);
				if (d >= 0) {
					d = (x - 28) * (HEIGHT -28) - (y - 14) * (WIDTH - 14 - 28);
					if (d <= 0) {
						spi_write2(0xFF, 0xFF);
						continue;
					}
				}

				d = (x - 14) * (28 - HEIGHT) - (y - HEIGHT + 14) * (WIDTH - 28 - 14);
				if (d <= 0) {
					d = (x - 28) * (28 - HEIGHT) - (y - HEIGHT + 14) * (WIDTH - 28 - 14);
					if (d >= 0) {
						spi_write2(0xFF, 0xFF);
						continue;
					}
				}

			}
			spi_write2(0xF8, 0X00);		
		}
	}
}

void display_unlocked(void) {
	spi_command_mode();
	spi_write(0x2A);
	spi_data_mode();
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x7F);

	spi_command_mode();
	spi_write(0x2B);
	spi_data_mode();
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x00);
	spi_write(0x7F);

	spi_command_mode();
	spi_write(0x2C);
	spi_data_mode();
	
	int x, y;
	
	double center_x = WIDTH / 2.0;
	double center_y = HEIGHT / 2.0;
	double radius_out = (WIDTH - 28) / 2.0;
	double radius_out_sq = radius_out * radius_out;
	double radius_in = (WIDTH - 28 -14) / 2.0;
	double radius_in_sq = radius_in * radius_in;
	
	double d;
	for (x =0; x < WIDTH; x++) {
		for (y =0; y < HEIGHT; y++) {
			// circle region
			d =  (x - center_x) * (x - center_x) + (y - center_y) * (y - center_y);
			if (d <= radius_out_sq && d >= radius_in_sq) {
				spi_write2(0xFF, 0XFF);
				continue;
			}
			spi_write2(0xA1, 0X01);
		}
	}
}

void open_gpio(void) {
	print("Opening GPIO...\n");

	f_gpio = open("/dev/gpio", ORDWR);

	if (f_gpio < 0) {
		bind("#G", "/dev", MAFTER);
		f_gpio = open("/dev/gpio", ORDWR);
		
		if (f_gpio < 0) {
			print("GPIO open error %r\n");
		}
	}
}

void init_gpio(void) {
	print("Initializing GPIO...\n");

	fprint(f_gpio, "function 16 out"); // Red Led
	fprint(f_gpio, "function 20 out"); // Green Led

	fprint(f_gpio, "function 19 out");  // Reset
	fprint(f_gpio, "function 26 out"); // D/C

	fprint(f_gpio, "set 19 1");
	fprint(f_gpio, "set 26 0");
}


void open_spi(void) {
	print("Opening SPI...\n");

	f_spi	 = open("/dev/spi0", ORDWR);

	if (f_spi < 0) {
		bind("#π", "/dev", MAFTER);
		f_spi = open("/dev/spi0", ORDWR);
		
		if (f_spi < 0) {
			print("SPI open error %r\n");
		}
	}

	f_spi_ctl = open("/dev/spictl", ORDWR);
		
	if (f_spi_ctl < 0) {
		print("SPI CTL open error %r\n");
	}	
}

void init_spi(void) {
	print("Initializing SPI...\n");

	fprint(f_spi_ctl, "clock 25");

	display_reset();

	// init display
	spi_command_mode();
	spi_write_delay(0x01, 120);
	spi_write_delay(0x11, 120);
	spi_write_delay(0x29, 120);
	spi_write_delay(0x13, 120);


	//set color mode 16 bits
	spi_command_mode();
	spi_write(0x3A);
	spi_data_mode();
	spi_write(0x05);	
}

void open_i2c(void) {
	// init I2C - CAP1188
	f_i2c = open("/dev/i2c.29.data", ORDWR);
	if(f_i2c < 0) {
		bind("#J29", "/dev", MAFTER);
		f_i2c = open("/dev/i2c.29.data", ORDWR);
		if(f_i2c < 0)
			print("I2C open error: %r\n");
	}

}

void init_i2c(void) {
	// allow multiple touches
	buf[0] = CAP1188_MTBLK;
	buf[1] = 0x00;
	pwrite(f_i2c, buf, 2, 0);
	sleep(50);
	// Have LEDs follow touches
	buf[0] = CAP1188_LEDLINK;
	buf[1] = 0xFF;
	pwrite(f_i2c, buf, 2, 0);
	sleep(50);
	// speed up off
	buf[0] = CAP1188_STANDBYCFG;
	buf[1] = 0x39;
	pwrite(f_i2c, buf, 2, 0);
	sleep(50);
}

void main(void) {
	open_gpio();
	init_gpio();

	open_spi();
	init_spi();

	open_i2c();
	init_i2c();

	padlock_lock();
	print("DONE\n");

	int state = 0;
	int lock_counter = 0;
	while (1) {
		//read input
		pread(f_i2c, buf, 1, CAP1188_SENINPUTSTATUS);

		if (buf[0] == UNLOCK_CODE) {
			state = 1;
			lock_counter = 0;
			padlock_unlock();
		}

		if (state == 1) {
			lock_counter++;
		}

		//auto lock after few secs
		if (state == 1 && lock_counter > 20) {
			state = 0;
			padlock_lock();

		}

		//clear interrupt
		buf[0] = CAP1188_MAIN;
		buf[1] = 0x00;
		pwrite(f_i2c, buf, 2, 0);

		//sleep for 100ms
		sleep(100);
	}
	
}
