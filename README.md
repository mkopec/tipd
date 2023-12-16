# tipd

(This will be) A tool to interact with Texas Instruments USB-PD controllers over i2c.

On machines where the PD controller is wired directly to the SoC's I2C bus, this tool can be used internally - but make sure you unload the driver, if present.

If the controller's I2C is not internally accessible (for example wired to EC), you may be able to use this with an external I2C adapter wired to the laptop's battery connector.
