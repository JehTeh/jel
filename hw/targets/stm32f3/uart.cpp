/** @file hw/targets/stm32f3/uart.cpp
 *  @brief UART definitions for the STM32 MCU.
 *
 *  @detail
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

/** C/C++ Standard Library Headers */
#include <cstdint>
/** jel Library Headers */
#include "hw/api_uart.hpp"
#include "hw/api_exceptions.hpp"
#include "hw/api_irq.hpp"

void isrEntry_Uart0() noexcept __attribute__((interrupt ("IRQ")));

namespace jel
{
namespace hw
{
namespace uart 
{

BasicUart* isrVectorDispatchTable[8];

struct BasicUartHardwareProperties
{
  UartInstance instance;
  uint32_t base;
  uint32_t uartSystemId;
  irq::Index isrChannelId;
  uint32_t io_txPort;
  uint8_t io_txPin;
  uint32_t io_muxTx;
  uint32_t io_rxPort;
  uint8_t io_rxPin;
  uint32_t io_muxRx;
};

const BasicUartHardwareProperties HwMap[] =
{
  
};

BasicUart::BasicUart(const BasicUart_Base::Config& config) : BasicUart_Base{config}, hw_{nullptr}
{
  for(const auto& hwp : HwMap)
  {
    if(hwp.instance == cfg_.instance)
    {
      hw_ = &hwp;
      break;
    }
  }
  if(hw_ == nullptr)
  {
    throw Exception{ExceptionCode::driverInstanceNotAvailable,
      "This UART instance is not available on this platform." }; 
  }
  isrVectorDispatchTable[static_cast<size_t>(hw_->instance)] = this;
  initializeHardware();
}

BasicUart::~BasicUart() noexcept
{
  if(hw_ != nullptr)
  {
    isrVectorDispatchTable[static_cast<size_t>(hw_->instance)] = nullptr;
//    UARTDisable(hw_->base);
  }
}

void BasicUart::reconfigure(const Config& config)
{
  cfg_ = config;
  initializeHardware();
}

char BasicUart::readRxBuffer()
{
//  return UARTCharGetNonBlocking(hw_->base);
  return 0;
}

void BasicUart::loadTxBuffer(const char c)
{
//  UARTCharPutNonBlocking(hw_->base, c);
}

bool BasicUart::isRxBufferReady()
{
//  return UARTCharsAvail(hw_->base) > 0 ? true : false;
  return false;
}

bool BasicUart::isTxBufferReady()
{
//  return !UARTBusy(hw_->base);
  return false;
}

void BasicUart::setRxIsrEnable(const bool enableIsr)
{
//  if(enableIsr) { UARTIntEnable(hw_->base, UART_INT_RX | UART_INT_RT); }
//  else { UARTIntDisable(hw_->base, UART_INT_RX | UART_INT_RT); }
}

void BasicUart::setTxIsrEnable(const bool enableIsr)
{
//  if(enableIsr) { UARTIntEnable(hw_->base, UART_INT_TX); }
//  else { UARTIntDisable(hw_->base, UART_INT_TX); }
}

void BasicUart::clearRxIsrFlags()
{
//  UARTIntClear(hw_->base, UART_INT_RX);
}

void BasicUart::clearTxIsrFlags()
{
//  UARTIntClear(hw_->base, UART_INT_TX);
}

void BasicUart::initializeHardware()
{
  /*
  SysCtlPeripheralEnable(hw_->uartSystemId);
  ROM_GPIOPinTypeUART(hw_->io_rxPort, hw_->io_rxPin); ROM_GPIOPinTypeUART(hw_->io_txPort, hw_->io_txPin);
  GPIOPinConfigure(hw_->io_muxRx); GPIOPinConfigure(hw_->io_muxTx);
  while(!SysCtlPeripheralReady(hw_->uartSystemId));
  UARTEnable(hw_->base);
  UARTClockSourceSet(hw_->base, UART_CLOCK_SYSTEM);
  ROM_UARTFIFOLevelSet(hw_->base, UART_FIFO_TX7_8, UART_FIFO_RX1_8);
  ROM_UARTFIFOEnable(hw_->base);
  uint32_t cfgWord = 0;
  switch(cfg_.parity)
  {
    case Parity::even:
      cfgWord |= UART_CONFIG_PAR_EVEN;
      break;
    case Parity::odd:
      cfgWord |= UART_CONFIG_PAR_ODD;
      break;
    case Parity::none:
      cfgWord |= UART_CONFIG_PAR_NONE;
      break;
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested parity setting is not supported by this UART."};
  }
  switch(cfg_.wordlen)
  {
    case WordLength::eight:
      cfgWord |= UART_CONFIG_WLEN_8;
      break;
    case WordLength::seven:
      cfgWord |= UART_CONFIG_WLEN_7;
      break;
    case WordLength::six:
      cfgWord |= UART_CONFIG_WLEN_6;
      break;
    case WordLength::five:
      cfgWord |= UART_CONFIG_WLEN_5;
      break;
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested word length setting is not supported by this UART."};
  }
  switch(cfg_.stop)
  {
    case StopBits::one:
      cfgWord |= UART_CONFIG_STOP_ONE;
      break;
    case StopBits::two:
      cfgWord |= UART_CONFIG_STOP_TWO;
      break;
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested stop bit setting is not supported by this UART."};
  }
  UARTConfigSetExpClk(hw_->base, systemClockFrequency_Hz(), static_cast<uint32_t>(cfg_.baud), cfgWord);
  switch(cfg_.rxBlockingMode)
  {
    case BlockingMode::isr:
      irq::InterruptController::enableInterrupt(hw_->isrChannelId);
      break;
    case BlockingMode::polling:
      break;
  }
  switch(cfg_.txBlockingMode)
  {
    case BlockingMode::isr:
      irq::InterruptController::enableInterrupt(hw_->isrChannelId);
      UARTTxIntModeSet(hw_->base, UART_TXINT_MODE_EOT);
      break;
    case BlockingMode::polling:
      break;
  }
*/
};

class InterruptDispatcher
{
public:
  static void uartEntry(const UartInstance instance) noexcept
  {
    /*
    BasicUart* uart = isrVectorDispatchTable[static_cast<size_t>(instance)];
    uint32_t flags = UARTIntStatus(uart->hw_->base, true);
    switch(flags)
    {
      case UART_INT_RT:
      case UART_INT_RX:
        uart->isr_RxBufferFull();
        break;
      case UART_INT_TX:
        uart->isr_TxBufferEmpty();
        break;
      default:
        UARTIntClear(uart->hw_->base, flags);
        break;
    }
    */
  }
};

} /** namespace uart */
} /** namespace hw */
} /** namespace jel */

void isrEntry_Uart0() noexcept
{
  using namespace jel::hw::uart;
  InterruptDispatcher::uartEntry(UartInstance::uart0);
}

