/*
 *
 *  Copyright (C) 2007-2009, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmdjpls
 *
 *  Author:  Martin Willkomm
 *
 *  Purpose: Decompress DICOM file with JPEG-LS transfer syntax
 *
 *  Last Update:      $Author: meichel $
 *  Update Date:      $Date: 2009-07-29 14:46:46 $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#ifdef HAVE_GUSI_H
#include <GUSI.h>
#endif

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"      /* for dcmtk version name */
#include "dcmtk/dcmimage/diregist.h"  /* include to support color images */
#include "dcmtk/dcmjpls/djlsutil.h"   /* for dcmjp2k/jpgls typedefs */
#include "dcmtk/dcmjpls/djdecode.h"   /* for JPEG-LS decoder */

#ifdef WITH_ZLIB
#include <zlib.h>      /* for zlibVersion() */
#endif

#ifdef USE_LICENSE_FILE
#include "oflice.h"
#endif

#ifndef OFFIS_CONSOLE_APPLICATION
#define OFFIS_CONSOLE_APPLICATION "dcmdjpls"
#endif

static char rcsid[] = "$dcmtk: " OFFIS_CONSOLE_APPLICATION " v"
  OFFIS_DCMTK_VERSION " " OFFIS_DCMTK_RELEASEDATE " $";

// ********************************************


#define SHORTCOL 3
#define LONGCOL 21

int main(int argc, char *argv[])
{

#ifdef HAVE_GUSI_H
  GUSISetup(GUSIwithSIOUXSockets);
  GUSISetup(GUSIwithInternetSockets);
#endif

  SetDebugLevel(( 0 ));

  const char *opt_ifname = NULL;
  const char *opt_ofname = NULL;

  int opt_debugMode = 0;
  OFBool opt_verbose = OFFalse;
  OFBool opt_oDataset = OFFalse;
  E_TransferSyntax opt_oxfer = EXS_LittleEndianExplicit;
  E_GrpLenEncoding opt_oglenc = EGL_recalcGL;
  E_EncodingType opt_oenctype = EET_ExplicitLength;
  E_PaddingEncoding opt_opadenc = EPD_noChange;
  OFCmdUnsignedInt opt_filepad = 0;
  OFCmdUnsignedInt opt_itempad = 0;
  E_FileReadMode opt_readMode = ERM_autoDetect;
  E_TransferSyntax opt_ixfer = EXS_Unknown;

  //Parameter
  JLS_UIDCreation opt_uidcreation = EJLSUC_default;
  JLS_PlanarConfiguration opt_planarconfig = EJLSPC_restore;
  OFBool opt_ignoreOffsetTable = OFFalse;

#ifdef USE_LICENSE_FILE
LICENSE_FILE_DECLARATIONS
#endif

  OFConsoleApplication app(OFFIS_CONSOLE_APPLICATION , "Decode JPEG-LS compressed DICOM file", rcsid);
  OFCommandLine cmd;
  cmd.setOptionColumns(LONGCOL, SHORTCOL);
  cmd.setParamColumn(LONGCOL + SHORTCOL + 4);

  cmd.addParam("dcmfile-in",  "DICOM input filename to be converted");
  cmd.addParam("dcmfile-out", "DICOM output filename");

  cmd.addGroup("general options:", LONGCOL, SHORTCOL + 2);
   cmd.addOption("--help",                      "-h",     "print this help text and exit", OFCommandLine::AF_Exclusive);
   cmd.addOption("--version",                             "print version information and exit", OFCommandLine::AF_Exclusive);
   cmd.addOption("--arguments",                           "print expanded command line arguments");
   cmd.addOption("--verbose",                   "-v",     "verbose mode, print processing details");
   cmd.addOption("--debug",                     "-d",     "debug mode, print debug information");

#ifdef USE_LICENSE_FILE
LICENSE_FILE_DECLARE_COMMAND_LINE_OPTIONS
#endif

   cmd.addGroup("input options:");
    cmd.addSubGroup("input file format:");
     cmd.addOption("--read-file",               "+f",     "read file format or data set (default)");
     cmd.addOption("--read-file-only",          "+fo",    "read file format only");
     cmd.addOption("--read-dataset",            "-f",     "read data set without file meta information");

  cmd.addGroup("processing options:");
    cmd.addSubGroup("planar configuration options:");
      cmd.addOption("--planar-restore",         "+pr",    "restore original planar configuration (default)");
      cmd.addOption("--planar-auto",            "+pa",    "automatically determine planar configuration\nfrom SOP class and color space");
      cmd.addOption("--color-by-pixel",         "+px",    "always store color-by-pixel");
      cmd.addOption("--color-by-plane",         "+pl",    "always store color-by-plane");

    cmd.addSubGroup("other processing options:");
      cmd.addOption("--ignore-offsettable",     "+io",    "ignore offset table when decompressing");

    cmd.addSubGroup("SOP Instance UID options:");
      cmd.addOption("--uid-default",            "+ud",    "keep same SOP Instance UID (default)");
      cmd.addOption("--uid-always",             "+ua",    "always assign new UID");

  cmd.addGroup("output options:");
    cmd.addSubGroup("output file format:");
      cmd.addOption("--write-file",             "+F",     "write file format (default)");
      cmd.addOption("--write-dataset",          "-F",     "write data set without file meta information");
    cmd.addSubGroup("output transfer syntax:");
      cmd.addOption("--write-xfer-little",      "+te",    "write with explicit VR little endian (default)");
      cmd.addOption("--write-xfer-big",         "+tb",    "write with explicit VR big endian TS");
      cmd.addOption("--write-xfer-implicit",    "+ti",    "write with implicit VR little endian TS");
    cmd.addSubGroup("post-1993 value representations:");
      cmd.addOption("--enable-new-vr",          "+u",     "enable support for new VRs (UN/UT) (default)");
      cmd.addOption("--disable-new-vr",         "-u",     "disable support for new VRs, convert to OB");
    cmd.addSubGroup("group length encoding:");
      cmd.addOption("--group-length-recalc",    "+g=",    "recalculate group lengths if present (default)");
      cmd.addOption("--group-length-create",    "+g",     "always write with group length elements");
      cmd.addOption("--group-length-remove",    "-g",     "always write without group length elements");
    cmd.addSubGroup("length encoding in sequences and items:");
      cmd.addOption("--length-explicit",        "+e",     "write with explicit lengths (default)");
      cmd.addOption("--length-undefined",       "-e",     "write with undefined lengths");
    cmd.addSubGroup("data set trailing padding (not with --write-dataset):");
      cmd.addOption("--padding-retain",         "-p=",    "do not change padding (default)");
      cmd.addOption("--padding-off",            "-p",     "no padding (implicit if --write-dataset)");
      cmd.addOption("--padding-create",         "+p",  2, "[f]ile-pad [i]tem-pad: integer",
                                                          "align file on multiple of f bytes\nand items on multiple of i bytes");

    /* evaluate command line */
    prepareCmdLineArgs(argc, argv, OFFIS_CONSOLE_APPLICATION);
    if (app.parseCommandLine(cmd, argc, argv, OFCommandLine::PF_ExpandWildcards))
    {
      /* check whether to print the command line arguments */
      if (cmd.findOption("--arguments"))
        app.printArguments();

      /* check exclusive options first */
      if (cmd.hasExclusiveOption())
      {
        if (cmd.findOption("--version"))
        {
          app.printHeader(OFTrue /*print host identifier*/);
          COUT << OFendl << "External libraries used:" << OFendl;
#ifdef WITH_ZLIB
          COUT << "- ZLIB, Version " << zlibVersion() << OFendl;
#endif
          COUT << "- " << "SPMG/JPEG-LS Implementation, Version 2.1" << OFendl;
          return 0;
        }
      }

      /* command line parameters */

      cmd.getParam(1, opt_ifname);
      cmd.getParam(2, opt_ofname);

      /* options */

      if (cmd.findOption("--verbose")) opt_verbose = OFTrue;
      if (cmd.findOption("--debug")) opt_debugMode = 5;

#ifdef USE_LICENSE_FILE
LICENSE_FILE_EVALUATE_COMMAND_LINE_OPTIONS
#endif

      cmd.beginOptionBlock();
      if (cmd.findOption("--planar-restore")) opt_planarconfig = EJLSPC_restore;
      if (cmd.findOption("--planar-auto")) opt_planarconfig = EJLSPC_auto;
      if (cmd.findOption("--color-by-pixel")) opt_planarconfig = EJLSPC_colorByPixel;
      if (cmd.findOption("--color-by-plane")) opt_planarconfig = EJLSPC_colorByPlane;
      cmd.endOptionBlock();

      if (cmd.findOption("--ignore-offsettable")) opt_ignoreOffsetTable = OFTrue;

      cmd.beginOptionBlock();
      if (cmd.findOption("--uid-default")) opt_uidcreation = EJLSUC_default;
      if (cmd.findOption("--uid-always")) opt_uidcreation = EJLSUC_always;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--read-file"))
      {
        opt_readMode = ERM_autoDetect;
        opt_ixfer = EXS_Unknown;
      }
      if (cmd.findOption("--read-file-only"))
      {
        opt_readMode = ERM_fileOnly;
        opt_ixfer = EXS_Unknown;
      }
      if (cmd.findOption("--read-dataset"))
      {
        opt_readMode = ERM_dataset;

        // This transfer syntax works as long as the content of encapsulated pixel
        // sequences is some kind of JPEG-LS bitstream. hmm?
        opt_ixfer = EXS_JPEGLSLossless;
      }
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-file")) opt_oDataset = OFFalse;
      if (cmd.findOption("--write-dataset")) opt_oDataset = OFTrue;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--write-xfer-little")) opt_oxfer = EXS_LittleEndianExplicit;
      if (cmd.findOption("--write-xfer-big")) opt_oxfer = EXS_BigEndianExplicit;
      if (cmd.findOption("--write-xfer-implicit")) opt_oxfer = EXS_LittleEndianImplicit;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--enable-new-vr"))
      {
        dcmEnableUnknownVRGeneration.set(OFTrue);
        dcmEnableUnlimitedTextVRGeneration.set(OFTrue);
      }
      if (cmd.findOption("--disable-new-vr"))
      {
        dcmEnableUnknownVRGeneration.set(OFFalse);
        dcmEnableUnlimitedTextVRGeneration.set(OFFalse);
      }
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--group-length-recalc")) opt_oglenc = EGL_recalcGL;
      if (cmd.findOption("--group-length-create")) opt_oglenc = EGL_withGL;
      if (cmd.findOption("--group-length-remove")) opt_oglenc = EGL_withoutGL;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--length-explicit")) opt_oenctype = EET_ExplicitLength;
      if (cmd.findOption("--length-undefined")) opt_oenctype = EET_UndefinedLength;
      cmd.endOptionBlock();

      cmd.beginOptionBlock();
      if (cmd.findOption("--padding-retain"))
      {
        if (opt_oDataset) app.printError("--padding-retain not allowed with --write-dataset");
        opt_opadenc = EPD_noChange;
      }
      if (cmd.findOption("--padding-off")) opt_opadenc = EPD_withoutPadding;
      if (cmd.findOption("--padding-create"))
      {
        if (opt_oDataset) app.printError("--padding-create not allowed with --write-dataset");
        app.checkValue(cmd.getValueAndCheckMin(opt_filepad, 0));
        app.checkValue(cmd.getValueAndCheckMin(opt_itempad, 0));
        opt_opadenc = EPD_withPadding;
      }
      cmd.endOptionBlock();
    }

    if (opt_debugMode)
        app.printIdentifier();
    SetDebugLevel((opt_debugMode));

    // register global decompression codecs
    DJLSDecoderRegistration::registerCodecs(opt_verbose, opt_uidcreation, opt_planarconfig, opt_ignoreOffsetTable);

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
      CERR << "Warning: no data dictionary loaded, "
           << "check environment variable: "
           << DCM_DICT_ENVIRONMENT_VARIABLE << OFendl;
    }

    // open input file
    if ((opt_ifname == NULL) || (strlen(opt_ifname) == 0))
    {
      CERR << "Error: invalid filename: <empty string>" << OFendl;
      return 1;
    }

    DcmFileFormat fileformat;

    if (opt_verbose)
      COUT << "reading input file " << opt_ifname << OFendl;

    OFCondition error = fileformat.loadFile(opt_ifname, opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode);
    if (error.bad())
    {
      CERR << "Error: " << error.text() << ": reading file: " <<  opt_ifname << OFendl;
      return 1;
    }

    DcmDataset *dataset = fileformat.getDataset();

    if (opt_verbose)
      COUT << "decompressing file" << OFendl;

    DcmXfer opt_oxferSyn(opt_oxfer);
    DcmXfer original_xfer(dataset->getOriginalXfer());

    error = dataset->chooseRepresentation(opt_oxfer, NULL);
    if (error.bad())
    {
      CERR << "Error: " << error.text() << ": decompressing file: " <<  opt_ifname << OFendl;
      if (error == EC_CannotChangeRepresentation)
        CERR << "Input transfer syntax " << original_xfer.getXferName() << " not supported" << OFendl;
      return 1;
    }

    if (! dataset->canWriteXfer(opt_oxfer))
    {
      CERR << "Error: no conversion to transfer syntax " << opt_oxferSyn.getXferName() << " possible" << OFendl;
      return 1;
    }

    if (opt_verbose)
      COUT << "creating output file " << opt_ofname << OFendl;

    fileformat.loadAllDataIntoMemory();
    error = fileformat.saveFile(opt_ofname, opt_oxfer, opt_oenctype, opt_oglenc,
      opt_opadenc, OFstatic_cast(Uint32, opt_filepad), OFstatic_cast(Uint32, opt_itempad), opt_oDataset);

    if (error != EC_Normal)
    {
      CERR << "Error: " << error.text() << ": writing file: " <<  opt_ofname << OFendl;
      return 1;
    }

    if (opt_verbose)
      COUT << "conversion successful" << OFendl;

    // deregister global decompression codecs
    DJLSDecoderRegistration::cleanup();

    return 0;
}


/*
 * CVS/RCS Log:
 * $Log: dcmdjpls.cc,v $
 * Revision 1.1  2009-07-29 14:46:46  meichel
 * Initial release of module dcmjpls, a JPEG-LS codec for DCMTK based on CharLS
 *
 * Revision 1.10  2009-03-19 12:15:25  joergr
 * Fixed wrong CVS log message.
 *
 * Revision 1.9  2009-03-19 12:14:22  joergr
 * Added more explicit message in case input transfer syntax is not supported.
 *
 * Revision 1.8  2008-09-25 15:38:27  joergr
 * Fixed outdated comment.
 *
 * Revision 1.7  2008-09-25 14:23:11  joergr
 * Moved output of resource identifier in order to avoid printing the same
 * information twice.
 *
 * Revision 1.6  2008-09-25 13:47:29  joergr
 * Added support for printing the expanded command line arguments.
 * Always output the resource identifier of the command line tool in debug mode.
 *
 * Revision 1.5  2007/06/15 14:35:45  meichel
 * Renamed CMake project and include directory from dcmjpgls to dcmjpls
 *
 * Revision 1.4  2007/06/15 10:39:15  meichel
 * Completed implementation of decoder, which now correctly processes all
 *   of the NEMA JPEG-LS sample images, including fragmented frames.
 *
 * Revision 1.3  2007/06/13 16:41:07  meichel
 * Code clean-up and removal of dead code
 *
 * Revision 1.2  2007/06/13 16:22:54  joergr
 * Fixed a couple of inconsistencies.
 *
 *
 */