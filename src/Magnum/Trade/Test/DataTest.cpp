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

#include <sstream>
#include <Corrade/Containers/Array.h>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/TestSuite/Compare/Container.h>
#include <Corrade/Utility/Algorithms.h>
#include <Corrade/Utility/DebugStl.h>
#include <Corrade/Utility/FormatStl.h>

#include "Magnum/Trade/Data.h"

namespace Magnum { namespace Trade { namespace Test { namespace {

struct DataTest: TestSuite::Tester {
    explicit DataTest();

    void dataChunkConstruct();

    void dataChunkDeserialize();
    void dataChunkDeserializeInvalid();

    void dataChunkSerializeHeader();
    void dataChunkSerializeHeaderTooShort();

    void debugDataFlag();
    void debugDataFlags();

    void debugDataChunkType();
    void debugDataChunkSignature();
};

constexpr char Data32[]{
    '\x80', '\x0a', '\x0d', '\x0a', 'B',
        #ifndef CORRADE_TARGET_BIG_ENDIAN
        'l', 'O',
        #else
        'O', 'l',
        #endif
    'B', 0, 0, 0, 0, 'F', 'F', 's', 42,
    24 + 5, 0, 0, 0,

    'h', 'e', 'l', 'l', 'o'
};

constexpr char Data64[]{
    '\x80', '\x0a', '\x0d', '\x0a', 'B',
        #ifndef CORRADE_TARGET_BIG_ENDIAN
        'L', 'O',
        #else
        'O', 'L',
        #endif
    'B', 0, 0, 0, 0, 'F', 'F', 's', 42,
    24 + 5, 0, 0, 0, 0, 0, 0, 0,

    'h', 'e', 'l', 'l', 'o'
};

constexpr char DataWrongVersion[sizeof(DataChunkHeader)]{127};

constexpr char DataInvalidCheckBytes32[]{
    '\x80', '\x0a', '\x0d', '\x0a', 'B',
        #ifndef CORRADE_TARGET_BIG_ENDIAN
        'l', 'O',
        #else
        'O', 'l',
        #endif
    'B', 1, 0, 0, 0, 'F', 'F', 's', 42,
    24, 0, 0, 0
};

constexpr char DataInvalidCheckBytes64[]{
    '\x80', '\x0a', '\x0d', '\x0a', 'B',
        #ifndef CORRADE_TARGET_BIG_ENDIAN
        'L', 'O',
        #else
        'O', 'L',
        #endif
    'B', 0, 1, 0, 0, 'F', 'F', 's', 42,
    24, 0, 0, 0, 0, 0, 0, 0
};

constexpr Containers::ArrayView<const char> Data = sizeof(void*) == 4 ?
    Containers::arrayView(Data32) : Containers::arrayView(Data64);
constexpr Containers::ArrayView<const char> DataInvalidCheckBytes =
    sizeof(void*) == 4 ?
    Containers::arrayView(DataInvalidCheckBytes32) :
    Containers::arrayView(DataInvalidCheckBytes64);

const struct {
    const char* name;
    Containers::ArrayView<const void> data;
    const char* message;
    bool isHeader;
} DataChunkDeserializeInvalidData[] {
    {"too short header", Containers::arrayView(Data).prefix(23),
        "expected at least 24 bytes for a header but got 23",
        false},
    {"too short chunk", Containers::arrayView(Data).except(1),
        "expected at least 29 bytes but got 28",
        true},
    {"wrong version", Containers::arrayView(DataWrongVersion),
        "expected version 128 but got 127",
        false},
    {"invalid signature",
        /* Using the 32-bit signature on 64-bit and vice versa */
        sizeof(void*) == 4 ?
            Containers::arrayView(Data64) : Containers::arrayView(Data32),
        sizeof(void*) == 4 ?
            "expected signature Trade::DataChunkSignature('B', 'l', 'O', 'B') but got Trade::DataChunkSignature('B', 'L', 'O', 'B')" :
            "expected signature Trade::DataChunkSignature('B', 'L', 'O', 'B') but got Trade::DataChunkSignature('B', 'l', 'O', 'B')",
        false},
    {"invalid check bytes", Containers::arrayView(DataInvalidCheckBytes),
        "invalid header check bytes",
        false},
};

constexpr struct {
    const char* name;
    std::size_t size;
} DataChunkSerializeData[] {
    {"no extra data", sizeof(DataChunkHeader)},
    {"1735 bytes extra data", sizeof(DataChunkHeader) + 1735}
};

DataTest::DataTest() {
    addTests({&DataTest::dataChunkConstruct,

              &DataTest::dataChunkDeserialize});

    addInstancedTests({&DataTest::dataChunkDeserializeInvalid},
        Containers::arraySize(DataChunkDeserializeInvalidData));

    addInstancedTests({&DataTest::dataChunkSerializeHeader},
        Containers::arraySize(DataChunkSerializeData));

    addTests({&DataTest::dataChunkSerializeHeaderTooShort});

    addTests({&DataTest::debugDataFlag,
              &DataTest::debugDataFlags,

              &DataTest::debugDataChunkType,
              &DataTest::debugDataChunkSignature});
}

void DataTest::dataChunkConstruct() {
    constexpr DataChunkType type = DataChunkType(Utility::Endianness::fourCC('F', 'F', 's', 42));

    DataChunk a{type};
    CORRADE_VERIFY(&a.dataChunkHeader() == static_cast<void*>(&a));
    CORRADE_COMPARE(a.dataChunkType(), type);
    CORRADE_COMPARE(a.serializedSize(), sizeof(DataChunkHeader));

    constexpr DataChunk ca{type};
    constexpr DataChunkHeader cheader = ca.dataChunkHeader();
    constexpr DataChunkType ctype = ca.dataChunkType();
    constexpr std::size_t csize = ca.serializedSize();
    CORRADE_COMPARE(cheader.type, type); /* Test at least something for this */
    CORRADE_COMPARE(ctype, type);
    CORRADE_COMPARE(csize, sizeof(DataChunkHeader));
}

void DataTest::dataChunkDeserialize() {
    {
        CORRADE_VERIFY(DataChunk::deserialize(Data));

        const DataChunk& chunk = DataChunk::from(Data);
        CORRADE_VERIFY(chunk.isDataChunkHeader());
        CORRADE_VERIFY(DataChunk::isDataChunk(Data));
        CORRADE_COMPARE(chunk.dataChunkType(), DataChunkType(Utility::Endianness::fourCC('F', 'F', 's', 42)));
    } {
        /* Verify the non-const variant also */
        Containers::Array<char> data{Containers::NoInit, Containers::arraySize(Data)};
        Utility::copy(Data, data);
        /** @todo the explicitly needed cast is UGLY, any ideas? */
        CORRADE_VERIFY(DataChunk::deserialize(Containers::ArrayView<void>{data}));

        DataChunk& chunk = DataChunk::from(Containers::ArrayView<void>{data});
        CORRADE_VERIFY(chunk.isDataChunkHeader());
        CORRADE_VERIFY(DataChunk::isDataChunk(data));
        CORRADE_COMPARE(chunk.dataChunkType(), DataChunkType(Utility::Endianness::fourCC('F', 'F', 's', 42)));
    }
}

void DataTest::dataChunkDeserializeInvalid() {
    auto&& data = DataChunkDeserializeInvalidData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    std::ostringstream out;
    Error redirectError{&out};
    CORRADE_VERIFY(!DataChunk::deserialize(data.data));
    CORRADE_COMPARE(out.str(),
        Utility::formatString("Trade::DataChunk::deserialize(): {}\n", data.message));

    /* Check that other APIs return consistent results also */
    CORRADE_VERIFY(!DataChunk::isDataChunk(data.data));
    if(data.data.size() >= sizeof(DataChunkHeader)) {
        const DataChunk& chunk = *reinterpret_cast<const DataChunk*>(data.data.data());
        CORRADE_COMPARE(chunk.isDataChunkHeader(), data.isHeader);
    } else CORRADE_VERIFY(!data.isHeader);
}

void DataTest::dataChunkSerializeHeader() {
    auto&& data = DataChunkSerializeData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    Containers::Array<char> out{Containers::NoInit, data.size};
    DataChunk c{DataChunkType(Utility::Endianness::fourCC('f', 'f', 'S', 42))};
    std::size_t size = c.serializeHeaderInto(out, 0xfeed);
    CORRADE_COMPARE(size, sizeof(DataChunkHeader));
    #ifndef CORRADE_TARGET_BIG_ENDIAN
    if(sizeof(void*) == 4) CORRADE_COMPARE_AS(out.prefix(size),
        Containers::arrayView<char>({
            '\x80', '\x0a', '\x0d', '\x0a', 'B', 'l', 'O', 'B', 0, 0,
            '\xed', '\xfe', 'f', 'f', 'S', 42,
            char(data.size & 0xff), char(data.size >> 8 & 0xff), 0, 0,
        }), TestSuite::Compare::Container);
    else CORRADE_COMPARE_AS(out.prefix(size),
        Containers::arrayView<char>({
            '\x80', '\x0a', '\x0d', '\x0a', 'B', 'L', 'O', 'B', 0, 0,
            '\xed', '\xfe', 'f', 'f', 'S', 42,
            char(data.size & 0xff), char(data.size >> 8 & 0xff), 0, 0, 0, 0, 0, 0
        }), TestSuite::Compare::Container);
    #else
    if(sizeof(void*) == 4) CORRADE_COMPARE_AS(out.prefix(size),
        Containers::arrayView<char>({
            '\x80', '\x0a', '\x0d', '\x0a', 'B', 'O', 'l', 'B', 0, 0,
            '\xed', '\xfe', 'f', 'f', 'S', 42,
            0, 0, char(data.size >> 8 & 0xff), char(data.size & 0xff)
        }), TestSuite::Compare::Container);
    else CORRADE_COMPARE_AS(out.prefix(size),
        Containers::arrayView<char>({
            '\x80', '\x0a', '\x0d', '\x0a', 'B', 'O', 'L', 'B', 0, 0,
            '\xed', '\xfe', 'f', 'f', 'S', 42,
            0, 0, 0, 0, 0, 0, char(data.size >> 8 & 0xff), char(data.size & 0xff)
        }), TestSuite::Compare::Container);
    #endif
}

void DataTest::dataChunkSerializeHeaderTooShort() {
    std::ostringstream out;
    Error redirectError{&out};
    char data[sizeof(DataChunkHeader) - 1];
    DataChunk{DataChunkType{}}.serializeHeaderInto(data);
    CORRADE_COMPARE(out.str(),
        "Trade::DataChunk::serializeHeaderInto(): data too small, expected at least 24 bytes but got 23\n");
}

void DataTest::debugDataFlag() {
    std::ostringstream out;

    Debug{&out} << DataFlag::Owned << DataFlag(0xf0);
    CORRADE_COMPARE(out.str(), "Trade::DataFlag::Owned Trade::DataFlag(0xf0)\n");
}

void DataTest::debugDataFlags() {
    std::ostringstream out;

    Debug{&out} << (DataFlag::Owned|DataFlag::Mutable) << DataFlags{};
    CORRADE_COMPARE(out.str(), "Trade::DataFlag::Owned|Trade::DataFlag::Mutable Trade::DataFlags{}\n");
}

void DataTest::debugDataChunkType() {
    std::ostringstream out;

    Debug{&out} << DataChunkType(Utility::Endianness::fourCC('M', 's', 'h', '\xab')) << DataChunkType{};
    CORRADE_COMPARE(out.str(), "Trade::DataChunkType('M', 's', 'h', 0xab) Trade::DataChunkType(0x0, 0x0, 0x0, 0x0)\n");
}

void DataTest::debugDataChunkSignature() {
    std::ostringstream out;

    Debug{&out} << DataChunkSignature::LittleEndian64 << DataChunkSignature{};
    CORRADE_COMPARE(out.str(), "Trade::DataChunkSignature('B', 'L', 'O', 'B') Trade::DataChunkSignature(0x0, 0x0, 0x0, 0x0)\n");
}

}}}}

CORRADE_TEST_MAIN(Magnum::Trade::Test::DataTest)
