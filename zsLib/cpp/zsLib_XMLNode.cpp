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

#define ZS_INTERNAL_XML_FILTER_BIT_DOCUMENT     0x0001
#define ZS_INTERNAL_XML_FILTER_BIT_ELEMENT      0x0002
#define ZS_INTERNAL_XML_FILTER_BIT_ATTRIBUTE    0x0004
#define ZS_INTERNAL_XML_FILTER_BIT_TEXT         0x0008
#define ZS_INTERNAL_XML_FILTER_BIT_COMMENT      0x0010
#define ZS_INTERNAL_XML_FILTER_BIT_DECLARATION  0x0020
#define ZS_INTERNAL_XML_FILTER_BIT_UNKNOWN      0x0040

#define ZS_INTERNAL_XML_FILTER_BIT_MASK_ALL     0x007F

#pragma warning(push)
#pragma warning(disable: 4290)

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
      // XML::internal::Node
      //

      //-----------------------------------------------------------------------
      Node::Node() noexcept :
        mUserData(NULL)
      {
      }

      //-----------------------------------------------------------------------
      Node::~Node() noexcept
      {
      }
      
      //-----------------------------------------------------------------------
      void Node::cloneChildren(const NodePtr &inSelf, NodePtr inNewObject) const noexcept
      {
        ZS_ASSERT(inSelf.get() == this);

        NodePtr child = inSelf->getFirstChild();
        while (child)
        {
          NodePtr newChild = Parser::cloneAssignParent(inNewObject, child);
          child = child->getNextSibling();
        }
      }

    } // namespace internal

    bool WalkSink::onDocumentEnter(DocumentPtr) noexcept        {return false;}
    bool WalkSink::onDocumentExit(DocumentPtr) noexcept         {return false;}
    bool WalkSink::onElementEnter(ElementPtr) noexcept          {return false;}
    bool WalkSink::onElementExit(ElementPtr) noexcept           {return false;}
    bool WalkSink::onAttribute(AttributePtr) noexcept           {return false;}
    bool WalkSink::onText(TextPtr) noexcept                     {return false;}
    bool WalkSink::onComment(CommentPtr) noexcept               {return false;}
    bool WalkSink::onDeclarationEnter(DeclarationPtr) noexcept  {return false;}
    bool WalkSink::onDeclarationExit(DeclarationPtr) noexcept   {return false;}
    bool WalkSink::onUnknown(UnknownPtr) noexcept               {return false;}

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // XML::Node
    //

    //-------------------------------------------------------------------------
    Node::Node() noexcept :
      internal::Node()
    {
    }

    //-------------------------------------------------------------------------
    Node::~Node() noexcept
    {
      clear();
    }

    //-------------------------------------------------------------------------
    DocumentPtr Node::getDocument() const noexcept
    {
      NodePtr root = getRoot();
      if (root->isDocument())
        return root->toDocument();

      return DocumentPtr();
    }

    //-------------------------------------------------------------------------
    static ULONG getMaskBit(Node::NodeType::Type inNodeType) noexcept
    {
      switch (inNodeType)
      {
        case Node::NodeType::Document:      return ZS_INTERNAL_XML_FILTER_BIT_DOCUMENT;
        case Node::NodeType::Element:       return ZS_INTERNAL_XML_FILTER_BIT_ELEMENT;
        case Node::NodeType::Attribute:     return ZS_INTERNAL_XML_FILTER_BIT_ATTRIBUTE;
        case Node::NodeType::Text:          return ZS_INTERNAL_XML_FILTER_BIT_TEXT;
        case Node::NodeType::Comment:       return ZS_INTERNAL_XML_FILTER_BIT_COMMENT;
        case Node::NodeType::Declaration:   return ZS_INTERNAL_XML_FILTER_BIT_DECLARATION;
        case Node::NodeType::Unknown:       return ZS_INTERNAL_XML_FILTER_BIT_UNKNOWN;
        default:                            ZS_ASSERT_FAIL("unknown XML type"); break;
      }
      return 0;
    }

    //-------------------------------------------------------------------------
    bool Node::walk(WalkSink &inWalker, const FilterList *inFilterList) const noexcept
    {
      // walks the tree in a non-recursive way to protect the stack from overflowing

      ULONG mask = ZS_INTERNAL_XML_FILTER_BIT_MASK_ALL;
      if (inFilterList)
      {
        mask = 0;
        for (FilterList::const_iterator iter = inFilterList->begin(); iter != inFilterList->end(); ++iter)
        {
          mask = mask | getMaskBit(*iter);
        }
      }

      NodePtr startChild = toNode();
      NodePtr child = startChild;
      bool allowChildren = true;
      while (child)
      {
        NodePtr nextSibling = child->getNextSibling();  // just in case the current element was orphaned
        NodePtr nextParent = child->getParent();

        bool doContinue = false;
        while (allowChildren) // using as a scope rather than as a loop
        {
          if (0 != (mask & getMaskBit(child->getNodeType())))
          {
            bool result = false;
            switch (child->getNodeType())
            {
              case NodeType::Document:      result = inWalker.onDocumentEnter(child->toDocument()); break;
              case NodeType::Element:       result = inWalker.onElementEnter(child->toElement()); break;
              case NodeType::Attribute:     result = inWalker.onAttribute(child->toAttribute()); break;
              case NodeType::Text:          result = inWalker.onText(child->toText()); break;
              case NodeType::Comment:       result = inWalker.onComment(child->toComment()); break;
              case NodeType::Declaration:   result = inWalker.onDeclarationEnter(child->toDeclaration()); break;
              case NodeType::Unknown:       result = inWalker.onUnknown(child->toUnknown()); break;
            }
            if (result)
              return true;

            if ((child->getNextSibling() != nextSibling) ||
                (child->getParent() != nextParent)) {
              // the node was orphaned, do not process its children anymore
              break;
            }
          }

          AttributePtr attribute;

          if (child->isElement())
          {
            ElementPtr element = child->toElement();
            attribute = element->getFirstAttribute();
          }
          else if (child->isDeclaration())
          {
            DeclarationPtr declaration = child->toDeclaration();
            attribute = declaration->getFirstAttribute();
          }

          // walk the attributes
          if (attribute)
          {
            if (0 != (mask & getMaskBit(attribute->getNodeType())))
            {
              while (attribute)
              {
                NodePtr next = attribute->getNextSibling(); // need to do this in advanced in case the attribute is orphaned

                if (inWalker.onAttribute(attribute))
                  return true;

                if (!next)
                  break;

                attribute = next->toAttribute();
              }
            }
          }

          if (child->getFirstChild())
          {
            child = child->getFirstChild();
            doContinue = true;
            break;
          }
          break;  // not really intending to loop
        }

        if (doContinue)
          continue;

        if ((child->isDeclaration()) &&
            (0 != (mask & ZS_INTERNAL_XML_FILTER_BIT_DECLARATION)))
        {
          // reached the exit point for the declaration
          if (inWalker.onDeclarationExit(child->toDeclaration()))
            return true;
        }

        // going to be exiting this node node by way of the sibling or the parent (but only do exit for nodes that have exit)
        if ((child->isElement()) &&
            (0 != (mask & ZS_INTERNAL_XML_FILTER_BIT_ELEMENT)))
        {
          if (inWalker.onElementExit(child->toElement()))
            return true;
        }

        if ((child->isDocument()) &&
            (0 != (mask & ZS_INTERNAL_XML_FILTER_BIT_DOCUMENT)))
        {
          if (inWalker.onDocumentExit(child->toDocument()))
            return true;
        }

        if (nextSibling)
        {
          // can't go to the next sibling if on the root
          if (child == startChild)
            break;

          allowChildren = true;
          child = nextSibling;
          continue;
        }

        // cannot walk above the start child
        if (child == startChild)
          break;

        child = nextParent;
        allowChildren = false;
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::walk(WalkSink &inWalker, NodeType::Type inType) const noexcept
    {
      FilterList filter;
      filter.push_back(inType);
      return walk(inWalker, &filter);
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getParent() const noexcept
    {
      return mParent.lock();
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getRoot() const noexcept
    {
      NodePtr found = toNode();
      NodePtr parent = found->getParent();
      while (parent)
      {
        found = parent;
        parent = parent->getParent();
      }
      return found;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getFirstChild() const noexcept
    {
      return mFirstChild;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getLastChild() const noexcept
    {
      return mLastChild;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getFirstSibling() const noexcept
    {
      NodePtr parent = getParent();
      if (parent)
        return parent->getFirstChild();

      return toNode();
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getLastSibling() const noexcept
    {
      NodePtr parent = getParent();
      if (parent)
        return parent->getLastChild();

      return toNode();
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getPreviousSibling() const noexcept
    {
      return mPreviousSibling;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getNextSibling() const noexcept
    {
      return mNextSibling;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getParentElement() const noexcept
    {
      NodePtr found = getParent();
      while (found)
      {
        if (found->isElement())
          return found->toElement();

        found = found->getParent();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getRootElement() const noexcept
    {
      ElementPtr found;
      if (isElement()) {
        found = toElement();
      } else {
        found = getParentElement();
      }

      if (!found)
        return ElementPtr();

      ElementPtr parent = found->getParentElement();
      while (parent)
      {
        found = parent;
        parent = parent->getParentElement();
      }
      return found;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getFirstChildElement() const noexcept
    {
      NodePtr found = getFirstChild();
      while (found)
      {
        if (found->isElement())
          return found->toElement();

        found = found->getNextSibling();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getLastChildElement() const noexcept
    {
      NodePtr found = getLastChild();
      while (found)
      {
        if (found->isElement())
          return found->toElement();

        found = found->getPreviousSibling();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getFirstSiblingElement() const noexcept
    {
      NodePtr found = getParent();
      if (found)
        return found->getFirstChildElement();

      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getLastSiblingElement() const noexcept
    {
      NodePtr found = getParent();
      if (found)
        return found->getLastChildElement();

      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getPreviousSiblingElement() const noexcept
    {
      NodePtr found = getPreviousSibling();
      while (found)
      {
        if (found->isElement())
          return found->toElement();

        found = found->getPreviousSibling();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getNextSiblingElement() const noexcept
    {
      NodePtr found = getNextSibling();
      while (found)
      {
        if (found->isElement())
          return found->toElement();

        found = found->getNextSibling();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findPreviousSiblingElement(String elementName) const noexcept
    {
      bool caseSensative = true;
      DocumentPtr document = getDocument();
      if (document)
        caseSensative = document->isElementNameIsCaseSensative();

      ElementPtr element = getPreviousSiblingElement();
      while (element)
      {
        if (caseSensative)
        {
          if (elementName == element->getValue())
            return element;
        }
        else
        {
          if (0 == elementName.compareNoCase(element->getValue()))
            return element;
        }
        element = element->getPreviousSiblingElement();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findNextSiblingElement(String elementName) const noexcept
    {
      bool caseSensative = true;
      DocumentPtr document = getDocument();
      if (document)
        caseSensative = document->isElementNameIsCaseSensative();

      ElementPtr element = getNextSiblingElement();
      while (element)
      {
        if (caseSensative)
        {
          if (elementName == element->getValue())
            return element;
        }
        else
        {
          if (0 == elementName.compareNoCase(element->getValue()))
            return element;
        }
        element = element->getNextSiblingElement();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findFirstChildElement(String elementName) const noexcept
    {
      bool caseSensative = true;
      DocumentPtr document = getDocument();
      if (document)
        caseSensative = document->isElementNameIsCaseSensative();

      ElementPtr element = getFirstChildElement();
      while (element)
      {
        if (caseSensative)
        {
          if (elementName == element->getValue())
            return element;
        }
        else
        {
          if (0 == elementName.compareNoCase(element->getValue()))
            return element;
        }
        element = element->getNextSiblingElement();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findLastChildElement(String elementName) const noexcept
    {
      bool caseSensative = true;
      DocumentPtr document = getDocument();
      if (document)
        caseSensative = document->isElementNameIsCaseSensative();

      ElementPtr element = getLastChildElement();
      while (element)
      {
        if (caseSensative)
        {
          if (elementName == element->getValue())
            return element;
        }
        else
        {
          if (0 == elementName.compareNoCase(element->getValue()))
            return element;
        }
        element = element->getPreviousSiblingElement();
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getParentChecked() const noexcept(false)
    {
      NodePtr result = getParent();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getRootChecked() const noexcept(false)
    {
      NodePtr result = getRoot();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getFirstChildChecked() const noexcept(false)
    {
      NodePtr result = getFirstChild();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getLastChildChecked() const noexcept(false)
    {
      NodePtr result = getLastChild();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getFirstSiblingChecked() const noexcept(false)
    {
      NodePtr result = getFirstSibling();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getLastSiblingChecked() const noexcept(false)
    {
      NodePtr result = getLastSibling();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getPreviousSiblingChecked() const noexcept(false)
    {
      NodePtr result = getPreviousSibling();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::getNextSiblingChecked() const noexcept(false)
    {
      NodePtr result = getNextSibling();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getParentElementChecked() const noexcept(false)
    {
      ElementPtr result = getParentElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getRootElementChecked() const noexcept(false)
    {
      ElementPtr result = getRootElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getFirstChildElementChecked() const noexcept(false)
    {
      ElementPtr result = getFirstChildElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getLastChildElementChecked() const noexcept(false)
    {
      ElementPtr result = getLastChildElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getFirstSiblingElementChecked() const noexcept(false)
    {
      ElementPtr result = getFirstSiblingElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getLastSiblingElementChecked() const noexcept(false)
    {
      ElementPtr result = getLastSiblingElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getPreviousSiblingElementChecked() const noexcept(false)
    {
      ElementPtr result = getPreviousSiblingElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::getNextSiblingElementChecked() const noexcept(false)
    {
      ElementPtr result = getNextSiblingElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findPreviousSiblingElementChecked(String elementName) const noexcept(false)
    {
      ElementPtr result = findPreviousSiblingElement(elementName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findNextSiblingElementChecked(String elementName) const noexcept(false)
    {
      ElementPtr result = findNextSiblingElement(elementName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findFirstChildElementChecked(String elementName) const noexcept(false)
    {
      ElementPtr result = findFirstChildElement(elementName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::findLastChildElementChecked(String elementName) const noexcept(false)
    {
      ElementPtr result = findLastChildElement(elementName);
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    void Node::orphan() noexcept
    {
      NodePtr parent = getParent();

      if (parent)
      {
        if (parent->mFirstChild.get() == this)
          parent->mFirstChild = mNextSibling;

        if (parent->mLastChild.get() == this)
          parent->mLastChild = mPreviousSibling;
      }

      if (mNextSibling)
        mNextSibling->mPreviousSibling = mPreviousSibling;

      if (mPreviousSibling)
        mPreviousSibling->mNextSibling = mNextSibling;

      // leave this node as an orphan
      mParent = NodePtr();
      mNextSibling = NodePtr();
      mPreviousSibling = NodePtr();
    }

    //-------------------------------------------------------------------------
    void Node::adoptAsFirstChild(NodePtr inNode) noexcept
    {
      if (!inNode)
        return;

      // first the node has to be orphaned
      inNode->orphan();

      // adopt this as the parent
      inNode->mParent = toNode();

      // point existing first child's previous sibling to new node
      if (mFirstChild)
        mFirstChild->mPreviousSibling = inNode;

      // point new node's next sibling to current first child
      inNode->mNextSibling = mFirstChild;

      // first child is now new node
      mFirstChild = inNode;

      // if there wasn't a last child then it is now pointing to the new node
      if (!mLastChild)
        mLastChild = inNode;
    }

    //-------------------------------------------------------------------------
    void Node::adoptAsLastChild(NodePtr inNode) noexcept
    {
      if (!inNode)
        return;

      // first the node has to be orphaned
      inNode->orphan();

      // adopt this as the parent
      inNode->mParent = toNode();

      // point existing last child's next sibling to new node
      if (mLastChild)
        mLastChild->mNextSibling = inNode;

      // point new node's previous sibling to current last child
      inNode->mPreviousSibling = mLastChild;

      // first child is now new node
      mLastChild = inNode;

      // if there wasn't a last child then it is now pointing to the new node
      if (!mFirstChild)
        mFirstChild = inNode;
    }

    //-------------------------------------------------------------------------
    void Node::adoptAsPreviousSibling(NodePtr inNode) noexcept
    {
      if (!inNode)
        return;

      NodePtr parent = mParent.lock();

      ZS_ASSERT(parent);  // you cannot add as a sibling if there is no parent

      // orphan the node first
      inNode->orphan();

      // nodes both share the same parent
      inNode->mParent = mParent;

      inNode->mPreviousSibling = mPreviousSibling;
      inNode->mNextSibling = toNode();

      if (mPreviousSibling)
        mPreviousSibling->mNextSibling = inNode;

      mPreviousSibling = inNode;
      if (parent->mFirstChild.get() == this)
        parent->mFirstChild = inNode;
    }

    //-------------------------------------------------------------------------
    void Node::adoptAsNextSibling(NodePtr inNode) noexcept
    {
      if (!inNode)
        return;

      NodePtr parent = mParent.lock();

      ZS_ASSERT(parent);  // you cannot add as a sibling if there is no parent

      // orphan the node first
      inNode->orphan();

      // nodes both share the same parent
      inNode->mParent = mParent;

      inNode->mPreviousSibling = toNode();
      inNode->mNextSibling = mNextSibling;

      if (mNextSibling)
        mNextSibling->mPreviousSibling = inNode;

      mNextSibling = inNode;
      if (parent->mLastChild.get() == this)
        parent->mLastChild = inNode;
    }

    //-------------------------------------------------------------------------
    bool Node::hasChildren() noexcept
    {
      return (bool)getFirstChild();
    }

    //-------------------------------------------------------------------------
    void Node::removeChildren() noexcept
    {
      ULONG depth = 1;

      NodePtr child = mFirstChild;
      NodePtr clean;

      while (true)
      {
        if (clean)
        {
          clean->orphan();
          clean->clear();
        }

        if (!child)
          break;

        if (child->mFirstChild)
        {
          ++depth;
          child = child->mFirstChild;
          continue;
        }

        if (child->mNextSibling)
        {
          clean = child;
          child = child->mNextSibling;
          continue;
        }

        if (1 == depth)
        {
          child->orphan();
          child->clear();
          break;
        }

        // get the parent
        clean = child;
        child = child->mParent.lock();
        --depth;
      }
    }

    //-------------------------------------------------------------------------
    void Node::clear() noexcept
    {
      removeChildren();
      mUserData = NULL;
    }

    //-------------------------------------------------------------------------
    void *Node::getUserData() const noexcept
    {
      return mUserData;
    }

    //-------------------------------------------------------------------------
    void Node::setUserData(void *inData) noexcept
    {
      mUserData = inData;
    }

    //-------------------------------------------------------------------------
    bool Node::isDocument() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isElement() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isAttribute() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isText() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isComment() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isDeclaration() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    bool Node::isUnknown() const noexcept
    {
      return false;
    }

    //-------------------------------------------------------------------------
    NodePtr Node::toNode() const noexcept
    {
      return NodePtr();
    }

    //-------------------------------------------------------------------------
    DocumentPtr Node::toDocument() const noexcept
    {
      return DocumentPtr();
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::toElement() const noexcept
    {
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    AttributePtr Node::toAttribute() const noexcept
    {
      return AttributePtr();
    }

    //-------------------------------------------------------------------------
    TextPtr Node::toText() const noexcept
    {
      return TextPtr();
    }

    //-------------------------------------------------------------------------
    CommentPtr Node::toComment() const noexcept
    {
      return CommentPtr();
    }

    //-------------------------------------------------------------------------
    DeclarationPtr Node::toDeclaration() const noexcept
    {
      return DeclarationPtr();
    }

    //-------------------------------------------------------------------------
    UnknownPtr Node::toUnknown() const noexcept
    {
      return UnknownPtr();
    }

    //-------------------------------------------------------------------------
    NodePtr Node::toNodeChecked() const noexcept(false)
    {
      NodePtr result = toNode();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    DocumentPtr Node::toDocumentChecked() const noexcept(false)
    {
      DocumentPtr result = toDocument();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr Node::toElementChecked() const noexcept(false)
    {
      ElementPtr result = toElement();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    AttributePtr Node::toAttributeChecked() const noexcept(false)
    {
      AttributePtr result = toAttribute();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    TextPtr Node::toTextChecked() const noexcept(false)
    {
      TextPtr result = toText();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    CommentPtr Node::toCommentChecked() const noexcept(false)
    {
      CommentPtr result = toComment();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    DeclarationPtr Node::toDeclarationChecked() const noexcept(false)
    {
      DeclarationPtr result = toDeclaration();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

    //-------------------------------------------------------------------------
    UnknownPtr Node::toUnknownChecked() const noexcept(false)
    {
      UnknownPtr result = toUnknown();
      ZS_THROW_CUSTOM_IF(Exceptions::CheckFailed, !result)
      return result;
    }

  } // namespace XML

} // namespace zsLib

#pragma warning(pop)
