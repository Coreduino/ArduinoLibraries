//*****************************************************************************
//
//   SD Library
//   Core SD Module                                                      SD.cpp
//
//*****************************************************************************
/*
 *   The SD library allows for reading from and writing to SD cards, e.g. on 
 *   the Arduino Ethernet or Data Logger Shields.
 *
 *   Library features:
 *
 *        Support for FAT16 and FAT32 file systems.
 *        Supports standard SD and SDHC cards.
 *        Supports short file names only (8.3 - Twelve ASCII characters).
 *        Supports paths separated by forward-slashes (/).
 *        Folder names must not exceed 12 characters.
 *        The working directory is always the root of the SD card.
 *        Supports opening multiple files.
 *
 */

//*****************************************************************************
//                                                             Copyright Claims
//*****************************************************************************
/*
 *   License: GNU General Public License V3 (Because sdfatlib is licensed 
 *   with this.)
 *
 *   (C) Copyright 2010 SparkFun Electronics
 */

//*****************************************************************************
//                                                                      Remarks
//*****************************************************************************
/*
 *   This library aims to expose a subset of SD card functionality in the form
 *   of a higher level "wrapper" object.
 *
 *   This library provides four key benefits:
 *
 *        *    Including 'SD.h' automatically creates a global 'SD' object 
 *             which can be interacted with in a similar manner to other 
 *             standard global objects like 'Serial' and 'Ethernet'.
 *
 *        *    Boilerplate initialisation code is contained in one method 
 *             named 'begin' and no further objects need to be created in 
 *             order to access the SD card.
 *
 *        *    Calls to 'open' can supply a full path name including parent 
 *             directories which simplifies interacting with files in 
 *             subdirectories.
 *
 *        *    Utility methods are provided to determine whether a file 
 *             exists and to create a directory heirarchy.
 *
 *   Note that not all functionality provided by the underlying "sdfatlib" 
 *   library is exposed.
 */

//*****************************************************************************
//                                                         Implementation Notes
//*****************************************************************************
/*
 *   In order to handle multi-directory path traversal, functionality that 
 *   requires this ability is implemented as callback functions.
 *
 *   Individual methods call the 'walkPath' function which performs the actual
 *   directory traversal (swapping between two different directory/file handles
 *   along the way) and at each level calls the supplied callback function.
 *
 *   Some types of functionality will take an action at each level (e.g. exists
 *   or make directory) which others will only take an action at the bottom
 *   level (e.g. open).
 */

//*****************************************************************************
//                                                                  Development
//*****************************************************************************
/*
 *   24-11-10  Created.
 *   09-04-12  Modified by Tom Igoe.
 *   03-08-13  Started to upgrade source code. Programify Ltd.
 */

//-----------------------------------------------------------------------------
//                                        C O M P I L E R   D I R E C T I V E S
//-----------------------------------------------------------------------------
#include "SD.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - -  Compile-Time Options

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - -  Manifest Constants
/*
 *   MAXLEN_COMPONENT defines the maximum length of any component (file and 
 *   folder names) of an SD card's file specification. Changing this
 */
#define   MAXLEN_COMPONENT   12

//-----------------------------------------------------------------------------
//                                                          S T R U C T U R E S
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                                            F U N C T I O N S
//-----------------------------------------------------------------------------
//        getNextPathComponent()

//-----------------------------------------------------------------------------
//                                                        G L O B A L   D A T A
//-----------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - - - Composite Data Structures


//-----------------------------------------------------------------------------
//                                                         getNextPathComponent
//-----------------------------------------------------------------------------
/*
 *   getNextPathComponent() extracts the next path element into a target 
 *   string. Using repeated calls, a file specification can be split into its
 *   component parts.
 *
 *   Call with:
 *
 *        cpPath         Points to a file name, pathway name or combination of
 *                       path name(s) and file name.
 *
 *        ipOffset       Points to an integer containing an offset at which the
 *                       previous path component finished. If this pointer is
 *                       NULL, then zero is assumed. If a pointer is supplied,
 *                       it will be returned containing the offset to where
 *                       the extracted element terminates within 'cpPath'.
 *
 *                       When getNextPathComponent() is called for the first
 *                       time, 'ipOffset' must be correctly initialized to a
 *                       value in the range 0 to the length of 'cpPath'.
 *
 *        cpBuffer       Points to a string to receive the path component.
 *                       The target string should have a length of at least
 *                       MAXLEN_COMPONENT + 1 characters. A null terminator
 *                       will be appended to the returned string.
 *
 *   Returns a boolean flag to indicate if more components remain within the
 *   supplied 'cpPath' string.
 */
bool getNextPathComponent (IN char * cpPath, IN OUT unsigned int * ipOffset, OUT char * cpBuffer)
{
     char      cChar ;
     int       iIndex ;
     int       iOffset ;

// Init
     iIndex  = 0 ;
// Retrieve offset to next component
     if (ipOffset != NULL)
          iOffset = *ipOffset ;
     else
          iOffset = 0 ;
// Skip root or other path separator
     if (cpPath [iOffset] == '/')
          iOffset ++ ;
// Copy the next next path segment
     while (iIndex < MAXLEN_COMPONENT)
     {
     // Get next character in path string
          cChar = cpPath [iOffset] ;
     // Stop if end of string encountered
          if (cChar == 0x00)
               break ;
     // Stop if path divider encountered
          if (cChar == '/')
               break ;
     // Transcribe character from supplied path to target buffer
          cpBuffer [iIndex] = cChar ;
          iIndex  ++ ;
          iOffset ++ ;
     }
// Terminate string in buffer
     cpBuffer [iIndex] = 0x00 ;
// Skip trailing separator so we can determine if this is the last component in the path or not
     if (cpPath [iOffset] == '/')
          iOffset ++ ;
// Save offset to next component
     if (ipOffset != NULL)
         *ipOffset = iOffset ;
// Return TRUE if not yet reached end of inbound path string
     return (cpPath [iOffset] != 0x00) ;
}

//-----------------------------------------------------------------------------
//                                                                     walkPath
//-----------------------------------------------------------------------------
/*

When given a file path (and parent directory--normally root),
this function traverses the directories in the path and at each
level calls the supplied callback function while also providing
the supplied object for context if required.

e.g. given the path '/foo/bar/baz'
the callback would be called at the equivalent of
'/foo', '/foo/bar' and '/foo/bar/baz'.

The implementation swaps between two different directory/file
handles as it traverses the directories and does not use recursion
in an attempt to use memory efficiently.

If a callback wishes to stop the directory traversal it should
return false--in this case the function will stop the traversal,
tidy up and return false.

If a directory path doesn't exist at some point this function will
also return false and not subsequently call the callback.

If a directory path specified is complete, valid and the callback
did not indicate the traversal should be interrupted then this
function will return true.

*/
boolean walkPath (char * filepath, SdFile & parentDir, boolean (*callback)(SdFile & parentDir, char * filePathComponent, boolean isLastComponent, void * object), void * object = NULL)
{
     boolean   moreComponents ;
     boolean   shouldContinue ;
     char      buffer [MAXLEN_COMPONENT + 1] ;
     unsigned int   offset = 0 ;

     SdFile         subfile1 ;
     SdFile         subfile2 ;
     SdFile *       p_parent ;
     SdFile *       p_child ;
     SdFile *       p_tmp_sdfile ;

// Init
     p_child  = & subfile1 ;
     p_parent = & parentDir ;
// 
     while (true)
     {
          moreComponents = getNextPathComponent (filepath, & offset, buffer) ;
          shouldContinue = callback ((*p_parent), buffer, ! moreComponents, object) ;
          if (! shouldContinue)
          {
          /// TODO: Don't repeat this code? If it's one we've created then we don't need the parent handle anymore.
               if (p_parent != &parentDir)
                    (*p_parent).close () ;
               return false ;
          }
          if (! moreComponents)
               break ;
          boolean exists = (*p_child).open(*p_parent, buffer, O_RDONLY);

    // If it's one we've created then we
    // don't need the parent handle anymore.
          if (p_parent != &parentDir)
               (*p_parent).close();
    
    // Handle case when it doesn't exist and we can't continue...
          if (exists)
          {
          // We alternate between two file handles as we go down the path
               if (p_parent == &parentDir)
                    p_parent = &subfile2;
               p_tmp_sdfile = p_parent;
               p_parent = p_child;
               p_child = p_tmp_sdfile;
          }
          else
               return false;
     }
     if (p_parent != &parentDir)
     {
          (*p_parent).close(); // TODO: Return/ handle different?
     }
     return true ;
}

//-----------------------------------------------------------------------------
//                                                          callback_pathExists
//-----------------------------------------------------------------------------
  /*

    Callback used to determine if a file/directory exists in parent
    directory.

    Returns true if file path exists.

  */
boolean callback_pathExists (SdFile& parentDir, char *filePathComponent, boolean isLastComponent, void *object)
{
  SdFile child;

  boolean exists = child.open(parentDir, filePathComponent, O_RDONLY);
  
  if (exists) {
     child.close(); 
  }
  
  return exists;
}

//-----------------------------------------------------------------------------
//                                                         callback_makeDirPath
//-----------------------------------------------------------------------------
  /*

    Callback used to create a directory in the parent directory if
    it does not already exist.

    Returns true if a directory was created or it already existed.

  */
boolean callback_makeDirPath (SdFile& parentDir, char *filePathComponent, boolean isLastComponent, void *object)
{
  boolean result = false;
  SdFile child;
  
  result = callback_pathExists(parentDir, filePathComponent, isLastComponent, object);
  if (!result) {
    result = child.makeDir(parentDir, filePathComponent);
  } 
  
  return result;
}


//-----------------------------------------------------------------------------
//                                                              callback_remove
//-----------------------------------------------------------------------------
  /*

boolean callback_openPath(SdFile& parentDir, char *filePathComponent, 
			  boolean isLastComponent, void *object) {

    Callback used to open a file specified by a filepath that may
    specify one or more directories above it.

    Expects the context object to be an instance of 'SDClass' and
    will use the 'file' property of the instance to open the requested
    file/directory with the associated file open mode property.

    Always returns true if the directory traversal hasn't reached the
    bottom of the directory heirarchy.

    Returns false once the file has been opened--to prevent the traversal
    from descending further. (This may be unnecessary.)

  if (isLastComponent) {
    SDClass *p_SD = static_cast<SDClass*>(object);
    p_SD->file.open(parentDir, filePathComponent, p_SD->fileOpenMode);
    if (p_SD->fileOpenMode == FILE_WRITE) {
      p_SD->file.seekSet(p_SD->file.fileSize());
    }
    // TODO: Return file open result?
    return false;
  }
  return true;
}
  */
boolean callback_remove (SdFile& parentDir, char *filePathComponent, boolean isLastComponent, void *object)
{
  if (isLastComponent) {
    return SdFile::remove(parentDir, filePathComponent);
  }
  return true;
}

//-----------------------------------------------------------------------------
//                                                               callback_rmdir
//-----------------------------------------------------------------------------
boolean callback_rmdir (SdFile& parentDir, char *filePathComponent, boolean isLastComponent, void *object)
{
  if (isLastComponent) {
    SdFile f;
    if (!f.open(parentDir, filePathComponent, O_READ)) return false;
    return f.rmDir();
  }
  return true;
}

//=============================================================================
//                                                                       .begin
//-----------------------------------------------------------------------------
/* Implementation of class used to create 'SDCard' object. */
boolean SDClass::begin (uint8_t csPin)
{
  /*

    Performs the initialisation required by the sdfatlib library.

    Return true if initialization succeeds, false otherwise.

   */
  return card.init(SPI_HALF_SPEED, csPin) &&
         volume.init(card) &&
         root.openRoot(volume);
}

//=============================================================================
//                                                                .getParentDir
//-----------------------------------------------------------------------------
// this little helper is used to traverse paths
SdFile SDClass::getParentDir (const char *filepath, int *index) {
  // get parent directory
  SdFile d1 = root; // start with the mostparent, root!
  SdFile d2;

  // we'll use the pointers to swap between the two objects
  SdFile *parent = &d1;
  SdFile *subdir = &d2;
  
  const char *origpath = filepath;

  while (strchr(filepath, '/')) {

    // get rid of leading /'s
    if (filepath[0] == '/') {
      filepath++;
      continue;
    }
    
    if (! strchr(filepath, '/')) {
      // it was in the root directory, so leave now
      break;
    }

    // extract just the name of the next subdirectory
    uint8_t idx = strchr(filepath, '/') - filepath;
    if (idx > 12)
      idx = 12;    // dont let them specify long names
    char subdirname[13];
    strncpy(subdirname, filepath, idx);
    subdirname[idx] = 0;

    // close the subdir (we reuse them) if open
    subdir->close();
    if (! subdir->open(parent, subdirname, O_READ)) {
      // failed to open one of the subdirectories
      return SdFile();
    }
    // move forward to the next subdirectory
    filepath += idx;

    // we reuse the objects, close it.
    parent->close();

    // swap the pointers
    SdFile *t = parent;
    parent = subdir;
    subdir = t;
  }

    *index = (int)(filepath - origpath);
// parent is now the parent diretory of the file!
     return *parent;
}

//=============================================================================
//                                                                        .open
//-----------------------------------------------------------------------------
File SDClass::open(const char *filepath, uint8_t mode) {
  /*

     Open the supplied file path for reading or writing.

     The file content can be accessed via the 'file' property of
     the 'SDClass' object--this property is currently
     a standard 'SdFile' object from 'sdfatlib'.

     Defaults to read only.

     If 'write' is true, default action (when 'append' is true) is to
     append data to the end of the file.

     If 'append' is false then the file will be truncated first.

     If the file does not exist and it is opened for writing the file
     will be created.

     An attempt to open a file for reading that does not exist is an
     error.

   */

  int pathidx;

  // do the interative search
  SdFile parentdir = getParentDir(filepath, &pathidx);
  // no more subdirs!

  filepath += pathidx;

  if (! filepath[0]) {
    // it was the directory itself!
    return File(parentdir, "/");
  }

  // Open the file itself
  SdFile file;

  // failed to open a subdir!
  if (!parentdir.isOpen())
    return File();

  // there is a special case for the Root directory since its a static dir
  if (parentdir.isRoot()) {
    if ( ! file.open(SD.root, filepath, mode)) {
      // failed to open the file :(
      return File();
    }
    // dont close the root!
  } else {
    if ( ! file.open(parentdir, filepath, mode)) {
      return File();
    }
    // close the parent
    parentdir.close();
  }

  if (mode & (O_APPEND | O_WRITE)) 
    file.seekSet(file.fileSize());
  return File(file, filepath);
}

/*
File SDClass::open(char *filepath, uint8_t mode) {
  //

     Open the supplied file path for reading or writing.

     The file content can be accessed via the 'file' property of
     the 'SDClass' object--this property is currently
     a standard 'SdFile' object from 'sdfatlib'.

     Defaults to read only.

     If 'write' is true, default action (when 'append' is true) is to
     append data to the end of the file.

     If 'append' is false then the file will be truncated first.

     If the file does not exist and it is opened for writing the file
     will be created.

     An attempt to open a file for reading that does not exist is an
     error.

   //

  // TODO: Allow for read&write? (Possibly not, as it requires seek.)

  fileOpenMode = mode;
  walkPath(filepath, root, callback_openPath, this);

  return File();

}
*/


//boolean SDClass::close() {
//  /*
//
//    Closes the file opened by the 'open' method.
//
//   */
//  file.close();
//}


//=============================================================================
//                                                                      .exists
//-----------------------------------------------------------------------------
boolean SDClass::exists(char *filepath)
{
  /*

     Returns true if the supplied file path exists.

   */
  return walkPath(filepath, root, callback_pathExists);
}


//boolean SDClass::exists(char *filepath, SdFile& parentDir) {
//  /*
//
//     Returns true if the supplied file path rooted at 'parentDir'
//     exists.
//
//   */
//  return walkPath(filepath, parentDir, callback_pathExists);
//}


//=============================================================================
//                                                                       .mkdir
//-----------------------------------------------------------------------------
boolean SDClass::mkdir(char *filepath) {
  /*
  
    Makes a single directory or a heirarchy of directories.

    A rough equivalent to 'mkdir -p'.
  
   */
  return walkPath(filepath, root, callback_makeDirPath);
}

//=============================================================================
//                                                                       .rmdir
//-----------------------------------------------------------------------------
boolean SDClass::rmdir(char *filepath) {
  /*
  
    Makes a single directory or a heirarchy of directories.

    A rough equivalent to 'mkdir -p'.
  
   */
  return walkPath(filepath, root, callback_rmdir);
}

//=============================================================================
//                                                                      .remove
//-----------------------------------------------------------------------------
boolean SDClass::remove(char *filepath)
{
  return walkPath(filepath, root, callback_remove);
}


//=============================================================================
// File                                                           .openNextFile
//-----------------------------------------------------------------------------
// allows you to recurse into a directory
File File::openNextFile (uint8_t mode)
{
  dir_t p;

  //Serial.print("\t\treading dir...");
  while (_file->readDir(&p) > 0) {

    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) {
      //Serial.println("end");
      return File();
    }

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') {
      //Serial.println("dots");
      continue;
    }

    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(&p)) {
      //Serial.println("notafile");
      continue;
    }

    // print file name with possible blank fill
    SdFile f;
    char name[13];
    _file->dirName(p, name);
    //Serial.print("try to open file ");
    //Serial.println(name);

    if (f.open(_file, name, mode)) {
      //Serial.println("OK!");
      return File(f, name);    
    } else {
      //Serial.println("ugh");
      return File();
    }
  }

  //Serial.println("nothing");
  return File();
}

//=============================================================================
//                                                             .rewindDirectory
//-----------------------------------------------------------------------------
void File::rewindDirectory(void)
{
  if (isDirectory())
    _file->rewind();
}

//-----------------------------------------------------------------------------
SDClass SD ;
