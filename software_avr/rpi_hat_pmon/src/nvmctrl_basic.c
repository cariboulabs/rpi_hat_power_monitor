/**
 * \file
 *
 * \brief NVMCTRL Basic driver implementation.
 *
 (c) 2020 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

/**
 * \defgroup doc_driver_nvmctrl_basic NVMCTRL Basic Driver
 * \ingroup doc_driver_nvmctrl
 *
 * \section doc_driver_nvmctrl_rev Revision History
 * - v0.0.0.1 Initial Commit
 *
 *@{
 */

#include <avr/pgmspace.h>
#include <ccp.h>
#include <string.h>
#include "nvmctrl_basic.h"

/**
 * \brief Initialize nvmctrl interface
 * \return Return value 0 if success
 */
int8_t FLASH_0_init()
{

	NVMCTRL.CTRLB = 1 << NVMCTRL_APPCODEWP_bp    /* Application Code Section write protect: enabled */
	                | 0 << NVMCTRL_BOOTRP_bp     /* Boot Section Read protected: disabled */
	                | 0 << NVMCTRL_APPDATAWP_bp  /* Application Data Section write protect: disabled */
	                | NVMCTRL_FLMAP_SECTION0_gc  /* Flash section 0 */
	                | 0 << NVMCTRL_FLMAPLOCK_bp; /* Flash Mapping Lock: disabled */

	// NVMCTRL.INTCTRL = 0 << NVMCTRL_EEREADY_bp; /* EEPROM Ready: disabled */

	return 0;
}

/**
 * \brief Read a byte from eeprom
 *
 * \param[in] eeprom_adr The byte-address in eeprom to read from
 *
 * \return The read byte
 */
uint8_t FLASH_0_read_eeprom_byte(eeprom_adr_t eeprom_adr)
{
	// Read operation will be stalled by hardware if any write is in progress
	return *(uint8_t *)(EEPROM_START + eeprom_adr);
}

/**
 * \brief Write a byte to eeprom
 *
 * \param[in] eeprom_adr The byte-address in eeprom to write to
 * \param[in] data The byte to write
 *
 * \return Status of write operation
 */
nvmctrl_status_t FLASH_0_write_eeprom_byte(eeprom_adr_t eeprom_adr, uint8_t data)
{
	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/* Program the EEPROM with desired value(s) */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_EEERWR_gc);

	/* Write byte to EEPROM */
	*(uint8_t *)(EEPROM_START + eeprom_adr) = data;

	/* Clear the current command */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	return NVM_OK;
}

/**
 * \brief Read a block from eeprom
 *
 * \param[in] eeprom_adr The byte-address in eeprom to read from
 * \param[in] data Buffer to place read data into
 *
 * \return Nothing
 */
void FLASH_0_read_eeprom_block(eeprom_adr_t eeprom_adr, uint8_t *data, size_t size)
{
	// Read operation will be stalled by hardware if any write is in progress
	memcpy(data, (uint8_t *)(EEPROM_START + eeprom_adr), size);
}

/**
 * \brief Write a block to eeprom
 *
 * \param[in] eeprom_adr The byte-address in eeprom to write to
 * \param[in] data The buffer to write
 *
 * \return Status of write operation
 */
nvmctrl_status_t FLASH_0_write_eeprom_block(eeprom_adr_t eeprom_adr, uint8_t *data, size_t size)
{
	uint8_t *write = (uint8_t *)(EEPROM_START + eeprom_adr);

	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/* Program the EEPROM with desired value(s) */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_EEERWR_gc);

	do {
		/* Write byte to EEPROM */
		*write++ = *data++;
		size--;
	} while (size != 0);

	/* Clear the current command */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	return NVM_OK;
}

/**
 * \brief Check if the EEPROM can accept data to be read or written
 *
 * \return The status of EEPROM busy check
 * \retval false The EEPROM can not receive data to be read or written
 * \retval true The EEPROM can receive data to be read or written
 */
bool FLASH_0_is_eeprom_ready()
{
	return (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm));
}

/**
 * \brief SPM based word write to flash
 *
 * \param[in] address   The word-address in flash to write
 * \param[in] word      The word data to write to the flash
 *
 * \return The read byte
 */
void FLASH_0_spm_write_word(uint32_t address, uint16_t word)
{
	__asm__ __volatile__("push r0\n\t"       /* back up R0*/
	                     "movw r0, %A1\n\t"  /* update R0,R1 pair with word*/
	                     "movw r30, %A0\n\t" /* update R30,R31 pair with address[Byte1,Byte0]*/
	                     "spm \n\t"          /* spm */
	                     "clr r1\n\t"        /* R1 is always assumed to be zero by the compiler. Resetting R1 to zero*/
	                     "pop r0 \n\t"       /* restore R0*/
	                     :
	                     : "r"((uint32_t)(address)), "r"((uint16_t)(word))
	                     : "r30", "r31" /* Clobber R30, R31 to indicate they are used here*/
	);
}

/**
 * \brief Read a byte from flash
 *
 * \param[in] flash_adr The byte-address in flash to read from
 *
 * \return The read byte
 */
uint8_t FLASH_0_read_flash_byte(flash_adr_t flash_adr)
{
	return pgm_read_byte_near(flash_adr);
}

/**
 * \brief Write a byte to flash
 *
 * \param[in] flash_adr The byte-address in flash to write to
 * \param[in] page_buffer A buffer in memory the size of a flash page, used as a scratchpad
 * \param[in] data The byte to write
 *
 * \return Status of the operation
 */
nvmctrl_status_t FLASH_0_write_flash_byte(flash_adr_t flash_adr, uint8_t *ram_buffer, uint8_t data)
{
	flash_adr_t start_of_page = (flash_adr_t)(flash_adr & ~((flash_adr_t)PROGMEM_PAGE_SIZE - 1));
	uint16_t *  word_buffer   = (uint16_t *)ram_buffer;
	uint16_t    i;
	/* Backup all the FLASH page data to ram_buffer and update the new data*/
	for (i = 0; i < flash_adr % PROGMEM_PAGE_SIZE; i++) {
		ram_buffer[i] = pgm_read_byte_near(start_of_page + i);
	}
	ram_buffer[i++] = data; // updating the new data
	for (; i < PROGMEM_PAGE_SIZE; i++) {
		ram_buffer[i] = pgm_read_byte_near(start_of_page + i);
	}

	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/* Erase the flash page */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc);
	/* dummy write to start erase operation */
	FLASH_0_spm_write_word(start_of_page, 0);

	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/*A change from one command to another must always go through NOCMD or NOOP*/
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	/* Program the page with desired value(s) */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLWR_gc);

	/* write the modified page data to FLASH*/
	for (i = 0; i < PROGMEM_PAGE_SIZE / 2; i++) {
		FLASH_0_spm_write_word(start_of_page + (i * 2), word_buffer[i]);
	}
	/* Clear the current command */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	if (NVMCTRL.STATUS & NVMCTRL_ERROR_gm)
		return NVM_ERROR;
	else
		return NVM_OK;
}

/**
 * \brief Erase a page in flash
 *
 * \param[in] flash_adr The byte-address in flash to erase. Must point to start-of-page.
 *
 * \return Status of the operation
 */
nvmctrl_status_t FLASH_0_erase_flash_page(flash_adr_t flash_adr)
{
	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/* Erase the flash page */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc);

	/* dummy write to start erase operation */
	FLASH_0_spm_write_word(flash_adr, 0);

	/* Clear the current command */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	if (NVMCTRL.STATUS & NVMCTRL_ERROR_gm)
		return NVM_ERROR;
	else
		return NVM_OK;
}

/**
 * \brief Write a page in flash.
 *
 * \param[in] flash_adr The byte-address of the flash page to write to. Must point to start-of-page.
 * \param[in] data The data to write to the flash page
 *
 * \return Status of the operation
 */
nvmctrl_status_t FLASH_0_write_flash_page(flash_adr_t flash_adr, uint8_t *data)
{
	uint16_t *word_buffer = (uint16_t *)data;

	/* check for the starting address of page*/
	if (flash_adr % PROGMEM_PAGE_SIZE != 0) {
		return NVM_ERROR;
	}

	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
		;

	/* Erase the flash page */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc);

	/* Perform a dummy write to this address to update the address register in NVMCTL */
	FLASH_0_spm_write_word(flash_adr, 0);

	/* Wait for completion of previous operation */
	while (NVMCTRL.STATUS & NVMCTRL_FBUSY_bm)
		;

	/*A change from one command to another must always go through NOCMD or NOOP*/
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	/* Write the flash page */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLWR_gc);

	/* Write data to the page buffer */
	for (uint16_t i = 0; i < PROGMEM_PAGE_SIZE / 2; i++) {
		FLASH_0_spm_write_word(flash_adr + (i * 2), word_buffer[i]);
	}

	/* Clear the current command */
	ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);

	if (NVMCTRL.STATUS & NVMCTRL_ERROR_gm)
		return NVM_ERROR;
	else
		return NVM_OK;
}

/**
 * \brief Writes a buffer to flash.
 * The flash does not need to be erased beforehand.
 * The flash address to write to does not need to be aligned to any specific boundary.
 *
 * \param[in] flash_adr The byte-address of the flash to write to
 * \param[in] data The data to write to the flash
 * \param[in] size The size of the data (in bytes) to write to the flash
 * \param[in] page_buffer A buffer in memory the size of a flash page, used as a scratch pad
 *
 * \return Status of the operation
 */
nvmctrl_status_t FLASH_0_write_flash_block(flash_adr_t flash_adr, uint8_t *data, size_t size, uint8_t *ram_buffer)
{
	flash_adr_t      data_space   = ((flash_adr) & ~((flash_adr_t)PROGMEM_PAGE_SIZE - 1));
	uint32_t         start_offset = flash_adr % PROGMEM_PAGE_SIZE;
	uint16_t         i;
	nvmctrl_status_t status = NVM_ERROR;

	// Step 1:
	// Fill page buffer with contents of first flash page to be written up
	// to the first flash address to be replaced by the new contents
	for (i = 0; i < start_offset; i++) {
		ram_buffer[i] = FLASH_0_read_flash_byte(data_space + i);
	}

	// Step 2:
	// Write all of the new flash contents to the page buffer, writing the
	// page buffer to flash every time the buffer contains a complete flash
	// page.
	while (size > 0) {
		ram_buffer[i++] = *data++;
		size--;
		if ((i % PROGMEM_PAGE_SIZE) == 0) {
			status = FLASH_0_write_flash_page(data_space, ram_buffer);
			i      = 0;
			data_space += PROGMEM_PAGE_SIZE;
			if (status == NVM_ERROR) {
				return NVM_ERROR;
			}
		}
	}

	// Step 3:
	// After step 2, the page buffer may be partially full with the last
	// part of the new data to write to flash. The remainder of the flash page
	// shall be unaltered. Fill up the remainder
	// of the page buffer with the original contents of the flash page, and do a
	// final flash page write.
	while (i) {
		ram_buffer[i] = FLASH_0_read_flash_byte(data_space + i);
		i++;
		if ((i % PROGMEM_PAGE_SIZE) == 0) {
			status = FLASH_0_write_flash_page(data_space, ram_buffer);
			i      = 0;
			data_space += PROGMEM_PAGE_SIZE;
		}
	}

	return status;
}

/**
 * \brief Writes a byte stream to flash.
 * The erase granularity of the flash (i.e. one page) will cause this operation
 * to erase an entire page at a time. To avoid corrupting other flash contents,
 * make sure that the memory range in flash being streamed to is starting on a page
 * boundary, and that enough flash pages are available to hold all data being written.
 *
 * The function will perform flash page operations such as erase and write
 * as appropriate, typically when the last byte in a page is written. If
 * the last byte written is not at the last address of a page, the "finalize"
 * parameter can be set to force a page write after this byte.
 *
 * This function is intended used in devices where RAM resources are too limited
 * to afford a buffer needed by the write and erase page functions, and where
 * performance needs and code size concerns leaves the byte write and block
 * write functions too expensive.
 *
 * \param[in] flash_adr The byte-address of the flash to write to
 * \param[in] data The data byte to write to the flash
 * \param[in] finalize Set to true for the final write to the buffer
 *
 * \return Status of the operation
 */
nvmctrl_status_t FLASH_0_write_flash_stream(flash_adr_t flash_adr, uint8_t data, bool finalize)
{
	static bool first_byte = true;
	uint16_t    word_data;

	if (first_byte) {
		/* check for the starting of page */
		if (flash_adr % PROGMEM_PAGE_SIZE != 0) {
			return NVM_ERROR;
		}
		first_byte = false;
	}
	/* check for a new page */
	if (flash_adr % PROGMEM_PAGE_SIZE == 0) {
		/* Erase the flash page and program with desired value(s) */
		/* Wait for completion of previous operation */
		while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
			;

		/* Erase the flash page */
		ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLPER_gc);
		/* dummy write to start erase operation */
		FLASH_0_spm_write_word(flash_adr, 0);

		/* Wait for completion of previous operation */
		while (NVMCTRL.STATUS & (NVMCTRL_EEBUSY_bm | NVMCTRL_FBUSY_bm))
			;

		/* Program the page with desired value(s) */
		ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_FLWR_gc);
	}

	if (flash_adr % 2)
		word_data = data << 8 | 0xFF;
	else
		word_data = 0xFF << 8 | data;

	// Write the new word value to the correct address. Making the flash_adr word aligned
	FLASH_0_spm_write_word(flash_adr & ~(flash_adr_t)1, word_data);

	if (finalize) {
		/* Clear the current command */
		ccp_write_spm((void *)&NVMCTRL.CTRLA, NVMCTRL_CMD_NONE_gc);
		first_byte = true;
	}

	if (NVMCTRL.STATUS & NVMCTRL_ERROR_gm)
		return NVM_ERROR;
	else
		return NVM_OK;
}
