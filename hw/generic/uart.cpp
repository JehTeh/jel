/** @file hw/generic/uart.cpp
 *  @brief Definitions for the generic BasicUart class.
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
#include <cassert>
/** jel Library Headers */
#include "hw/api_uart.hpp"
#include "os/api_time.hpp"
#include "os/api_threads.hpp"
#include "os/api_system.hpp"

namespace jel
{
namespace hw
{
namespace uart 
{


BasicUart_Base::BasicUart_Base(const Config& config) : cfg_{config}
{

}

size_t BasicUart_Base::read(char* buffer, const size_t bufferLen, const Duration& timeout)
{
  if(bufferLen == 0) { return 0; }
  assert(buffer);
  setRxIsrEnable(false);
  rx_.buffer = const_cast<char*>(buffer);
  rx_.pos = 0;
  rx_.totalLen = bufferLen;
  rx_.flag.unlock();
  Timestamp startTime = SteadyClock::now();
  switch(rx_.mode)
  {
    //In ISR mode the hardware receive buffer is first flushed of characters. If there is still room
    //for receiving more characters, the ISR is enabled and the thread will sleep on the ISR done
    //flag for the timeout duration.
    case BlockingMode::isr:
      while(isRxBufferReady() && (rx_.pos < rx_.pos))
      {
        rx_.buffer[rx_.pos++] = readRxBuffer();
      }
      if(rx_.pos >= rx_.totalLen)
      {
        return rx_.pos;
      }
      //We need to ensure the RX flag is not available before the RX interrupt is enabled. If this
      //is done after enabling the rx interrupt then we could end up with a flag that never gets
      //posted.
      rx_.flag.lock(Duration::milliseconds(0));
      setRxIsrEnable(true); 
      rx_.flag.lock(timeout);
      break;
    //Polling mode simply spins on the receive buffer, grabbing a timestamp after each check to see
    //if we have timed out. Note that depending on how timestamps are implemented, this can actually
    //negatively impact performance significantly on a system. Generally, character reception is
    //better done via ISR.
    case BlockingMode::polling:
      while(rx_.pos < rx_.totalLen) 
      {
        while(!isRxBufferReady()); //Wait for a character.
        rx_.buffer[rx_.pos++] = readRxBuffer();
        if((SteadyClock::now() - startTime) >= timeout)
        { 
          return rx_.pos;
        }
      }
      break;
  }
  return rx_.pos;
}

void BasicUart_Base::write(const char* cStr, const size_t length_chars)
{
  assert(cStr); if(length_chars == 0) { return; }
  setTxIsrEnable(false);
  //Setup the tx_ operational state object.
  tx_.buffer = cStr;
  tx_.pos = 0;
  tx_.totalLen = length_chars;
  tx_.flag.unlock();
  //A switch is used here to allow easy expansion of modes in the future.
  switch(tx_.mode)
  {
    case BlockingMode::isr:
      //Ensure the transmit flag is cleared. This means if the isr posts it later we will see it.
      tx_.flag.lock(Duration::milliseconds(0));
      clearTxIsrFlags(); 
      //Load as many characters as we can into the buffer. On targets with buffering inside the
      //UART, this will work for multiple chars. Without an internal buffer this will only run once.
      while(isTxBufferReady())
      {
        loadTxBuffer(tx_.buffer[tx_.pos++]); 
        if(tx_.pos >= tx_.totalLen) //If we loaded all the characters being transmitted, 
        { //then we can just post the done flag and return.
          tx_.flag.unlock(); 
          return;
        }
      }
      //There are still characters left to transmit. This will be handled from here on out by the
      //ISR, which will post the done flag when it finishes. Therefore, we enable the interrupt then
      //wait on the flag.
      setTxIsrEnable(true);
      tx_.flag.lock();
      break;
    case BlockingMode::polling:
      //Polling mode is simple, just spin while hardware buffer is sending characters and keep
      //loading it until completion.
      while(tx_.pos < tx_.totalLen)
      {
        while(!isTxBufferReady());
        loadTxBuffer(tx_.buffer[tx_.pos++]);
      }
      break;
  }
}

void BasicUart_Base::write(const char c)
{
  //We don't modify the tx_ state object here on purpose. This means transmissions can become
  //corrupted, it is on the application to ensure this isn't called when another transmission is
  //ongoing. Currently this also just relies on polling, IMO the overhead of implementing an ISR for
  //single characters generally isn't worth it.
  while(!isTxBufferReady()) { os::ThisThread::yield(); };
  loadTxBuffer(c);
}

bool BasicUart_Base::isBusy(const Duration& timeout) 
{
  os::LockGuard lg(tx_.flag, timeout);
  if(lg.isLocked()) { return false; }
  return true;
}

void BasicUart_Base::isr_RxBufferFull() noexcept
{
  auto onExit = ToScopeGuard([&]() { clearRxIsrFlags(); });
  switch(rx_.mode)
  {
    case BlockingMode::isr:
      while(isRxBufferReady())
      {
        if(rx_.pos >= rx_.totalLen)
        {
          rx_.buffer[rx_.totalLen] = 0;
          setRxIsrEnable(false);
          rx_.flag.unlock();
          return;
        }
        rx_.buffer[rx_.pos++] = readRxBuffer();
      }
      break;
    default:
      assert(false); 
      break;
  }
}

void BasicUart_Base::isr_TxBufferEmpty() noexcept
{
  auto onExit = ToScopeGuard([&]() { clearTxIsrFlags(); });
  switch(tx_.mode)
  {
    case BlockingMode::isr:
      while(isTxBufferReady())
      {
        if(tx_.pos >= tx_.totalLen)
        {
          setTxIsrEnable(false);
          tx_.flag.unlock();
          return;
        }
        loadTxBuffer(tx_.buffer[tx_.pos++]);
      }
      break;
    default:
      assert(false);
      break;
  }
}

} /** namespace uart */
} /** namespace hw */
} /** namespace jel */

