// Copyright (c) 2005-2021 Jay Berkenbilt
//
// This file is part of qpdf.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Versions of qpdf prior to version 7 were released under the terms
// of version 2.0 of the Artistic License. At your option, you may
// continue to consider qpdf to be licensed under those terms. Please
// see the manual for additional information.

#ifndef QPDFNUMBERTREEOBJECTHELPER_HH
#define QPDFNUMBERTREEOBJECTHELPER_HH

#include <qpdf/QPDFObjectHelper.hh>
#include <qpdf/QPDFObjGen.hh>
#include <map>
#include <memory>

#include <qpdf/DLL.h>

// This is an object helper for number trees. See section 7.9.7 in the
// PDF spec (ISO 32000) for a description of number trees.

class NNTreeImpl;
class NNTreeIterator;
class NNTreeDetails;

class QPDFNumberTreeObjectHelper: public QPDFObjectHelper
{
  public:
    // The qpdf object is required so that this class can issue
    // warnings, attempt repairs, and add indirect objects.
    QPDF_DLL
    QPDFNumberTreeObjectHelper(QPDFObjectHandle, QPDF&,
                               bool auto_repair = true);

    // ABI: Legacy Constructor will be removed in QPDF 11. A
    // QPDFNumberTreeObjectHelper constructed in this way can't be
    // modified or repaired and will silently ignore problems in the
    // structure.
    QPDF_DLL
    QPDFNumberTreeObjectHelper(QPDFObjectHandle);

    // ABI: = default
    QPDF_DLL
    virtual ~QPDFNumberTreeObjectHelper()
    {
    }

    // Create an empty number tree
    QPDF_DLL
    static QPDFNumberTreeObjectHelper newEmpty(QPDF&, bool auto_repair = true);

    typedef long long int numtree_number;

    // Return overall minimum and maximum indices
    QPDF_DLL
    numtree_number getMin();
    QPDF_DLL
    numtree_number getMax();

    // Return whether the number tree has an explicit entry for this
    // number.
    QPDF_DLL
    bool hasIndex(numtree_number idx);

    // Find an object with a specific index. If found, returns true
    // and initializes oh. See also find().
    QPDF_DLL
    bool findObject(numtree_number idx, QPDFObjectHandle& oh);
    // Find the object at the index or, if not found, the object whose
    // index is the highest index less than the requested index. If
    // the requested index is less than the minimum, return false.
    // Otherwise, return true, initialize oh to the object, and set
    // offset to the difference between the requested index and the
    // actual index. For example, if a number tree has values for 3
    // and 6 and idx is 5, this method would return true, initialize
    // oh to the value with index 3, and set offset to 2 (5 - 3). See
    // also find().
    QPDF_DLL
    bool findObjectAtOrBelow(numtree_number idx, QPDFObjectHandle& oh,
                             numtree_number& offset);

    class iterator: public std::iterator<
        std::bidirectional_iterator_tag,
        std::pair<numtree_number, QPDFObjectHandle>,
        void,
        std::pair<numtree_number, QPDFObjectHandle>*,
        std::pair<numtree_number, QPDFObjectHandle>>
    {
        friend class QPDFNumberTreeObjectHelper;
      public:
        QPDF_DLL
        bool valid() const;
        QPDF_DLL
        iterator& operator++();
        QPDF_DLL
        iterator operator++(int)
        {
            iterator t = *this;
            ++(*this);
            return t;
        }
        QPDF_DLL
        iterator& operator--();
        QPDF_DLL
        iterator operator--(int)
        {
            iterator t = *this;
            --(*this);
            return t;
        }
        QPDF_DLL
        reference operator*();
        QPDF_DLL
        bool operator==(iterator const& other) const;
        QPDF_DLL
        bool operator!=(iterator const& other) const
        {
            return ! operator==(other);
        }

      private:
        iterator(std::shared_ptr<NNTreeIterator> const&);
        std::shared_ptr<NNTreeIterator> impl;
    };

    // The iterator looks like map iterator, so i.first is a string
    // and i.second is a QPDFObjectHandle. Incrementing end() brings
    // you to the first item. Decrementing end() brings you to the
    // last item.
    QPDF_DLL
    iterator begin() const;
    QPDF_DLL
    iterator end() const;
    // Return a bidirectional iterator that points to the last item.
    QPDF_DLL
    iterator last() const;

    // Find the entry with the given key. If return_prev_if_not_found
    // is true and the item is not found, return the next lower item.
    QPDF_DLL
    iterator find(numtree_number key, bool return_prev_if_not_found = false);

    // Insert a new item. If the key already exists, it is replaced.
    QPDF_DLL
    iterator insert(numtree_number key, QPDFObjectHandle value);

    // Return the contents of the number tree as a map. Note that
    // number trees may be very large, so this may use a lot of RAM.
    // It is more efficient to use QPDFNumberTreeObjectHelper's
    // iterator.
    typedef std::map<numtree_number, QPDFObjectHandle> idx_map;
    QPDF_DLL
    idx_map getAsMap() const;

    // Split a node if the number of items exceeds this value. There's
    // no real reason to ever set this except for testing.
    QPDF_DLL
    void setSplitThreshold(int);

  private:
    class Members
    {
        friend class QPDFNumberTreeObjectHelper;
        typedef QPDFNumberTreeObjectHelper::numtree_number numtree_number;

      public:
        QPDF_DLL
        ~Members();

      private:
        Members(QPDFObjectHandle& oh, QPDF*, bool auto_repair);
        Members(Members const&) = delete;

        std::shared_ptr<NNTreeImpl> impl;
    };

    PointerHolder<Members> m;
};

#endif // QPDFNUMBERTREEOBJECTHELPER_HH
