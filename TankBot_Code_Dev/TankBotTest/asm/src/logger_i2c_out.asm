/*
 * Logger I2C Output Service
 *
 * org: 11/17/2014
 * auth: Nels "Chip" Pearson
 *
 * Target: Tank Bot Demo Board, 20MHz, ATmega164P
 *
 * This service is used to output debugging text to an I2C Logger device.
 *
 * A table of predefined labels are provided.
 *
 * Dependentcies
 *   i2c_master.asm
 *
 */

.DSEG
log_buffer:		.BYTE	18		; 16 char buffer.

.CSEG
