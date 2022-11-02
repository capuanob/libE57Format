/*
 * ImageFile.cpp - implementation of public functions of the ImageFile class.
 *
 * Original work Copyright 2009 - 2010 Kevin Ackley (kackley@gwi.net)
 * Modified work Copyright 2018 - 2020 Andy Maloney <asmaloney@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

//! @file ImageFile.cpp

#include "ImageFileImpl.h"

using namespace e57;

/*!
@class e57::ImageFile
@brief   An ASTM E57 3D format file object.
@details
@section imagefile_ClassOverview Class overview
The ImageFile class represents the state of an ASTM E57 format data file.
An ImageFile may be created from an E57 file on the disk (read mode).
An new ImageFile may be created to write an E57 file to disk (write mode).

E57 files are organized in a tree structure.
Each ImageFile object has a predefined root node (of type StructureNode).
In a write mode ImageFile, the root node is initially empty.
In a read mode ImageFile, the root node is populated by the tree stored in the
.e57 file on disk.

@section imagefile_OpenClose The open/close state
An ImageFile object, opened in either mode (read/write), can be in one of two
states: open or closed. An ImageFile in the open state is ready to perform
transfers of data and to be interrogated. An ImageFile in the closed state
cannot perform any further transfers, and has very limited ability to be
interrogated. Note entering the closed state is different than destroying the
ImageFile object. An ImageFile object can still exist and be in the closed
state. When created, the ImageFile is initially open.

The ImageFile state can transition to the closed state in two ways.
The programmer can call ImageFile::close after all required processing has
completed. The programmer can call ImageFile::cancel if it is determined that
the ImageFile is no longer needed.

@section imagefile_Extensions Extensions

Basically in an E57 file, "extension = namespace + rules + meaning".
The "namespace" ensures that element names don't collide.
The "rules" may be written on paper, or partly codified in a computer grammar.
The "meaning" is a definition of what was measured, what the numbers in the file
mean.

Extensions are identified by URIs.
Extensions are not identified by prefixes.
Prefixes are a shorthand, used in a particular file, to make the element names
more palatable for humans. When thinking about a prefixed element name, in your
mind you should immediately substitute the URI for the prefix. For example,
think "http://www.example.com/DemoExtension:extra2" rather than "demo:extra2",
if the prefix "demo" is declared in the file to be a shorthand for the URI
"http://www.example.com/DemoExtension".

The rules are statements of: what is valid, what element names are possible,
what values are possible. The rules establish the answer to the following yes/no
question: "Is this extended E57 file valid?". The rules divide all possible
files into two sets: valid files and invalid files.

The "meanings" part of the above equation defines what the files in the first
set, the valid files, actually mean. This definition usually comes in the form
of documentation of the content of each new element in the format and how they
relate to the other elements.

An element name in an E57 file is a member of exactly one namespace (either the
default namespace defined in the ASTM standard, or an extension namespace).
Rules about the structure of an E57 extension (what element names can appear
where), are implicitly assumed only to govern the element names within the
namespace of the extension. Element names in other namespaces are unconstrained.
This is because a reader is required to ignore elements in namespaces that are
unfamiliar (to treat them as if they didn't exist). This enables a writer to
"tack on" new elements into pre-defined structures (e.g. structures defined in
the ASTM standard), without fear that it will confuse a reader that is only
familiar with the old format. This allows an extension designer to communicate
to two sets of readers: the old readers that will understand the information in
the old base format, and the new-fangled readers that will be able to read the
base format and the extra information stored in element names in the extended
namespace.

@section ImageFile_invariant Class Invariant
A class invariant is a list of statements about an object that are always true
before and after any operation on the object. An invariant is useful for testing
correct operation of an implementation. Statements in an invariant can involve
only externally visible state, or can refer to internal implementation-specific
state that is not visible to the API user. The following C++ code checks
externally visible state for consistency and throws an exception if the
invariant is violated:
@dontinclude ImageFile.cpp
@skip beginExample ImageFile::checkInvariant
@until endExample ImageFile::checkInvariant
*/

/*!
@brief   Open an ASTM E57 imaging data file for reading/writing.
@param   [in] fname File name to open.
Support of '\' as a directory separating character is system dependent.
For maximum portability, it is recommended that '/' be used as a directory
separator in file names. Special device file name support are implementation
dependent (e.g. "\\.\PhysicalDrive3" or
"/dev/hd3"). It is recommended that files that meet all of the requirements for
a legal ASTM E57 file format use the extension @c ".e57". It is recommended that
files that utilize the low-level E57 element data types, but do not have all the
required element names required by ASTM E57 file format standard use the file
extension @c "._e57".
@param   [in] mode Either "w" for writing or "r" for reading.
@param   [in] checksumPolicy The percentage of checksums we compute and verify
as an int. Clamped to 0-100.
@details

@par Write Mode
In write mode, the file cannot be already open.
A file with name given by @a fname is immediately created on the disk.
This file may grow as a result of operations on the ImageFile.
Which API functions write data to the file are implementation dependent.
Thus any API operation that stores data may fail as a result of insufficient
free disk space. Read API operations are legal for an ImageFile opened in write
mode.

@par Read Mode
Read mode files may be shared.
Write API operations are not legal for an ImageFile opened in read mode (i.e.
the ImageFile is read-only). There is no API support for appending data onto an
existing E57 data file.

@post    Resulting ImageFile is in @c open state if constructor succeeds (no
exception thrown).
@return  A smart ImageFile handle referencing the underlying object.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_OPEN_FAILED
@throw   ::E57_ERROR_LSEEK_FAILED
@throw   ::E57_ERROR_READ_FAILED
@throw   ::E57_ERROR_WRITE_FAILED
@throw   ::E57_ERROR_BAD_CHECKSUM
@throw   ::E57_ERROR_BAD_FILE_SIGNATURE
@throw   ::E57_ERROR_UNKNOWN_FILE_VERSION
@throw   ::E57_ERROR_BAD_FILE_LENGTH
@throw   ::E57_ERROR_XML_PARSER_INIT
@throw   ::E57_ERROR_XML_PARSER
@throw   ::E57_ERROR_BAD_XML_FORMAT
@throw   ::E57_ERROR_BAD_CONFIGURATION
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     IntegerNode, ScaledIntegerNode, FloatNode,
StringNode, BlobNode, StructureNode, VectorNode, CompressedVectorNode,
E57Exception, E57Utilities::E57Utilities
*/
ImageFile::ImageFile( const ustring &fname, const ustring &mode, ReadChecksumPolicy checksumPolicy ) :
   impl_( new ImageFileImpl( checksumPolicy ) )
{
   /// Do second phase of construction, now that ImageFile object is complete.
   impl_->construct2( fname, mode );
}

ImageFile::ImageFile( const char *input, const uint64_t size, ReadChecksumPolicy checksumPolicy ) :
   impl_( new ImageFileImpl( checksumPolicy ) )
{
   impl_->construct2( input, size );
}

/*!
@brief   Get the pre-established root StructureNode of the E57 ImageFile.
@details The root node of an ImageFile always exists and is always type
StructureNode. The root node is empty in a newly created write mode ImageFile.
@pre     This ImageFile must be open (i.e. isOpen()).
@return  A smart StructureNode handle referencing the underlying object.
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     StructureNode.
*/
StructureNode ImageFile::root() const
{
   return StructureNode( impl_->root() );
}

/*!
@brief   Complete any write operations on an ImageFile, and close the file on
the disk.
@details
Completes the writing of the state of the ImageFile to the disk.
Some API implementations may store significant portions of the state of the
ImageFile in memory. This state is moved into the disk file before it is closed.
Any errors in finishing the writing are reported by throwing an exception.
If an exception is thrown, depending on the error code, the ImageFile may enter
the closed state. If no exception is thrown, then the file on disk will be an
accurate representation of the ImageFile.

@b Warning: if the ImageFile::close function is not called, and the ImageFile
destructor is invoked with the ImageFile in the open state, the associated disk
file will be deleted and the ImageFile will @em not be saved to the disk (the
same outcome as calling ImageFile::cancel). The reason for this is that any
error conditions can't be reported from a destructor, so the user can't be
assured that the destruction/close completed successfully. It is strongly
recommended that this close function be called before the ImageFile is
destroyed.

It is not an error if ImageFile is already closed.
@post    ImageFile is in @c closed state.
@throw   ::E57_ERROR_LSEEK_FAILED
@throw   ::E57_ERROR_READ_FAILED
@throw   ::E57_ERROR_WRITE_FAILED
@throw   ::E57_ERROR_CLOSE_FAILED
@throw   ::E57_ERROR_BAD_CHECKSUM
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::cancel, ImageFile::isOpen
*/
void ImageFile::close()
{
   impl_->close();
}

/*!
@brief   Stop I/O operations and delete a partially written ImageFile on the
disk.
@details
If the ImageFile is write mode, the associated file on the disk is closed and
deleted, and the ImageFile goes to the closed state. If the ImageFile is read
mode, the behavior is same as calling ImageFile::close, but no exceptions are
thrown. It is not an error if ImageFile is already closed.
@post    ImageFile is in @c closed state.
@throw   No E57Exceptions.
@see     ImageFile::ImageFile, ImageFile::close, ImageFile::isOpen
*/
void ImageFile::cancel()
{
   impl_->cancel();
}

/*!
@brief   Test whether ImageFile is still open for accessing.
@post    No visible state is modified.
@return  true if ImageFile is in @c open state.
@throw   No E57Exceptions.
@see     ImageFile::ImageFile, ImageFile::close
*/
bool ImageFile::isOpen() const
{
   return impl_->isOpen();
}

/*!
@brief   Test whether ImageFile was opened in write mode.
@post    No visible state is modified.
@return  true if ImageFile was opened in write mode.
@throw   No E57Exceptions.
@see     ImageFile::ImageFile, ImageFile::isOpen
*/
bool ImageFile::isWritable() const
{
   return impl_->isWriter();
}

/*!
@brief   Get the file name the ImageFile was created with.
@post    No visible state is modified.
@return  The file name the ImageFile was created with.
@throw   No E57Exceptions.
@see     Cancel.cpp example, ImageFile::ImageFile
*/
ustring ImageFile::fileName() const
{
   return impl_->fileName();
}

/*!
@brief   Get current number of open CompressedVectorWriter objects writing to
ImageFile.
@details
CompressedVectorWriter objects that still exist, but are in the closed state
aren't counted. CompressedVectorWriter objects are created by the
CompressedVectorNode::writer function.
@pre     This ImageFile must be open (i.e. isOpen()).
@post    No visible state is modified.
@return  The current number of open CompressedVectorWriter objects writing to
ImageFile.
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     CompressedVectorNode::writer, CompressedVectorWriter
*/
int ImageFile::writerCount() const
{
   return impl_->writerCount();
}

/*!
@brief   Get current number of open CompressedVectorReader objects reading from
ImageFile.
@details
CompressedVectorReader objects that still exist, but are in the closed state
aren't counted. CompressedVectorReader objects are created by the
CompressedVectorNode::reader function.
@pre     This ImageFile must be open (i.e. isOpen()).
@post    No visible state is modified.
@return  The current number of open CompressedVectorReader objects reading from
ImageFile.
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     CompressedVectorNode::reader, CompressedVectorReader
*/
int ImageFile::readerCount() const
{
   return impl_->readerCount();
}

/*!
@brief   Declare the use of an E57 extension in an ImageFile being written.
@param   [in] prefix    The shorthand name of the extension to use in element
names.
@param   [in] uri       The Uniform Resource Identifier string to associate with
the prefix in the ImageFile.
@details
The (@a prefix, @a uri) pair is registered in the known extensions of the
ImageFile. Both @a prefix and @a uri must be unique in the ImageFile. It is not
legal to declare a URI associated with the default namespace (@a prefix = "").
It is not an error to declare a namespace and not use it in an element name.
It is an error to use a namespace prefix in an element name that is not declared
beforehand.

A writer is free to "hard code" the prefix names in the element name strings
that it uses (since it established the prefix declarations in the file). A
reader cannot assume that any given prefix is always mapped to the same URI or
vice versa. A reader might check an ImageFile, and if the prefixes aren't the
way it likes, the reader could give up.

A better scheme would be to lookup the URI that the reader is familiar with, and
store the prefix that the particular file uses in a variable. Then every time
the reader needs to form a prefixed element name, it can assemble the full
element name from the stored prefix variable and the constant documented base
name string. This is less convenient than using a single "hard coded" string
constant for an element name, but it is robust against any choice of prefix/URI
combination.

See the class discussion at bottom of ImageFile page for more details about
namespaces.
@pre     This ImageFile must be open (i.e. isOpen()).
@pre     ImageFile must have been opened in write mode (i.e. isWritable()).
@pre     prefix != ""
@pre     uri != ""
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_FILE_IS_READ_ONLY
@throw   ::E57_ERROR_DUPLICATE_NAMESPACE_PREFIX
@throw   ::E57_ERROR_DUPLICATE_NAMESPACE_URI
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsCount, ImageFile::extensionsLookupPrefix, ImageFile::extensionsLookupUri
*/
void ImageFile::extensionsAdd( const ustring &prefix, const ustring &uri )
{
   impl_->extensionsAdd( prefix, uri );
}

/*!
@brief   Look up an E57 extension prefix in the ImageFile.
@param   [in] prefix The shorthand name of the extension to look up.
@details
If @a prefix = "" or @a prefix is declared in the ImageFile, then the function returns true. It is an error if @a prefix
contains an illegal character combination for E57 namespace prefixes.
@pre     This ImageFile must be open (i.e. isOpen()).
@post    No visible state is modified.
@return  true if prefix is declared in the ImageFile.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsLookupUri
*/
bool ImageFile::extensionsLookupPrefix( const ustring &prefix ) const
{
   ustring uri;
   return impl_->extensionsLookupPrefix( prefix, uri );
}

/*!
@brief   Get URI associated with an E57 extension prefix in the ImageFile.
@param   [in] prefix    The shorthand name of the extension to look up.
@param   [out] uri      The URI that was associated with the given @a prefix.
@details
If @a prefix = "", then @a uri is set to the default namespace URI, and the
function returns true. if @a prefix is declared in the ImageFile, then @a uri is
set the corresponding URI, and the function returns true. It is an error if @a
prefix contains an illegal character combination for E57 namespace prefixes. It
is not an error if @a prefix is well-formed, but not defined in the ImageFile
(the function just returns false).
@pre     This ImageFile must be open (i.e. isOpen()).
@post    No visible state is modified.
@return  true if prefix is declared in the ImageFile.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsLookupUri
*/
bool ImageFile::extensionsLookupPrefix( const ustring &prefix, ustring &uri ) const
{
   return impl_->extensionsLookupPrefix( prefix, uri );
}

/*!
@brief   Get an E57 extension prefix associated with a URI in the ImageFile.
@param   [in] uri       The URI of the extension to look up.
@param   [out] prefix   The shorthand prefix that was associated with the given
@a uri.
@details
If @a uri is declared in the ImageFile, then @a prefix is set the corresponding
prefix, and the function returns true. It is an error if @a uri contains an
illegal character combination for E57 namespace URIs. It is not an error if @a
uri is well-formed, but not defined in the ImageFile (the function just returns
false).
@pre     This ImageFile must be open (i.e. isOpen()).
@pre     uri != ""
@post    No visible state is modified.
@return  true if URI is declared in the ImageFile.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsLookupPrefix
*/
bool ImageFile::extensionsLookupUri( const ustring &uri, ustring &prefix ) const
{
   return impl_->extensionsLookupUri( uri, prefix );
}

/*!
@brief   Get number of E57 extensions declared in the ImageFile.
@details
The default E57 namespace does not count as an extension.
@pre     This ImageFile must be open (i.e. isOpen()).
@post    No visible state is modified.
@return  The number of E57 extensions defined in the ImageFile.
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsPrefix, ImageFile::extensionsUri
*/
size_t ImageFile::extensionsCount() const
{
   return impl_->extensionsCount();
}

/*!
@brief   Get an E57 extension prefix declared in an ImageFile by index.
@param   [in] index The index of the prefix to get, starting at 0.
@details
The order that the prefixes are stored in is not necessarily the same as the
order they were created. However the prefix order will correspond to the URI
order. The default E57 namespace is not counted as an extension.
@pre     This ImageFile must be open (i.e. isOpen()).
@pre     0 <= index < extensionsCount()
@post    No visible state is modified.
@return  The E57 extension prefix at the given index.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsCount, ImageFile::extensionsUri
*/
ustring ImageFile::extensionsPrefix( const size_t index ) const
{
   return impl_->extensionsPrefix( index );
}

/*!
@brief   Get an E57 extension URI declared in an ImageFile by index.
@param   [in] index The index of the URI to get, starting at 0.
@details
The order that the URIs are stored is not necessarily the same as the order they
were created. However the URI order will correspond to the prefix order. The
default E57 namespace is not counted as an extension.
@pre     This ImageFile must be open (i.e. isOpen()).
@pre     0 <= index < extensionsCount()
@post    No visible state is modified.
@return  The E57 extension URI at the given index.
@throw   ::E57_ERROR_BAD_API_ARGUMENT
@throw   ::E57_ERROR_IMAGEFILE_NOT_OPEN
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::extensionsCount, ImageFile::extensionsPrefix
*/
ustring ImageFile::extensionsUri( const size_t index ) const
{
   return impl_->extensionsUri( index );
}

/*!
@brief   Test whether an E57 element name has an extension prefix.
@details
The element name has a prefix if the function
elementNameParse(elementName,prefix,dummy) would succeed, and returned prefix !=
"".
@param   [in] elementName   The string element name to test.
@post    No visible state is modified.
@return  True if the E57 element name has an extension prefix.
@throw   No E57Exceptions.
*/
bool ImageFile::isElementNameExtended( const ustring &elementName ) const
{
   return impl_->isElementNameExtended( elementName );
}

/*!
@brief   Parse element name into prefix and localPart substrings.
@param   [in] elementName   The string element name to parse into prefix and
local parts.
@param   [out] prefix       The prefix (if any) in the @a elementName.
@param   [out] localPart    The part of the element name after the prefix.
@details
A legal element name may be in prefixed (ID:ID) or unprefixed (ID) form,
where ID is a string whose first character is in {a-z,A-Z,_} followed by zero or
more characters in {a-z,A-Z,_,0-9,-,.}. If in prefixed form, the prefix does not
have to be declared in the ImageFile.
@post    No visible state is modified.
@throw   ::E57_ERROR_BAD_PATH_NAME
@throw   ::E57_ERROR_INTERNAL           All objects in undocumented state
@see     ImageFile::isElementNameExtended
*/
void ImageFile::elementNameParse( const ustring &elementName, ustring &prefix, ustring &localPart ) const
{
   impl_->elementNameParse( elementName, prefix, localPart );
}

/*!
@brief   Diagnostic function to print internal state of object to output stream
in an indented format.
@copydetails Node::dump()
*/
#ifdef E57_DEBUG
void ImageFile::dump( int indent, std::ostream &os ) const
{
   impl_->dump( indent, os );
}
#else
void ImageFile::dump( int indent, std::ostream &os ) const
{
}
#endif

/*!
@brief Check whether ImageFile class invariant is true
@param   [in] doRecurse   If true, also check invariants of all children or
sub-objects recursively.
@details
This function checks at least the assertions in the documented class invariant
description (see class reference page for this object). Other internal
invariants that are implementation-dependent may also be checked. If any
invariant clause is violated, an E57Exception with errorCode of
E57_ERROR_INVARIANCE_VIOLATION is thrown.

Checking the invariant recursively may be expensive if the tree is large, so
should be used judiciously, in debug versions of the application.
@post    No visible state is modified.
@throw   ::E57_ERROR_INVARIANCE_VIOLATION or any other E57 ErrorCode
@see     Node::checkInvariant
*/
// beginExample ImageFile::checkInvariant
void ImageFile::checkInvariant( bool doRecurse ) const
{
   // If this ImageFile is not open, can't test invariant (almost every call
   // would throw)
   if ( !isOpen() )
   {
      return;
   }

   // root() node must be a root node
   if ( !root().isRoot() )
   {
      throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
   }

   // Can't have empty fileName
   if ( fileName().empty() )
   {
      throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
   }

   int wCount = writerCount();
   int rCount = readerCount();

   // Can't have negative number of readers
   if ( rCount < 0 )
   {
      throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
   }

   // Can't have negative number of writers
   if ( wCount < 0 )
   {
      throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
   }

   // Can't have more than one writer
   if ( 1 < wCount )
   {
      throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
   }

   // If have writer
   if ( wCount > 0 )
   {
      // Must be in write-mode
      if ( !isWritable() )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }

      // Can't have any readers
      if ( rCount > 0 )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }
   }

   // Extension prefixes and URIs are unique
   const size_t eCount = extensionsCount();
   for ( size_t i = 0; i < eCount; i++ )
   {
      for ( size_t j = i + 1; j < eCount; j++ )
      {
         if ( extensionsPrefix( i ) == extensionsPrefix( j ) )
         {
            throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
         }
         if ( extensionsUri( i ) == extensionsUri( j ) )
         {
            throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
         }
      }
   }

   // Verify lookup functions are correct
   for ( size_t i = 0; i < eCount; i++ )
   {
      ustring goodPrefix = extensionsPrefix( i );
      ustring goodUri = extensionsUri( i );
      ustring prefix;
      ustring uri;
      if ( !extensionsLookupPrefix( goodPrefix, uri ) )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }
      if ( uri != goodUri )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }
      if ( !extensionsLookupUri( goodUri, prefix ) )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }
      if ( prefix != goodPrefix )
      {
         throw E57_EXCEPTION1( E57_ERROR_INVARIANCE_VIOLATION );
      }
   }

   // If requested, check all objects "below" this one
   if ( doRecurse )
   {
      root().checkInvariant( doRecurse );
   }
}
// endExample ImageFile::checkInvariant

/*!
@brief   Test if two ImageFile handles refer to the same underlying ImageFile
@param   [in] imf2        The ImageFile to compare this ImageFile with
@post    No visible object state is modified.
@return  @c true if ImageFile handles refer to the same underlying ImageFile.
@throw   No E57Exceptions
*/
bool ImageFile::operator==( ImageFile imf2 ) const
{
   return ( impl_ == imf2.impl_ );
}

/*!
@brief   Test if two ImageFile handles refer to different underlying ImageFile
@param   [in] imf2        The ImageFile to compare this ImageFile with
@post    No visible object state is modified.
@return  @c true if ImageFile handles refer to different underlying ImageFiles.
@throw   No E57Exceptions
*/
bool ImageFile::operator!=( ImageFile imf2 ) const
{
   return ( impl_ != imf2.impl_ );
}

//! @cond documentNonPublic   The following isn't part of the API, and isn't documented.
ImageFile::ImageFile( ImageFileImplSharedPtr imfi ) : impl_( imfi )
{
}
//! @endcond
