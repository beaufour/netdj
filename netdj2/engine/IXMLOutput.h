// -*- c++ -*-
/**
 * \file IXMLOutput.h
 * class IXMLOutput
 *
 * $Id$
 *
 */

#ifndef __IXMLOUTPUT_H__
#define __IXMLOUTPUT_H__

class QDomElement;

namespace NetDJ
{
  
  /**
   * Interface providing AsXML()-function to get the object's
   * representation in XML.
   */
  class IXMLOutput {
  public:
    /**
     * Retrieve XML reprensentation of object
     *
     * @param aDocument       The XML document to use
     * @return                The XML representation
   */
    virtual QDomElement* AsXML(QDomDocument* aDocument) const = 0;
  };
}

#endif
