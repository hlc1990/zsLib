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

#include <zsLib/XML.h>
#include <zsLib/Exception.h>

namespace zsLib {ZS_DECLARE_SUBSYSTEM(zslib)}

namespace zsLib
{

  namespace XML
  {

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // XML::intenral::Unknown
      //

      //-----------------------------------------------------------------------
      Unknown::Unknown() noexcept
      {
      }

      //-----------------------------------------------------------------------
      Unknown::~Unknown() noexcept
      {
      }

      //-----------------------------------------------------------------------
      void Unknown::parse(XML::ParserPos &ioPos, const char *start, const char *ending) noexcept
      {
        Parser::AutoStack stack(ioPos);

        size_t endingLength = 0;
        if (ending)
          endingLength = strlen(ending);

        mValue.clear();

        ZS_ASSERT('<' == *ioPos);

        if (start) {
          size_t startLength = strlen(start);
          ZS_ASSERT(0 == strncmp(ioPos, start, startLength));
          ioPos += startLength;
          mValue = start+1; // insert all but the starting '<'
        } else {
          ++ioPos;
        }

        while (*ioPos)
        {
          if (ending) {
            if (0 == strncmp(ioPos, ending, endingLength))
              break;
          } else {
            if ('>' == *ioPos)
              break;
          }
          mValue += *ioPos;
          ++ioPos;
        }

        bool found = false;
        if (ending) {
          found = (0 == strncmp(ioPos, ending, endingLength));
          if (found) {
            mValue += String(ending).substr(0, endingLength-1);
            ioPos += endingLength;
          }
        } else {
          if ('>' == *ioPos)
          {
            found = true;
            ++ioPos;
          }
        }

        if (!found)
        {
          // did not find end to text element
          (ioPos.getParser())->addWarning(ParserWarningType_NoEndUnknownTagFound);
        }
      }

      //-----------------------------------------------------------------------
      size_t Unknown::getOutputSizeXML(ZS_MAYBE_USED() const GeneratorPtr &inGenerator) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        size_t result = 0;
        result += strlen("<");
        result += mValue.getLength();
        result += strlen(">");
        return result;
      }

      //-----------------------------------------------------------------------
      void Unknown::writeBufferXML(ZS_MAYBE_USED() const GeneratorPtr &inGenerator, char * &ioPos) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        Generator::writeBuffer(ioPos, "<");
        Generator::writeBuffer(ioPos, mValue);
        Generator::writeBuffer(ioPos, ">");
      }

      //-----------------------------------------------------------------------
      size_t Unknown::getOutputSizeJSON(ZS_MAYBE_USED() const GeneratorPtr &inGenerator) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        return 0;
      }

      //-----------------------------------------------------------------------
      void Unknown::writeBufferJSON(ZS_MAYBE_USED() const GeneratorPtr &inGenerator, ZS_MAYBE_USED() char * &ioPos) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        ZS_MAYBE_USED(ioPos);
      }

      //-----------------------------------------------------------------------
      NodePtr Unknown::cloneAssignParent(NodePtr inParent) const noexcept
      {
        UnknownPtr newObject(XML::Unknown::create());
        Parser::safeAdoptAsLastChild(inParent, newObject);
        newObject->mValue = mValue;
        (mThis.lock())->cloneChildren(mThis.lock(), newObject);
        return newObject;
      }

    } // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // XML::Unknown
    //

    //-------------------------------------------------------------------------
    Unknown::Unknown(const make_private &) noexcept :
      internal::Unknown()
    {
    }

    //-------------------------------------------------------------------------
    UnknownPtr Unknown::create() noexcept
    {
      UnknownPtr object(make_shared<Unknown>(make_private{}));
      object->mThis = object;
      return object;
    }

    //-------------------------------------------------------------------------
    bool Unknown::hasChildren() noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    void Unknown::removeChildren() noexcept
    {
    }

    //-------------------------------------------------------------------------
    NodePtr Unknown::clone() const noexcept
    {
      return cloneAssignParent(NodePtr());
    }

    //-------------------------------------------------------------------------
    void Unknown::clear() noexcept
    {
      mValue.clear();
      Node::clear();
    }

    //-------------------------------------------------------------------------
    String Unknown::getValue() const noexcept
    {
      return mValue;
    }

    //-------------------------------------------------------------------------
    void Unknown::adoptAsFirstChild(NodePtr inNode) noexcept
    {
      ZS_ASSERT_FAIL("unknown elements cannot have child nodes");
    }

    //-------------------------------------------------------------------------
    void Unknown::adoptAsLastChild(NodePtr inNode) noexcept
    {
      ZS_ASSERT_FAIL("unknown elements cannot have child nodes");
    }

    //-------------------------------------------------------------------------
    Node::NodeType::Type Unknown::getNodeType() const noexcept
    {
      return NodeType::Unknown;
    }

    //-------------------------------------------------------------------------
    bool Unknown::isUnknown() const noexcept
    {
      return true;
    }

    //-------------------------------------------------------------------------
    NodePtr Unknown::toNode() const noexcept
    {
      return mThis.lock();
    }

    //-------------------------------------------------------------------------
    UnknownPtr Unknown::toUnknown() const noexcept
    {
      return mThis.lock();
    }

  } // namespace XML

} // namespace zsLib
