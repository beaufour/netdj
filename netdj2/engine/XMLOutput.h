// -*- c++ -*-
/**
 * \file XMLOutput.h
 * class XMLOutput
 *
 * $Id$
 *
 */

#ifndef __XMLOUTPUT_H__
#define __XMLOUTPUT_H__

/**
 * Interface providing asXML()-function to get the object's
 * representation in XML.
 */
class XMLOutput {
public:
  /**
   * Create XML structure for object.
   *
   * @param doc     The DomDocument to use
   * @param root    The DomElement to use as root
   */
  virtual void asXML(QDomDocument& doc, QDomElement& root) const = 0;

};

#endif
