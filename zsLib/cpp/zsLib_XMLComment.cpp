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
      // XML::internal::Comment
      //

      //-----------------------------------------------------------------------
      Comment::Comment() noexcept
      {
      }

      //-----------------------------------------------------------------------
      Comment::~Comment() noexcept
      {
      }

      //-----------------------------------------------------------------------
      void Comment::parse(XML::ParserPos &ioPos) noexcept
      {
        Parser::AutoStack stack(ioPos);

        mValue.clear();

        ZS_ASSERT(ioPos.isString("<!--"));

        ioPos += strlen("<!--");

        while ((*ioPos) &&
               (!ioPos.isString("-->")))
        {
          mValue += *ioPos;
          ++ioPos;
        }

        if (ioPos.isEOF())
        {
          // did not find end to text element
          (ioPos.getParser())->addWarning(ParserWarningType_NoEndCommentFound);
        }
        else
        {
          ioPos += strlen("-->");
        }
      }

      //-----------------------------------------------------------------------
      size_t Comment::getOutputSizeXML(ZS_MAYBE_USED() const GeneratorPtr &inGenerator) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        size_t result = 0;
        result += strlen("<!--");
        result += mValue.getLength();
        result += strlen("-->");
        return result;
      }

      //-----------------------------------------------------------------------
      void Comment::writeBufferXML(ZS_MAYBE_USED() const GeneratorPtr &inGenerator, char * &ioPos) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        Generator::writeBuffer(ioPos, "<!--");
        Generator::writeBuffer(ioPos, mValue);
        Generator::writeBuffer(ioPos, "-->");
      }

      //-----------------------------------------------------------------------
      size_t Comment::getOutputSizeJSON(ZS_MAYBE_USED() const GeneratorPtr &inGenerator) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        size_t result = 0;
        return result;
      }

      //-----------------------------------------------------------------------
      void Comment::writeBufferJSON(ZS_MAYBE_USED() const GeneratorPtr &inGenerator, ZS_MAYBE_USED() char * &ioPos) const noexcept
      {
        ZS_MAYBE_USED(inGenerator);
        ZS_MAYBE_USED(ioPos);
      }

      //-----------------------------------------------------------------------
      NodePtr Comment::cloneAssignParent(NodePtr inParent) const noexcept
      {
        CommentPtr newObject(XML::Comment::create());
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
    // XML::Comment
    //

    //-------------------------------------------------------------------------
    Comment::Comment(const make_private &) noexcept :
      internal::Comment()
    {
    }

    //-------------------------------------------------------------------------
    CommentPtr Comment::create() noexcept
    {
      CommentPtr object(make_shared<Comment>(make_private{}));
      object->mThis = object;
      return object;
    }

    //-------------------------------------------------------------------------
    bool Comment::hasChildren() noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    void Comment::removeChildren() noexcept
    {
    }

    //-------------------------------------------------------------------------
    NodePtr Comment::clone() const noexcept
    {
      return cloneAssignParent(NodePtr());
    }

    //-------------------------------------------------------------------------
    void Comment::clear() noexcept
    {
      mValue.clear();
    }

    //-------------------------------------------------------------------------
    String Comment::getValue() const noexcept
    {
      return mValue;
    }

    //-------------------------------------------------------------------------
    void Comment::adoptAsFirstChild(NodePtr inNode) noexcept
    {
      ZS_ASSERT_FAIL("comments are not allowed to have child nodes");
    }

    //-------------------------------------------------------------------------
    void Comment::adoptAsLastChild(NodePtr inNode) noexcept
    {
      ZS_ASSERT_FAIL("comments are not allowed to have child nodes");
    }

    //-------------------------------------------------------------------------
    Node::NodeType::Type Comment::getNodeType() const noexcept
    {
      return NodeType::Comment;
    }

    //-------------------------------------------------------------------------
    bool Comment::isComment() const noexcept
    {
      return true;
    }

    //-------------------------------------------------------------------------
    NodePtr Comment::toNode() const noexcept
    {
      return mThis.lock();
    }

    //-------------------------------------------------------------------------
    CommentPtr Comment::toComment() const noexcept
    {
      return mThis.lock();
    }

  } // namespace XML

} // namespace zsLib
