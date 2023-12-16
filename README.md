# tipd

A tool to interact with Texas Instruments USB-PD controllers over i2c.

On machines where the PD controller is wired directly to the SoC's I2C bus, this tool can be used internally - but make sure you unload the driver, if present.

If the controller's I2C is not internally accessible (for example wired to EC), you may be able to use this with an external I2C adapter wired to the laptop's battery connector.

## Building

```bash
make
```

## Examples

Display helptext:

```bash
$ ./tipd -h                                                                                                                                                                                                         main!?
usage: tipd -b BUS -a ADDR
	BUS: i2c bus index to use
	ADDR: address of the TPS slave interface
```

Dumping registers:

```bash
$ ./tipd -b 0 -a 0x27 -d
Successfully opened /dev/i2c-0
Acquired I2C slave
Current mode: APP
REG 0x01    LEN  4    VAL 41 43 45 4c
REG 0x02    LEN  4    VAL 01 00 00 00
REG 0x03    LEN  4    VAL 41 50 50 20
REG 0x04    LEN  4    VAL 00 00 00 00
[...]
```

Writing registers:

```bash
$ ./tipd -b 0 -a 0x27 -w -r 0x16 -v 3a7802cf0000004f000100 -l 11
Successfully opened /dev/i2c-0
Acquired I2C slave
Current mode: APP
WRITE REG 0x16 VAL 3a 78 02 cf 00 00 00 4f 00 01 00 OK
```

Monitoring events:

```bash
$ ./tipd -b 0 -a 0x27 -m
Successfully opened /dev/i2c-0
Acquired I2C slave
Current mode: APP
plug_insert_or_removal
plug_insert_or_removal
srccap_msg_rdy
new_contract_as_cons
```

