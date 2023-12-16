#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_REG_LEN 64

#define TPS_REG_MODE 0x03
#define TPS_REG_INT_EVENT1 0x14
#define TPS_REG_INT_EVENT2 0x15
#define TPS_REG_INT_MASK1 0x16
#define TPS_REG_INT_MASK2 0x17
#define TPS_REG_INT_CLEAR1 0x18
#define TPS_REG_INT_CLEAR2 0x19

#define BITFIELD(field) uint8_t field : 1;

struct int_event {
		BITFIELD(pd_soft_reset)
		BITFIELD(pd_hard_reset)
		BITFIELD(cable_reset)
		BITFIELD(plug_insert_or_removal)
		BITFIELD(pr_swap_complete)
		BITFIELD(dr_swap_complete)
		BITFIELD(fr_swap_complete)
		BITFIELD(rdo_recv_from_sink)

		BITFIELD(bist)
		BITFIELD(overcurrent)
		BITFIELD(attn_recv)
		BITFIELD(vdm_recv)
		BITFIELD(new_contract_as_cons)
		BITFIELD(new_contract_as_prov)
		BITFIELD(srccap_msg_rdy)
		BITFIELD(snkcap_msg_rdy)

		uint8_t unused2;
		uint8_t unused3;
		uint8_t unused4;
		uint8_t unused5;
		uint8_t unused6;
		uint8_t unused7;
		uint8_t unused8;
		uint8_t unused9;
		uint8_t unused10;
} __packed;

static void print_tps_reg(uint8_t *data, uint8_t len) {
		if (data == NULL)
				return;

		printf("LEN %2d    VAL ", len);

		for (int i = 0; i < len; ++i)
				printf("%02x ", data[i]);

		printf("\n");
}

/* Dump all registers */
static void regdump(int file) {
		uint8_t reg;
		int32_t res;
		uint8_t buf[MAX_REG_LEN];

		for (reg = 1; reg <= 0x7f; ++reg) {
				res = i2c_smbus_read_block_data(file, reg, buf);

				printf("REG 0x%02x    ", reg);
				if (res < 0)
						printf("ERR %d\n", res);
				else
						print_tps_reg(buf, res);
		}
}

#define STR(x) #x
#define PRINT_EVENT(evt, name) {\
		if (evt->name) { \
				printf(STR(name));\
				printf("\n");\
		}\
}

/* Print events that occured */
static void print_events(struct int_event *event) {
		PRINT_EVENT(event, pd_soft_reset)
		PRINT_EVENT(event, pd_hard_reset)
		PRINT_EVENT(event, cable_reset)
		PRINT_EVENT(event, plug_insert_or_removal)
		PRINT_EVENT(event, pr_swap_complete)
		PRINT_EVENT(event, dr_swap_complete)
		PRINT_EVENT(event, fr_swap_complete)
		PRINT_EVENT(event, rdo_recv_from_sink)

		PRINT_EVENT(event, bist)
		PRINT_EVENT(event, overcurrent)
		PRINT_EVENT(event, attn_recv)
		PRINT_EVENT(event, vdm_recv)
		PRINT_EVENT(event, new_contract_as_cons)
		PRINT_EVENT(event, new_contract_as_prov)
		PRINT_EVENT(event, srccap_msg_rdy)
		PRINT_EVENT(event, snkcap_msg_rdy)
}

/* Monitor events */
static void monitor(int file) {
		int32_t res;
		uint8_t event1[MAX_REG_LEN];
		uint8_t event2[MAX_REG_LEN];
		struct int_event event;
		bool stop = false;

		while (!stop) {
				memset(event1, 0, 11);
				memset(event2, 0, 11);

				if ((res = i2c_smbus_read_block_data(file, TPS_REG_INT_EVENT1, event1)) < 0)
						exit(res);
				if ((res = i2c_smbus_write_block_data(file, TPS_REG_INT_CLEAR1, 11, event1)) < 0)
						exit(res);

				memcpy (&event, event1, 11);
				print_events(&event);

				//stop = true;
		}

}

/* Print current operational mode */
static void print_mode(int file) {
		int32_t res;
		uint8_t buf[MAX_REG_LEN];

		res = i2c_smbus_read_block_data(file, TPS_REG_MODE, buf);

		if (res < 4) {
				printf("Could not retrieve current working mode!\n");
				printf("Exiting for safety.\n");
				exit(1);
		}

		buf[4] = '\0';

		printf("Current mode: %s\n", buf);
}

static void write_reg(int file, uint8_t reg, uint8_t *data, uint8_t len) {
		int32_t res;
		if ((res = i2c_smbus_write_block_data(file, reg, len, data)) < 0)
				printf("ERR %d\n", res);
		else
				printf("OK\n");
}

static void usage(void) {
		printf("Usage: tipd -b BUS -a ADDR -d -m -w [-r, -v, -l]\n");
		printf("\tBUS: I2C bus index to use\n");
		printf("\tADDR: Address of the TPS slave interface\n");
		printf("\t  -d: Dump all registers\n");
		printf("\t  -m: Monitor events\n");
		printf("\t  -w: Write register\n");
		printf("\t    -r: Register to write\n");
		printf("\t    -v: Hexadecimal hexstring of value to write (little endian)\n");
		printf("\t    -l: How many bytes to write\n");


}

int main(int argc, char **argv) {
		int file, opt;
		char filename[20];

		/* Device parameters */
		unsigned int bus, addr, reg, len;
		char value_str[MAX_REG_LEN*2];
		uint8_t value[MAX_REG_LEN];
		/* Requirements for commands */
		bool have_bus, have_addr, have_reg, have_len, have_value;
		/* Tasks to execute */
		bool do_regdump, do_monitor, do_write;

		do_regdump = do_monitor = do_write = false;

		while((opt = getopt(argc, argv, "hb:a:dmwr:l:v:")) != -1) {
				switch(opt) {
				case 'h':
						usage();
						exit(0);
				case 'b':
						if (!(sscanf(optarg, "%x", &bus) == 1)) {
								usage();
								exit(1);
						}

						have_bus = true;
						break;
				case 'a':
						if (!(sscanf(optarg, "0x%x", &addr) == 1) &&
						    !(sscanf(optarg, "%x", &addr) == 1)) {
								usage();
								exit(1);
						}
						have_addr = true;
						break;
				case 'd':
						do_regdump = true;
						break;
				case 'm':
						do_monitor = true;
						break;
				case 'w':
						do_write = true;
						break;
				case 'r':
						if (!(sscanf(optarg, "0x%x", &reg) == 1) &&
						    !(sscanf(optarg, "%x", &reg) == 1)) {
								usage();
								exit(1);
						}
						have_reg = true;
						break;
				case 'l':
						if (!(sscanf(optarg, "%d", &len) == 1)) {
								usage();
								exit(1);
						}
						have_len = true;
						break;
				case 'v':
						if (!(sscanf(optarg, "%s", value_str) == 1)) {
								usage();
								exit(1);
						}
						have_value = true;
						break;
				case '?':
						usage();
						exit(1);
						break;
				default:
						break;
				}
		}

		if (!have_bus || !have_addr) {
				printf("missing required arguments!\n");
				usage();
				exit(1);
		}

		snprintf(filename, 19, "/dev/i2c-%d", bus);
		file = open(filename, O_RDWR);
		if (file < 0) {
				printf("Could not open i2c bus\n");
				printf("Add yourself to i2c group or run this program as root\n");
				exit(1);
		}
		fprintf(stderr, "Successfully opened %s\n", filename);

		if (ioctl(file, I2C_SLAVE, addr) < 0) {
				printf("Could not acquire I2C slave\n");
				printf("Add yourself to i2c group or run this program as root\n");
				exit(1);
		}
		fprintf(stderr, "Acquired I2C slave\n");

		print_mode(file);

		if (!do_regdump && !do_monitor && !do_write)
				printf("No command specified\n");

		if (do_write) {
				if (!have_reg || !have_len || !have_value) {
						usage();
						exit(1);
				}
				printf("WRITE REG 0x%x VAL ", reg);
				for (int i = 0; i < len; ++i) {
						if (!(sscanf(value_str+(i*2), "%2hhx", &value[i]) == 1)) {
								usage();
								exit(1);
						}
				}
				for (int i = 0; i < len; ++i) {
						printf("%02x ", value[i]);
				}
				write_reg(file, reg, value, len);
		}

		if (do_regdump)
				regdump(file);

		if (do_monitor)
				monitor(file);

		return 0;
}
