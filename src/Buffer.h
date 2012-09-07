#ifndef Magnum_Buffer_h
#define Magnum_Buffer_h
/*
    Copyright © 2010, 2011, 2012 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Magnum.

    Magnum is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Magnum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Magnum::Buffer
 */

#include <vector>

#include "Magnum.h"

namespace Magnum {

/**
@brief Class for managing buffers

@todo Support for buffer copying (OpenGL 3.1, @extension{ARB,copy_buffer})
@todo Support for AMD's query buffer (@extension{AMD,query_buffer_object})
 */
class Buffer {
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) = delete;
    Buffer& operator=(const Buffer& other) = delete;
    Buffer& operator=(Buffer&& other) = delete;

    public:
        /** @brief %Buffer target */
        enum class Target: GLenum {
            /** Used for storing vertex attributes. */
            Array = GL_ARRAY_BUFFER,

            #ifndef MAGNUM_TARGET_GLES
            /**
             * Source for copies.
             * @requires_gl
             * @requires_gl31 Extension @extension{ARB,copy_buffer}
             */
            CopyRead = GL_COPY_READ_BUFFER,

            /**
             * Target for copies.
             * @requires_gl
             * @requires_gl31 Extension @extension{ARB,copy_buffer}
             */
            CopyWrite = GL_COPY_WRITE_BUFFER,
            #endif

            /** Used for storing vertex indices. */
            ElementArray = GL_ELEMENT_ARRAY_BUFFER

            #ifndef MAGNUM_TARGET_GLES
            ,

            /**
             * Source for texture update operations.
             * @requires_gl
             */
            PixelUnpack = GL_PIXEL_UNPACK_BUFFER,

            /**
             * Target for pixel pack operations.
             * @requires_gl
             */
            PixelPack = GL_PIXEL_PACK_BUFFER,

            /**
             * Source for texel fetches.
             *
             * @see BufferedTexture
             * @requires_gl
             * @requires_gl31 Extension @extension{ARB,texture_buffer_object}
             */
            Texture = GL_TEXTURE_BUFFER,

            /**
             * Target for transform feedback.
             * @requires_gl
             * @requires_gl30 Extension @extension{EXT,transform_feedback}
             */
            TransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,

            /**
             * Used for storing uniforms.
             * @requires_gl
             * @requires_gl31 Extension @extension{ARB,uniform_buffer_object}
             */
            Uniform = GL_UNIFORM_BUFFER,

            /**
             * Used for supplying arguments for instanced drawing.
             * @requires_gl
             * @requires_gl40 Extension @extension{ARB,draw_indirect}
             */
            DrawIndirect = GL_DRAW_INDIRECT_BUFFER
            #endif
        };

        /** @brief %Buffer usage */
        enum class Usage: GLenum {
            /**
             * Set once by the application and used infrequently for drawing.
             */
            StreamDraw = GL_STREAM_DRAW,

            #ifndef MAGNUM_TARGET_GLES
            /**
             * Set once as output from an OpenGL command and used infequently
             * for drawing.
             * @requires_gl
             */
            StreamRead = GL_STREAM_READ,

            /**
             * Set once as output from an OpenGL command and used infrequently
             * for drawing or copying to other buffers.
             * @requires_gl
             */
            StreamCopy = GL_STREAM_COPY,
            #endif

            /**
             * Set once by the application and used frequently for drawing.
             */
            StaticDraw = GL_STATIC_DRAW,

            #ifndef MAGNUM_TARGET_GLES
            /**
             * Set once as output from an OpenGL command and queried many
             * times by the application.
             * @requires_gl
             */
            StaticRead = GL_STATIC_READ,

            /**
             * Set once as output from an OpenGL command and used frequently
             * for drawing or copying to other buffers.
             * @requires_gl
             */
            StaticCopy = GL_STATIC_COPY,
            #endif

            /**
             * Updated frequently by the application and used frequently
             * for drawing or copying to other images.
             */
            DynamicDraw = GL_DYNAMIC_DRAW

            #ifndef MAGNUM_TARGET_GLES
            ,

            /**
             * Updated frequently as output from OpenGL command and queried
             * many times from the application.
             * @requires_gl
             */
            DynamicRead = GL_DYNAMIC_READ,

            /**
             * Updated frequently as output from OpenGL command and used
             * frequently for drawing or copying to other images.
             * @requires_gl
             */
            DynamicCopy = GL_DYNAMIC_COPY
            #endif
        };

        /**
         * @brief Unbind any buffer from given target
         * @param target     %Target
         *
         * @see @fn_gl{BindBuffer}
         */
        inline static void unbind(Target target) {
            glBindBuffer(static_cast<GLenum>(target), 0);
        }

        /**
         * @brief Constructor
         * @param defaultTarget Default target (used when calling bind()
         *      without parameter)
         *
         * Generates new OpenGL buffer.
         * @see @fn_gl{GenBuffers}
         */
        inline Buffer(Target defaultTarget): _defaultTarget(defaultTarget) {
            glGenBuffers(1, &buffer);
        }

        /**
         * @brief Destructor
         *
         * Deletes associated OpenGL buffer.
         * @see @fn_gl{DeleteBuffers}
         */
        inline virtual ~Buffer() {
            glDeleteBuffers(1, &buffer);
        }

        /** @brief Default bind type */
        inline Target defaultTarget() const { return _defaultTarget; }

        /** @brief OpenGL internal buffer ID */
        inline GLuint id() const { return buffer; }

        /**
         * @brief Bind buffer
         *
         * Binds buffer with default target.
         * @see bind(Target)
         */
        inline void bind() { bind(_defaultTarget); }

        /**
         * @brief Bind buffer
         * @param target     %Target
         *
         * @see @fn_gl{BindBuffer}
         */
        inline void bind(Target target) {
            glBindBuffer(static_cast<GLenum>(target), buffer);
        }

        /**
         * @brief Set buffer data
         * @param size      Data size
         * @param data      Pointer to data
         * @param usage     %Buffer usage
         *
         * Sets buffer data with default target.
         * @see setData(Target, GLsizeiptr, const GLvoid*, Usage)
         */
        inline void setData(GLsizeiptr size, const GLvoid* data, Usage usage) {
            setData(_defaultTarget, size, data, usage);
        }

        /**
         * @brief Set buffer data
         * @param data      Fixed-size array with data
         * @param usage     %Buffer usage
         *
         * Sets buffer data with default target. More convenient for setting
         * data from fixed-size arrays than
         * setData(GLsizeiptr, const GLvoid*, Usage).
         * @see setData(Target, GLsizeiptr, const GLvoid*, Usage)
         */
        template<size_t size, class T> inline void setData(const T(&data)[size], Usage usage) {
            setData(_defaultTarget, data, usage);
        }

        /**
         * @brief Set buffer data
         * @param data      Vector with data
         * @param usage     %Buffer usage
         *
         * Sets buffer data with default target.
         * @see setData(Target, GLsizeiptr, const GLvoid*, Usage)
         */
        template<class T> inline void setData(const std::vector<T>& data, Usage usage) {
            setData(_defaultTarget, data, usage);
        }

        /**
         * @brief Set buffer data
         * @param target    %Target
         * @param size      Data size
         * @param data      Pointer to data
         * @param usage     %Buffer usage
         *
         * @see bind(Target), @fn_gl{BufferData}
         */
        inline void setData(Target target, GLsizeiptr size, const GLvoid* data, Usage usage) {
            bind(target);
            glBufferData(static_cast<GLenum>(target), size, data, static_cast<GLenum>(usage));
        }

        /**
         * @brief Set buffer data
         * @param target    %Target
         * @param data      Fixed-size array with data
         * @param usage     %Buffer usage
         *
         * More convenient for setting data from fixed-size arrays than
         * setData(Target, GLsizeiptr, const GLvoid*, Usage).
         *
         * @see setData(Target, GLsizeiptr, const GLvoid*, Usage)
         */
        template<size_t size, class T> inline void setData(Target target, const T(&data)[size], Usage usage) {
            setData(target, size*sizeof(T), data, usage);
        }

        /**
         * @brief Set buffer data
         * @param target    %Target
         * @param data      Vector with data
         * @param usage     %Buffer usage
         *
         * @see setData(Target, GLsizeiptr, const GLvoid*, Usage)
         */
        template<class T> inline void setData(Target target, const std::vector<T>& data, Usage usage) {
            setData(target, data.size()*sizeof(T), data.data(), usage);
        }

        /**
         * @brief Set buffer subdata
         * @param offset    Offset
         * @param size      Data size
         * @param data      Pointer to data
         *
         * Sets buffer subdata with default target.
         * @see setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*)
         */
        inline void setSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data) {
            setSubData(_defaultTarget, offset, size, data);
        }

        /**
         * @brief Set buffer subdata
         * @param offset    Offset
         * @param data      Fixed-size array with data
         *
         * Sets buffer subdata with default target. More convenient for
         * setting data from fixed-size arrays than
         * setSubData(GLintptr, GLsizeiptr, const GLvoid*).
         * @see setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*)
         */
        template<size_t size, class T> inline void setSubData(GLintptr offset, const T(&data)[size]) {
            setSubData(_defaultTarget, offset, data);
        }

        /**
         * @brief Set buffer subdata
         * @param offset    Offset
         * @param data      Vector with data
         *
         * Sets buffer subdata with default target.
         * @see setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*)
         */
        template<class T> inline void setSubData(GLintptr offset, const std::vector<T>& data) {
            setSubData(_defaultTarget, offset, data);
        }

        /**
         * @brief Set buffer subdata
         * @param target    %Target
         * @param offset    Offset
         * @param size      Data size
         * @param data      Pointer to data
         *
         * @see bind(Target), @fn_gl{BufferSubData}
         */
        inline void setSubData(Target target, GLintptr offset, GLsizeiptr size, const GLvoid* data) {
            bind(target);
            glBufferSubData(static_cast<GLenum>(target), offset, size, data);
        }

        /**
         * @brief Set buffer subdata
         * @param target    %Target
         * @param offset    Offset
         * @param data      Fixed-size array with data
         *
         * More convenient for setting data from fixed-size arrays than
         * setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*).
         * @see setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*)
         */
        template<size_t size, class T> inline void setSubData(Target target, GLintptr offset, const T(&data)[size]) {
            setSubData(target, offset, size*sizeof(T), data);
        }

        /**
         * @brief Set buffer subdata
         * @param target    %Target
         * @param offset    Offset
         * @param data      Vector with data
         *
         * @see setSubData(Target, GLintptr, GLsizeiptr, const GLvoid*)
         */
        template<class T> inline void setSubData(Target target, GLintptr offset, const std::vector<T>& data) {
            setSubData(target, offset, data.size()*sizeof(T), data.data());
        }

    private:
        GLuint buffer;
        Target _defaultTarget;
};

}

#endif
