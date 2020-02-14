/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "Data.h"

#include <cctype>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/EnumSet.hpp>

namespace Magnum { namespace Trade {

static_assert(sizeof(DataChunkHeader) == 16 + sizeof(std::size_t),
    "DataChunkHeader has unexpected size");
static_assert(alignof(DataChunkHeader) == sizeof(std::size_t),
    "DataChunkHeader has unexpected alignment");
static_assert(sizeof(DataChunk) == sizeof(DataChunkHeader),
    "DataChunk base should have the same size as the header alone");

Debug& operator<<(Debug& debug, const DataFlag value) {
    debug << "Trade::DataFlag" << Debug::nospace;

    switch(value) {
        /* LCOV_EXCL_START */
        #define _c(v) case DataFlag::v: return debug << "::" #v;
        _c(Owned)
        _c(Mutable)
        #undef _c
        /* LCOV_EXCL_STOP */
    }

    return debug << "(" << Debug::nospace << reinterpret_cast<void*>(UnsignedByte(value)) << Debug::nospace << ")";
}

Debug& operator<<(Debug& debug, const DataFlags value) {
    return Containers::enumSetDebugOutput(debug, value, "Trade::DataFlags{}", {
        DataFlag::Owned,
        DataFlag::Mutable});
}

namespace {
    Debug& printFourCC(Debug& debug, UnsignedInt value, const char* name) {
        debug << name << Debug::nospace;

        for(std::size_t i = 0; i != 4; ++i) {
            if(i) debug << Debug::nospace << ",";

            const int c = value & 255;
            if(std::isprint(c)) {
                const char data[] = {'\'', char(c), '\'', '\0'};
                debug << data;
            } else {
                debug << reinterpret_cast<void*>(c);
            }

            value >>= 8;
        }

        return debug << Debug::nospace << ")";
    }
}

Debug& operator<<(Debug& debug, const DataChunkType value) {
    return printFourCC(debug, Containers::enumCastUnderlyingType(value), "Trade::DataChunkType(");
}

Debug& operator<<(Debug& debug, const DataChunkSignature value) {
    return printFourCC(debug, Containers::enumCastUnderlyingType(value), "Trade::DataChunkSignature(");
}

namespace {
    constexpr DataChunkHeader DataChunkHeaderPrefix{
        128, {'\x0a'}, {'\x0d', '\x0a'}, DataChunkSignature::Current, 0, 0,
        /* Type and size isn't checked when validating and gets overwritten
           when serializing */
        DataChunkType{}, 0
    };

    static_assert(DataChunkHeaderPrefix.version & 0x80,
        "version needs the high bit set to prevent detection as a text file");
}

bool DataChunk::isDataChunk(Containers::ArrayView<const void> data) {
    return data && data.size() >= sizeof(DataChunkHeader) &&
        std::memcmp(data.data(), &DataChunkHeaderPrefix, 10) == 0 &&
        reinterpret_cast<const DataChunkHeader*>(data.data())->size <= data.size();
}

bool DataChunk::isDataChunkHeader() const {
    return std::memcmp(this, &DataChunkHeaderPrefix, 10) == 0;
}

const DataChunk* DataChunk::deserialize(Containers::ArrayView<const void> data) {
    if(data.size() < sizeof(DataChunkHeader)) {
        Error{} << "Trade::DataChunk::deserialize(): expected at least" << sizeof(DataChunkHeader) << "bytes for a header but got" << data.size();
        return nullptr;
    }

    const auto& header = *reinterpret_cast<const DataChunkHeader*>(data.data());
    if(header.version != DataChunkHeaderPrefix.version) {
        Error{} << "Trade::DataChunk::deserialize(): expected version" << DataChunkHeaderPrefix.version << "but got" << header.version;
        return nullptr;
    }
    if(header.signature != DataChunkSignature::Current) {
        Error{} << "Trade::DataChunk::deserialize(): expected signature" << DataChunkSignature::Current << "but got" << header.signature;
        return nullptr;
    }
    if(std::memcmp(data.data(), &DataChunkHeaderPrefix, 12) != 0) {
        Error{} << "Trade::DataChunk::deserialize(): invalid header check bytes";
        return nullptr;
    }
    if(header.size > data.size()) {
        Error{} << "Trade::DataChunk::deserialize(): expected at least" << header.size << "bytes but got" << data.size();
        return nullptr;
    }

    return reinterpret_cast<const DataChunk*>(data.data());
}

DataChunk* DataChunk::deserialize(Containers::ArrayView<void> data) {
    return const_cast<DataChunk*>(deserialize(Containers::ArrayView<const void>{data}));
}

const DataChunk& DataChunk::from(Containers::ArrayView<const void> data) {
    const DataChunk* deserialized = deserialize(data);
    /* Assert that the chunk is valid. Don't print any assert message as it's
       already printed by deserialize(). */
    CORRADE_ASSERT(deserialized, Debug::nospace, *deserialized);
    return *deserialized;
}

DataChunk& DataChunk::from(Containers::ArrayView<void> data) {
    return const_cast<DataChunk&>(from(Containers::ArrayView<const void>{data}));
}

std::size_t DataChunk::serializeHeaderInto(const Containers::ArrayView<char> out, UnsignedShort extra) const {
    CORRADE_ASSERT(out.size() >= sizeof(DataChunkHeader),
        "Trade::DataChunk::serializeHeaderInto(): data too small, expected at least" << sizeof(DataChunkHeader) << "bytes but got" << out.size(), {});

    auto& header = *reinterpret_cast<DataChunkHeader*>(out.data());
    header = DataChunkHeaderPrefix;
    header.size = out.size();
    header.type = _header.type;
    header.extra = extra;
    return sizeof(DataChunkHeader);
}

namespace Implementation {
    void nonOwnedArrayDeleter(char*, std::size_t) { /* does nothing */ }
}

}}
