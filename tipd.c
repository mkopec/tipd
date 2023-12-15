#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <i2c/smbus.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_REG_LEN 65

static void print_tps_reg(uint8_t *data, uint8_t len) {
		if (data == NULL)
				return;
		
		printf("LEN %2d\tVAL 0x", len);

		for (int i = 0; i < len; ++i)
				printf("%x", data[i]);
		
		printf("\n");
}

int main(int argc, char **argv) {
		int file;
		int adapter_nr = 0; /* probably dynamically determined */
		char filename[20];

		snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
		file = open(filename, O_RDWR);
		if (file < 0) {
				printf("Could not open file: error %d\n", file);
				exit(1);
		}

		int addr = 0x23; /* The I2C address */

		if (ioctl(file, I2C_SLAVE, addr) < 0) {
				/* ERROR HANDLING; you can check errno to see what went wrong */
				printf("Could not acquire ioctl\n");
				exit(1);
		}

		uint8_t reg;
		int32_t res;

		uint8_t buf[MAX_REG_LEN];
		
		for (int i = 0; i < 0x7f; ++i) {
				reg = i+1;

				printf("REG %2x\t", reg);

				res = i2c_smbus_read_block_data(file, reg, buf);
				if (res < 0) {
						/* ERROR HANDLING: I2C transaction failed */
						printf("ERR %x\n", res);
				} else {
						/* res contains the read word */
						print_tps_reg(buf, res);
				}
		}

		return 0;
}
