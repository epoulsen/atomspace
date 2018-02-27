/*
 * opencog/atoms/base/Handle.h
 *
 * Copyright (C) 2008-2010 OpenCog Foundation
 * Copyright (C) 2002-2007 Novamente LLC
 * Copyright (C) 2013 Linas Vepstas <linasvepstas@gmail.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OPENCOG_HANDLE_H
#define _OPENCOG_HANDLE_H

#include <iostream>
#include <climits>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <set>
#include <unordered_set>
#include <vector>

#include <opencog/util/Counter.h>
#include <opencog/atoms/base/types.h>

// Comment this out if you want to enforce more determinism in the
// AtomSpace. For instance atoms are indexed according to content
// rather address, etc.
// #define REPRODUCIBLE_ATOMSPACE

/** \addtogroup grp_atomspace
 *  @{
 */
namespace opencog
{

//! UUID == Universally Unique Identifier
typedef unsigned long UUID;
typedef size_t ContentHash;

class Atom;
typedef std::shared_ptr<Atom> AtomPtr;

//! contains an unique identificator
class Handle : public AtomPtr
{

friend class Atom;
friend class content_based_atom_ptr_less;
friend class content_based_handle_less;

private:
    static bool atoms_less(const Atom*, const Atom*);
    static bool content_based_atoms_less(const Atom*, const Atom*);

    static const AtomPtr NULL_POINTER;

public:
    static const ContentHash INVALID_HASH = std::numeric_limits<size_t>::max();
    static const Handle UNDEFINED;

    explicit Handle(const AtomPtr& atom) : AtomPtr(atom) {}
    explicit Handle() {}
    ~Handle() {}

    ContentHash value(void) const;

    inline Handle& operator=(const AtomPtr& a) {
        this->AtomPtr::operator=(a);
        return *this;
    }

    // Cython can't access operator->() so we'll duplicate here.
    inline Atom* atom_ptr() {
        return get();
    }

    inline const Atom* const_atom_ptr() const {
        return get();
    }

    // Allows expressions like "if(h)..." to work when h has a non-null pointer.
    explicit inline operator bool() const noexcept {
        if (get()) return true;
        return false;
    }

    inline bool operator==(std::nullptr_t) const noexcept {
        return get() == 0x0;
    }
    inline bool operator!=(std::nullptr_t) const noexcept {
        return get() != 0x0;
    }
    inline bool operator==(const Atom* ap) const noexcept {
        return get() == ap;
    }
    inline bool operator!=(const Atom* ap) const noexcept {
        return get() != ap;
    }

    inline bool operator==(const Handle& h) const noexcept {
        return get() == h.get();
    }
    inline bool operator!=(const Handle& h) const noexcept {
        return get() != h.get();
    }
    bool operator< (const Handle& h) const noexcept;

    /**
     * Returns a negative value, zero or a positive value if the first
     * argument is respectively smaller than, equal to, or larger than
     * the second argument. The atom hashes are compared, so the
     * comparison is content-based, and is stable, independent of the
     * the address space layout.
     *
     * @param The first handle element.
     * @param The second handle element.
     * @return A negative value, zero or a positive value if the first
     * argument is respectively smaller than, equal to, or larger then the
     * second argument.
     */
    static int compare(const Handle&, const Handle&);
};

static inline bool operator== (std::nullptr_t, const Handle& rhs) noexcept
    { return rhs == (Atom*) nullptr; }

static inline bool operator!= (std::nullptr_t, const Handle& rhs) noexcept
    { return rhs != (Atom*) nullptr; }

bool content_eq(const opencog::Handle& lh,
                const opencog::Handle& rh) noexcept;

//! Boost needs this function to be called by exactly this name.
std::size_t hash_value(Handle const&);

struct handle_less
{
   bool operator()(const Handle& hl, const Handle& hr) const
   {
       return hl.operator<(hr);
   }
};

//! a pair of Handles
typedef std::pair<Handle, Handle> HandlePair;

//! a list of handles
typedef std::vector<Handle> HandleSeq;

//! a set of lists of handles
typedef std::set<HandleSeq> HandleSeqSet;

//! a list of lists of handles
typedef std::vector<HandleSeq> HandleSeqSeq;

//! a set of handles. Default handle set container. Usually takes less
//! RAM and has faster iteration.
typedef std::set<Handle> HandleSet;

//! a hash table. Usually has faster insertion.
typedef std::unordered_set<Handle> UnorderedHandleSet;

//! an ordered map from Handle to Handle set
typedef std::map<Handle, HandleSet> HandleMultimap;

//! an ordered map from Handle to Handle
typedef std::map<Handle, Handle> HandleMap;

//! a sequence of ordered handle maps
typedef std::vector<HandleMap> HandleMapSeq;

//! a set of ordered handle maps
typedef std::set<HandleMap> HandleMapSet;

//! a sequence of handle pairs
typedef std::vector<HandlePair> HandlePairSeq;

//! a map from handle to double
typedef Counter<Handle, double> HandleCounter;

//! a map from handle to unsigned
typedef Counter<Handle, unsigned> HandleUCounter;

//! a handle iterator
typedef std::iterator<std::forward_iterator_tag, Handle> HandleIterator;

bool content_eq(const opencog::HandleSeq& lhs,
                const opencog::HandleSeq& rhs);

bool content_eq(const opencog::HandleSet& lhs,
                const opencog::HandleSet& rhs);

struct content_based_atom_ptr_less
{
    bool operator()(const Atom* al, const Atom* ar) const
    {
        return Handle::content_based_atoms_less(al, ar);
    }
};

struct content_based_handle_less
{
    bool operator()(const Handle& hl, const Handle& hr) const
    {
        return Handle::content_based_atoms_less(hl.const_atom_ptr(),
                                                hr.const_atom_ptr());
    }
};

struct handle_seq_less
{
    bool operator()(const HandleSeq& hsl, const HandleSeq& hsr) const
    {
        size_t sl = hsl.size();
        size_t sr = hsr.size();
        if (sl != sr) return sl < sr;
        for (size_t i=0; i<sl; i++)
        {
            if (hsl[i] != hsl[i]) return hsl[i].operator<(hsr[i]);
        }
        return false;
    }
};

struct handle_seq_ptr_less
{
    bool operator()(const HandleSeq* hsl, const HandleSeq* hsr) const
    {
        return handle_seq_less().operator()(*hsl, *hsr);
    }
};

//! append string representation of the Hash to the string
static inline std::string operator+ (const char *lhs, Handle h)
{
    std::string rhs = lhs;
    char buff[25];
    snprintf(buff, 24, "%lu)", h.value());
    return rhs + buff;
}

//! append string representation of the Hash to the string
static inline std::string operator+ (const std::string &lhs, Handle h)
{
    char buff[25];
    snprintf(buff, 24, "%lu)", h.value());
    return lhs + buff;
}

// Debugging helpers see
// http://wiki.opencog.org/w/Development_standards#Print_OpenCog_Objects
// The reason indent is not an optional argument with default is
// because gdb doesn't support that, see
// http://stackoverflow.com/questions/16734783 for more explanation.
#define OC_TO_STRING_INDENT "  "
std::string oc_to_string(const Handle& h, const std::string& indent);
std::string oc_to_string(const Handle& h);
std::string oc_to_string(const HandlePair& hp, const std::string& indent);
std::string oc_to_string(const HandlePair& hp);
std::string oc_to_string(const HandleSeq& hs, const std::string& indent);
std::string oc_to_string(const HandleSeq& hs);
std::string oc_to_string(const HandleSeqSeq& hss, const std::string& indent);
std::string oc_to_string(const HandleSeqSeq& hss);
std::string oc_to_string(const HandleSet& ohs, const std::string& indent);
std::string oc_to_string(const HandleSet& ohs);
std::string oc_to_string(const UnorderedHandleSet& uhs, const std::string& indent);
std::string oc_to_string(const UnorderedHandleSet& uhs);
std::string oc_to_string(const HandleMap& hm, const std::string& indent);
std::string oc_to_string(const HandleMap& hm);
std::string oc_to_string(const HandleMultimap& hmm, const std::string& indent);
std::string oc_to_string(const HandleMultimap& hmm);
std::string oc_to_string(const HandleMapSeq& hms, const std::string& indent);
std::string oc_to_string(const HandleMapSeq& hms);
std::string oc_to_string(const HandleMapSet& hms, const std::string& indent);
std::string oc_to_string(const HandleMapSet& hms);
std::string oc_to_string(const HandlePairSeq& hps, const std::string& indent);
std::string oc_to_string(const HandlePairSeq& hps);
std::string oc_to_string(const HandleCounter& hc, const std::string& indent);
std::string oc_to_string(const HandleCounter& hc);
std::string oc_to_string(const HandleUCounter& huc, const std::string& indent);
std::string oc_to_string(const HandleUCounter& huc);
std::string oc_to_string(Type type, const std::string& indent);
std::string oc_to_string(Type type);
std::string oc_to_string(const TypeSet& types, const std::string& indent);
std::string oc_to_string(const TypeSet& types);
std::string oc_to_string(const AtomPtr& aptr, const std::string& indent);
std::string oc_to_string(const AtomPtr& aptr);

} // namespace opencog

namespace std {
ostream& operator<<(ostream&, const opencog::HandleMap&);
ostream& operator<<(ostream&, const opencog::HandleSeq&);
ostream& operator<<(ostream&, const opencog::HandleSet&);
ostream& operator<<(ostream&, const opencog::UnorderedHandleSet&);

#ifdef THIS_USED_TO_WORK_GREAT_BUT_IS_BROKEN_IN_GCC472
// The below used to work, but broke in gcc-4.7.2. The reason it
// broke is that it fails to typedef result_type and argument_type,
// which ... somehow used to work automagically?? It doesn't any more.
// I have no clue why gcc-4.7.2 broke this, and neither does google or
// stackoverflow.

template<>
inline std::size_t
std::hash<opencog::Handle>::operator()(const opencog::Handle& h) const
{
    return hash_value(h);
}

#else

// This works for me, per note immediately above.
template<>
struct hash<opencog::Handle>
{
    typedef std::size_t result_type;
    typedef opencog::Handle argument_type;
    std::size_t operator()(const opencog::Handle& h) const noexcept
    { return hash_value(h); }
};

// content-based equality
template<>
struct equal_to<opencog::Handle>
{
    typedef bool result_type;
    typedef opencog::Handle first_argument;
    typedef opencog::Handle second_argument;
    bool
    operator()(const opencog::Handle& lh,
               const opencog::Handle& rh) const noexcept
    {
        if (lh == rh) return true;
        if (nullptr == lh or nullptr == rh) return false;
        return opencog::content_eq(lh, rh);
    }
};

template<>
struct hash<opencog::HandlePair>
{
    typedef std::size_t result_type;
    typedef opencog::HandlePair argument_type;
    std::size_t
    operator()(const opencog::HandlePair& hp) const noexcept
    { return hash_value(hp.first) + hash_value(hp.second); }
};

// content-based equality
template<>
struct equal_to<opencog::HandlePair>
{
    typedef bool result_type;
    typedef opencog::HandlePair first_argument;
    typedef opencog::HandlePair second_argument;
    bool
    operator()(const opencog::HandlePair& lhp,
               const opencog::HandlePair& rhp) const noexcept
    {
        if (lhp == rhp) return true;
        std::equal_to<opencog::Handle> eq;
        return eq.operator()(lhp.first, rhp.first) and
               eq.operator()(lhp.second, rhp.second);
    }
};

#endif // THIS_USED_TO_WORK_GREAT_BUT_IS_BROKEN_IN_GCC472

} // ~namespace std

/** @}*/
#endif // _OPENCOG_HANDLE_H
