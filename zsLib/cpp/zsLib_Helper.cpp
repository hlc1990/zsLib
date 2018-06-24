/*

 Copyright (c) 2014, Robin Raymond
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#include <zsLib/internal/zsLib_Helper.h>

#ifndef ZSLIB_EVENTING_NOOP
#include <zsLib/internal/zsLib.events.h>
#else
#include <zsLib/eventing/noop.h>
#endif //ndef ZSLIB_EVENTING_NOOP

#include <zsLib/Singleton.h>
#include <zsLib/Stringize.h>
#include <zsLib/Numeric.h>
#include <zsLib/helpers.h>
#include <zsLib/IPAddress.h>
#include <zsLib/XML.h>

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  ZS_DECLARE_TYPEDEF_PTR(XML::Element, Element);
  ZS_DECLARE_TYPEDEF_PTR(XML::Node, Node);
  ZS_DECLARE_TYPEDEF_PTR(XML::Text, Text);
  ZS_DECLARE_TYPEDEF_PTR(XML::Attribute, Attribute);
  ZS_DECLARE_TYPEDEF_PTR(XML::WalkSink, WalkSink);
  ZS_DECLARE_TYPEDEF_PTR(XML::Generator, Generator);

  ZS_DECLARE_TYPEDEF_PTR(zsLib::internal::Helper, Helper);

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (forwards)
    //

    void setup();
#ifdef WINUWP
#ifdef __cplusplus_winrt
    void setup(Windows::UI::Core::CoreDispatcher ^dispatcher) noexcept;
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
    void setup(winrt::Windows::UI::Core::CoreDispatcher dispatcher) noexcept;
#endif //CPPWINRT_VERSION
#endif //WINUWP

    
    //-------------------------------------------------------------------------
    static void set8(void* memory, size_t offset, BYTE v)
    {
      static_cast<BYTE*>(memory)[offset] = v;
    }

    //-------------------------------------------------------------------------
    static BYTE get8(const void* memory, size_t offset)
    {
      return static_cast<const BYTE*>(memory)[offset];
    }

    //-------------------------------------------------------------------------
    Helper::Params Helper::slog(const char *message)
    {
      return Params(message, "zsLib::Helper");
    }

    //-------------------------------------------------------------------------
    Helper::Params Helper::slog(
                                const char *logObjectName,
                                const char *message
                                )
    {
      return Params(message, logObjectName);
    }
  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // zsLib::IHelper
  //

  //---------------------------------------------------------------------------
  void IHelper::setup() noexcept
  {
    internal::setup();
  }

#ifdef WINUWP

#ifdef __cplusplus_winrt
  //---------------------------------------------------------------------------
  void IHelper::setup(Windows::UI::Core::CoreDispatcher ^dispatcher) noexcept
  {
    internal::setup(dispatcher);
  }
#endif //__cplusplus_winrt

#ifdef CPPWINRT_VERSION
  //---------------------------------------------------------------------------
  void IHelper::setup(winrt::Windows::UI::Core::CoreDispatcher dispatcher) noexcept
  {
    internal::setup(dispatcher);
  }
#endif //CPPWINRT_VERSION

#endif //WINUWP

  //---------------------------------------------------------------------------
  RecursiveLockPtr IHelper::getGlobalLock() noexcept
  {
    static SingletonLazySharedPtr<RecursiveLock> singleton(make_shared<RecursiveLock>());
    return singleton.singleton();
  }


  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                bool &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<bool>(str);
      } catch(const Numeric<bool>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                CHAR &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<CHAR>(str);
      } catch(const Numeric<CHAR>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                UCHAR &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<UCHAR>(str);
      } catch(const Numeric<UCHAR>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                SHORT &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<SHORT>(str);
      } catch(const Numeric<SHORT>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                USHORT &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<USHORT>(str);
      } catch(const Numeric<USHORT>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                LONG &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<LONG>(str);
      } catch(const Numeric<LONG>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                ULONG &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<ULONG>(str);
      } catch(const Numeric<ULONG>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                LONGLONG &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<LONGLONG>(str);
      } catch(const Numeric<LONGLONG>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                ULONGLONG &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<ULONGLONG>(str);
      } catch(const Numeric<ULONGLONG>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                INT &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<INT>(str);
      } catch(const Numeric<INT>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                UINT &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<UINT>(str);
      } catch(const Numeric<UINT>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                FLOAT &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<FLOAT>(str);
      } catch(const Numeric<FLOAT>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                DOUBLE &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<DOUBLE>(str);
      } catch(const Numeric<DOUBLE>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                String &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    outValue = getElementTextAndDecode(elem->findFirstChildElement(subElementName));
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Time &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Time>(str);
      } catch(const Numeric<Time>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Milliseconds &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Milliseconds>(str);
      } catch(const Numeric<Milliseconds>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Microseconds &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Microseconds>(str);
      } catch(const Numeric<Microseconds>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<bool> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<CHAR> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<UCHAR> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<SHORT> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<USHORT> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<LONG> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<ULONG> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<LONGLONG> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<ULONGLONG> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<INT> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<UINT> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<FLOAT> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<DOUBLE> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<decltype(outValue.mType)>(str);
      } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<String> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    ElementPtr subEl = elem->findFirstChildElement(subElementName);
    if (subEl) {
      outValue = getElementTextAndDecode(elem->findFirstChildElement(subElementName));
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<Time> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Time>(str);
      } catch(const Numeric<Time>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<Milliseconds> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Milliseconds>(str);
      } catch(const Numeric<Milliseconds>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::getElementValue(
                                ElementPtr elem,
                                ZS_MAYBE_USED() const char *logObjectName,
                                const char *subElementName,
                                Optional<Microseconds> &outValue
                                ) noexcept
  {
    ZS_MAYBE_USED(logObjectName);
    if (!elem) return;
    String str = getElementText(elem->findFirstChildElement(subElementName));
    if (str.hasData()) {
      try {
        outValue = Numeric<Microseconds>(str);
      } catch(const Numeric<Microseconds>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, internal::Helper::slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  bool value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  CHAR value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(static_cast<INT>(value))));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  UCHAR value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  SHORT value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  USHORT value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  LONG value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  ULONG value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  LONGLONG value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  ULONGLONG value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  INT value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  UINT value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  FLOAT value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  DOUBLE value
                                  ) noexcept
  {
    if (!elem) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const String &value,
                                  bool adoptEmptyValue
                                  ) noexcept
  {
    if (!elem) return;

    if (value.isEmpty()) {
      if (!adoptEmptyValue) return;
    }
    elem->adoptAsLastChild(createElementWithTextAndJSONEncode(subElementName, value));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Time &value
                                  ) noexcept
  {
    if (!elem) return;
    if (Time() == value) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Milliseconds &value
                                  ) noexcept
  {
    if (!elem) return;
    if (Milliseconds() == value) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Microseconds &value
                                  ) noexcept
  {
    if (!elem) return;
    if (Microseconds() == value) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value)));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<bool> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<CHAR> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(static_cast<INT>(value.value()))));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<UCHAR> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<SHORT> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<USHORT> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<LONG> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<ULONG> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<LONGLONG> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<ULONGLONG> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<INT> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<UINT> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<FLOAT> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<DOUBLE> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<String> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithTextAndJSONEncode(subElementName, value.value()));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<Time> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<Milliseconds> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::adoptElementValue(
                                  ElementPtr elem,
                                  const char *subElementName,
                                  const Optional<Microseconds> &value
                                  ) noexcept
  {
    if (!elem) return;
    if (!value.hasValue()) return;
    elem->adoptAsLastChild(createElementWithNumber(subElementName, string(value.value())));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const char *value) noexcept
  {
    ZS_ASSERT(parentEl);
    ZS_ASSERT(name);

    if (!value) return;
    if ('\0' == *value) return;

    ElementPtr element = Element::create(name);

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValueAndJSONEncode(value);
    element->adoptAsFirstChild(tmpTxt);

    parentEl->adoptAsLastChild(element);
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const String &value) noexcept
  {
    ZS_ASSERT(parentEl);
    ZS_ASSERT(name);

    if (value.isEmpty()) return;

    ElementPtr element = Element::create(name);

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValueAndJSONEncode(value);
    element->adoptAsFirstChild(tmpTxt);

    parentEl->adoptAsLastChild(element);
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppendNumber(ElementPtr &parentEl, const char *name, const String &value) noexcept
  {
    ZS_ASSERT(parentEl);
    ZS_ASSERT(name);

    if (value.isEmpty()) return;

    ElementPtr element = Element::create(name);

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValue(value, Text::Format_JSONNumberEncoded);
    element->adoptAsFirstChild(tmpTxt);

    parentEl->adoptAsLastChild(element);
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, bool value, bool ignoreIfFalse) noexcept
  {
    ZS_ASSERT(name);

    if (ignoreIfFalse) {
      if (!value) return;
    }

    debugAppend(parentEl, ZS_PARAM(name, value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const Log::Param &param) noexcept
  {
    ZS_ASSERT(parentEl);

    if (!param.param()) return;

    parentEl->adoptAsLastChild(param.param());
  }
      
  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, ElementPtr childEl) noexcept
  {
    ZS_ASSERT(parentEl);
    ZS_ASSERT(name);

    if (!childEl) return;

    ElementPtr element = Element::create(name);
    element->adoptAsLastChild(childEl);
    parentEl->adoptAsLastChild(element);
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, ElementPtr childEl) noexcept
  {
    ZS_ASSERT(parentEl);
    if (!childEl) return;
    parentEl->adoptAsLastChild(childEl);
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, CHAR value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string((INT)value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, UCHAR value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string((UINT)value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, SHORT value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string((INT)value));
  }

  //-------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, USHORT value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string((UINT)value));
  }

  //-------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, INT value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, UINT value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, LONG value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, ULONG value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, LONGLONG value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, ULONGLONG value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, FLOAT value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0.0f == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, DOUBLE value, bool ignoreIfZero) noexcept
  {
    if (ignoreIfZero) if (0.0 == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Time &value) noexcept
  {
    if (Time() == value) return;
    debugAppendNumber(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Hours &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Minutes &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Seconds &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Milliseconds &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Microseconds &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  void IHelper::debugAppend(ElementPtr &parentEl, const char *name, const Nanoseconds &value) noexcept
  {
    if (Hours() == value) return;
    debugAppend(parentEl, name, zsLib::string(value));
  }

  //---------------------------------------------------------------------------
  String IHelper::toString(
                           ElementPtr element,
                           bool formatAsJson
                           ) noexcept
  {
    if (!element) return String();

    if (formatAsJson) {
      GeneratorPtr generator = Generator::createJSONGenerator();
      std::unique_ptr<char[]> output = generator->write(element);

      return output.get();
    }

    GeneratorPtr generator = Generator::createXMLGenerator();
    std::unique_ptr<char[]> output = generator->write(element);

    return output.get();
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::toJSON(const char *str) noexcept
  {
    if (!str) return ElementPtr();

    DocumentPtr doc = Document::createFromParsedJSON(str);

    ElementPtr childEl = doc->getFirstChildElement();
    if (!childEl) return ElementPtr();
        
    childEl->orphan();
    return childEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::toXML(const char *str) noexcept
  {
    if (!str) return ElementPtr();

    DocumentPtr doc = Document::createFromParsedXML(str);

    ElementPtr childEl = doc->getFirstChildElement();
    if (!childEl) return ElementPtr();

    childEl->orphan();
    return childEl;
  }

  //---------------------------------------------------------------------------
  String IHelper::getAttributeID(ElementPtr el) noexcept
  {
    return IHelper::getAttribute(el, "id");
  }

  //---------------------------------------------------------------------------
  void IHelper::setAttributeIDWithText(ElementPtr elem, const String &value) noexcept
  {
    if (value.isEmpty()) return;
    IHelper::setAttributeWithText(elem, "id", value);
  }

  //---------------------------------------------------------------------------
  void IHelper::setAttributeIDWithNumber(ElementPtr elem, const String &value) noexcept
  {
    if (value.isEmpty()) return;
    IHelper::setAttributeWithNumber(elem, "id", value);
  }

  //---------------------------------------------------------------------------
  String IHelper::getAttribute(
                              ElementPtr node,
                              const String &attributeName
                              ) noexcept
  {
    if (!node) return String();

    AttributePtr attribute = node->findAttribute(attributeName);
    if (!attribute) return String();

    return attribute->getValue();
  }

  //---------------------------------------------------------------------------
  void IHelper::setAttributeWithText(
                                    ElementPtr elem,
                                    const String &attrName,
                                    const String &value
                                    ) noexcept
  {
    if (!elem) return;
    if (value.isEmpty()) return;

    AttributePtr attr = Attribute::create();
    attr->setName(attrName);
    attr->setValue(value);

    elem->setAttribute(attr);
  }

  //---------------------------------------------------------------------------
  void IHelper::setAttributeWithNumber(
                                      ElementPtr elem,
                                      const String &attrName,
                                      const String &value
                                      ) noexcept
  {
    if (!elem) return;
    if (value.isEmpty()) return;

    AttributePtr attr = Attribute::create();
    attr->setName(attrName);
    attr->setValue(value);
    attr->setQuoted(false);

    elem->setAttribute(attr);
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElement(const String &elName) noexcept
  {
    ElementPtr tmp = Element::create();
    tmp->setValue(elName);
    return tmp;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithText(
                                            const String &elName,
                                            const String &textVal
                                            ) noexcept
  {
    ElementPtr tmp = Element::create(elName);

    if (textVal.isEmpty()) return tmp;

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValue(textVal, Text::Format_JSONStringEncoded);

    tmp->adoptAsFirstChild(tmpTxt);

    return tmp;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithNumber(
                                              const String &elName,
                                              const String &numberAsStringValue
                                              ) noexcept
  {
    ElementPtr tmp = Element::create(elName);

    if (numberAsStringValue.isEmpty()) {
      TextPtr tmpTxt = Text::create();
      tmpTxt->setValue("0", Text::Format_JSONNumberEncoded);
      tmp->adoptAsFirstChild(tmpTxt);
      return tmp;
    }

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValue(numberAsStringValue, Text::Format_JSONNumberEncoded);
    tmp->adoptAsFirstChild(tmpTxt);

    return tmp;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithTime(
                                            const String &elName,
                                            Time time
                                            ) noexcept
  {
    return createElementWithNumber(elName, IHelper::timeToString(time));
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithTextAndJSONEncode(
                                                         const String &elName,
                                                         const String &textVal
                                                         ) noexcept
  {
    ElementPtr tmp = Element::create(elName);
    if (textVal.isEmpty()) return tmp;

    TextPtr tmpTxt = Text::create();
    tmpTxt->setValueAndJSONEncode(textVal);
    tmp->adoptAsFirstChild(tmpTxt);
    return tmp;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithTextID(
                                              const String &elName,
                                              const String &idValue
                                              ) noexcept
  {
    ElementPtr tmp = createElement(elName);

    if (idValue.isEmpty()) return tmp;

    setAttributeIDWithText(tmp, idValue);
    return tmp;
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::createElementWithNumberID(
                                                const String &elName,
                                                const String &idValue
                                                ) noexcept
  {
    ElementPtr tmp = createElement(elName);

    if (idValue.isEmpty()) return tmp;

    setAttributeIDWithNumber(tmp, idValue);
    return tmp;
  }

  //---------------------------------------------------------------------------
  TextPtr IHelper::createText(const String &textVal) noexcept
  {
    TextPtr tmpTxt = Text::create();
    tmpTxt->setValue(textVal);

    return tmpTxt;
  }

  //---------------------------------------------------------------------------
  String IHelper::getElementText(ElementPtr el) noexcept
  {
    if (!el) return String();
    return el->getText();
  }

  //---------------------------------------------------------------------------
  String IHelper::getElementTextAndDecode(ElementPtr el) noexcept
  {
    if (!el) return String();
    return el->getTextDecoded();
  }

  //---------------------------------------------------------------------------
  String IHelper::timeToString(const Time &value) noexcept
  {
    if (Time() == value) return String();
    return string(value);
  }

  //---------------------------------------------------------------------------
  Time IHelper::stringToTime(const String &str) noexcept
  {
    if (str.isEmpty()) return Time();
    if ("0" == str) return Time();

    try {
      return Numeric<Time>(str);
    } catch(const Numeric<Time>::ValueOutOfRange &) {
      ZS_LOG_WARNING(Detail, Helper::slog("unable to convert value to time") + ZS_PARAM("value", str))
    }

    return Time();
  }

  //---------------------------------------------------------------------------
  ElementPtr IHelper::cloneAsCanonicalJSON(ElementPtr element) noexcept
  {
    if (!element) return element;

    class Walker : public WalkSink
    {
    public:
      Walker() {}

      bool onElementEnter(ElementPtr inElement) noexcept override
      {
        typedef std::list<NodePtr> ChildrenList;
        typedef std::list<AttributePtr> ChildrenAttributeList;

        // sort the elements and "other" node types
        {
          ChildrenList children;

          while (inElement->hasChildren()) {
            NodePtr child = inElement->getFirstChild();
            child->orphan();
            children.push_back(child);
          }

          NodePtr lastInsert;
          bool insertedElement = false;
          while (children.size() > 0)
          {
            NodePtr currentNode = children.front();
            children.pop_front();

            ElementPtr currentEl = (currentNode->isElement() ? currentNode->toElement() : ElementPtr());

            if (!insertedElement) {
              inElement->adoptAsLastChild(currentNode);
              lastInsert = currentNode;
              insertedElement = true;
              continue;
            }

            if (!currentEl) {
              lastInsert->adoptAsNextSibling(currentNode);
              lastInsert = currentNode;
              continue;
            }

            insertedElement = true; // will have to be true now as this child is an element

            String currentName = currentEl->getValue();

            // I know this isn't optmized as it's insertion sort but this was not meant to canonicalize the entire text of a book, but rather tiny snippets of JSON...

            bool inserted = false;

            ElementPtr childEl = inElement->getFirstChildElement();
            while (childEl) {
              String childName = childEl->getValue();

              if (currentName < childName) {
                // must insert before this child
                childEl->adoptAsPreviousSibling(currentEl);
                lastInsert = currentEl;
                inserted = true;
                break;
              }

              childEl = childEl->getNextSiblingElement();
            }

            if (inserted)
              continue;

            inElement->adoptAsLastChild(currentEl);
            lastInsert = currentEl;
          }

          ZS_ASSERT(children.size() < 1);
        }

        // sort the attributes
        {
          ChildrenAttributeList children;

          while (inElement->getFirstAttribute()) {
            AttributePtr child = inElement->getFirstAttribute();
            child->orphan();
            children.push_back(child);
          }

          while (children.size() > 0)
          {
            AttributePtr current = children.front();
            children.pop_front();

            // I know this isn't optimized as it's insertion sort but this was not meant to canonicalize the entire text of a book, but rather tiny snippets of JSON...

            AttributePtr child = inElement->getFirstAttribute();
            if (!child) {
              inElement->setAttribute(current);
              continue;
            }

            bool inserted = false;
            String currentName = current->getName();

            while (child) {
              String childName = child->getName();

              if (currentName < childName) {
                // must insert before this child
                child->adoptAsPreviousSibling(current);
                inserted = true;
                break;
              }

              NodePtr nextNode = child->getNextSibling();
              if (!nextNode) break;
              child = nextNode->toAttribute();
            }

            if (inserted)
              continue;

            // there *MUST* be a last attribute or crash
            inElement->getLastAttributeChecked()->adoptAsNextSibling(current);
          }

          ZS_ASSERT(children.size() < 1);
        }

        return false;
      }

    private:
    };

    if (ZS_IS_LOGGING(Insane)) {
      ZS_LOG_BASIC(Helper::slog("vvvvvvvvvvvv -- PRE-SORT  -- vvvvvvvvvvvv"))
      {
        GeneratorPtr generator = Generator::createJSONGenerator();
        std::unique_ptr<char[]> output = generator->write(element);
        ZS_LOG_BASIC( ((CSTR)output.get()) )
      }
      ZS_LOG_BASIC(Helper::slog("^^^^^^^^^^^^ -- PRE-SORT  -- ^^^^^^^^^^^^"))
    }
    ElementPtr convertEl = element->clone()->toElement();

    Node::FilterList filter;
    filter.push_back(Node::NodeType::Element);
    Walker walker;
    convertEl->walk(walker, &filter);

    if (ZS_IS_LOGGING(Insane)) {
      // let's output some logging...
      ZS_LOG_BASIC(Helper::slog("vvvvvvvvvvvv -- POST-SORT -- vvvvvvvvvvvv"))
      {
        GeneratorPtr generator = Generator::createJSONGenerator();
        std::unique_ptr<char[]> output = generator->write(convertEl);
        ZS_LOG_BASIC( ((CSTR)output.get()) )
      }
      ZS_LOG_BASIC(Helper::slog("^^^^^^^^^^^^ -- POST-SORT -- ^^^^^^^^^^^^"))
    }

    return convertEl;
  }

  //---------------------------------------------------------------------------
  void IHelper::split(
                      const String &input,
                      SplitMap &outResult,
                      char splitChar
                      ) noexcept
  {
    if (0 == input.size()) return;

    size_t start = input.find(splitChar);
    size_t end = String::npos;

    Index index = 0;
    if (String::npos == start) {
      outResult[index] = input;
      return;
    }

    if (0 != start) {
      // case where start is not a split char
      outResult[index] = input.substr(0, start);
      ++index;
    }
        
    do {
      end = input.find(splitChar, start+1);
          
      if (end == String::npos) {
        // there is no more splits left so copy from start split char to end
        outResult[index] = input.substr(start+1);
        ++index;
        break;
      }
          
      // take the mid-point of the string
      if (end != start+1) {
        outResult[index] = input.substr(start+1, end-(start+1));
        ++index;
      } else {
        outResult[index] = String();
        ++index;
      }

      // the next starting point will be the current end point
      start = end;
    } while (true);
  }
      
  //---------------------------------------------------------------------------
  void IHelper::split(
                      const String &input,
                      SplitMap &outResult,
                      const char *inSplitStr
                      ) noexcept
  {
    String splitStr(inSplitStr);

    Index index = 0;

    if (0 == input.size()) return;
    if (splitStr.isEmpty()) {
      outResult[index] = input;
      return;
    }

    size_t start = input.find(splitStr);
    size_t end = String::npos;

    if (String::npos == start) {
      outResult[index] = input;
      return;
    }

    if (0 != start) {
      // case where start is not a split str
      outResult[index] = input.substr(0, start);
      ++index;
    }

    do {
      end = input.find(splitStr, start+splitStr.length());

      if (end == String::npos) {
        // there is no more splits left so copy from start / to end
        outResult[index] = input.substr(start+splitStr.length());
        ++index;
        break;
      }

      // take the mid-point of the string
      if (end != start+splitStr.length()) {
        outResult[index] = input.substr(start+splitStr.length(), end-(start+splitStr.length()));
        ++index;
      } else {
        outResult[index] = String();
        ++index;
      }

      // the next starting point will be the current end point
      start = end;
    } while (true);
  }
      
  //---------------------------------------------------------------------------
  void IHelper::splitPruneEmpty(
                                SplitMap &ioResult,
                                bool reindex
                                ) noexcept
  {
    if (!reindex) {
      for (auto iter_doNotUse = ioResult.begin(); iter_doNotUse != ioResult.end();) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        const String &value = (*current).second;
        if (value.hasData()) continue;
        ioResult.erase(current);
      }
      return;
    }

    Index index = 0;
    SplitMap temp;
    for (auto iter = ioResult.begin(); iter != ioResult.end(); ++iter) {
      const String &value = (*iter).second;
      if (value.isEmpty()) continue;

      temp[index] = value;
      ++index;
    }

    ioResult = temp;
  }

  //---------------------------------------------------------------------------
  void IHelper::splitTrim(SplitMap &ioResult) noexcept
  {
    for (auto iter = ioResult.begin(); iter != ioResult.end(); ++iter) {
      auto &value = (*iter).second;
      value.trim();
    }
  }

  //---------------------------------------------------------------------------
  String IHelper::combine(
                          const SplitMap &input,
                          const char *combineStr
                          ) noexcept
  {
    String result;
    String spacer(combineStr);

    for (auto iter = input.begin(); iter != input.end(); ++iter) {
      auto &value = (*iter).second;
      if (result.hasData()) {
        result.append(spacer);
      }
      result.append(value);
    }
    return result;
  }

  //---------------------------------------------------------------------------
  String IHelper::combine(
                          const StringList &input,
                          const char *combineStr
                          ) noexcept
  {
    String result;
    String spacer(combineStr);

    for (auto iter = input.begin(); iter != input.end(); ++iter) {
      auto &value = (*iter);
      if (result.hasData()) {
        result.append(spacer);
      }
      result.append(value);
    }
    return result;
  }

  //---------------------------------------------------------------------------
  const String &IHelper::get(
                             const SplitMap &inResult,
                             Index index
                             ) noexcept
  {
    static String empty;
    SplitMap::const_iterator found = inResult.find(index);
    if (found == inResult.end()) return empty;
    return (*found).second;
  }

  //---------------------------------------------------------------------------
  String IHelper::getDebugString(
                                 const BYTE *buffer,
                                 size_t bufferSizeInBytes,
                                 size_t bytesPerGroup,
                                 size_t maxLineLength
                                 ) noexcept
  {
    static char convertArray[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

    if (!buffer) return String();
    if (0 == bufferSizeInBytes) return String();

    ZS_ASSERT(bytesPerGroup > 0);
    ZS_ASSERT(maxLineLength > 0);

    // two chars needed, one for new line, one for NUL byte
    char bufferFillLine[255+2];
    memset(&(bufferFillLine[0]), 0, sizeof(bufferFillLine));

    char *fillLine = &(bufferFillLine[0]);

    std::unique_ptr<char[]> temp;
    if (maxLineLength > 255) {
      temp = std::unique_ptr<char[]>(new char[maxLineLength+2]);
      memset(temp.get(), 0, sizeof(char)*(maxLineLength+2));
      fillLine = temp.get();
    }

    String result;
    bool firstLine = true;

    // each byte takes two hex digits and one representative character, each group needs one space between each group
    size_t charsPerGroup = (bytesPerGroup * 3) + 1;

    size_t groupsPerLine = maxLineLength / charsPerGroup;
    groupsPerLine = (groupsPerLine < 1 ? 1: groupsPerLine);

    while (bufferSizeInBytes > 0) {

      const BYTE *start = buffer;
      size_t totalBytesWritten = 0;

      char *fill = fillLine;

      for (size_t groups = 0; (groups < groupsPerLine); ++groups) {
        size_t bytesInNextGroup = (bufferSizeInBytes < bytesPerGroup ? bufferSizeInBytes : bytesPerGroup);
        size_t bytesMissingInGroup = bytesPerGroup - bytesInNextGroup;
        for (size_t pos = 0; pos < bytesInNextGroup; ++pos, ++buffer, ++totalBytesWritten) {
          BYTE value = *buffer;

          *fill = convertArray[value / 16];
          ++fill;
          *fill = convertArray[value % 16];
          ++fill;
        }

        if (!firstLine) {
          for (size_t pos = 0; pos < bytesMissingInGroup; ++pos) {
            // no more bytes available in the group thus insert two spaces per byte instead
            *fill = ' ';
            ++fill;
            *fill = ' ';
            ++fill;
          }
        }

        *fill = ' ';
        ++fill;

        bufferSizeInBytes -= bytesInNextGroup;
      }

      buffer = start;
      for (size_t pos = 0; pos < totalBytesWritten; ++pos, ++buffer) {
        if (isprint(*buffer)) {
          *fill = *buffer;
        } else {
          *fill = '.';
        }
        ++fill;
      }

      if ((bufferSizeInBytes > 0) ||
          (!firstLine)) {
        *fill = '\n';
        ++fill;
      }

      *fill = 0;

      result += fillLine;
      firstLine = false;
    }

    return result;
  }
  
  //---------------------------------------------------------------------------
  WORD IHelper::getBE16(const void* memory) noexcept
  {
    return static_cast<WORD>((internal::get8(memory, 0) << 8) |
                             (internal::get8(memory, 1) << 0));
  }

  //---------------------------------------------------------------------------
  DWORD IHelper::getBE32(const void* memory) noexcept
  {
    return (static_cast<DWORD>(internal::get8(memory, 0)) << 24) |
           (static_cast<DWORD>(internal::get8(memory, 1)) << 16) |
           (static_cast<DWORD>(internal::get8(memory, 2)) << 8) |
           (static_cast<DWORD>(internal::get8(memory, 3)) << 0);
  }

  //---------------------------------------------------------------------------
  QWORD IHelper::getBE64(const void* memory) noexcept
  {
    return (static_cast<QWORD>(internal::get8(memory, 0)) << 56) |
           (static_cast<QWORD>(internal::get8(memory, 1)) << 48) |
           (static_cast<QWORD>(internal::get8(memory, 2)) << 40) |
           (static_cast<QWORD>(internal::get8(memory, 3)) << 32) |
           (static_cast<QWORD>(internal::get8(memory, 4)) << 24) |
           (static_cast<QWORD>(internal::get8(memory, 5)) << 16) |
           (static_cast<QWORD>(internal::get8(memory, 6)) << 8) |
           (static_cast<QWORD>(internal::get8(memory, 7)) << 0);
  }

  //---------------------------------------------------------------------------
  void IHelper::setBE16(void* memory, WORD v) noexcept
  {
    internal::set8(memory, 0, static_cast<BYTE>(v >> 8));
    internal::set8(memory, 1, static_cast<BYTE>(v >> 0));
  }

  //---------------------------------------------------------------------------
  void IHelper::setBE32(void* memory, DWORD v) noexcept
  {
    internal::set8(memory, 0, static_cast<BYTE>(v >> 24));
    internal::set8(memory, 1, static_cast<BYTE>(v >> 16));
    internal::set8(memory, 2, static_cast<BYTE>(v >> 8));
    internal::set8(memory, 3, static_cast<BYTE>(v >> 0));
  }

  //---------------------------------------------------------------------------
  void IHelper::setBE64(void* memory, QWORD v) noexcept
  {
    internal::set8(memory, 0, static_cast<BYTE>(v >> 56));
    internal::set8(memory, 1, static_cast<BYTE>(v >> 48));
    internal::set8(memory, 2, static_cast<BYTE>(v >> 40));
    internal::set8(memory, 3, static_cast<BYTE>(v >> 32));
    internal::set8(memory, 4, static_cast<BYTE>(v >> 24));
    internal::set8(memory, 5, static_cast<BYTE>(v >> 16));
    internal::set8(memory, 6, static_cast<BYTE>(v >> 8));
    internal::set8(memory, 7, static_cast<BYTE>(v >> 0));
  }


  //-----------------------------------------------------------------------
  void IHelper::parseIPs(
                         const String  &ipList,
                         IPAddressSet &outIPs,
                         char splitChar
                         ) noexcept
  {
    outIPs.clear();

    if (ipList.isEmpty()) return;

    SplitMap splits;

    split(ipList, splits, splitChar);

    for (size_t i = 0; i < splits.size(); ++i) {
      String value = (*(splits.find(i))).second;

      value.trim();

      try {
        IPAddress ip(value);
        ip.setPort(0);  // strip off any port

        outIPs.insert(ip);
      } catch (IPAddress::Exceptions::ParseError &) {
      }
    }
  }

  //-----------------------------------------------------------------------
  bool IHelper::containsIP(
                           const IPAddressSet &inIPs,
                           const IPAddress &inIP,
                           bool emptySetReturns
                           ) noexcept
  {
    if (inIPs.size() < 1) return emptySetReturns;

    IPAddress ip(inIP);
    if (ip.getPort()) {
      ip.setPort(0);
    }

    return inIPs.find(ip) != inIPs.end();
  }

} // namespace zsLib
