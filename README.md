# tipd

## Disclaimer

Only use this tool if you really know what you're doing. Improper configuration
of the USB-PD controller can result in serious hardware damage.

## What is this

A tool to interact with Texas Instruments USB-PD controllers over i2c.

On machines where the PD controller is wired directly to the SoC's I2C bus, this
tool can be used internally - but make sure you unload the driver, if present.

If the controller's I2C is not internally accessible (for example wired to EC),
you may be able to use this with an external I2C adapter wired to the laptop's
battery connector.

Keep in mind that using monitor mode can prevent the EC from handling PD
interrupts properly, so **make sure** nobody else is listening on IRQ1.

For example, ThinkPad T14 Gen1 AMD has I2C1 wired to SoC and I2C2 wired to EC.
In this case you can clear the IntEvent1 interrupts without causing problems,
but clearing IntEvent2 breaks a lot of stuff.

TODO: Implement listening on either IntEvent1 or IntEvent2

## Building

```bash
make
```

## Examples

Display helptext:

```bash
$ ./tipd -h                                                                                                                                                                                                         main!?
Usage: tipd -b BUS -a ADDR -d -m -w [-r, -v, -l]
	BUS: I2C bus index to use
	ADDR: Address of the TPS slave interface
	  -d: Dump all registers
	  -m: Monitor events
	  -w: Write register
	    -r: Register to write
	    -v: Hexadecimal hexstring of value to write (little endian)
	    -l: How many bytes to write
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

