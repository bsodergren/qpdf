#include <qpdf/QPDFNameTreeObjectHelper.hh>
#include <qpdf/NNTree.hh>

class NameTreeDetails: public NNTreeDetails
{
  public:
    virtual std::string const& itemsKey() const override
    {
        static std::string k("/Names");
        return k;
    }
    virtual bool keyValid(QPDFObjectHandle oh) const override
    {
        return oh.isString();
    }
    virtual int compareKeys(
        QPDFObjectHandle a, QPDFObjectHandle b) const override
    {
        if (! (keyValid(a) && keyValid(b)))
        {
            // We don't call this without calling keyValid first
            throw std::logic_error("comparing invalid keys");
        }
        auto as = a.getUTF8Value();
        auto bs = b.getUTF8Value();
        return ((as < bs) ? -1 : (as > bs) ? 1 : 0);
    }
};

static NameTreeDetails name_tree_details;

QPDFNameTreeObjectHelper::Members::~Members()
{
}

QPDFNameTreeObjectHelper::Members::Members(
    QPDFObjectHandle& oh, QPDF* q, bool auto_repair) :
    impl(std::make_shared<NNTreeImpl>(name_tree_details, q, oh, auto_repair))
{
}

QPDFNameTreeObjectHelper::QPDFNameTreeObjectHelper(
    QPDFObjectHandle oh, QPDF& q, bool auto_repair) :
    QPDFObjectHelper(oh),
    m(new Members(oh, &q, auto_repair))
{
}

QPDFNameTreeObjectHelper::QPDFNameTreeObjectHelper(QPDFObjectHandle oh) :
    QPDFObjectHelper(oh),
    m(new Members(oh, nullptr, false))
{
}

QPDFNameTreeObjectHelper::~QPDFNameTreeObjectHelper()
{
}

QPDFNameTreeObjectHelper
QPDFNameTreeObjectHelper::newEmpty(QPDF& qpdf, bool auto_repair)
{
    return QPDFNameTreeObjectHelper(
        QPDFObjectHandle::parse("<< /Names [] >>"), qpdf, auto_repair);
}

QPDFNameTreeObjectHelper::iterator::iterator(
    std::shared_ptr<NNTreeIterator> const& i) :
    impl(i)
{
}

bool
QPDFNameTreeObjectHelper::iterator::valid() const
{
    return impl->valid();
}

QPDFNameTreeObjectHelper::iterator&
QPDFNameTreeObjectHelper::iterator::operator++()
{
    ++(*impl);
    return *this;
}

QPDFNameTreeObjectHelper::iterator&
QPDFNameTreeObjectHelper::iterator::operator--()
{
    --(*impl);
    return *this;
}

QPDFNameTreeObjectHelper::iterator::reference
QPDFNameTreeObjectHelper::iterator::operator*()
{
    auto p = **impl;
    return std::make_pair(p.first.getUTF8Value(), p.second);
}

bool
QPDFNameTreeObjectHelper::iterator::operator==(iterator const& other) const
{
    return *(impl) == *(other.impl);
}

QPDFNameTreeObjectHelper::iterator
QPDFNameTreeObjectHelper::begin() const
{
    return iterator(std::make_shared<NNTreeIterator>(this->m->impl->begin()));
}

QPDFNameTreeObjectHelper::iterator
QPDFNameTreeObjectHelper::end() const
{
    return iterator(std::make_shared<NNTreeIterator>(this->m->impl->end()));
}

QPDFNameTreeObjectHelper::iterator
QPDFNameTreeObjectHelper::last() const
{
    return iterator(std::make_shared<NNTreeIterator>(this->m->impl->last()));
}

QPDFNameTreeObjectHelper::iterator
QPDFNameTreeObjectHelper::find(std::string const& key,
                               bool return_prev_if_not_found)
{
    auto i = this->m->impl->find(QPDFObjectHandle::newUnicodeString(key),
                                 return_prev_if_not_found);
    return iterator(std::make_shared<NNTreeIterator>(i));
}

QPDFNameTreeObjectHelper::iterator
QPDFNameTreeObjectHelper::insert(std::string const& key,
                                 QPDFObjectHandle value)
{
    auto i = this->m->impl->insert(
        QPDFObjectHandle::newUnicodeString(key), value);
    return iterator(std::make_shared<NNTreeIterator>(i));
}

bool
QPDFNameTreeObjectHelper::hasName(std::string const& name)
{
    auto i = find(name);
    return (i != end());
}

bool
QPDFNameTreeObjectHelper::findObject(
    std::string const& name, QPDFObjectHandle& oh)
{
    auto i = find(name);
    if (i == end())
    {
        return false;
    }
    oh = (*i).second;
    return true;
}

void
QPDFNameTreeObjectHelper::setSplitThreshold(int t)
{
    this->m->impl->setSplitThreshold(t);
}

std::map<std::string, QPDFObjectHandle>
QPDFNameTreeObjectHelper::getAsMap() const
{
    std::map<std::string, QPDFObjectHandle> result;
    result.insert(begin(), end());
    return result;
}
