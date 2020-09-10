//         ATMEL Microcontroller Software Support  -  Colorado Springs, CO -
// ----------------------------------------------------------------------------
// DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
// DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

/** \file
 *  \brief  Functions for I2C Physical Hardware Independent Layer of ECCX08 Library
 *  \author Atmel Crypto Products
 *  \date   May 3, 2013
 */
//#define ECCX08_GPIO_WAKEUP
#include <string.h>
#include "hardware.h"
#include "eccX08_physical.h"				// declarations that are common to all interface implementations
#include "eccX08_lib_return_codes.h"		// declarations of function return codes
#include "i2c_async.h"


/** \brief This enumeration lists all packet types sent to a ECCX08 device.
 *
 * The following byte stream is sent to a ECCX08 I2C device:
 *    {I2C start} {I2C address} {word address} [{data}] {I2C stop}.
 * Data are only sent after a word address of value #ECCX08_I2C_PACKET_FUNCTION_NORMAL.
 */
enum i2c_word_address
{
	ECCX08_I2C_PACKET_FUNCTION_RESET,	//!< Reset device.
	ECCX08_I2C_PACKET_FUNCTION_SLEEP,	//!< Put device into Sleep mode.
	ECCX08_I2C_PACKET_FUNCTION_IDLE,	//!< Put device into Idle mode.
	ECCX08_I2C_PACKET_FUNCTION_NORMAL	//!< Write / evaluate data that follow this word address byte.
};


/** \brief This enumeration lists flags for I2C read or write addressing. */
typedef enum 
{
	I2C_WRITE_CMD = 0x00,		//!< write command flag
	I2C_READ_CMD  = 0x01		//!< read command flag
}i2c_read_write_flag;


//! I2C address is set when calling #eccX08p_init or #eccX08p_set_device_id.
static uint8_t device_address = ECCX08_I2C_DEFAULT_ADDRESS;


/** \brief This I2C function sets the I2C address.
 *         Communication functions will use this address.
 *
 *  \param[in] id I2C address
 */
void eccX08p_set_device_id(uint8_t id)
{
	device_address = id;
}


/** \brief This I2C function initializes the hardware.
 */
void eccX08p_init(void)
{
	I2C1_Initialize();
	device_address = ECCX08_I2C_DEFAULT_ADDRESS;
}


/** \brief This I2C function initializes the i2c speed.
 */
void eccX08p_i2c_set_spd(uint32_t spd_in_khz)
{
//	i2c_set_speed(spd_in_khz);
	I2C1_Initialize();
}


//#ifndef DEBUG_DIAMOND
//#   define DEBUG_DIAMOND
//#endif
/** \brief This I2C function generates a Wake-up pulse and delays.
 * \return status of the operation
 */
uint8_t eccX08p_wakeup(void)
{
    // Generate wakeup pulse by writing a 0 on the I2C bus.
	uint8_t dummy_byte = 0;
	uint8_t i2c_status = i2c_send_start();
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;

	// To send eight zero bits it takes 10E6 / I2C clock * 8 us.
	DELAY_US(60);    

	// We have to send at least one byte between an I2C Start and an I2C Stop.
	(void) i2c_send_bytes(1, &dummy_byte);
	i2c_status = i2c_send_stop();
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;

    DELAY_MS(3);

	return ECCX08_SUCCESS;
}


/** \brief This function creates a Start condition and sends the TWI address.
 * \param[in] read #I2C_READ for reading, #I2C_WRITE for writing
 * \return status of the I2C operation
 */
static uint8_t eccX08p_send_slave_address(uint8_t read)
{
    device_address = 0xC0;
	uint8_t sla = device_address | read;
	uint8_t ret_code = i2c_send_start();
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ret_code;
		
	ret_code = i2c_send_bytes(1, &sla);
	
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		(void) i2c_send_stop();
		
	return ret_code;
}


/** \brief This function sends a I2C packet enclosed by a I2C start and stop to a ECCX08 device.
 *
 *         This function combines a I2C packet send sequence that is common to all packet types.
 *         Only if word_address is #I2C_PACKET_FUNCTION_NORMAL, count and buffer parameters are
 *         expected to be non-zero.
 * @param[in] word_address packet function code listed in #i2c_word_address
 * @param[in] count number of bytes in data buffer
 * @param[in] buffer pointer to data buffer
 * @return status of the operation
 */
static uint8_t eccX08p_i2c_send(uint8_t word_address, uint8_t count, uint8_t *buffer)
{
	uint8_t ret_code;
	uint8_t cmd_byte = word_address;
	uint8_t cmd[256];

	i2c_send_start();
    i2c_send_bytes(1, &device_address);

	if ((buffer==NULL)&&(count==0))
	{	// for dummy write (send address only)
		ret_code = i2c_send_bytes(count, buffer);
	}
	else if((buffer==NULL)&&(count>0))
	{
		cmd[0] = cmd_byte;
		ret_code = i2c_send_bytes(count, cmd);
	}
	else
	{
		cmd[0] = cmd_byte;
		if (count >0)
			memcpy(&cmd[1], buffer, count );
		ret_code = i2c_send_bytes(count+1, cmd);
	}
    i2c_send_stop();
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;
	else
		return ECCX08_SUCCESS;
}


/** \brief This I2C function sends a command to the device.
 * \param[in] count number of bytes to send
 * \param[in] command pointer to command buffer
 * \return status of the operation
 */
uint8_t eccX08p_send_command(uint8_t count, uint8_t *command)
{
	return eccX08p_i2c_send(ECCX08_I2C_PACKET_FUNCTION_NORMAL, count, command);
}


/** \brief This I2C function puts the ECCX08 device into idle state.
 * \return status of the operation
 */
uint8_t eccX08p_idle(void)
{
	return eccX08p_i2c_send(ECCX08_I2C_PACKET_FUNCTION_IDLE, 1, NULL);
}


/** \brief This I2C function puts the ECCX08 device into low-power state.
 *  \return status of the operation
 */
uint8_t eccX08p_sleep(void)
{
	return eccX08p_i2c_send(ECCX08_I2C_PACKET_FUNCTION_SLEEP, 1, NULL);
}


/** \brief This I2C function resets the I/O buffer of the ECCX08 device.
 * \return status of the operation
 */
uint8_t eccX08p_reset_io(void)
{
	return eccX08p_i2c_send(ECCX08_I2C_PACKET_FUNCTION_RESET, 1, NULL);
}


/** \brief This I2C function receives a response from the ECCX08 device.
 *
 * @param[in] size size of rx buffer
 * @param[out] response pointer to rx buffer
 * @return status of the operation
 */
uint8_t eccX08p_receive_response(uint8_t size, uint8_t *response)
{
    
    uint8_t count;

	// Address the device and indicate that bytes are to be read.
	uint8_t i2c_status = eccX08p_send_slave_address(I2C_READ_CMD);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS) {
		// Translate error so that the Communication layer
		// can distinguish between a real error or the
		// device being busy executing a command.
		if (i2c_status == I2C_FUNCTION_RETCODE_NACK)
			i2c_status = ECCX08_RX_NO_RESPONSE;

		return i2c_status;
	}

	// Receive count byte.
	i2c_status = i2c_receive_byte(response);
	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;

	count = response[ECCX08_BUFFER_POS_COUNT];
	if ((count < ECCX08_RSP_SIZE_MIN) || (count > size)) {
		(void) i2c_send_stop();
		return ECCX08_INVALID_SIZE;
	}

	i2c_status = i2c_receive_bytes(count - 1, &response[ECCX08_BUFFER_POS_DATA]);

	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;
	else
		return ECCX08_SUCCESS;
//	uint8_t count;
//	
//	// Address the device and indicate that bytes are to be read.
//    i2c_send_start();
//    i2c_send_bytes(1, device_address);
//	uint8_t i2c_status = i2c_receive_bytes(size, response);
//	
//	count = response[ECCX08_BUFFER_POS_COUNT];
////	printf("count: %d\r\n", count);
////	printf("size: %d\r\n", size);
//	if ((count < ECCX08_RSP_SIZE_MIN) || (count > size))
//	{
//		(void) i2c_send_stop();
//		return ECCX08_INVALID_SIZE;
//	}
//	if (i2c_status != I2C_FUNCTION_RETCODE_SUCCESS)
//		return ECCX08_COMM_FAIL;
//	else
//		return ECCX08_SUCCESS;
}


/** \brief This I2C function resynchronizes communication.
 *
 * Parameters are not used for I2C.\n
 * Re-synchronizing communication is done in a maximum of three steps
 * listed below. This function implements the first step. Since
 * steps 2 and 3 (sending a Wake-up token and reading the response)
 * are the same for I2C and SWI, they are
 * implemented in the communication layer (#eccX08c_resync).
  <ol>
     <li>
       To ensure an IO channel reset, the system should send
       the standard I2C software reset sequence, as follows:
       <ul>
         <li>a Start condition</li>
         <li>nine cycles of SCL, with SDA held high</li>
         <li>another Start condition</li>
         <li>a Stop condition</li>
       </ul>
       It should then be possible to send a read sequence and
       if synchronization has completed properly the ATECCX08 will
       acknowledge the device address. The chip may return data or
       may leave the bus floating (which the system will interpret
       as a data value of 0xFF) during the data periods.\n
       If the chip does acknowledge the device address, the system
       should reset the internal address counter to force the
       ATECCX08 to ignore any partial input command that may have
       been sent. This can be accomplished by sending a write
       sequence to word address 0x00 (Reset), followed by a
       Stop condition.
     </li>
     <li>
       If the chip does NOT respond to the device address with an ACK,
       then it may be asleep. In this case, the system should send a
       complete Wake token and wait t_whi after the rising edge. The
       system may then send another read sequence and if synchronization
       has completed the chip will acknowledge the device address.
     </li>
     <li>
       If the chip still does not respond to the device address with
       an acknowledge, then it may be busy executing a command. The
       system should wait the longest TEXEC and then send the
       read sequence, which will be acknowledged by the chip.
     </li>
  </ol>
 * \param[in] size size of rx buffer
 * \param[out] response pointer to response buffer
 * \return status of the operation
 * \todo Run MAC test in a loop until a communication error occurs and this routine is executed.
 */
uint8_t eccX08p_resync(uint8_t size, uint8_t *response)
{
	uint8_t nine_clocks = 0xFF;
	uint8_t ret_code = i2c_send_start();
	
	// Do not evaluate the return code that most likely indicates error,
	// since nine_clocks is unlikely to be acknowledged.
	(void) i2c_send_bytes(1, &nine_clocks);
	
	// Send another Start. The function sends also one byte,
	// the I2C address of the device, because I2C specification
	// does not allow sending a Stop right after a Start condition.
	ret_code = eccX08p_send_slave_address(I2C_READ_CMD);
	
	// Send only a Stop if the above call succeeded.
	// Otherwise the above function has sent it already.
	if (ret_code == I2C_FUNCTION_RETCODE_SUCCESS)
		ret_code = i2c_send_stop();
		
	// Return error status if we failed to re-sync.
	if (ret_code != I2C_FUNCTION_RETCODE_SUCCESS)
		return ECCX08_COMM_FAIL;
		
	// Try to send a Reset IO command if re-sync succeeded.
	return eccX08p_reset_io();
}
