/*
 *
 *  Copyright (C) 2010, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
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
 *  Module: dcmsr
 *
 *  Author: Joerg Riesmeier
 *
 *  Purpose:
 *    classes: DSRMacularGridThicknessAndVolumeReportConstraintChecker
 *
 *  Last Update:      $Author: joergr $
 *  Update Date:      $Date: 2010-09-30 08:56:44 $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */


#ifndef DSRMACCC_H
#define DSRMACCC_H

#include "dcmtk/config/osconfig.h"   /* make sure OS specific configuration is included first */

#include "dcmtk/dcmsr/dsriodcc.h"


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Class for checking the content relationship constraints of the
 *  Macular Grid Thickness And Volume Report IOD
 */
class DSRMacularGridThicknessAndVolumeReportConstraintChecker
  : public DSRIODConstraintChecker
{

  public:

    /** default constructor
     */
    DSRMacularGridThicknessAndVolumeReportConstraintChecker();

    /** destructor
     */
    virtual ~DSRMacularGridThicknessAndVolumeReportConstraintChecker();

    /** check whether by-reference relationships are allowed for this SR IOD
     ** @return always returns OFFalse, i.e. by-reference relationships are not allowed
     */
    virtual OFBool isByReferenceAllowed() const;

    /** check whether this SR IOD requires template support
     ** @return always returns OFTrue, i.e. template support is required
     */
    virtual OFBool isTemplateSupportRequired() const;

    /** get identifier of the root template
     ** @return root template identifier (TID) "2100"
     */
    virtual const char *getRootTemplateIdentifier() const;

    /** get the associated document type of the SR IOD
     ** @return document type (DSRTypes::DT_MacularGridThicknessAndVolumeReport)
     */
    virtual E_DocumentType getDocumentType() const;

    /** check whether specified content relationship is allowed for this IOD
     ** @param  sourceValueType   value type of the source content item to be checked
     *  @param  relationshipType  type of relationship between source and target item
     *  @param  targetValueType   value type of the target content item to be checked
     *  @param  byReference       optional flag indicating whether the node/relationship
     *                            should be added by-value (default) or by-reference
     ** @return OFTrue if content relationship is allowed, OFFalse otherwise
     */
    virtual OFBool checkContentRelationship(const E_ValueType sourceValueType,
                                            const E_RelationshipType relationshipType,
                                            const E_ValueType targetValueType,
                                            const OFBool byReference = OFFalse) const;
};


#endif


/*
 *  CVS/RCS Log:
 *  $Log: dsrmaccc.h,v $
 *  Revision 1.1  2010-09-30 08:56:44  joergr
 *  Added support for the Macular Grid Thickness and Volume Report IOD.
 *
 *
 */