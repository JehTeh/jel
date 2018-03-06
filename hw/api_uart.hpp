/** @file hw/api_uart.hpp
 *  @brief Abstracted definitions for serial communication via UART.
 *
 *  @detail
 *    The BasicUart driver class is provided by jel to allow for rapidly implementing a simple
 *    transmit/receive UART driver. It is a virtual class that wraps typical manufacturer driver/HAL
 *    calls such as loadTxChar()/enableTxIsr() in purely virtual functions then builds ontop of them
 *    with some basic logic. This often simplifies porting a basic UART driver to another MCU.
 *
 *    Furthermore, the Uart driver class implements both a SerialReaderInterface and a
 *    SerialWriterInterface from the jel os layer. This allows any hardware platform implementing
 *    this basic Uart class to rely on the uart for all jel I/O.
 *
 *  @author Jonathan Thomson 
 */
/**
 * MIT License
 * 
 * Copyright 2018, Jonathan Thomson 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

/** C/C++ Standard Library Headers */

/** jel Library Headers */
#include "os/api_io.hpp"
#include "os/api_locks.hpp"

namespace jel
{
namespace hw
{
namespace uart
{

/** @enum Baudrate
 *  @brief Sampling of common baudrate settings.
 *
 *  Baudrates are passed to the UART API functions wrapped in this enum for readability and reduced
 *  chance of errors when using standard baudrates. Note that it is perfecly legal to cast any
 *  unsigned 32b integer value into a Baudrate and pass that to the UART API instead; this enum type
 *  is strictly to prevent errors like a missing zero.
 *   */
enum class Baudrate : uint32_t
{
  bps9600 = 9'600,
  bps19200 = 19'200,
  bps38400 = 38'400,
  bps57600 = 57'600,
  bps115200 = 115'200,
  bps128kBit = 128'000,
  bps256kBit = 256'000,
  bps512kBit = 512'000,
  bps1Mbit = 1'000'000,
  bps2Mbit = 2'000'000,
  bps3Mbit = 3'000'000,
  bps4Mbit = 4'000'000,
  bps5Mbit = 5'000'000,
};

/** @enum Parity
 *  @brief Parity options for the UART API.
 * */
enum class Parity
{
  none,
  even,
  odd
};

/** @enum StopBits
 *  @brief Stop bit settings for the UART API.
 *  */
enum class StopBits
{
  one,
  two
};

/** enum WordLength
 *  @brief The length of the data packet portion of each word (in bits).
 * */
enum class WordLength : uint8_t
{
  eight = 8,
  seven = 7,
  six = 6,
  five = 5,
};

/** @enum BlockingMode
 *  @brief The type of blocking to use for send and receive transfers.
 *
 *  Generally, it is recommended polling mode be avoided, as it prevents other threads from
 *  executing while the calling thread is waiting on the hardware.
 * */
enum class BlockingMode
{
  polling,
  isr,
};

/** @enum UartInstance
 *  @brief An instance tag used to distinguish between multiple UART peripherals on the same target.
 *
 *  @note
 *    Not all peripherals will be implemented on all targets.
 *  */
enum class UartInstance : uint8_t
{
  uart0 = 0, uart1, uart2, uart3,
  uart4, uart5, uart6, uart7,
};

/** @class BasicUart_Base
 *  @brief A virtual base class that implements common UART functionality.
 *  
 *  The BasicUart_Base class provides generic send and receive buffer management functionality
 *  through numerous virtual functions that would be used to directly access the hardware on most
 *  platforms. 
 * */
class BasicUart_Base : public os::SerialReaderInterface, public os::SerialWriterInterface
{
public:
  /** @struct Config
   *  @brief Configuration parameters used to set the line properties the UART is communicating on.
   *  */
  struct Config
  {
    UartInstance instance = UartInstance::uart0;
    Baudrate baud = Baudrate::bps115200;
    Parity parity = Parity::none;
    StopBits stop = StopBits::one;
    WordLength wordlen = WordLength::eight;
    BlockingMode rxBlockingMode = BlockingMode::isr;
    BlockingMode txBlockingMode = BlockingMode::isr;
  };
  BasicUart_Base(const Config& config);
  virtual ~BasicUart_Base() noexcept {}
  virtual size_t read(char* buffer, const size_t bufferLen) override;
  virtual void write(const char* cStr, const size_t length_chars) override;
  virtual void write(const char c) override;
  virtual bool isBusy(const Duration& timeout) override;
  virtual size_t waitForChars(const Duration& timeout) override;
  virtual void reconfigure(const Config& newConfig) = 0;
protected:
  /**   */
  template<typename BufferType>
  struct OpState
  {
    volatile size_t pos;
    volatile size_t totalLen;
    volatile BufferType* buffer;
    os::Semaphore flag;
  };
  Config cfg_;
  OpState<char> rx_;
  OpState<const char> tx_;
  /** Reads a character from the hardware receive buffer. The character should be removed from the
   * buffer after this operation. */
  virtual char readRxBuffer() = 0;
  /** Writes a character to the hardware transmit buffer. The transmit buffer should be sending the
   * character/queing the character for send once the write is completed. */
  virtual void loadTxBuffer(const char c) = 0;
  /** Returns true if there is currently a character waiting to be read from the hardware receive
   * buffer. On systems where more than one character can be buffered, true should be returned even
   * if only one character is in the buffer. */
  virtual bool isRxBufferReady() = 0;
  /** Returns true if the hardware transmit buffer has room to load another character. This includes
   * returning true on a system where multiple characters can be buffered in hardware for
   * transmission even if only 1 space is available in the transmit fifo. */
  virtual bool isTxBufferReady() = 0;
  /** Either enables or disables the RX character ready interrupt. */
  virtual void setRxIsrEnable(const bool enableIsr) = 0;
  /** Either enables or disables the TX character complete interrupt. */
  virtual void setTxIsrEnable(const bool enableIsr) = 0;
  /** Clears the RxBufferFull ISR flag. */
  virtual void clearRxIsrFlags() = 0;
  /** Clears the TxBufferEmtpy ISR flag. */
  virtual void clearTxIsrFlags() = 0;
  /** Interrupt routine called when characters are ready to be read from the hardware receive
   * buffer. */
  void isr_RxBufferFull() noexcept;
  /** Interrupt routine called when the hardware transmit buffer is ready to accept new characters.
   * */
  void isr_TxBufferEmpty() noexcept;
};

/** @struct BasicUartHardwareProperties
 *  @brief Forward declaration of the hardware properties for a UART perpheral.
 *
 *  @note
 *    This forward declaration is defined on a per-target basis when implementing a UART. The
 *    application should not make use of it at any point.
 * */
struct BasicUartHardwareProperties;
class InterruptDispatcher;

/** @class BasicUart
 *  @brief Instantation of the BasicUart_Base class.
 *  
 *  The BasicUart class provides a standard UART serial port instance, which is defined by each
 *  target as appropriate for that platform. The default weakly linked generic example does not
 *  offer any functionality.
 *  */
class BasicUart : public BasicUart_Base
{
public:
  BasicUart(const BasicUart_Base::Config& config);
  ~BasicUart() noexcept;
  BasicUart(const BasicUart&) = delete;
  BasicUart(BasicUart&&) = delete;
  BasicUart& operator=(const BasicUart&) = delete;
  BasicUart& operator=(BasicUart&&) = delete;
  void reconfigure(const Config& config) final override;
#ifdef HW_TARGET_STM32F302RCT6
  virtual size_t read(char* buffer, const size_t bufferLen) final override;
  virtual void write(const char* cStr, const size_t length_chars) final override;
  virtual void write(const char c) final override;
  virtual bool isBusy(const Duration& timeout) final override;
  virtual size_t waitForChars(const Duration& timeout) final override;
#endif
private:
  friend InterruptDispatcher;
  const BasicUartHardwareProperties* hw_;
  char readRxBuffer() final override;
  void loadTxBuffer(const char c) final override;
  bool isRxBufferReady() final override;
  bool isTxBufferReady() final override;
  void setRxIsrEnable(const bool enableIsr) final override;
  void setTxIsrEnable(const bool enableIsr) final override;
  void clearRxIsrFlags() final override;
  void clearTxIsrFlags() final override;
  void initializeHardware();
};

} /** namespace uart */
} /** namespace hw */
} /** namespace jel */

