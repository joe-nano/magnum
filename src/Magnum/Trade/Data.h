#ifndef Magnum_Trade_Data_h
#define Magnum_Trade_Data_h
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

/** @file
 * @brief Class @ref Magnum::Trade::DataChunk, struct @ref Magnum::Trade::DataChunkHeader, enum @ref Magnum::Trade::DataFlag, @ref Magnum::Trade::DataChunkSignature, @ref Magnum::Trade::DataChunkType, enum set @ref Magnum::Trade::DataFlags
 * @m_since_latest
 */

#include <cstddef>
#include <Corrade/Containers/EnumSet.h>
#include <Corrade/Utility/Endianness.h>

#include "Magnum/Magnum.h"
#include "Magnum/Trade/visibility.h"

namespace Magnum { namespace Trade {

/**
@brief Data flag
@m_since_latest

@see @ref DataFlags, @ref AnimationData::dataFlags(),
    @ref ImageData::dataFlags(), @ref MeshData::indexDataFlags(),
    @ref MeshData::vertexDataFlags()
*/
enum class DataFlag: UnsignedByte {
    /**
     * Data is owned by the instance. If this flag is not set, the instance
     * might be for example referencing a memory-mapped file or a constant
     * memory.
     */
    Owned = 1 << 0,

    /**
     * Data is mutable. If this flag is not set, the instance might be for
     * example referencing a readonly memory-mapped file or a constant memory.
     */
    Mutable = 2 << 0

    /** @todo owned by importer, owned by the GPU, ... */
};

/**
@debugoperatorenum{DataFlag}
@m_since_latest
*/
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, DataFlag value);

/**
@brief Data flags
@m_since_latest

@see @ref AnimationData::dataFlags(), @ref ImageData::dataFlags(),
    @ref MeshData::indexDataFlags(), @ref MeshData::vertexDataFlags()
*/
typedef Containers::EnumSet<DataFlag> DataFlags;

CORRADE_ENUMSET_OPERATORS(DataFlags)

/**
@debugoperatorenum{DataFlags}
@m_since_latest
*/
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, DataFlags value);

/**
@brief Data chunk type
@m_since_latest

A [FourCC](https://en.wikipedia.org/wiki/FourCC)-like identifier of the data
contained in the chunk. All identifiers starting with an uppercase leter are
reserved for Magnum itself, custom application-specific data types should use a
lowercase letter instead. Casing of the three remaining characters doesn't have
any specified effect in the current version of the header, and doesn't need to
be alphanumeric either (for example, Magnum's own types use the last character
to specify a version of given data type).
*/
enum class DataChunkType: UnsignedInt {
    Mesh = Utility::Endianness::fourCC('M', 's', 'h', 0),

    #if 0
    /* None of these used yet, here just to layout the naming scheme */
    Animation = Utility::Endianness::fourCC('A', 'n', 'i', 0),
    Camera = Utility::Endianness::fourCC('C', 'a', 'm', 0),
    Image1D = Utility::Endianness::fourCC('I', 'm', '1', 0),
    Image2D = Utility::Endianness::fourCC('I', 'm', '2', 0),
    Image3D = Utility::Endianness::fourCC('I', 'm', '3', 0),
    Light = Utility::Endianness::fourCC('L', 'g', 't', 0),
    Material = Utility::Endianness::fourCC('M', 't', 'l', 0),
    Scene = Utility::Endianness::fourCC('S', 'c', 'n', 0),
    Texture = Utility::Endianness::fourCC('T', 'e', 'x', 0)
    #endif
};

/**
@debugoperatorenum{DataChunkType}
@m_since_latest
*/
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, DataChunkType value);

/**
@brief Data chunk signature
@m_since_latest

Reads as `BLOB` letters for a Little-Endian 64 bit data chunk. For Big-Endian
the order is reversed (thus `BOLB`), 32-bit data have the `L` letter lowercase.
@see @ref DataChunkHeader::signature
*/
enum class DataChunkSignature: UnsignedInt {
    /** Little-Endian 32-bit data. The letters `BlOB`. */
    LittleEndian32 = Utility::Endianness::fourCC('B', 'l', 'O', 'B'),

    /** Little-Endian 64-bit data. The letters `BLOB`. */
    LittleEndian64 = Utility::Endianness::fourCC('B', 'L', 'O', 'B'),

    /** Big-Endian 32-bit data. The letters `BOlB`. */
    BigEndian32 = Utility::Endianness::fourCC('B', 'O', 'l', 'B'),

    /** Big-Endian 64-bit data. The letters `BOLB`. */
    BigEndian64 = Utility::Endianness::fourCC('B', 'O', 'L', 'B'),

    /** Signature matching this platform. Alias to one of the above. */
    Current
        #ifndef DOXYGEN_GENERATING_OUTPUT
        =
        #ifndef CORRADE_TARGET_BIG_ENDIAN
        sizeof(std::size_t) == 8 ? LittleEndian64 : LittleEndian32
        #else
        sizeof(std::size_t) == 8 ? BigEndian64 : BigEndian32
        #endif
        #endif
};

/**
@debugoperatorenum{DataChunkSignature}
@m_since_latest

@see @ref DataChunkHeader::signature
*/
MAGNUM_TRADE_EXPORT Debug& operator<<(Debug& debug, DataChunkSignature value);

/**
@brief Data chunk header
@m_since_latest

Header used by all serializable data chunks. Since the goal of the
serialization format is to be a direct equivalent to the in-memory data layout,
there's four different variants of the header based on whether it's running on
a 32-bit or 64-bit system and whether the machine is Little- or Big-Endian.
A 64-bit variant of the header has 24 bytes to support data larger than 4 GB,
the 32-bit variant is 20 bytes. Apart from the @ref size member, the header is
designed to contain the same amount of information on both, and its size is
chosen so the immediately following data can be aligned to either 4 or 8 bytes
without needing to add extra padding.

Similarly to [RIFF](https://en.wikipedia.org/wiki/Resource_Interchange_File_Format),
one of the main goals of the format is to have an ability to combine arbitrary
chunks together in the most trivial way possible as well as extracting them
back. To achieve that, each chunk header stores a @ref type and @ref size,
allowing application to pick chunks that they're interested in and reliably
skip the others. Compared to RIFF the file doesn't have any "global" chunk in
order to make file concatenation possible:

@code{.sh}
cat chair.blob table.blob > furniture.blob
@endcode

For live (non-deserialized) data, the header is all zeros. For deserialized
data, its contents are as follows; vaguely inspired by the
[PNG file header](https://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header).

Byte offset | Byte size | Contents
----------- | --------- | -----------------------------------------------------
0           | 1         | Header version. Has the high bit set to prevent the file from being detected as text. Currently set to @cpp 128 @ce.
1           | 1         | Unix EOL (LF, @cpp '\x0a' @ce), to detect unwanted Unix-to-DOS line ending conversion
2           | 2         | DOS EOL (CR+LF, @cpp '\x0d', '\x0a' @ce), to detect unwanted DOS-to-Unix line ending conversion
4           | 4         | File signature. Differs based on bitness and endianness, see @ref DataChunkSignature for more information
8           | 2         | Two zero bytes (@cpp '\x00', '\x00' @ce), to prevent the data from being treated and copied as a null-terminated (wide) string
10          | 2         | Extra data. Not used by the header itself, meant to be used by a particular data type to store various flags.
12          | 4         | Data chunk type, see @ref DataChunkType for more information
16          | 4 or 8    | Data chunk size, including the header size. Stored in endianness and size matching the signature field.

For a particular header version the first 10 bytes is static and thus can be
used for file validation. After the header are directly the chunk data. For performance reasons it's recommended to have the data padded to be a multiple
of 4 or 8 bytes to ensure the immediately following chunk is correctly aligned
as well, but it's not a strict recommendation.

Current version of the header doesn't have any checksum field in order to make
it easy to modify the data in-place, but this might change in the future.
@see @ref DataChunk
*/
struct DataChunkHeader {
    UnsignedByte version;           /**< @brief Header version */
    char eolUnix[1];                /**< @brief Unix EOL */
    char eolDos[2];                 /**< @brief Dos EOL */
    DataChunkSignature signature;   /**< @brief Signature */
    UnsignedShort zero;             /**< @brief Two zero bytes */
    UnsignedShort extra;            /**< @brief Extra data */
    DataChunkType type;             /**< @brief Chunk type */
    std::size_t size;               /**< @brief Chunk size */
};

/**
@brief Base for serializable chunked data
@m_since_latest

Wraps @ref DataChunkHeader and provides base interface for serialization and
deserialization. See @ref DataChunkHeader for a detailed description of the
chunk format.
*/
class MAGNUM_TRADE_EXPORT DataChunk {
    public:
        /**
         * @brief Check if given data blob is a valid chunk
         *
         * Returns @cpp true @ce if the chunk header is valid, matches current
         * platform and @p data is large enough to contain the whole chunk,
         * @cpp false @ce otherwise. The function doesn't print any diagnostic
         * messages on validation failure, use @ref deserialize() instead if
         * you need to know why.
         */
        static bool isDataChunk(Containers::ArrayView<const void> data);

        /**
         * @brief Try to deserialize from a memory-mappable representation
         *
         * Checks that @p data is large enough to contain a valid chunk,
         * validates the header and then returns @p data reinterpreted as a
         * @ref DataChunk pointer. On failure prints an error message and
         * returns @cpp nullptr @ce. The caller is expected to explicitly check
         * the return value, see @ref from() for an alternative.
         * @see @ref isDataChunk(), @ref serializeHeaderInto()
         */
        static const DataChunk* deserialize(Containers::ArrayView<const void> data);

        /** @overload */
        static DataChunk* deserialize(Containers::ArrayView<void> data);

        /**
         * @brief Deserialize from a memory-mappable representation
         *
         * Same as @ref deserialize(), but the function asserts if @p data is
         * not a valid chunk. Use when you don't need / want to explicitly
         * handle validation errors.
         * @see @ref isDataChunk(), @ref serializeHeaderInto()
         */
        static const DataChunk& from(Containers::ArrayView<const void> data);

        /** @overload */
        static DataChunk& from(Containers::ArrayView<void> data);

        /**
         * @brief Constructor
         *
         * When called, initializes the header to all zeros, except the @p type
         * field (which is reused later by @ref serializeHeaderInto()). This
         * turns it into an invalid chunk --- the designed way to access
         * serialized chunked data is by reinterpreting an existing block
         * of memory using @ref deserialize() or @ref from().
         */
        constexpr explicit DataChunk(DataChunkType type): _header{0, {0}, {0, 0}, DataChunkSignature{}, 0, 0, type, 0} {}

        /**
         * @brief Check if this instance is a valid chunk header
         *
         * Compared to @ref isDataChunk(Containers::ArrayView<const void>)
         * doesn't check that the actual chunk data are valid, only if the
         * header is.
         */
        bool isDataChunkHeader() const;

        /**
         * @brief Data chunk type
         *
         * For a live instance returns the type specified in constructor, for a
         * chunk deserialized using @ref deserialize() or @ref from() returns
         * the specified type.
         */
        constexpr DataChunkType dataChunkType() const { return _header.type; }

        /**
         * @brief Data chunk header
         *
         * @see @ref isDataChunk(), @ref isDataChunkHeader(),
         *      @ref dataChunkType()
         */
        constexpr const DataChunkHeader& dataChunkHeader() const { return _header; }

//         /**
//          * @brief Get next data chunk
//          * @param data      The full range of the data
//          *
//          * Expects the @cpp this @ce pointer is contained in @p data and the
//          * data chunk is valid. Returns a pointer to the immediately following
//          * data chunk or @cpp nullptr @ce if there's none. Note that the
//          * function doesn't check for validity of the next data chunk.
//          * @see @ref validateDataChunk()
//          */
//         // TODO: no this should fail when the next is not valid... but what to
//         // return? or return data.end() when there's no next? but void has no
//         // end :(
//         const DataChunk* nextDataChunk(Containers::ArrayView<const void> data) const;
//
//         /** @overload */
//         DataChunk* nextDataChunk(Containers::ArrayView<void> data) const;

        /**
         * @brief Size of serialized data
         *
         * Equivalent to size of the @ref DataChunkHeader class.
         */
        constexpr std::size_t serializedSize() const {
            return sizeof(DataChunkHeader);
        }

        /**
         * @brief Serialize a data chunk header into an existing array
         * @param[out] out      Where to write the output
         * @return  Number of bytes written. Same as @ref serializedSize().
         *
         * Expects that @p data is at least @ref serializedSize(). Fills in
         *
         * -    @ref DataChunkHeader::type with the value used in constructor,
         * -    @ref DataChunkHeader::extra with @p extra, and
         * -    @ref DataChunkHeader::size with @p data size.
         *
         * @see @ref deserialize()
         */
        std::size_t serializeHeaderInto(Containers::ArrayView<char> out, UnsignedShort extra = 0) const;

    private:
        DataChunkHeader _header;
};

namespace Implementation {
    /* Used internally by MeshData */
    MAGNUM_TRADE_EXPORT void nonOwnedArrayDeleter(char*, std::size_t);
}

}}

#endif
