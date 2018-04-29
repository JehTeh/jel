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
/** STM HAL Headers */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister"
#include "usart.h"
#pragma GCC diagnostic pop

void isrEntry_Uart1() noexcept __attribute__((interrupt ("IRQ")));

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
  USART_TypeDef* halinst;
  UART_HandleTypeDef* haltd;
};


const BasicUartHardwareProperties HwMap[] =
{
  { UartInstance::uart1, USART1, &huart1 }, 
  { UartInstance::uart2, USART2, &huart2 }, 
  { UartInstance::uart3, USART3, &huart3 }, 
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
  }
}

size_t BasicUart::read(char* buffer, const size_t bufferLen) 
{
  if(bufferLen == 0) { return 0; }
  assert(buffer);
  rx_.buffer = const_cast<char*>(buffer);
  rx_.pos = 0;
  rx_.totalLen = bufferLen;
  rx_.flag.unlock();
  for(size_t i = 0; i < bufferLen; i++) { buffer[i] = 0; }
  switch(cfg_.rxBlockingMode)
  {
    //In ISR mode the hardware receive buffer is first flushed of characters. If there is still room
    //for receiving more characters, the ISR is enabled and the thread will sleep on the ISR done
    //flag for the timeout duration.
    case BlockingMode::isr:
      rx_.flag.lock(Duration::milliseconds(0));
      if(rx_.pos >= rx_.totalLen)
      {
        return rx_.pos;
      }
      HAL_UART_AbortReceive_IT(hw_->haltd);
      HAL_UART_Receive_IT(hw_->haltd, reinterpret_cast<uint8_t*>(buffer), bufferLen);
      //Enable the interrupt then sleep on the semaphore. Once RX ISR is finished, we should be
      //woken up.
      break;
    //Polling mode simply spins on the receive buffer, grabbing a timestamp after each check to see
    //if we have timed out. Note that depending on how timestamps are implemented, this can actually
    //negatively impact performance significantly on a system. Generally, character reception is
    //better done via ISR.
    case BlockingMode::polling:
      while(rx_.pos < rx_.totalLen) 
      {
        while(!isRxBufferReady());
        rx_.buffer[rx_.pos++] = readRxBuffer();
      }
      break;
  }
  return rx_.pos;

}
void BasicUart::write(const char* cStr, const size_t length_chars) 
{
  switch(cfg_.txBlockingMode)
  {
    case BlockingMode::polling:
      HAL_UART_Transmit(hw_->haltd, reinterpret_cast<uint8_t*>(const_cast<char*>(cStr)),
        length_chars, HAL_MAX_DELAY);
      break;
    case BlockingMode::isr:
      HAL_UART_AbortTransmit_IT(hw_->haltd);
      tx_.flag.unlock();
      tx_.flag.lock(Duration::zero());
      HAL_UART_Transmit_IT(hw_->haltd, reinterpret_cast<uint8_t*>(const_cast<char*>(cStr)),
        length_chars);
      break;
  }
}

void BasicUart::write(const char c)
{
  uint8_t cui = c;
  HAL_UART_Transmit(hw_->haltd, &cui,
    1, HAL_MAX_DELAY);
}

bool BasicUart::isBusy(const Duration& timeout)
{
  auto lg = LockGuard(tx_.flag, timeout);
  if(lg.isLocked())
  {
    return false;
  }
  return true;
}

size_t BasicUart::waitForChars(const Duration& timeout)
{
  auto lg = LockGuard(rx_.flag, timeout);
  if(lg.isLocked())
  {
    return rx_.pos;
  }
  return std::strlen(const_cast<char*>(rx_.buffer));
}

void BasicUart::reconfigure(const Config& config)
{
  cfg_ = config;
  initializeHardware();
}

char BasicUart::readRxBuffer()
{
  return 0;
}

void BasicUart::loadTxBuffer(const char c)
{
  (void)c;
}

bool BasicUart::isRxBufferReady()
{
  return false;
}

bool BasicUart::isTxBufferReady()
{
  return false;
}

void BasicUart::setRxIsrEnable(const bool enableIsr)
{
  (void)enableIsr;
}

void BasicUart::setTxIsrEnable(const bool enableIsr)
{
  (void)enableIsr;
}

void BasicUart::clearRxIsrFlags()
{
}

void BasicUart::clearTxIsrFlags()
{
}

void BasicUart::initializeHardware()
{
  UART_HandleTypeDef* ucfg = hw_->haltd;
  ucfg->Instance = hw_->halinst;
  ucfg->Init.BaudRate = static_cast<uint32_t>(cfg_.baud);
  ucfg->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  ucfg->Init.Mode = UART_MODE_TX_RX;
  ucfg->Init.OverSampling = UART_OVERSAMPLING_16;
  ucfg->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  ucfg->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  switch(cfg_.parity)
  {
    case Parity::even:
      ucfg->Init.Parity = UART_PARITY_EVEN;
      break;
    case Parity::odd:
      ucfg->Init.Parity = UART_PARITY_ODD;
      break;
    case Parity::none:
      ucfg->Init.Parity = UART_PARITY_NONE;
      break;
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested parity setting is not supported by this UART."};
  }
  switch(cfg_.wordlen)
  {
    case WordLength::eight:
      ucfg->Init.WordLength = UART_WORDLENGTH_8B;
      break;
    case WordLength::seven:
    case WordLength::six:
    case WordLength::five:
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested word length setting is not supported by this UART."};
  }
  switch(cfg_.stop)
  {
    case StopBits::one:
      ucfg->Init.StopBits = UART_STOPBITS_1;
      break;
    case StopBits::two:
      ucfg->Init.StopBits = UART_STOPBITS_2;
      break;
    default:
      throw Exception{ExceptionCode::driverFeatureNotSupported,
        "The requested stop bit setting is not supported by this UART."};
  }
  switch(cfg_.rxBlockingMode)
  {
    case BlockingMode::isr:
      break;
    case BlockingMode::polling:
      break;
  }
  switch(cfg_.txBlockingMode)
  {
    case BlockingMode::isr:
      break;
    case BlockingMode::polling:
      break;
  }
  if(HAL_UART_Init(ucfg) != HAL_OK)
  {
    throw Exception{ExceptionCode::driverInstantiationFailed,
      "Failed to initialize UART driver %d.", static_cast<uint32_t>(cfg_.instance) };
  }
};

class InterruptDispatcher
{
public:
  enum class Flags
  {
    TX_COMPLETE,
    RX_COMPLETE,
  };
  static void uartEntry(const UartInstance instance, const Flags flags) noexcept
  {
    BasicUart* uart = isrVectorDispatchTable[static_cast<size_t>(instance)];
    switch(flags)
    {
      case Flags::TX_COMPLETE:
        uart->tx_.flag.unlock();
        break;
      case Flags::RX_COMPLETE:
        uart->rx_.pos++;
        if(uart->rx_.pos >= uart->rx_.totalLen)
        {
          uart->rx_.buffer[uart->rx_.totalLen] = 0;
          uart->rx_.flag.unlock();
          return;
        }
        uint8_t* b = reinterpret_cast<uint8_t*>(const_cast<char*>(uart->rx_.buffer));
        HAL_UART_Receive_IT(uart->hw_->haltd, b, uart->rx_.totalLen);
        break;
    }
  }
};

} /** namespace uart */
} /** namespace hw */
} /** namespace jel */

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* uart)
{
  using namespace jel::hw::uart;
  if(uart->Instance == USART1)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart1, InterruptDispatcher::Flags::TX_COMPLETE);
  }
  else if(uart->Instance == USART2)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart2, InterruptDispatcher::Flags::TX_COMPLETE);
  }
  else if(uart->Instance == USART3)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart3, InterruptDispatcher::Flags::TX_COMPLETE);
  }
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* uart)
{
  using namespace jel::hw::uart;
  if(uart->Instance == USART1)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart1, InterruptDispatcher::Flags::RX_COMPLETE);
  }
  else if(uart->Instance == USART2)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart2, InterruptDispatcher::Flags::RX_COMPLETE);
  }
  else if(uart->Instance == USART3)
  {
    InterruptDispatcher::uartEntry(UartInstance::uart3, InterruptDispatcher::Flags::RX_COMPLETE);
  }
}
